#pragma once

#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "predictor/server/model_manager/framework/xgboost/xgboost_model.h"
#include "folly/concurrency/AtomicSharedPtr.h"
#include "folly/Synchronized.h"

namespace predictor {

class XGBoostFramework : public ModelFramework {
 public:
  ~XGBoostFramework() = default;
};

}  // namespace predictor
