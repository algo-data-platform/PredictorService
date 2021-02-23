#pragma once

#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/if/gen-cpp2/PredictorService.h"

namespace predictor {

class TFFramework : public ModelFramework {
 public:
  ~TFFramework() = default;
};

}  // namespace predictor
