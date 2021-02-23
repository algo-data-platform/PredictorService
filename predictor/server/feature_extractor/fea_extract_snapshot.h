#pragma once

#include "common/metrics/metrics.h"
#include "common/serialize_util.h"
#include "common/util.h"
#include "feature_master/if/gen-cpp2/feature_master_types.tcc"
#include "predictor/if/gen-cpp2/predictor_types.tcc"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"
#include <iostream>
#include "predictor/global_resource/resource_manager.h"
#include "tensorflow/core/example/example.pb.h"
#include "City.h"

#define CTRL_A "\001"
#define CTRL_B "\002"

namespace predictor {

void originalFeatureSnapshot(const PredictRequest& request);
void predictRespSnapshot(const PredictResponse& response, const std::string& model_name = "");

void originalFeatureSnapshot(const CalculateVectorRequest& request);

bool shouldSnapshot(const std::string& req_id);
void generateOriginalFeatures(std::string* ori_features, const std::vector<feature_master::Feature>& feature_list);

}  // namespace predictor
