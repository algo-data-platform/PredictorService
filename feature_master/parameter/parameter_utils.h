/*
 * Some unitity functions which can be used by other modules
 *
 * */

#pragma once

#include <memory>

#include "feature_master/parameter/parameter_extractor.h"

namespace feature_master {

std::unique_ptr<feature_master::ParameterExtractorManager> createParameterExtractorManager(
    const std::string& config_file_name);

}  // namespace feature_master
