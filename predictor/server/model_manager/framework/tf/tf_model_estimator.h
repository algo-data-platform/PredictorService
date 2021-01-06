#pragma once

#include "predictor/server/model_manager/framework/tf/tf_model.h"

namespace predictor {

class TFModelEstimator : public TFModel {
 public:
  NO_COPY_NO_MOVE(TFModelEstimator);
  virtual ~TFModelEstimator() = default;
  TFModelEstimator();

  bool init(const std::string& path_prefix, const rapidjson::Document& document) override;

 protected:
  void buildTfCache();
  bool tfInit(const std::string& path_prefix, const rapidjson::Document& document);
  bool checkTfInputStructure();
};

}  // namespace predictor
