#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include <fstream>
#include <iostream>
#include <string>

#include "common/serialize_util.h"
#include "feature_master/parameter/parameter_extractor.h"
#include "feature_master/if/gen-cpp2/feature_master_types.h"
#include "feature_master/if/gen-cpp2/feature_master_types.tcc"

void packFeatures(feature_master::Features* features) {
  feature_master::Feature feature;
  feature.set_feature_name("id1");
  feature.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature.set_string_values({"Tom", "Mike"});

  feature_master::Feature feature2;
  feature2.set_feature_name("id2");
  feature2.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature2.set_string_values({"Lily", "Mike"});

  feature_master::Feature feature3;
  feature3.set_feature_name("ctr1");
  feature3.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature3.set_float_values({0.11, 0.35});

  feature_master::Feature feature4;
  feature4.set_feature_name("ctr2");
  feature4.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature4.set_string_values({"Mike", "Tom"});

  feature_master::Feature feature5;
  feature5.set_feature_name("city");
  feature5.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature5.set_string_values({"Beijing", "Shanghai", "Guangzhou"});

  feature_master::Feature feature6;
  feature6.set_feature_name("gender");
  feature6.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature6.set_string_values({"male", "female"});

  feature_master::Feature feature7;
  feature7.set_feature_name("tags");
  feature7.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature7.set_string_values({"good", "bad"});

  feature_master::Feature feature8;
  feature8.set_feature_name("follow_list");
  feature8.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature8.set_string_values({"football"});

  features->set_features(std::vector<feature_master::Feature>{feature,  feature2, feature3, feature4,
                                                              feature5, feature6, feature7, feature8});
}

int write_string_to_file_append(const std::string& file_string, const std::string str) {
  std::ofstream OsWrite(file_string, std::ofstream::app);
  OsWrite << str;
  OsWrite << std::endl;
  OsWrite.close();
  return 0;
}

int main(int argc, char* argv[]) {
  std::string file = "/tmp/test_compact.txt";
  feature_master::Features features;
  std::string str;

  packFeatures(&features);
  if (!common::serializeThriftObjToCompact(&str, features)) {
    std::cout << "Err: serializeThriftObjToCompact failed!" << std::endl;
    return false;
  }
  // write two rows
  write_string_to_file_append(file, str);
  write_string_to_file_append(file, str);

  // read from file
  std::ifstream in;
  in.open("/tmp/test_compact.txt");
  std::string line;
  feature_master::Features eval_features;
  while (getline(in, line)) {
    if (!common::deserializeThriftObjFromCompact(&eval_features, line)) {
      std::cout << "Err: serializeThriftObjToCompact failed!" << std::endl;
      return false;
    }

    if (eval_features != features) {
      std::cout << "Err: reading is not the same as writing" << std::endl;
      return -1;
    }
  }

  std::cout << ">>>>>>>>>>>>>>>>>Test OK!" << std::endl;
  in.close();
  return 0;
}

