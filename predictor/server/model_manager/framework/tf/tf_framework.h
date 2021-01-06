#pragma once

#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/server/model_manager/framework/tf/tf_model.h"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "folly/concurrency/AtomicSharedPtr.h"
#include "folly/Synchronized.h"

namespace predictor {

class TFFramework : public ModelFramework {
 public:
  ~TFFramework() = default;
  bool calculateVector(CalculateVectorResponse* calculate_vector_response,
                       const CalculateVectorRequest& calculate_vector_request,
                       const std::string& model_full_name) override;
};

}  // namespace predictor
