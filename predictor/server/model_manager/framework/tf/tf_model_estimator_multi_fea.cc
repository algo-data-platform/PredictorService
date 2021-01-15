#include "predictor/server/model_manager/framework/tf/tf_model_estimator_multi_fea.h"

#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/core/example/example.pb.h"

#include "folly/Synchronized.h"

#include "common/file/file_util.h"
#include "predictor/server/model_manager/framework/tf/tf_model_estimator.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/global_resource/resource_manager.h"
#include "predictor/util/predictor_util.h"

#include <sys/time.h>

namespace predictor {

namespace {

constexpr char LOG_CATEGORY[] = "tf_model_estimator_multi_fea.cc: ";

}  // namespace

TFModelEstimatorMultiFea::TFModelEstimatorMultiFea() : TFModelEstimator() {}

bool TFModelEstimatorMultiFea::init(const std::string& path_prefix,
                                const rapidjson::Document& document) {
  if (!tfInit(path_prefix, document)) {
    return false;
  }

  buildTfCache();
  return true;
}

bool TFModelEstimatorMultiFea::tfCalculateOutputs(
    std::vector<tensorflow::Tensor>* outputs,
    const std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>>& examples,
    const std::vector<std::string>& output_tags) {
  if (!outputs) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "invalid outputs ptr";
    return false;
  }

  std::vector<std::pair<std::string, tensorflow::Tensor>> inputs;
  for (const auto& imap : inputs_map_) {
    auto fea_name = imap.first;
    const auto& tensor_info = imap.second;
    if (tensor_info.tensor_shape().dim().size() < 2) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "imap.second.tensor_shape().dim().size() < 2";
      return false;
    }
    size_t len = tensor_info.tensor_shape().dim(1).size();
    tensorflow::Tensor x(tensor_info.dtype(), tensorflow::TensorShape({examples.size(), len}));
    for (size_t j = 0; j < examples.size(); j++) {
      const auto& fea_info = examples[j].second;
      auto iter = fea_info.find(fea_name);
      if (fea_info.end() != iter) {
        switch (iter->second->feature_type) {
          case feature_master::FeatureType::STRING_LIST: {
            const auto& string_values = iter->second->get_string_values();

            if (tensor_info.dtype() != tensorflow::DataType::DT_STRING) {
              FB_LOG_EVERY_MS(ERROR, 2000)
                  << "Wrong tensor type as input feature type, feature name is=" << fea_name.c_str()
                  << "input type is = string, "
                  << "tensor type is=" << tensor_info.dtype();
              return false;
            }
            if (string_values.size() < len) {
              size_t index = 0;
              for (; index < string_values.size(); index++) {
                x.matrix<std::string>()(j, index) = string_values[index];
              }
              for (; index < len; index++) {
                x.matrix<std::string>()(j, index) = "";
              }
            } else {
              size_t index = 0;
              for (; index < len; index++) {
                x.matrix<std::string>()(j, index) = string_values[index];
              }
            }

            break;
          }
          case feature_master::FeatureType::INT64_LIST: {
            const auto& int64_values = iter->second->get_int64_values();
            if (tensor_info.dtype() != tensorflow::DataType::DT_INT64) {
              FB_LOG_EVERY_MS(ERROR, 2000)
                  << "Wrong tensor type as input feature type, feature name is=" << fea_name.c_str()
                  << "input type is = int, "
                  << "tensor type is=" << tensor_info.dtype();
              return false;
            }

            if (int64_values.size() < len) {
              size_t index = 0;
              for (; index < int64_values.size(); index++) {
                x.matrix<tensorflow::int64>()(j, index) = int64_values[index];
              }
              for (; index < len; index++) {
                x.matrix<tensorflow::int64>()(j, index) = 0;
              }
            } else {
              size_t index = 0;
              for (; index < len; index++) {
                x.matrix<tensorflow::int64>()(j, index) = int64_values[index];
              }
            }

            break;
          }
          case feature_master::FeatureType::FLOAT_LIST: {
            const auto& float_values = iter->second->get_float_values();
            if (tensor_info.dtype() != tensorflow::DataType::DT_FLOAT) {
              FB_LOG_EVERY_MS(ERROR, 2000)
                  << "Wrong tensor type as input feature type, feature name is=" << fea_name.c_str()
                  << "input type is = float, "
                  << "tensor type is=" << tensor_info.dtype();
              return false;
            }

            if (float_values.size() < len) {
              size_t index = 0;
              for (; index < float_values.size(); index++) {
                x.matrix<float>()(j, index) = float_values[index];
              }
              for (; index < len; index++) {
                x.matrix<float>()(j, index) = 0.0;
              }
            } else {
              size_t index = 0;
              for (; index < len; index++) {
                x.matrix<float>()(j, index) = float_values[index];
              }
            }
            break;
          }
          default: {
            break;
          }
        }
      } else {
        FB_LOG_EVERY_MS(ERROR, 2000) << "failed to predict! feature name has not find = " << fea_name;
        return false;
      }
    }
    inputs.emplace_back(tensor_info.name(), std::move(x));
  }

  tensorflow::Status status = bundle_->session->Run(inputs, output_tags, {}, outputs);
  if (!status.ok()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "failed to predict! status=" << status;
    return false;
  }

  return true;
}

bool TFModelEstimatorMultiFea::calculateVector(CalculateVectorResponse* calculate_vector_response,
                                           const CalculateVectorRequest& calculate_vector_request) {
    std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>> examples;
  calculate_vector_response->set_req_id(calculate_vector_request.get_req_id());
  calculate_vector_response->set_model_timestamp(model_timestamp_);
  const std::vector<feature_master::Feature>& common_feature_list =
      calculate_vector_request.get_features().get_features();
  std::unordered_map<std::string, const feature_master::Feature*> fea;
  for (const auto& common_feature : common_feature_list) {
    const std::string& feature_name = common_feature.get_feature_name();
    fea[feature_name] = &common_feature;
  }

  examples.emplace_back(1, std::move(fea));
  return tfCalculateVector<std::unordered_map<std::string, const feature_master::Feature*>>(
      calculate_vector_response, examples, calculate_vector_request.get_output_names());
}


bool TFModelEstimatorMultiFea::calculateBatchVector(CalculateBatchVectorResponse* batch_response,
                                           const CalculateBatchVectorRequest& batch_request) {
  std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>> examples;
  batch_response->set_req_id(batch_request.get_req_id());
  batch_response->set_model_timestamp(model_timestamp_);
  const std::map<int64_t, feature_master::Features> &feature_map = batch_request.get_features_map();

  for (const auto &item_features : feature_map) {
    int64_t adid = item_features.first;
    std::unordered_map<std::string, const feature_master::Feature*> fea;
    for (const auto& feature : item_features.second.get_features()) {
      const std::string& feature_name = feature.get_feature_name();
      fea[feature_name] = &feature;
    }
    examples.emplace_back(adid, std::move(fea));
  }

  return tfCalculateVector<std::unordered_map<std::string, const feature_master::Feature*>,
                          CalculateBatchVectorResponse>(batch_response, examples,
                                                        batch_request.get_output_names());
}

REGISTER_MODEL(TFModelEstimatorMultiFea, estimator_multi_fea).describe("This is a tf estimator multi fea model.");

}  // namespace predictor
