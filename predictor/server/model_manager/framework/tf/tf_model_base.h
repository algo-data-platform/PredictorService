#pragma once

#include "predictor/server/model_manager/framework/model.h"
#include "common/factory_registry.h"

#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "common/util.h"
#include "common/file/file_util.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/config/config_util.h"

DECLARE_int64(tf_thread_num);

namespace predictor {

using VectorMapType = std::map<std::string, std::vector<double>>;

template <class FeatureType>
class TFModelBase : public Model {
 public:
  virtual ~TFModelBase() = default;
  bool loadModelFile() override;
  bool predict(PredictResponse* response, const PredictRequest& request) override;
  bool calculateVector(CalculateVectorResponse* response,
                       const CalculateVectorRequest& request) override;
 protected:
  virtual bool buildTfCache();
  virtual bool tfCalculateOutputs(std::vector<tensorflow::Tensor>* outputs,
                                  const std::vector<std::pair<int64_t, FeatureType>>& examples) { return false; }
  tensorflow::SessionOptions makeTfSessionOptions() const;

  bool constructPredictResponse(PredictResponse* response,
                                      const std::vector<tensorflow::Tensor>& outputs,
                                      const std::vector<std::pair<int64_t, FeatureType>>& examples,
                                      const std::string& req_id);
  bool constructVectorMap(VectorMapType* vector_map, int64_t item_idx, const std::vector<std::string>& output_names,
                          size_t expected_output_size,
                          const std::vector<tensorflow::Tensor>& outputs);
 protected:
  TfConfig tf_config_;  // new tf config that wraps everything
  std::shared_ptr<tensorflow::SavedModelBundle> bundle_;
  ::google::protobuf::Map<std::string, tensorflow::TensorInfo> inputs_map_;
};

template <class FeatureType>
bool TFModelBase<FeatureType>::loadModelFile() {
  // init tf config
  const std::string tf_config_filename = common::pathJoin(model_package_dir_, MODEL_DIR, TF_CONFIG);
  if (!util::initFromFile(&tf_config_, tf_config_filename)) {
    util::logAndMetricError("TfConfig_initFromFile_fail");
    return false;
  }
  // load tf model via tensorflow::LoadSavedModel()
  // requires the following 4 params:
  //    param 1: session options
  tensorflow::SessionOptions session_options = makeTfSessionOptions();
  //    param 2: tags
  std::unordered_set<std::string> tags(tf_config_.tags.begin(), tf_config_.tags.end());
  //    param 3: export_dir (where the saved model is)
  const std::string export_dir = common::pathJoin(model_package_dir_, MODEL_DIR, tf_config_.export_dir);
  //    param 4: bundle ptr
  bundle_.reset(new tensorflow::SavedModelBundle());
  // call tensorflow::LoadSavedModel()
  tensorflow::Status load_status =
    tensorflow::LoadSavedModel(session_options, tensorflow::RunOptions(), export_dir, tags, bundle_.get());
  if (!load_status.ok()) {
    LOG(ERROR) << "Loading tf model failed. export_dir=" << export_dir << " status=" << load_status.ToString();
    return false;
  }
  // construct inputs_map_
  auto signature_map = bundle_->meta_graph_def.signature_def();
  auto signature = signature_map[tf_config_.signature_def];
  inputs_map_ = signature.inputs();
  // build cache
  if (!buildTfCache()) {
    return false;
  }
  return true;
}

template <class FeatureType>
tensorflow::SessionOptions TFModelBase<FeatureType>::makeTfSessionOptions() const {
  tensorflow::SessionOptions options;
  if (FLAGS_tf_thread_num <= 0) {
    return options;
  }

  options.config.session_inter_op_thread_pool();
  options.config.add_session_inter_op_thread_pool();
  std::string thread_pool_name{"tf_pool_size_"};
  thread_pool_name.append(folly::to<std::string>(FLAGS_tf_thread_num));
  options.config.mutable_session_inter_op_thread_pool(0)->set_global_name(thread_pool_name);
  options.config.mutable_session_inter_op_thread_pool(0)->set_num_threads(FLAGS_tf_thread_num);

  return options;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::buildTfCache() {
  // trigger tf cold start
  LOG(INFO) << "running an empty request to trigger the building of cache";
  std::vector<std::pair<std::string, tensorflow::Tensor>> inputs;
  std::vector<std::pair<int64_t, std::string>> examples;
  examples.emplace_back(std::make_pair(1, ""));
  int example_num = examples.size();
  for (const auto& imap : inputs_map_) {
    const auto& tensor_info = imap.second;
    tensorflow::Tensor x(tensor_info.dtype(), tensorflow::TensorShape({example_num}));

    for (size_t j = 0; j < (size_t)example_num; j++) {
      x.vec<std::string>()(j) = examples[j].second;
    }
    inputs.emplace_back(make_pair(tensor_info.name(), std::move(x)));
  }

  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status status = bundle_->session->Run(inputs, tf_config_.output_tensor_names, {}, &outputs);
  if (!status.ok()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "failed to predict! status=" << status;
    return false;
  }
  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::predict(PredictResponse* response, const PredictRequest& request) {
  if (request.get_item_features().empty()) {
    return true;
  }

  const MetricTagsMap model_tag_map{{TAG_MODEL, model_full_name_}, {TAG_BUSINESS_LINE, business_line_}};
  std::vector<std::pair<int64_t, FeatureType>> item_features;
  {
    metrics::Timer model_timer(util::buildTimers(FEATURE_CONSUMING, model_tag_map));
    extractFeaturesInParallel<FeatureType>(&item_features, request);
  }

  {
    metrics::Timer model_timer(util::buildTimers(TF_CONSUMING, model_tag_map));
    std::vector<tensorflow::Tensor> outputs;
    if (!tfCalculateOutputs(&outputs, item_features)) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "tfCalculateOutputs failed, model_name=" << model_full_name_;
      return false;
    } else {
      constructPredictResponse(response, outputs, item_features, request.get_req_id());
    }
  }

  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::calculateVector(CalculateVectorResponse* response,
                                               const CalculateVectorRequest& request) {
  if (tf_config_.output_tensor_names.size() != request.get_output_names().size()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "request get_output_names size=" << request.get_output_names().size()
                                 << " does not match config output_tensor_names size="
                                 << tf_config_.output_tensor_names.size();
    return false;
  }

  response->set_req_id(request.get_req_id());
  response->set_model_timestamp(tf_config_.model_timestamp);

  const auto& common_features = request.get_features().get_features();
  std::vector<feature_master::Feature> item_features;
  FeatureType extracted_features;
  // CalculateVectorRequest should contain just one item so no parallel needed
  if (!feature_extractor_->extractItemFeatures(&extracted_features, item_features, common_features)) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "extractItemFeatures failed for req_id=" << request.get_req_id();
    return false;
  }

  std::vector<std::pair<int64_t, FeatureType> > examples;
  examples.emplace_back(1, std::move(extracted_features));

  std::vector<tensorflow::Tensor> outputs;
  {
    const MetricTagsMap model_tag_map{{TAG_MODEL, model_full_name_}, {TAG_BUSINESS_LINE, business_line_}};
    metrics::Timer model_timer(util::buildTimers(TF_CONSUMING, model_tag_map));
    if (!tfCalculateOutputs(&outputs, examples)) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "tfCalculateOutputs failed";
      return false;
    }
  }
  if (outputs.size() != tf_config_.output_tensor_names.size()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "tf calculated outputs size=" << outputs.size()
                                 << " does not match config output_tensor_names size="
                                 << tf_config_.output_tensor_names.size();
    return false;
  }

  // populate response
  VectorMapType vector_map;
  if (!constructVectorMap(&vector_map, 0, request.get_output_names(), 1, outputs)) {
    return false;
  }

  response->set_vector_map(std::move(vector_map));
  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::constructVectorMap(VectorMapType *vector_map,
                                                  int64_t item_idx,
                                                  const std::vector<std::string>& output_names,
                                                  size_t expected_output_size,
                                                  const std::vector<tensorflow::Tensor>& outputs) {
  for (size_t i = 0; i < outputs.size(); ++i) {
    if (2 != outputs[i].shape().dims()) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "wrong dims of tensor, which is " << outputs[i].shape().dims();
      return false;
    }

    size_t output_size = outputs[i].shape().dim_size(0);
    size_t vector_len = outputs[i].shape().dim_size(1);
    if (expected_output_size != output_size) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "input size and output size don't match. input size = " << expected_output_size
                                     << ", output size = " << output_size;
        return false;
    }
    auto tmap = outputs[i].tensor<float, 2>();
    std::vector<double> v;
    for (size_t j = 0; j < vector_len; ++j) {
      v.emplace_back(tmap(item_idx, j));
    }
    vector_map->emplace(output_names[i], std::move(v));
  }

  return true;
}


template <class FeatureType>
bool TFModelBase<FeatureType>::constructPredictResponse(PredictResponse* response,
                                      const std::vector<tensorflow::Tensor>& outputs,
                                      const std::vector<std::pair<int64_t, FeatureType>>& examples,
                                      const std::string& req_id) {
  if (1 != outputs.size()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "wrong size of outputs, which is " << outputs.size();
    return false;
  }
  if (1 > outputs[0].shape().dims()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "wrong dims of tensor, which is " << outputs[0].shape().dims();
    return false;
  }
  size_t output_size = outputs[0].shape().dim_size(0);
  size_t example_num = examples.size();
  if (example_num != output_size) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "input size and output size don't match. input size = " << example_num
                                  << ", output size = " << output_size;
    return false;
  }
  auto tmap = outputs[0].tensor<float, 2>();
  std::map<int64_t, feature_master::PredictResults>& results_map = response->results_map;
  for (size_t i = 0; i < output_size; ++i) {
    feature_master::PredictResults predict_results;
    std::map<std::string, double>& preds = predict_results.preds;
    if (tf_config_.negative_sampling_ratio != 0 && tf_config_.negative_sampling_ratio != 1) {
      preds["ctr"] = util::restoreFromNegativeSampling(tmap(i), tf_config_.negative_sampling_ratio);
    } else {
      preds["ctr"] = tmap(i);
    }
    results_map[examples[i].first] = std::move(predict_results);
  }
  response->set_req_id(req_id);
  return true;
}

}  // namespace predictor

