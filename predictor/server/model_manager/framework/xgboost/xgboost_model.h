#pragma once

#include "predictor/server/model_manager/framework/model.h"
#include "common/factory_registry.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/FutureExecutor.h"

namespace predictor {

class XGBoostModel : public Model {
 public:
  virtual ~XGBoostModel() = default;
  bool init(const std::string& path_prefix, const rapidjson::Document& document) override {
    return false;
  }
  bool predict(PredictResponse* predict_response, const PredictRequest& predict_request) override {
    return false;
  }
};

}  // namespace predictor

