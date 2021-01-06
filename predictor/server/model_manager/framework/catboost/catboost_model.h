#pragma once

#include "predictor/server/model_manager/framework/model.h"
#include "common/factory_registry.h"
#include "catboost/model_interface/wrapped_calcer.h"

namespace predictor {
class CatboostModel : public Model {
 public:
  virtual ~CatboostModel() = default;
  bool init(const std::string& path_prefix, const rapidjson::Document& document) override;
  bool predict(PredictResponse* response, const PredictRequest& request) override;

 private:
  ModelCalcerWrapper catboost_calcer_;
};

}  // namespace predictor

