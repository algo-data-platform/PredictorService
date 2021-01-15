#include "predictor/server/model_manager/framework/tf/tf_model_estimator_single_fea.h"

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

#include <sys/time.h>

namespace predictor {

namespace {

constexpr char LOG_CATEGORY[] = "tf_model_estimator_single_fea.cc: ";

}  // namespace

TFModelEstimatorSingleFea::TFModelEstimatorSingleFea() : TFModelEstimator() {}

bool TFModelEstimatorSingleFea::init(const std::string& path_prefix,
                                const rapidjson::Document& document) {
  if (!tfInit(path_prefix, document)) {
    return false;
  }

  buildTfCache();
  return true;
}

bool TFModelEstimatorSingleFea::predict(PredictResponse* predict_response, const PredictRequest& request) {
  std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>> examples;
  const auto& common_features = request.get_common_features().get_features();
  for (const auto& kv : request.get_item_features()) {
    std::unordered_map<std::string, const feature_master::Feature*> fea;
    int64_t adid = kv.first;
    const auto& item_features = kv.second.get_features();
    for (const auto& common_feature : common_features) {
      const std::string& feature_name = common_feature.get_feature_name();
      fea[feature_name] = &common_feature;
    }
    for (const auto& item_feature : item_features) {
      const std::string& feature_name = item_feature.get_feature_name();
      fea[feature_name] = &item_feature;
    }

    auto example = std::make_pair(adid, std::move(fea));
    examples.emplace_back(std::move(example));
  }

  {
    tfPredictWithNegativeSampling(predict_response, examples, request.get_req_id());
  }
  return true;
}

bool TFModelEstimatorSingleFea::tfCalculateOutputs(
    std::vector<tensorflow::Tensor>* outputs,
    const std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>>& examples,
    const std::vector<std::string>& output_tags) {
  if (!outputs) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "invalid outputs ptr";
    return false;
  }
  // construct tf predict input by tf examples
  std::vector<std::pair<std::string, tensorflow::Tensor>> inputs;
  int example_num = examples.size();
  for (const auto& imap : inputs_map_) {
    auto fea_name = imap.first;
    const auto& tensor_info = imap.second;
    tensorflow::Tensor x(tensor_info.dtype(), tensorflow::TensorShape({example_num, 1}));
    for (size_t j = 0; j < (size_t)example_num; j++) {
      const auto& fea_info = examples[j].second;
      auto iter = fea_info.find(fea_name);
      if (fea_info.end() != iter) {
        switch (iter->second->feature_type) {
          case feature_master::FeatureType::STRING_LIST: {
            const auto& string_values = iter->second->get_string_values();
            if (string_values.size() != 1) {
              // later have to support multi feature values
              FB_LOG_EVERY_MS(ERROR, 2000) << "Wrong size of input feature = " << fea_name;
              return false;
            }

            if (tensor_info.dtype() != tensorflow::DataType::DT_STRING) {
              FB_LOG_EVERY_MS(ERROR, 2000) << "Wrong tensor type as input feature type";
              return false;
            }

            x.matrix<std::string>()(j, 0) = string_values[0];
            break;
           }
           case feature_master::FeatureType::INT64_LIST: {
            const auto& int64_values = iter->second->get_int64_values();
            if (int64_values.size() != 1) {
              // later have to support multi feature values
              FB_LOG_EVERY_MS(ERROR, 2000) << "Wrong size of input feature = " << fea_name;
              return false;
            }

            if (tensor_info.dtype() != tensorflow::DataType::DT_INT64) {
              FB_LOG_EVERY_MS(ERROR, 2000) << "Wrong tensor type as input feature type";
              return false;
            }

            x.matrix<tensorflow::int64>()(j, 0) = int64_values[0];
            break;
           }
           case feature_master::FeatureType::FLOAT_LIST: {
            const auto& float_values = iter->second->get_float_values();
            if (float_values.size() != 1) {
              // later have to support multi feature values
              FB_LOG_EVERY_MS(ERROR, 2000) << "Wrong size of input feature = " << fea_name;
              return false;
            }

            if (tensor_info.dtype() != tensorflow::DataType::DT_FLOAT) {
              FB_LOG_EVERY_MS(ERROR, 2000) << "Wrong tensor type as input feature type";
              return false;
            }

            x.matrix<float>()(j, 0) = float_values[0];
            break;
           }
           default: { break; }
        }
      } else {
        FB_LOG_EVERY_MS(ERROR, 2000) << "failed to predict! feature name has not find = " << fea_name;
        return false;
      }
    }
    inputs.emplace_back(make_pair(tensor_info.name(), std::move(x)));
  }

  tensorflow::Status status = bundle_->session->Run(inputs, output_tags, {}, outputs);
  if (!status.ok()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "failed to predict! status=" << status;
    return false;
  }

  return true;
}

REGISTER_MODEL(TFModelEstimatorSingleFea, estimator_single_fea).describe("This is a tf estimator single fea model.");

}  // namespace predictor
