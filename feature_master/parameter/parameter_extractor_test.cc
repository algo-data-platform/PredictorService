#include <fstream>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "folly/init/Init.h"
#include "feature_master/parameter/parameter_extractor.h"

namespace feature_master {

/*
 * 1. read test config from test_data
 * 2. build some sample Features
 * 3. extract paramters from Features based on config
 * 4. verify those parameters
 * */
class ParameterExtractorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::ifstream ifile("test_data/parameter_test.json");
    if (!ifile) {
      std::cerr << "Failed to open test data file!" << std::endl;
    }
    std::ostringstream buf;
    char ch;
    while (buf && ifile.get(ch)) buf.put(ch);
    json_config_ = buf.str();
  }
  void TearDown() override {}

  std::string json_config_;
};

TEST_F(ParameterExtractorTest, Sanity) { EXPECT_EQ(1, 1); }

// Test plan: convert a Parameter to a sign_t, and then convert it back to a new Parameter. The new Parameter should
// match the old one.
TEST_F(ParameterExtractorTest, GenParameterSign) {
  group_t group = 1;
  value_t value = 123412412;
  Parameter param(group, value);
  sign_t key = GenParameterSign(param);
  Parameter new_param(key);
  EXPECT_EQ(group, new_param.group_);
  EXPECT_EQ(value, new_param.value_);
}

// Test plan:
// 1. Test if ParameterExtractorHelper::ParseJsonConfig is runnable.
// 2. Test if it can successfully parse every different case of the json config
// TODO(changyu1): what is the order of the ParseJsonConfig output?
TEST_F(ParameterExtractorTest, ParseJsonConfig) {
  ParameterConfigList parameter_configs;
  ParameterExtractorHelper::ParseJsonConfig(&parameter_configs, json_config_);

  EXPECT_EQ(5, parameter_configs.size());

  const auto& emp_ctr = parameter_configs[1];
  EXPECT_EQ("emp_ctr", emp_ctr.parameter_name_);
  EXPECT_EQ(4, emp_ctr.group_);
  EXPECT_EQ("discrete", emp_ctr.extractor_);
  EXPECT_EQ("1,0.001,100,0,100", emp_ctr.extractor_args_);
  EXPECT_EQ(2, emp_ctr.feature_names_.size());
  EXPECT_EQ("ctr1", emp_ctr.feature_names_[0]);
  EXPECT_EQ("ctr2", emp_ctr.feature_names_[1]);

  const auto& uid = parameter_configs[2];
  EXPECT_EQ("uid", uid.parameter_name_);
  EXPECT_EQ(1, uid.group_);
  EXPECT_EQ("id", uid.extractor_);
  EXPECT_EQ("", uid.extractor_args_);
  EXPECT_EQ(2, uid.feature_names_.size());
  EXPECT_EQ("id1", uid.feature_names_[0]);

  const auto& city = parameter_configs[3];
  EXPECT_EQ("city-gender-tags", city.parameter_name_);
  EXPECT_EQ(3, city.group_);
  EXPECT_EQ("cross", city.extractor_);
  EXPECT_EQ("", city.extractor_args_);
  EXPECT_EQ(0, city.feature_names_.size());
  EXPECT_EQ(1, city.feature_groups_.size());
  EXPECT_EQ(3, city.feature_groups_[0].size());
  EXPECT_EQ("city", city.feature_groups_[0][0]);
  EXPECT_EQ("gender", city.feature_groups_[0][1]);
  EXPECT_EQ("tags", city.feature_groups_[0][2]);

  const auto& follow = parameter_configs[4];
  EXPECT_EQ("follow_list", follow.parameter_name_);
}

TEST_F(ParameterExtractorTest, ParameterExtractorManager) {
  ParameterExtractorManager manager(json_config_);
  Parameters parameters;
  Feature feature;
  feature.set_feature_name("id1");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Tom", "Mike"});
  Feature feature2;
  feature2.set_feature_name("id2");
  feature2.set_feature_type(FeatureType::STRING_LIST);
  feature2.set_string_values({"Lily", "Mike"});
  Feature feature3;
  feature3.set_feature_name("ctr1");
  feature3.set_feature_type(FeatureType::FLOAT_LIST);
  feature3.set_float_values({0.11, 0.35});
  Feature feature4;
  feature4.set_feature_name("ctr2");
  feature4.set_feature_type(FeatureType::STRING_LIST);
  feature4.set_string_values({"Mike", "Tom"});
  Feature feature5;
  feature5.set_feature_name("city");
  feature5.set_feature_type(FeatureType::STRING_LIST);
  feature5.set_string_values({"Beijing", "Shanghai", "Guangzhou"});
  Feature feature6;
  feature6.set_feature_name("gender");
  feature6.set_feature_type(FeatureType::STRING_LIST);
  feature6.set_string_values({"male", "female"});
  Feature feature7;
  feature7.set_feature_name("tags");
  feature7.set_feature_type(FeatureType::STRING_LIST);
  feature7.set_string_values({"good", "bad"});
  Feature feature8;
  feature8.set_feature_name("follow_list");
  feature8.set_feature_type(FeatureType::STRING_LIST);
  feature8.set_string_values({"football"});
  Features features;
  features.set_features(
      std::vector<Feature>{feature, feature2, feature3, feature4, feature5, feature6, feature7, feature8});
  bool ret = manager.ExtractParameters(&parameters, features);
  EXPECT_TRUE(ret);
  // The total number of parameters should be the sum of all individual features, which is 16, excludes ctr2 which is 2
  // because string is not supported for discrete parameter, plus the number of city-gender-tags, which is 3*2*2=12. So
  // the final number is 16 - 2 + 12 = 26
  EXPECT_EQ(26, parameters.size());
  EXPECT_EQ(5, parameters[0].group_);
  EXPECT_EQ(5, parameters[1].group_);
  EXPECT_EQ(5, parameters[2].group_);
  EXPECT_EQ(5, parameters[3].group_);
  EXPECT_EQ(5, parameters[4].group_);
  EXPECT_EQ(5, parameters[5].group_);
  EXPECT_EQ(5, parameters[6].group_);
  EXPECT_EQ(4, parameters[7].group_);
  EXPECT_EQ(4, parameters[8].group_);
  EXPECT_EQ(1, parameters[9].group_);
  EXPECT_EQ(1, parameters[10].group_);
  EXPECT_EQ(1, parameters[11].group_);
  EXPECT_EQ(1, parameters[12].group_);
  EXPECT_EQ(2, parameters[13].group_);
  EXPECT_EQ(3, parameters[14].group_);
  EXPECT_EQ(3, parameters[15].group_);
  EXPECT_EQ(3, parameters[16].group_);
  EXPECT_EQ(3, parameters[17].group_);
  EXPECT_EQ(3, parameters[18].group_);
  EXPECT_EQ(3, parameters[19].group_);
  EXPECT_EQ(3, parameters[20].group_);
  EXPECT_EQ(3, parameters[21].group_);
  EXPECT_EQ(3, parameters[22].group_);
  EXPECT_EQ(3, parameters[23].group_);
  EXPECT_EQ(3, parameters[24].group_);
  EXPECT_EQ(3, parameters[25].group_);
  // 990172869651399961ul is the hash value based on the group number and the feature value
  EXPECT_EQ(990172869651399961ul, parameters[0].value_);
  EXPECT_EQ(11398982471131821021ul, parameters[1].value_);
  EXPECT_EQ(2967877961002940389ul, parameters[2].value_);
  EXPECT_EQ(1118936920830451299ul, parameters[3].value_);
  EXPECT_EQ(17516046262329946906ul, parameters[7].value_);
  EXPECT_EQ(17593846279824460452ul, parameters[8].value_);
  EXPECT_EQ(13902575994019907086ul, parameters[9].value_);
  EXPECT_EQ(148537672375360645ul, parameters[10].value_);
  EXPECT_EQ(3375648457712180357ul, parameters[13].value_);
  EXPECT_EQ(17062756011621284817ul, parameters[14].value_);
  EXPECT_EQ(17787894808655991922ul, parameters[15].value_);
}

// Test plan:
// 1. Test if ParameterExtractorId::extract
// 2. Test if two features with same value generate the same parameter
TEST_F(ParameterExtractorTest, ParameterExtractorId) {
  // case 1: features with string list values
  std::vector<std::string> feature_names{"id1", "id2"};
  ParameterConfig feature_config("uid", 2, "id", feature_names, "");
  ParameterExtractorId extractor(feature_config);
  Parameters parameters;
  Feature feature;
  feature.set_feature_name("id1");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Tom", "Mike"});
  Feature feature2;
  feature2.set_feature_name("id2");
  feature2.set_feature_type(FeatureType::STRING_LIST);
  feature2.set_string_values({"Tom", "Mike"});
  FeatureMap feature_map;
  feature_map["id1"] = &feature;
  feature_map["id2"] = &feature2;
  bool ret = extractor.extract(&parameters, feature_map);
  EXPECT_TRUE(ret);
  EXPECT_EQ(4, parameters.size());
  EXPECT_EQ(2, parameters[0].group_);
  EXPECT_EQ(2, parameters[1].group_);
  EXPECT_EQ(2, parameters[2].group_);
  EXPECT_EQ(2, parameters[3].group_);
  // 691592406323062ul is the hash value based on the group number and the feature value
  EXPECT_EQ(13902575994019907086ul, parameters[0].value_);
  EXPECT_EQ(148537672375360645ul, parameters[1].value_);
  EXPECT_EQ(parameters[0].value_, parameters[2].value_);

  // case 2: features with int64 list values
  std::vector<std::string> feature_names2{"id3", "id4"};
  ParameterConfig feature_config2("uid", 3, "id", feature_names2, "");
  ParameterExtractorId extractor2(feature_config2);
  Parameters parameters2;
  Feature feature3;
  feature3.set_feature_name("id3");
  feature3.set_feature_type(FeatureType::INT64_LIST);
  feature3.set_int64_values({4321, 1234});
  Feature feature4;
  feature4.set_feature_name("id4");
  feature4.set_feature_type(FeatureType::INT64_LIST);
  feature4.set_int64_values({4321, 1234});
  FeatureMap feature_map2;
  feature_map2["id3"] = &feature3;
  feature_map2["id4"] = &feature4;
  ret = extractor2.extract(&parameters2, feature_map2);
  EXPECT_TRUE(ret);
  unsigned parameters_size = feature3.get_int64_values().size() + feature4.get_int64_values().size();
  EXPECT_EQ(parameters_size, parameters.size());
  EXPECT_EQ(3, parameters2[0].group_);
  EXPECT_EQ(3, parameters2[1].group_);
  EXPECT_EQ(3, parameters2[2].group_);
  EXPECT_EQ(3, parameters2[2].group_);
  // 602826910931964ul is the hash value based on the group number and the feature value
  EXPECT_EQ(3767865266511159274ul, parameters2[0].value_);
  EXPECT_EQ(14585574172588466967ul, parameters2[1].value_);
  EXPECT_EQ(parameters2[0].value_, parameters2[2].value_);

  // case 3: features with double list values
  std::vector<std::string> feature_names3{"id5", "id6"};
  ParameterConfig feature_config3("uid", 4, "id", feature_names3, "");
  ParameterExtractorId extractor3(feature_config3);
  Parameters parameters3;
  Feature feature5;
  feature5.set_feature_name("id5");
  feature5.set_feature_type(FeatureType::FLOAT_LIST);
  feature5.set_float_values({4321.12414141, 1234.1});
  Feature feature6;
  feature6.set_feature_name("id6");
  feature6.set_feature_type(FeatureType::FLOAT_LIST);
  feature6.set_float_values({4321.12414141, 1234.1});
  FeatureMap feature_map3;
  feature_map3["id5"] = &feature5;
  feature_map3["id6"] = &feature6;
  ret = extractor3.extract(&parameters3, feature_map3);
  EXPECT_TRUE(ret);
  EXPECT_EQ(4, parameters3.size());
  EXPECT_EQ(4, parameters3[0].group_);
  EXPECT_EQ(4, parameters3[1].group_);
  EXPECT_EQ(4, parameters3[2].group_);
  EXPECT_EQ(4, parameters3[3].group_);
  // 44581187059677ul is the hash value based on the group number and the feature value
  EXPECT_EQ(5770942322161612102ul, parameters3[0].value_);
  EXPECT_EQ(9030683747672447924ul, parameters3[1].value_);
  EXPECT_EQ(parameters3[0].value_, parameters3[2].value_);
}

TEST_F(ParameterExtractorTest, ParameterExtractorDiscrete) {
  // case 1: string-type feature
  std::vector<std::string> feature_names{"ctr1"};
  ParameterConfig feature_config("emp_ctr", 2, "discrete", feature_names, "1,0.001,100,0,100");
  ParameterExtractorDiscrete extractor(feature_config);
  Parameters parameters;
  Feature feature;
  feature.set_feature_name("ctr1");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Tom", "Mike"});
  FeatureMap feature_map;
  feature_map["ctr1"] = &feature;
  bool ret = extractor.extract(&parameters, feature_map);
  EXPECT_TRUE(ret);
  // no parameters generated because ctr1 has string values, which is not supported by discrete extractor
  EXPECT_EQ(0, parameters.size());

  // case 2: int-type feature
  // case 2.1: different feature values with the same discrete values should result in same parameter values. As shown
  // below, feature2=[10, 34], feature3=[11, 30]. With denominator=10, both of their discrete value is [10, 30].
  // Therefore their parameter values should be the same
  std::vector<std::string> feature_names2{"age1", "age2"};
  ParameterConfig feature_config2("age", 2, "discrete", feature_names2, "100,1,10,0,10");
  ParameterExtractorDiscrete extractor2(feature_config2);
  Parameters parameters2;
  Feature feature2;
  feature2.set_feature_name("age1");
  feature2.set_feature_type(FeatureType::INT64_LIST);
  feature2.set_int64_values({10, 34});
  FeatureMap feature_map2;
  feature_map2["age1"] = &feature2;
  Feature feature3;
  feature3.set_feature_name("age2");
  feature3.set_feature_type(FeatureType::INT64_LIST);
  feature3.set_int64_values({11, 40});
  feature_map2["age2"] = &feature3;
  ret = extractor2.extract(&parameters2, feature_map2);
  EXPECT_TRUE(ret);
  EXPECT_EQ(4, parameters2.size());
  EXPECT_EQ(2, parameters2[0].group_);
  EXPECT_EQ(2, parameters2[1].group_);
  EXPECT_EQ(2, parameters2[2].group_);
  EXPECT_EQ(2, parameters2[3].group_);
  EXPECT_EQ(10143279697293281662ul, parameters2[0].value_);
  EXPECT_EQ(3153007625117323233ul, parameters2[1].value_);
  EXPECT_EQ(parameters2[0].value_, parameters2[2].value_);
  EXPECT_NE(parameters2[1].value_, parameters2[3].value_);

  // case 2.2: feature value bigger than max should result in same parameter value of max feature value
  std::vector<std::string> feature_names3{"age1", "age2"};
  ParameterConfig feature_config3("age", 3, "discrete", feature_names3, "100,1,100,0,100");
  ParameterExtractorDiscrete extractor3(feature_config3);
  Parameters parameters3;
  Feature feature4;
  feature4.set_feature_name("age1");
  feature4.set_feature_type(FeatureType::INT64_LIST);
  feature4.set_int64_values({222, 34});
  FeatureMap feature_map3;
  feature_map3["age1"] = &feature4;
  Feature feature5;
  feature5.set_feature_name("age2");
  feature5.set_feature_type(FeatureType::INT64_LIST);
  feature5.set_int64_values({100, 39});
  feature_map3["age2"] = &feature5;
  ret = extractor3.extract(&parameters3, feature_map3);
  EXPECT_TRUE(ret);
  EXPECT_EQ(4, parameters3.size());
  EXPECT_EQ(3, parameters3[0].group_);
  EXPECT_EQ(3, parameters3[1].group_);
  EXPECT_EQ(3, parameters3[2].group_);
  EXPECT_EQ(3, parameters3[3].group_);
  EXPECT_EQ(6036457646356932801ul, parameters3[0].value_);
  EXPECT_EQ(17593846279824460452ul, parameters3[1].value_);
  EXPECT_EQ(parameters3[0].value_, parameters3[2].value_);
  EXPECT_NE(parameters3[1].value_, parameters3[3].value_);

  // case 3: double-type feature
  // case 3.1: similar with 2.1, different feature values with the same discrete values should result in same parameter
  // values.
  std::vector<std::string> feature_names4{"temp1", "temp2"};
  ParameterConfig feature_config4("temperature", 2, "discrete", feature_names4, "100,0.001,100,0,100");
  ParameterExtractorDiscrete extractor4(feature_config4);
  Parameters parameters4;
  Feature feature6;
  feature6.set_feature_name("temp1");
  feature6.set_feature_type(FeatureType::FLOAT_LIST);
  feature6.set_float_values({10.4, 34.5});
  FeatureMap feature_map4;
  feature_map4["temp1"] = &feature6;
  Feature feature7;
  feature7.set_feature_name("temp2");
  feature7.set_feature_type(FeatureType::FLOAT_LIST);
  feature7.set_float_values({10.3, 34.6});
  feature_map4["temp2"] = &feature7;
  ret = extractor4.extract(&parameters4, feature_map4);
  EXPECT_TRUE(ret);
  EXPECT_EQ(4, parameters4.size());
  EXPECT_EQ(2, parameters4[0].group_);
  EXPECT_EQ(2, parameters4[1].group_);
  EXPECT_EQ(2, parameters4[2].group_);
  EXPECT_EQ(2, parameters4[3].group_);
  EXPECT_EQ(857753361285495786ul, parameters4[0].value_);
  EXPECT_EQ(501957400116423268ul, parameters4[1].value_);
  EXPECT_EQ(parameters4[0].value_, parameters4[2].value_);
  EXPECT_EQ(parameters4[1].value_, parameters4[3].value_);
}

// TODO(changyu1): add dedicated test case for ParameterExtractorCross. The basic functionality is tested in
// ParameterExtractorManager test case.
TEST_F(ParameterExtractorTest, ParameterExtractorCross) {
  // case 1: basic testing of ParameterExtractorCross
  FeatureGroups feature_groups;
  feature_groups.push_back({"city", "age"});
  ParameterConfig feature_config("city-age", 2, EXTRACTOR_TYPE_CROSS, feature_groups, "");
  ParameterExtractorCross extractor(feature_config);
  Parameters parameters;
  Feature feature;
  feature.set_feature_name("city");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Beijing", "Shanghai"});
  Feature feature2;
  feature2.set_feature_name("age");
  feature2.set_feature_type(FeatureType::INT64_LIST);
  feature2.set_int64_values({89, 54, 12});
  FeatureMap feature_map;
  feature_map["city"] = &feature;
  feature_map["age"] = &feature2;
  bool ret = extractor.extract(&parameters, feature_map);
  EXPECT_TRUE(ret);
  EXPECT_EQ(6, parameters.size());
  EXPECT_EQ(2, parameters[0].group_);
  EXPECT_EQ(2, parameters[1].group_);
  EXPECT_EQ(2, parameters[2].group_);
  EXPECT_EQ(2, parameters[3].group_);
  EXPECT_EQ(2, parameters[4].group_);
  EXPECT_EQ(2, parameters[5].group_);
  EXPECT_EQ(14036320209624190076ul, parameters[0].value_);
  EXPECT_EQ(17000505436883623944ul, parameters[4].value_);
  EXPECT_EQ(8149443577140474565ul, parameters[5].value_);

  // case 2: independency of each combination of features.
  // For example, city=["Beijing", "Shanghai"], age=[18, 18], id=["id1", "id2"], cross parameter is city-age-id. In this
  // case, we will generate 8 parameters for city-age-id. Among them, some of the combinations are the same. For
  // example, we will have two Beijing-18-id1. These two parameters should have the same values.
  FeatureGroups feature_groups2;
  feature_groups2.push_back({"city", "age", "id"});
  ParameterConfig feature_config2("city-age-id", 2, EXTRACTOR_TYPE_CROSS, feature_groups2, "");
  ParameterExtractorCross extractor2(feature_config2);
  Feature feature3;
  feature3.set_feature_name("city");
  feature3.set_feature_type(FeatureType::STRING_LIST);
  feature3.set_string_values({"Beijing", "Shanghai"});
  Feature feature4;
  feature4.set_feature_name("age");
  feature4.set_feature_type(FeatureType::INT64_LIST);
  feature4.set_int64_values({18, 18, });
  Feature feature5;
  feature5.set_feature_name("id");
  feature5.set_feature_type(FeatureType::STRING_LIST);
  feature5.set_string_values({"id1", "id2"});
  FeatureMap feature_map2;
  feature_map2["city"] = &feature3;
  feature_map2["age"] = &feature4;
  feature_map2["id"] = &feature5;
  Parameters parameters2;
  ret = extractor2.extract(&parameters2, feature_map2);
  EXPECT_TRUE(ret);
  EXPECT_EQ(8, parameters2.size());
  EXPECT_EQ(2, parameters2[0].group_);
  EXPECT_EQ(2, parameters2[1].group_);
  EXPECT_EQ(2, parameters2[2].group_);
  EXPECT_EQ(2, parameters2[3].group_);
  EXPECT_EQ(2, parameters2[4].group_);
  EXPECT_EQ(2, parameters2[5].group_);
  EXPECT_EQ(2, parameters2[6].group_);
  EXPECT_EQ(2, parameters2[7].group_);
  EXPECT_EQ(parameters2[0].value_, parameters2[2].value_);
  EXPECT_EQ(parameters2[4].value_, parameters2[6].value_);
}

// Test the case when the input does NOT have all features in config
TEST_F(ParameterExtractorTest, ParameterExtractorManagerMissingFeatures) {
  ParameterExtractorManager manager(json_config_);
  Parameters parameters;
  Feature feature;
  feature.set_feature_name("id1");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Tom", "Mike"});
  Feature feature2;
  feature2.set_feature_name("id2");
  feature2.set_feature_type(FeatureType::STRING_LIST);
  feature2.set_string_values({"Lily", "Mike"});
  Feature feature3;
  feature3.set_feature_name("ctr1");
  feature3.set_feature_type(FeatureType::FLOAT_LIST);
  feature3.set_float_values({0.11, 0.35});
  Feature feature4;
  feature4.set_feature_name("ctr2");
  feature4.set_feature_type(FeatureType::STRING_LIST);
  feature4.set_string_values({"Mike", "Tom"});
  Feature feature5;
  feature5.set_feature_name("city");
  feature5.set_feature_type(FeatureType::STRING_LIST);
  feature5.set_string_values({"Beijing", "Shanghai", "Guangzhou"});
  Feature feature6;
  feature6.set_feature_name("gender");
  feature6.set_feature_type(FeatureType::STRING_LIST);
  feature6.set_string_values({"male", "female"});
  Feature feature7;
  feature7.set_feature_name("tags");
  feature7.set_feature_type(FeatureType::STRING_LIST);
  feature7.set_string_values({"good", "bad"});
  Feature feature8;
  feature8.set_feature_name("follow_list");
  feature8.set_feature_type(FeatureType::STRING_LIST);
  feature8.set_string_values({"football"});
  Features features;
  // compared with ParameterExtractorManager test case, we are missing feature2, which is an individual id parameter,
  // and feature5, which is a component of cross parameter
  features.set_features(
      std::vector<Feature>{feature, /*feature2,*/ feature3, feature4, /*feature5,*/ feature6, feature7, feature8});
  bool ret = manager.ExtractParameters(&parameters, features);
  EXPECT_TRUE(ret);
  // The total number of parameters should be the sum of all individual features, which is 11, excludes ctr2 which is 2
  // because string is not supported for discrete parameter, plus 0 cross parameter since we are missing "city", which
  // is a component of the cross parameter city-gender-tags. The final number is 11 - 2 = 9
  // EXPECT_EQ(26, parameters.size());
  EXPECT_EQ(9, parameters.size());
  EXPECT_EQ(5, parameters[0].group_);
  EXPECT_EQ(5, parameters[1].group_);
  EXPECT_EQ(5, parameters[2].group_);
  EXPECT_EQ(5, parameters[3].group_);
  EXPECT_EQ(4, parameters[4].group_);
  EXPECT_EQ(4, parameters[5].group_);
  EXPECT_EQ(1, parameters[6].group_);
  EXPECT_EQ(1, parameters[7].group_);
  EXPECT_EQ(2, parameters[8].group_);
  // 990172869651399961ul is the hash value based on the group number and the feature value
  EXPECT_EQ(1118936920830451299ul, parameters[0].value_);
  EXPECT_EQ(15654829452869785370ul, parameters[1].value_);
  EXPECT_EQ(923178507387520354ul, parameters[2].value_);
  EXPECT_EQ(18250178709229817440ul, parameters[3].value_);
  EXPECT_EQ(17516046262329946906ul, parameters[4].value_);
  EXPECT_EQ(13902575994019907086ul, parameters[6].value_);
}

TEST_F(ParameterExtractorTest, ParameterExtractorIdWithHashSize) {
  std::vector<std::string> feature_names{"id1", "id2"};
  ParameterConfig feature_config("uid", 2, "id", feature_names, "", 2 /* hash_size*/);
  ParameterExtractorId extractor(feature_config);
  Parameters parameters;

  Feature feature;
  feature.set_feature_name("id1");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Tom", "Mike"});

  Feature feature2;
  feature2.set_feature_name("id2");
  feature2.set_feature_type(FeatureType::STRING_LIST);
  feature2.set_string_values({"Tom", "Mike"});

  FeatureMap feature_map;
  feature_map["id1"] = &feature;
  feature_map["id2"] = &feature2;

  bool ret = extractor.extract(&parameters, feature_map);
  EXPECT_TRUE(ret);

  EXPECT_EQ(4, parameters.size());
  EXPECT_EQ(2, parameters[0].group_);
  EXPECT_EQ(2, parameters[1].group_);
  EXPECT_EQ(2, parameters[2].group_);
  EXPECT_EQ(2, parameters[3].group_);

  // 13902575994019907086ul was the hash value, after mod 2 it became 0
  EXPECT_EQ(0, parameters[0].value_);
  // 148537672375360645ul was the hash value, after mod 2 it became 1
  EXPECT_EQ(1, parameters[1].value_);
  EXPECT_EQ(0, parameters[2].value_);
  EXPECT_EQ(1, parameters[3].value_);
}

TEST_F(ParameterExtractorTest, ParameterExtractorDiscreteWithHashSize) {
  std::vector<std::string> feature_names2{"age1", "age2"};
  ParameterConfig feature_config2("age", 2, "discrete", feature_names2, "100,1,10,0,10", 3 /* hash_size */);
  ParameterExtractorDiscrete extractor2(feature_config2);
  Parameters parameters2;

  Feature feature2;
  feature2.set_feature_name("age1");
  feature2.set_feature_type(FeatureType::INT64_LIST);
  feature2.set_int64_values({10, 34});

  FeatureMap feature_map2;
  feature_map2["age1"] = &feature2;

  Feature feature3;
  feature3.set_feature_name("age2");
  feature3.set_feature_type(FeatureType::INT64_LIST);
  feature3.set_int64_values({11, 40});
  feature_map2["age2"] = &feature3;

  bool ret = extractor2.extract(&parameters2, feature_map2);
  EXPECT_TRUE(ret);

  EXPECT_EQ(4, parameters2.size());
  EXPECT_EQ(2, parameters2[0].group_);
  EXPECT_EQ(2, parameters2[1].group_);
  EXPECT_EQ(2, parameters2[2].group_);
  EXPECT_EQ(2, parameters2[3].group_);

  // 10143279697293281662ul was the hash value, after mod 3 it became 1
  EXPECT_EQ(1, parameters2[0].value_);
  // 3153007625117323233ul was the hash value, after mod 3 it became 0
  EXPECT_EQ(0, parameters2[1].value_);
  EXPECT_EQ(parameters2[0].value_, parameters2[2].value_);
  EXPECT_NE(parameters2[1].value_, parameters2[3].value_);
}

TEST_F(ParameterExtractorTest, ParameterExtractorCrossWithHashSize) {
  FeatureGroups feature_groups;
  feature_groups.push_back({"city", "age"});
  ParameterConfig feature_config("city-age", 2, EXTRACTOR_TYPE_CROSS, feature_groups, "", 17 /* hash_size */);
  ParameterExtractorCross extractor(feature_config);
  Parameters parameters;

  Feature feature;
  feature.set_feature_name("city");
  feature.set_feature_type(FeatureType::STRING_LIST);
  feature.set_string_values({"Beijing", "Shanghai"});

  Feature feature2;
  feature2.set_feature_name("age");
  feature2.set_feature_type(FeatureType::INT64_LIST);
  feature2.set_int64_values({89, 54, 12});

  FeatureMap feature_map;
  feature_map["city"] = &feature;
  feature_map["age"] = &feature2;

  bool ret = extractor.extract(&parameters, feature_map);
  EXPECT_TRUE(ret);

  EXPECT_EQ(6, parameters.size());
  EXPECT_EQ(2, parameters[0].group_);
  EXPECT_EQ(2, parameters[1].group_);
  EXPECT_EQ(2, parameters[2].group_);
  EXPECT_EQ(2, parameters[3].group_);
  EXPECT_EQ(2, parameters[4].group_);
  EXPECT_EQ(2, parameters[5].group_);

  // 14036320209624190076ul mod 17 = 14, etc
  EXPECT_EQ(14, parameters[0].value_);
  EXPECT_EQ(15, parameters[1].value_);
  EXPECT_EQ(4, parameters[2].value_);
  EXPECT_EQ(5, parameters[3].value_);
  EXPECT_EQ(13, parameters[4].value_);
  EXPECT_EQ(5, parameters[5].value_);
}

}  // namespace feature_master

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
