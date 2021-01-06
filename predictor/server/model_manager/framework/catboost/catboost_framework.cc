#include "predictor/server/model_manager/framework/catboost/catboost_framework.h"
#include "predictor/util/predictor_util.h"

namespace predictor {

REGISTER_FRAMEWORK(CatboostFramework, catboost).describe("This is a catboost framework.");

}  // namespace predictor
