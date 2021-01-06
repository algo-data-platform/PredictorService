#pragma once

#include "predictor/server/model_manager/framework/model.h"
#include "common/factory_registry.h"

#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "common/util.h"

#include "predictor/util/predictor_constants.h"
#include "predictor/util/predictor_util.h"

namespace predictor {
class TFModel : public Model {
 public:
  virtual ~TFModel() = default;
 protected:
  template <class T>
  bool tfPredictWithNegativeSampling(PredictResponse* predict_response,
                                     const std::vector<std::pair<int64_t, T>>& examples, const std::string& req_id) {
    // time consuming of tf
    const MetricTagsMap model_tag_map{{TAG_MODEL, model_full_name_}, {TAG_BUSINESS_LINE, business_line_}};
    metrics::Timer model_timer(util::buildTimers(TF_CONSUMING, model_tag_map));

    std::vector<tensorflow::Tensor> outputs;
    std::vector<std::string> output_tags{predict_tag_};
    if (!tfCalculateOutputs(&outputs, examples, output_tags)) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "tfCalculateOutputs failed";
      return false;
    }
    // populate response
    return constructResponseFromTFOutputs(predict_response, outputs, examples, req_id);
  }

  template <class T>
  bool constructResponseFromTFOutputs(PredictResponse* predict_response, const std::vector<tensorflow::Tensor>& outputs,
                                      const std::vector<std::pair<int64_t, T>>& examples, const std::string& req_id) {
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

  bool constructVectorResponseFromTFOutputs(CalculateVectorResponse* calculate_vector_response,
                                            const std::vector<std::string>& output_names,
                                            const std::vector<tensorflow::Tensor>& outputs) {
    std::map<std::string, std::vector<double>> vector_map;
    for (size_t i = 0; i < outputs.size(); ++i) {
      if (2 != outputs[i].shape().dims()) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "wrong dims of tensor, which is " << outputs[0].shape().dims();
        return false;
      }
      size_t output_size = outputs[i].shape().dim_size(0);
      size_t vector_len = outputs[i].shape().dim_size(1);
      if (1 != output_size) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "input size and output size don't match. input size = " << 1
                                     << ", output size = " << output_size;
        return false;
      }
      auto tmap = outputs[i].tensor<float, 2>();
      std::vector<double> v;
      for (size_t j = 0; j < vector_len; ++j) {
        v.emplace_back(tmap(0, j));
      }

      vector_map[output_names[i]] = std::move(v);
    }
    calculate_vector_response->set_vector_map(std::move(vector_map));
    return true;
  }

  template <class T>
  bool tfCalculateVector(CalculateVectorResponse* calculate_vector_response,
                         const std::vector<std::pair<int64_t, T>>& examples,
                         const std::vector<std::string>& output_names) {
    const MetricTagsMap model_tag_map{{TAG_MODEL, model_full_name_}, {TAG_BUSINESS_LINE, business_line_}};
    metrics::Timer model_timer(util::buildTimers(TF_CONSUMING, model_tag_map));

    std::vector<std::string> output_tags;
    std::vector<tensorflow::Tensor> outputs;
    for (const auto& output_name : output_names) {
      auto iter = output_tags_map_.find(output_name);
      if (output_tags_map_.end() != iter) {
        output_tags.emplace_back(iter->second);
      } else {
        FB_LOG_EVERY_MS(ERROR, 60000) << "cannot find output tag for output_name=" << output_name;
      }
    }
    {
      if (!tfCalculateOutputs(&outputs, examples, output_tags)) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "tfCalculateOutputs failed";
        return false;
      }
      if (output_tags.size() != outputs.size()) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "outputs size=" << outputs.size()
                                     << " does not match output tags size=" << output_tags.size();
        return false;
      }
    }
    // populate response
    return constructVectorResponseFromTFOutputs(calculate_vector_response, output_names, outputs);
  }

  virtual bool tfCalculateOutputs(
      std::vector<tensorflow::Tensor>* outputs,
      const std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>>& examples,
      const std::vector<std::string>& output_tags) {
    return false;
  }

 protected:
  std::shared_ptr<tensorflow::SavedModelBundle> bundle_;
  ::google::protobuf::Map<std::string, tensorflow::TensorInfo> inputs_map_;
  std::string predict_tag_;
  std::string model_tag_;
  std::string predict_signature_;
  std::map<std::string, std::string> output_tags_map_;  // output name -> output tag
  std::string model_timestamp_;
  double negative_sampling_ratio_{1};
};

}  // namespace predictor

