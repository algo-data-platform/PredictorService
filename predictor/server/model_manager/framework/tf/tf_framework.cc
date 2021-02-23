#include "predictor/server/model_manager/framework/tf/tf_framework.h"
#include "predictor/util/predictor_util.h"

namespace predictor {
REGISTER_FRAMEWORK(TFFramework, tf).describe("This is a tf framework.");
}  // namespace predictor
