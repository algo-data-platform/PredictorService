#include "predictor/server/model_manager/framework/xgboost/xgboost_framework.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/util/predictor_util.h"

namespace predictor {

REGISTER_FRAMEWORK(XGBoostFramework, xgboost).describe("This is a xgboost framework.");

}  // namespace predictor
