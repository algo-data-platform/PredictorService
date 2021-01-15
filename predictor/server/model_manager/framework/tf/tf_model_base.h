#pragma once

#include "predictor/server/model_manager/framework/model.h"
#include "common/factory_registry.h"

#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "common/util.h"
#include "common/file/file_util.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"

DECLARE_int64(tf_thread_num);

namespace predictor {


template <class FeatureType>
class TFModelBase : public Model {
 public:
  virtual ~TFModelBase() = default;
  bool init(const std::string& path_prefix, const rapidjson::Document& document) override;
  bool predict(PredictResponse* predict_response, const PredictRequest& request) override;

 protected:
  virtual bool buildTfCache();
  virtual bool initExtraConfig(const std::string& path_prefix, const rapidjson::Document& document);
  virtual bool tfCalculateOutputs(std::vector<tensorflow::Tensor>* outputs,
                                  const std::vector<std::pair<int64_t, FeatureType>>& examples,
                                  const std::vector<std::string>& output_tags) { return false; }

  bool initFeatureExtractor(const std::string& path_prefix, const rapidjson::Document& document);
  bool initTfConfig(const std::string& path_prefix, const rapidjson::Document& document);
  bool constructResponseFromTFOutputs(PredictResponse* predict_response,
                                      const std::vector<tensorflow::Tensor>& outputs,
                                      const std::vector<std::pair<int64_t, FeatureType>>& examples,
                                      const std::string& req_id);

 protected:
  std::shared_ptr<tensorflow::SavedModelBundle> bundle_;
  ::google::protobuf::Map<std::string, tensorflow::TensorInfo> inputs_map_;
  std::string predict_tag_;
  std::string model_tag_;
  std::string predict_signature_;
  std::map<std::string, std::string> output_tags_map_;  // output name -> output tag
  std::string model_timestamp_;
  // 负采样参数
  double negative_sampling_ratio_{1};
};


template <class FeatureType>
bool TFModelBase<FeatureType>::init(const std::string& path_prefix,
                                const rapidjson::Document& document) {
  if (!initTfConfig(path_prefix, document)) {
    return false;
  }

  if (!buildTfCache()) {
    return false;
  }

  if (!initFeatureExtractor(path_prefix, document)) {
    return false;
  }

  return initExtraConfig(path_prefix, document);
}

template <class FeatureType>
bool TFModelBase<FeatureType>::initExtraConfig(
    const std::string& path_prefix, const rapidjson::Document& document) {
  std::string runtime_prefix =
      common::pathJoin(path_prefix, document[XFEA_RUNTIME_DIR].GetString());

  std::string extra_config_file_name = common::pathJoin(runtime_prefix, EXTRA_CONFIG_FILE_NAME);
  LOG(INFO) << "extra_config_file_name=" << extra_config_file_name;
  std::string extra_config_json;
  if (!common::FileUtil::read_file_str(&extra_config_json, extra_config_file_name)) {
    LOG(ERROR) << "no extra config provided";
  } else {
    rapidjson::Document extra_config_document;
    extra_config_document.Parse(extra_config_json.c_str());
    if (extra_config_document.HasMember(NEGATIVE_SAMPLING_RATIO)) {
      negative_sampling_ratio_ = extra_config_document[NEGATIVE_SAMPLING_RATIO].GetDouble();
      if (negative_sampling_ratio_ < 0.000001) {
        LOG(ERROR) << "invalid negative_sampling_ratio_=" << negative_sampling_ratio_ << ". reset to 1.";
        negative_sampling_ratio_ = 1;
      }
    }
  }
  LOG(INFO) << "negative_sampling_ratio_=" << negative_sampling_ratio_;

  return true;
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
  std::vector<std::string> output_tags;
  if (output_tags_map_.empty()) {
    output_tags.emplace_back(predict_tag_);
  } else {
    for (const auto& iter : output_tags_map_) {
      output_tags.emplace_back(iter.second);
    }
  }
  bundle_->session->Run(inputs, output_tags, {}, &outputs);

  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::initTfConfig(const std::string& path_prefix,
                                        const rapidjson::Document& document) {
  if (!document.HasMember(XFEA_RUNTIME_DIR)) {
    LOG(ERROR) << "invalid TFModelEstimatorXfea config!";
    return false;
  }

  std::string runtime_prefix = common::pathJoin(path_prefix, document[XFEA_RUNTIME_DIR].GetString());
  // init tf config
  std::string tf_config_file_name = common::pathJoin(runtime_prefix, TF_CONFIG_FILE);
  LOG(INFO) << "tf_config_file_name=" << tf_config_file_name;
  std::string tf_config_json;
  if (!common::FileUtil::read_file_str(&tf_config_json, tf_config_file_name)) {
    LOG(ERROR) << "failed to translate file to json for config file=" << tf_config_file_name;
    return false;
  }
  rapidjson::Document tf_document;
  tf_document.Parse(tf_config_json.c_str());

  // fetch config
  if (!tf_document.HasMember(CONFIG_MODEL_DIR) || !tf_document.HasMember(TF_PREDICT_TAG) ||
      !tf_document.HasMember(TF_MODEL_TAG) || !tf_document.HasMember(TF_PREDICT_SIGNATURE)) {
    LOG(ERROR) << "invalid tf config";
    return false;
  }

  std::string model_dir = common::pathJoin(runtime_prefix, tf_document[CONFIG_MODEL_DIR].GetString());
  predict_tag_ = tf_document[TF_PREDICT_TAG].GetString();
  model_tag_ = tf_document[TF_MODEL_TAG].GetString();
  predict_signature_ = tf_document[TF_PREDICT_SIGNATURE].GetString();
  if (tf_document.HasMember(TF_OUTPUT_TAGS)) {
    const rapidjson::Value& output_tags = tf_document[TF_OUTPUT_TAGS];
    if (output_tags.IsObject()) {
      for (auto iter = output_tags.MemberBegin(); iter != output_tags.MemberEnd(); iter++) {
        std::string output_name = iter->name.GetString();
        std::string output_tag = iter->value.GetString();
        LOG(INFO) << "Adding output_name=" << output_name << ", output_tag=" << output_tag;
        output_tags_map_[output_name] = output_tag;
      }
    }
  }

  if (tf_document.HasMember(MODEL_TIMESTAMP)) {
    model_timestamp_ = tf_document[MODEL_TIMESTAMP].GetString();
    LOG(INFO) << "model_timestamp_=" << model_timestamp_;
  }
  LOG(INFO) << "model_dir=" << model_dir << ", predict tag=" << predict_tag_ << ", model_tag=" << model_tag_
               << ", predict signature=" << predict_signature_;

  // load tf model
  std::unordered_set<std::string> tags = {model_tag_};
  tensorflow::SessionOptions options;
  if (FLAGS_tf_thread_num > 0) {
    options.config.session_inter_op_thread_pool();
    options.config.add_session_inter_op_thread_pool();
    std::string thread_pool_name{"tf_pool_size_"};
    thread_pool_name.append(folly::to<std::string>(FLAGS_tf_thread_num));
    options.config.mutable_session_inter_op_thread_pool(0)->set_global_name(thread_pool_name);
    options.config.mutable_session_inter_op_thread_pool(0)->set_num_threads(FLAGS_tf_thread_num);
  }
  bundle_.reset(new tensorflow::SavedModelBundle());
  tensorflow::Status load_status =
      tensorflow::LoadSavedModel(options, tensorflow::RunOptions(), model_dir, tags, bundle_.get());
  if (!load_status.ok()) {
    LOG(ERROR) << "Loading tf model failed. dir:" << model_dir << " status:" << load_status.ToString();
    return false;
  }

  // construct input map
  auto signature_map = bundle_->meta_graph_def.signature_def();
  auto signature_predict = signature_map[std::string(predict_signature_)];
  inputs_map_ = signature_predict.inputs();
  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::initFeatureExtractor(const std::string& path_prefix,
                                                     const rapidjson::Document& document) {
  std::string runtime_prefix =
      common::pathJoin(path_prefix, document[XFEA_RUNTIME_DIR].GetString());
  rapidjson::Document xfea_document;

  std::shared_ptr<predictor::FeatureExtractorFactory> factory =
      common::FactoryRegistry<predictor::FeatureExtractorFactory>::Find(
          document[CONFIG_FEATURE_CLASS_NAME].GetString());
  if (factory == nullptr) {
    LOG(ERROR) << "failed to Find factory " << document[CONFIG_FEATURE_CLASS_NAME].GetString();
    return false;
  }

  feature_extractor_ = factory->body();
  if (!feature_extractor_->init(runtime_prefix, xfea_document)) {
    LOG(ERROR) << "failed to init extractor!";
    return false;
  }

  LOG(INFO) << "init feature extractor successfully";
  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::predict(PredictResponse* predict_response, const PredictRequest& request) {
  if (request.get_item_features().empty()) {
    return true;
  }

  const MetricTagsMap model_tag_map{{TAG_MODEL, model_full_name_}, {TAG_BUSINESS_LINE, business_line_}};
  std::vector<std::pair<int64_t, FeatureType>> examples;
  {
    metrics::Timer model_timer(util::buildTimers(FEATURE_CONSUMING, model_tag_map));
    extractFeaturesInParallel<FeatureType>(&examples, request);
  }

  {
    metrics::Timer model_timer(util::buildTimers(TF_CONSUMING, model_tag_map));
    std::vector<tensorflow::Tensor> outputs;
    std::vector<std::string> output_tags{predict_tag_};
    if (!tfCalculateOutputs(&outputs, examples, output_tags)) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "tfCalculateOutputs failed, model_name=" << model_full_name_;
      return false;
    } else {
      constructResponseFromTFOutputs(predict_response, outputs, examples, request.get_req_id());
    }
  }

  return true;
}

template <class FeatureType>
bool TFModelBase<FeatureType>::constructResponseFromTFOutputs(PredictResponse* predict_response,
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
  std::map<int64_t, feature_master::PredictResults>& results_map = predict_response->results_map;
  for (size_t i = 0; i < output_size; ++i) {
    feature_master::PredictResults predict_results;
    std::map<std::string, double>& preds = predict_results.preds;
    if (negative_sampling_ratio_ != 0 && negative_sampling_ratio_ != 1) {
      preds["ctr"] = util::restoreFromNegativeSampling(tmap(i), negative_sampling_ratio_);
    } else {
      preds["ctr"] = tmap(i);
    }
    results_map[examples[i].first] = std::move(predict_results);
  }
  predict_response->set_req_id(req_id);
  return true;
}


}  // namespace predictor

