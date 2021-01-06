#pragma once

#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "common/util.h"

#include "predictor/server/model_manager/framework/tf/tf_model_estimator.h"

namespace predictor {

class TFModelEstimatorSingleFea : public TFModelEstimator {
 public:
  NO_COPY_NO_MOVE(TFModelEstimatorSingleFea);
  virtual ~TFModelEstimatorSingleFea() = default;
  TFModelEstimatorSingleFea();

  bool init(const std::string& path_prefix, const rapidjson::Document& document) override;
  bool predict(PredictResponse* predict_response, const PredictRequest& request) override;

protected:
  bool tfCalculateOutputs(
      std::vector<tensorflow::Tensor>* outputs,
      const std::vector<std::pair<int64_t, std::unordered_map<std::string, const feature_master::Feature*>>>& examples,
      const std::vector<std::string>& output_tags) override;

};

}  // namespace predictor
