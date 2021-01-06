#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#include "folly/init/Init.h"
#include "folly/String.h"
#include "gflags/gflags.h"
#include "feature_master/parameter/parameter_extractor.h"

DEFINE_string(parameter_extractor_config, "parameter_extractor_test_tool.config", "parameter_extractor_config");
DEFINE_string(examples_file, "examples.txt", "values need to be extracted");

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  std::ifstream ifile(FLAGS_parameter_extractor_config);
  if (!ifile) {
    std::cerr << "Failed to open test config file=" << FLAGS_parameter_extractor_config << std::endl;
  }
  std::ostringstream buf;
  char ch;
  while (buf && ifile.get(ch)) buf.put(ch);
  std::string json_config = buf.str();

  feature_master::ParameterExtractorManager manager(json_config);

  std::ifstream ifs(FLAGS_examples_file);
  std::string line;
  while (std::getline(ifs, line)) {
    std::vector<std::string> tokens;
    folly::split('\t', line, tokens);
    if (tokens.size() < 2) {
      std::cerr << "params size err!" << std::endl;
      return -1;
    }
    feature_master::Parameters parameters;
    feature_master::Feature feature;
    feature.set_feature_name(tokens[0]);
    feature.set_feature_type(feature_master::FeatureType::STRING_LIST);
    feature.set_string_values({tokens[1]});
    feature_master::Features features;
    features.set_features(std::vector<feature_master::Feature>{feature});
    manager.ExtractParameters(&parameters, features);
    std::cout << tokens[1] << " | " << parameters[0].group_ << " | " << parameters[0].value_ << " | "
              << feature_master::GenParameterSign(parameters[0]) << std::endl;
  }
  return 0;
}
