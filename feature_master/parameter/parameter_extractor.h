#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "feature_master/parameter/parameter.h"
#include "feature_master/if/gen-cpp2/feature_master_types.h"

namespace feature_master {

using FeatureList = std::vector<Feature>;
using StringList = std::vector<std::string>;
using FeatureGroups = std::vector<StringList>;
// feature name -> parameter index list
struct ParameterConfig {
  ParameterConfig() = default;
  ~ParameterConfig() = default;
  ParameterConfig(const ParameterConfig&) = default;
  ParameterConfig(ParameterConfig&&) = default;
  ParameterConfig& operator=(const ParameterConfig&) = default;
  ParameterConfig& operator=(ParameterConfig&&) = default;
  explicit ParameterConfig(const std::string& parameter_name, group_t group, const std::string& extractor,
                           const std::vector<std::string>& feature_names, const std::string& extractor_args,
                           uint64_t hash_size = 0)
      : parameter_name_(parameter_name),
        group_(group),
        extractor_(extractor),
        feature_names_(feature_names),
        extractor_args_(extractor_args),
        hash_size_(hash_size) {}
  explicit ParameterConfig(const std::string& parameter_name, group_t group, const std::string& extractor,
                           const FeatureGroups& feature_groups, const std::string& extractor_args,
                           uint64_t hash_size = 0)
      : parameter_name_(parameter_name),
        group_(group),
        extractor_(extractor),
        feature_groups_(feature_groups),
        extractor_args_(extractor_args),
        hash_size_(hash_size) {}
  std::string parameter_name_;
  group_t group_;
  std::string extractor_;
  std::vector<std::string> feature_names_;
  FeatureGroups feature_groups_;  // for cross extractor
  std::string extractor_args_;
  uint64_t hash_size_;
};

using ParameterConfigList = std::vector<ParameterConfig>;

class ParameterExtractorHelper {
 public:
  static bool ParseJsonConfig(ParameterConfigList* parameter_configs, const std::string& json_config);
};

class ParameterExtractor;
// feature name -> feature const pointer
using FeatureMap = std::unordered_map<std::string, const Feature*>;
using ExtractorList = std::vector<std::shared_ptr<const ParameterExtractor>>;
class ParameterExtractorManager {
 public:
  ParameterExtractorManager() {}
  explicit ParameterExtractorManager(const std::string& json_config);
  bool init(const std::string& json_config);
  bool initFromFile(const std::string& json_file);
  bool ExtractParameters(Parameters* parameters, const Features& features) const;
  // TODO(changyu): unit test
  bool ExtractParameters(Parameters* parameters, const std::vector<const Features*>& features_list) const;

 private:
  ExtractorList extractor_list_;
  bool is_config_valid_;
};

class ParameterExtractor {
 public:
  explicit ParameterExtractor(const ParameterConfig& config) : config_(config) {}
  explicit ParameterExtractor(ParameterConfig&& config) : config_(config) {}
  virtual bool extract(Parameters* parameters, const FeatureMap& feature_map) const = 0;
  virtual std::string name() const = 0;
  virtual ~ParameterExtractor() {}

 protected:
  ParameterConfig config_;
  virtual void genParameterValues(Parameters* parameters_ptr, const Feature& feature) const;
};

class ParameterExtractorId : public ParameterExtractor {
 public:
  explicit ParameterExtractorId(const ParameterConfig& config) : ParameterExtractor(config) {}
  explicit ParameterExtractorId(ParameterConfig&& config) : ParameterExtractor(config) {}
  bool extract(Parameters* parameters, const FeatureMap& feature_map) const override;
  std::string name() const override { return "id"; }
  virtual ~ParameterExtractorId() {}
};

class ParameterExtractorCross : public ParameterExtractor {
 public:
  explicit ParameterExtractorCross(const ParameterConfig& config) : ParameterExtractor(config) {}
  explicit ParameterExtractorCross(ParameterConfig&& config) : ParameterExtractor(config) {}
  bool extract(Parameters* parameters, const FeatureMap& feature_map) const override;
  std::string name() const override { return "cross"; }
  virtual ~ParameterExtractorCross() {}

 protected:
  void genCrossParameterValues(Parameters* parameters,
                               const FeatureMap* features,
                               const StringList& feature_names,
                               value_t key,
                               unsigned index) const;
};

class ParameterExtractorDiscrete : public ParameterExtractor {
 public:
  explicit ParameterExtractorDiscrete(const ParameterConfig& config);
  explicit ParameterExtractorDiscrete(ParameterConfig&& config);
  bool extract(Parameters* parameter, const FeatureMap& feature_map) const override;
  std::string name() const override { return "discrete"; }
  virtual ~ParameterExtractorDiscrete() {}

 protected:
  virtual void genParameterValues(Parameters* parameters_ptr, const Feature& feature) const;

 private:
  bool initArgs();
  template <typename VALUE_TYPE>
  int64_t findBucket(VALUE_TYPE value) const {
    int64_t bucket_number = std::floor(((value + smooth_) / denom_) * buckets_);
    return std::min(std::max(bucket_number, min_bucket_number_), max_bucket_number_);
  }
  double denom_;
  double smooth_;
  int64_t min_bucket_number_;
  int64_t max_bucket_number_;
  unsigned buckets_;
};

}  // namespace feature_master
