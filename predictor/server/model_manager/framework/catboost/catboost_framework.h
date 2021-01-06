#pragma once

#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/server/model_manager/framework/catboost/catboost_model.h"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "folly/concurrency/AtomicSharedPtr.h"
#include "folly/Synchronized.h"

namespace predictor {

class CatboostFramework : public ModelFramework {
 public:
  ~CatboostFramework() = default;
};
}  // namespace predictor
