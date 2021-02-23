#include "predictor/server/model_manager/framework/tf/tf_model_estimator_native.h"
#include "predictor/global_resource/resource_manager.h"
#include "predictor/util/predictor_util.h"
#include "tensorflow/core/platform/types.h"

#include <iostream>

namespace {
constexpr char LOG_CATEGORY[] = "tf_model_estimator_native.cc: ";
}  // namespace

namespace predictor {

bool TFModelEstimatorNative::tfCalculateOutputs(
  std::vector<tensorflow::Tensor>* outputs,
  const std::vector<std::pair<int64_t, FeatureMatserMap> >& item_features) {
  // construct tf inputs by item_features
  std::vector<std::pair<std::string, tensorflow::Tensor>> inputs;
  for (const auto& input : inputs_map_) {
    const auto &fea_name = input.first;
    const auto &tensor_info = input.second;
    const auto tensor_type = tensor_info.dtype();
    if (tensor_info.tensor_shape().dim().size() < 2) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "input tensor dim() size() < 2";
      return false;
    }
    const size_t tensor_len = tensor_info.tensor_shape().dim(1).size();  // tensor len
    tensorflow::Tensor tensor(tensor_type, tensorflow::TensorShape({item_features.size(), tensor_len}));
    for (size_t i = 0; i < item_features.size(); ++i) {
      const auto &fea_map = item_features[i].second;
      const auto &iter = fea_map.find(fea_name);
      if (iter == fea_map.end()) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "no such fea_name in fea_map: " << fea_name;
        return false;
      }

      bool rc = false;
      const auto &cur_fea = iter->second;
      switch (cur_fea->feature_type) {
        case feature_master::FeatureType::STRING_LIST: {
          rc = buildTensor<std::string, std::string>(&tensor, i, fea_name, cur_fea->get_string_values(),
                                        tensor_type, tensorflow::DataType::DT_STRING, tensor_len, "");
          break;
        }
        case feature_master::FeatureType::INT64_LIST: {
          rc = buildTensor<int64_t, tensorflow::int64>(&tensor, i, fea_name, cur_fea->get_int64_values(),
                                    tensor_type, tensorflow::DataType::DT_INT64, tensor_len, 0);
          break;
        }
        case feature_master::FeatureType::FLOAT_LIST: {
          rc = buildTensor<double, float>(&tensor, i, fea_name, cur_fea->get_float_values(),
                                   tensor_type, tensorflow::DataType::DT_FLOAT, tensor_len, 0.0);
          break;
        }
        default: {
          break;
        }
      }
      if (!rc) {
        return false;
      }
    }
    inputs.emplace_back(tensor_info.name(), std::move(tensor));
  }

  // run tf session
  tensorflow::Status status = bundle_->session->Run(inputs, tf_config_.output_tensor_names, {}, outputs);
  if (!status.ok()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "failed to predict! status=" << status;
    return false;
  }

  return true;
}

REGISTER_MODEL(TFModelEstimatorNative, estimator_native).describe("This is a tf estimator native model.");

}  // namespace predictor
