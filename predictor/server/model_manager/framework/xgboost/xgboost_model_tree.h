#pragma once

#include <atomic>
#include "predictor/server/model_manager/framework/xgboost/xgboost_model.h"
#include "predictor/server/model_manager/framework/xgboost/xgboost_predictor.h"
#include "thirdparty/rapidjson/document.h"
#include "folly/Synchronized.h"
#include "folly/container/F14Map.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/FutureExecutor.h"
#include "common/util.h"

namespace predictor {

using StringList = std::vector<std::string>;

class XGBoostModelTree : public XGBoostModel {
 public:
  XGBoostModelTree();
  virtual ~XGBoostModelTree() = default;
  NO_COPY_NO_MOVE(XGBoostModelTree);

  bool init(const std::string& path_prefix, const rapidjson::Document& document) override;
  bool predict(PredictResponse* predict_response, const PredictRequest& request) override;

 private:
  xgb::XGBoostPredictor xgboost_predictor_;
};

}  // namespace predictor
