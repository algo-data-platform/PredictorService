#include "feature_master/parameter/parameter_extractor.h"

#include <vector>
#include <fstream>

#include "City.h"
#include "thirdparty/rapidjson/document.h"
#include "glog/logging.h"
#include "folly/String.h"
#include "common/util.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace feature_master {

namespace {

constexpr char LOG_CATEGORY[] = "parameter_extractor.cc: ";

}  // namespace

namespace {
ParameterExtractor* ParameterExtractorFactory(const std::string& type, const ParameterConfig& feature_config) {
  if (type == EXTRACTOR_TYPE_ID) {
    return new ParameterExtractorId(feature_config);
  } else if (type == EXTRACTOR_TYPE_DISCRETE) {
    return new ParameterExtractorDiscrete(feature_config);
  } else if (type == EXTRACTOR_TYPE_CROSS) {
    return new ParameterExtractorCross(feature_config);
  } else {
    return nullptr;
  }

  return nullptr;
}

constexpr uint64_t SEED = 1ul;
}  // namespace

bool ParameterExtractorHelper::ParseJsonConfig(ParameterConfigList* parameter_configs, const std::string& json_config) {
  if (parameter_configs == nullptr) {
    return false;
  }
  // Parse the json config file
  rapidjson::Document document;
  document.Parse(json_config.c_str());
  rapidjson::StringBuffer json_buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(json_buffer);
  document.Accept(writer);
  AD_VLOG(DEBUG_LEVEL) << "dumping feature extractor config: " << json_buffer.GetString();
  if (document.HasMember(PARAMETER_EXTRACTOR_CONFIG)) {
    const rapidjson::Value& configs = document[PARAMETER_EXTRACTOR_CONFIG];
    for (rapidjson::Value::ConstMemberIterator itr = configs.MemberBegin(); itr != configs.MemberEnd(); ++itr) {
      const std::string& parameter_name = itr->name.GetString();
      const rapidjson::Value& config_value = itr->value;
      if (!config_value.HasMember(GROUP) || !config_value.HasMember(EXTRACTOR) ||
          !config_value.HasMember(FEATURE_NAMES)) {
        return false;
      }
      group_t group = config_value[GROUP].GetInt();
      std::string extractor = config_value[EXTRACTOR].GetString();
      std::string extractor_args;
      if (config_value.HasMember(EXTRACTOR_ARGS)) {
        extractor_args = config_value[EXTRACTOR_ARGS].GetString();
      }
      std::vector<std::string> feature_names;
      for (const auto& item : config_value[FEATURE_NAMES].GetArray()) {
        feature_names.push_back(item.GetString());
      }
      // optional hyper-parameter 'hash_size'
      uint64_t hash_size = 0;
      if (config_value.HasMember(HASH_SIZE) && config_value[HASH_SIZE].GetInt() > 0) {
        hash_size = config_value[HASH_SIZE].GetInt();
      }

      if (EXTRACTOR_TYPE_CROSS == extractor) {
        FeatureGroups feature_groups;
        for (const auto& feature_name : feature_names) {
          StringList tmp_feature_names;
          folly::split("|", feature_name, tmp_feature_names);
          feature_groups.push_back(std::move(tmp_feature_names));
        }
        (*parameter_configs)
            .push_back(ParameterConfig(parameter_name, group, extractor, feature_groups, extractor_args, hash_size));
      } else {
        (*parameter_configs)
            .push_back(ParameterConfig(parameter_name, group, extractor, feature_names, extractor_args, hash_size));
      }
    }
  }
  return true;
}

// Parse json config file and then for each parameter config, create and store its extractor.
// Since cross extractor needs to use the cached value of other parameters, it needs to be run at the end. Therefore we
// process non-cross extractor
// first, and then cross extractor
ParameterExtractorManager::ParameterExtractorManager(const std::string& json_config) { init(json_config); }

bool ParameterExtractorManager::initFromFile(const std::string& json_file) {
  std::ifstream ifs(json_file.c_str());
  if (!ifs) {
    LOG(ERROR) << "can'f find file: " << json_file;
    return false;
  }
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  return init(buffer.str());
}

bool ParameterExtractorManager::init(const std::string& json_config) {
  ParameterConfigList parameter_configs;
  is_config_valid_ = ParameterExtractorHelper::ParseJsonConfig(&parameter_configs, json_config);
  if (!is_config_valid_) {
    return false;
  }

  // Process non-cross extractor first
  for (const auto& parameter_config : parameter_configs) {
    if (EXTRACTOR_TYPE_CROSS == parameter_config.extractor_) {
      continue;
    }
    ParameterExtractor* extractor_ptr = ParameterExtractorFactory(parameter_config.extractor_, parameter_config);
    if (nullptr == extractor_ptr) {
      continue;
    }
    extractor_list_.push_back(std::shared_ptr<ParameterExtractor>(extractor_ptr));
  }

  // Process cross extractor
  for (const auto& parameter_config : parameter_configs) {
    // Process non-cross extractor first
    if (EXTRACTOR_TYPE_CROSS != parameter_config.extractor_) {
      continue;
    }
    ParameterExtractor* extractor_ptr = ParameterExtractorFactory(parameter_config.extractor_, parameter_config);
    if (nullptr == extractor_ptr) {
      continue;
    }
    extractor_list_.push_back(std::shared_ptr<ParameterExtractor>(extractor_ptr));
  }

  return true;
}

bool ParameterExtractorManager::ExtractParameters(Parameters* parameters, const Features& features) const {
  if (!is_config_valid_) {
    return false;
  }

  FeatureMap fea_map;
  for (const auto& feature : features.get_features()) {
    fea_map[feature.get_feature_name()] = &feature;
  }

  try {
    for (const auto& item : extractor_list_) {
      if (!item->extract(parameters, fea_map)) {
        return false;
      }
    }
  }
  catch (...) {
    std::stringstream sstream;
    for (const auto& feature : features.get_features()) {
      sstream << feature.get_feature_name() << "|";
    }
    LOG(ERROR) << "Error while processing features " << sstream.str();
  }
  return true;
}

// TODO(changyu): unit test and refactor
bool ParameterExtractorManager::ExtractParameters(Parameters* parameters,
                                                  const std::vector<const Features*>& features_list) const {
  if (!is_config_valid_) {
    return false;
  }

  FeatureMap fea_map;
  for (const auto& features : features_list) {
    for (const auto& feature : features->get_features()) {
      fea_map[feature.get_feature_name()] = &feature;
    }
  }

  try {
    for (const auto& item : extractor_list_) {
      if (!item->extract(parameters, fea_map)) {
        return false;
      }
    }
  }
  catch (...) {
    std::stringstream sstream;
    for (const auto& features : features_list) {
      for (const auto& feature : features->get_features()) {
        sstream << feature.get_feature_name() << "|";
      }
    }
    LOG(ERROR) << "Error while processing features " << sstream.str();
  }
  return true;
}

void ParameterExtractor::genParameterValues(Parameters* parameters_ptr, const Feature& feature) const {
  value_t value;
  switch (feature.get_feature_type()) {
    case FeatureType::STRING_LIST: {
      const std::vector<std::string>& str_values = feature.get_string_values();
      for (const auto& strValue : str_values) {
        value = CityHash64(strValue.c_str(), strValue.size());
        if (config_.hash_size_) {
          value %= config_.hash_size_;
        }
        parameters_ptr->push_back(Parameter(config_.group_, value));
      }
      break;
    }
    case FeatureType::INT64_LIST: {
      const std::vector<int64_t>& int_values = feature.get_int64_values();
      for (const auto& int_value : int_values) {
        value = CityHash64(reinterpret_cast<const char*>(&int_value), sizeof(int64_t));
        if (config_.hash_size_) {
          value %= config_.hash_size_;
        }
        parameters_ptr->push_back(Parameter(config_.group_, value));
      }
      break;
    }
    case FeatureType::FLOAT_LIST: {
      const std::vector<double>& float_values = feature.get_float_values();
      for (const auto& float_value : float_values) {
        value = CityHash64(reinterpret_cast<const char*>(&float_value), sizeof(double));
        if (config_.hash_size_) {
          value %= config_.hash_size_;
        }
        parameters_ptr->push_back(Parameter(config_.group_, value));
      }
      break;
    }
    default: {
      LOG(ERROR) << "Invalid feature type " << static_cast<int>(feature.get_feature_type());
      break;
    }
  }
}

bool ParameterExtractor::extract(Parameters* parameters, const FeatureMap& feature_map) const {
  const auto& feature_names = config_.feature_names_;
  for (const auto& feature_name : feature_names) {
    auto iter = feature_map.find(feature_name);
    if (iter == feature_map.end()) {
      AD_VLOG(DEBUG_LEVEL) << "Missing feature=" << feature_name;
      continue;
    } else {
      genParameterValues(parameters, *(iter->second));
    }
  }
  return true;
}

bool ParameterExtractorId::extract(Parameters* parameters, const FeatureMap& feature_map) const {
  return ParameterExtractor::extract(parameters, feature_map);
}

bool ParameterExtractorCross::extract(Parameters* parameters, const FeatureMap& features) const {
  const FeatureGroups& feature_groups = config_.feature_groups_;
  for (const auto& feature_names : feature_groups) {
    value_t key = SEED;
    genCrossParameterValues(parameters, &features, feature_names, key, 0);
  }

  return true;
}

// Function to calculate cross parameter based on the value of each individual parameter it contains. For example, say
// we have two id parameters: hobby=['reading', 'basketball'], friends=['Tom', 'Mike']. And we have a cross parameter
// 'hobby-friends'. To calcualte it, we need to calulate the hash value of ['reading', 'Tom'], ['reading', 'Mike'],
// ['basketball', 'Tom'], ['basketball', 'Mike']
//
// Inputs:
//   parameters - store the pre-calculated value of those individual parameters.
//   feature_names - list of feature names of this cross parameter
//   group - group of this cross parameter
//   key - hash value calculated from the previous level of recursion
//   index - index of feature_names
// Outputs:
//   parameters
void ParameterExtractorCross::genCrossParameterValues(Parameters* parameters, const FeatureMap* features,
                                                      const StringList& feature_names, value_t key,
                                                      unsigned index) const {
  if (index >= feature_names.size()) {
    if (config_.hash_size_) {
      key %= config_.hash_size_;
    }
    parameters->push_back(Parameter(config_.group_, key));
    return;
  }

  auto feature_iter = features->find(feature_names[index]);
  if (features->end() == feature_iter) {
    return;
  }

  const Feature& feature = *(feature_iter->second);
  switch (feature.get_feature_type()) {
    case FeatureType::STRING_LIST: {
      const std::vector<std::string>& str_values = feature.get_string_values();
      for (const auto& strValue : str_values) {
        value_t local_key = key;
        local_key = CityHash64WithSeed(strValue.c_str(), strValue.size(), local_key);
        genCrossParameterValues(parameters, features, feature_names, local_key, index + 1);
      }
      break;
    }
    case FeatureType::INT64_LIST: {
      const std::vector<int64_t>& int_values = feature.get_int64_values();
      for (const auto& int_value : int_values) {
        value_t local_key = key;
        local_key = CityHash64WithSeed(reinterpret_cast<const char*>(&int_value), sizeof(int64_t), local_key);
        genCrossParameterValues(parameters, features, feature_names, local_key, index + 1);
      }
      break;
    }
    case FeatureType::FLOAT_LIST: {
      const std::vector<double>& float_values = feature.get_float_values();
      for (const auto& float_value : float_values) {
        value_t local_key = key;
        local_key = CityHash64WithSeed(reinterpret_cast<const char*>(&float_value), sizeof(double), local_key);
        genCrossParameterValues(parameters, features, feature_names, local_key, index + 1);
      }
      break;
    }
    default: {
      LOG(ERROR) << "Invalid feature type " << static_cast<int>(feature.get_feature_type());
      break;
    }
  }
}

bool ParameterExtractorDiscrete::initArgs() {
  const std::string& args_str = config_.extractor_args_;
  std::vector<std::string> args;
  folly::split(",", args_str, args);
  if (args.size() < 5) {
    LOG(ERROR) << "Wrong size of arg list! Expected=5, actual=" << args.size();
    return false;
  }
  try {
    denom_ = folly::to<double>(args[0]);
    smooth_ = folly::to<double>(args[1]);
    max_bucket_number_ = folly::to<int64_t>(args[2]);
    min_bucket_number_ = folly::to<int64_t>(args[3]);
    buckets_ = folly::to<unsigned>(args[4]);
  }
  catch (...) {
    LOG(ERROR) << "stod failed for " << args[0] << ", " << args[1] << ", " << args[2] << args[3] << args[4];
    return false;
  }
  return true;
}

ParameterExtractorDiscrete::ParameterExtractorDiscrete(const ParameterConfig& config) : ParameterExtractor(config) {
  initArgs();
}

ParameterExtractorDiscrete::ParameterExtractorDiscrete(ParameterConfig&& config) : ParameterExtractor(config) {
  initArgs();
}

// TODO(changyu1): refactor might be needed as it share a decent part of ParameterExtractor::genParameterValue. However
// I have not figured out a good way to do it. I thought of two ways:
// 1. Preprocess feature to get the discrete values and then pass into ParameterExtractor::genParameterValue. But this
// envolves a copy of Feature, which is expensive.
// 2. Pass in some callback function into ParameterExtractor::genParameterValue to do some preprocessing and override it
// in ParameterExtractorDiscrete. But it will still affect the performance because we need to at least run these
// callback functions, although they might do nothing.
void ParameterExtractorDiscrete::genParameterValues(Parameters* parameters_ptr, const Feature& feature) const {
  value_t value = 0;
  switch (feature.get_feature_type()) {
    case FeatureType::STRING_LIST: {
      LOG(ERROR) << "Discrete extracotr does not support string-type feature";
      break;
    }
    case FeatureType::INT64_LIST: {
      const std::vector<int64_t>& int_values = feature.get_int64_values();
      for (const auto& int_value : int_values) {
        int64_t bucket_number = findBucket(int_value);
        value = CityHash64(reinterpret_cast<const char*>(&bucket_number), sizeof(int64_t));
        if (config_.hash_size_) {
          value %= config_.hash_size_;
        }
        parameters_ptr->push_back(Parameter(config_.group_, value));
      }
      break;
    }
    case FeatureType::FLOAT_LIST: {
      const std::vector<double>& float_values = feature.get_float_values();
      for (const auto& float_value : float_values) {
        int64_t bucket_number = findBucket(float_value);
        value = CityHash64(reinterpret_cast<const char*>(&bucket_number), sizeof(int64_t));
        if (config_.hash_size_) {
          value %= config_.hash_size_;
        }
        parameters_ptr->push_back(Parameter(config_.group_, value));
      }
      break;
    }
    default: {
      LOG(ERROR) << "Invalid feature type " << static_cast<int>(feature.get_feature_type());
      break;
    }
  }
}

bool ParameterExtractorDiscrete::extract(Parameters* parameters, const FeatureMap& feature_map) const {
  return ParameterExtractor::extract(parameters, feature_map);
}

}  // namespace feature_master

