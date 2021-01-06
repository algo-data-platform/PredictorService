#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/serialize_util.h"
#include "feature_master/parameter/parameter_extractor.h"
#include "feature_master/if/gen-cpp2/feature_master_types.h"
#include "feature_master/if/gen-cpp2/feature_master_types.tcc"

namespace common {
class SerializerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    google::InitGoogleLogging("SerializerTest");
    packFeatures();
  }
  void TearDown() override { google::ShutdownGoogleLogging(); }
  void packFeatures() {
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

    features_.set_features(std::vector<feature_master::Feature>{feature,  feature2, feature3, feature4,
                                                                feature5, feature6, feature7, feature8});
  }

  feature_master::Features& getFeatures() { return features_; }

  feature_master::Features features_;
};

TEST_F(SerializerTest, Sanity) { EXPECT_EQ(1, 1); }

TEST_F(SerializerTest, SerializeCompact) {
  std::string str;
  common::serializeThriftObjToCompact(&str, features_);

  feature_master::Features features;
  common::deserializeThriftObjFromCompact(&features, str);
  EXPECT_EQ(features, getFeatures());
}

TEST_F(SerializerTest, SerializeBinary) {
  std::string str;
  common::serializeThriftObjToBinary(&str, getFeatures());

  feature_master::Features features;
  common::deserializeThriftObjFromBinary(&features, str);
  EXPECT_EQ(features, getFeatures());
}

TEST_F(SerializerTest, SerializeJson) {
  std::string str;
  common::serializeThriftObjToJson(&str, getFeatures());

  feature_master::Features features;
  common::deserializeThriftObjFromJson(&features, str);
  EXPECT_EQ(features, getFeatures());
}

}  // namespace common
