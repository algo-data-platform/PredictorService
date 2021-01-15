#pragma once

#include <string>
#include "predictor/if/gen-cpp2/predictor_config_types.tcc"

namespace predictor {
bool initFromFile(ModelConfig *config, const std::string &config_filename);
std::string getModelFullName(const ModelConfig &config);
}  // namespace predictor
