#pragma once

#include "predictor/server/model_manager/framework/tf/tf_model_base.h"
#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "common/util.h"

namespace predictor {

using FeatureMatserMap = std::unordered_map<std::string, const feature_master::Feature*>;

class TFModelEstimatorNative : public TFModelBase<FeatureMatserMap> {
 public:
  TFModelEstimatorNative() = default;
  virtual ~TFModelEstimatorNative() = default;
  NO_COPY_NO_MOVE(TFModelEstimatorNative);

 protected:
  bool tfCalculateOutputs(std::vector<tensorflow::Tensor>* outputs,
                          const std::vector<std::pair<int64_t, FeatureMatserMap> >& examples) override;
  bool buildTfCache() override { return true; }
  template <class StdType, class TensorType>
  static bool buildTensor(tensorflow::Tensor *tensor,
                          size_t i,
                          const std::string &fea_name,
                          const std::vector<StdType> &fea_vals,
                          tensorflow::DataType tensor_type,
                          tensorflow::DataType expected_type,
                          size_t tensor_len,
                          StdType default_val) {
    if (tensor_type != expected_type) {
      FB_LOG_EVERY_MS(ERROR, 2000)
        << "tensor type=" << tensor_type << " does not match expected type=" << expected_type
        << "input feature name=" << fea_name;
      return false;
    }
    if (fea_vals.size() < tensor_len) {
      size_t index = 0;
      for (; index < fea_vals.size(); index++) {
        tensor->matrix<TensorType>()(i, index) = fea_vals[index];
      }
      for (; index < tensor_len; index++) {
        tensor->matrix<TensorType>()(i, index) = default_val;
      }
    } else {
      size_t index = 0;
      for (; index < tensor_len; index++) {
        tensor->matrix<TensorType>()(i, index) = fea_vals[index];
      }
    }
    return true;
  }
};

}  // namespace predictor
