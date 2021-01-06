/*
 * 抽取并且离散化之后的 uint64 ID (sign_t 签名) 的构成:
 *
 * 14 bits group_t + 50 bits value_t
 *
 * group_t: 将相似的 ID 聚合, 由配置来定义
 * value_t: 由原始特征变换或者组合而来
 * */

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>


namespace feature_master {

constexpr char PARAMETER_EXTRACTOR_CONFIG[] = "parameter_extractor_config";
constexpr char GROUP[] = "group";
constexpr char EXTRACTOR[]= "extractor";
constexpr char FEATURE_NAMES[] = "feature_names";
constexpr char EXTRACTOR_ARGS[] = "extractor_args";
constexpr char HASH_SIZE[] = "hash_size";

constexpr char EXTRACTOR_TYPE_ID[] = "id";
constexpr char EXTRACTOR_TYPE_DISCRETE[] = "discrete";
constexpr char EXTRACTOR_TYPE_CROSS[] = "cross";

typedef uint16_t group_t;
typedef uint64_t value_t;
typedef uint64_t sign_t;

constexpr uint16_t kParameterGroupBits = 14;
constexpr uint16_t kParameterValueBits = 50;

constexpr uint64_t PARAMETER_VALUE_MASK = ((1ul << kParameterValueBits) - 1);

inline group_t GetParameterGroup(sign_t sign) { return sign >> kParameterValueBits; }

inline value_t GetParameterValue(sign_t sign) { return sign & PARAMETER_VALUE_MASK; }

inline int GetParameterShard(sign_t sign, int shard_num) { return sign % shard_num; }

struct Parameter {
  explicit Parameter(const group_t group, const value_t value) : group_(group), value_(value) {}
  explicit Parameter(const sign_t sign) : group_(GetParameterGroup(sign)), value_(GetParameterValue(sign)) {}
  Parameter() = default;
  group_t group_;
  value_t value_;
};

using Parameters = std::vector<Parameter>;

inline sign_t GenParameterSign(group_t group, value_t value) {
  return (static_cast<sign_t>(group) << kParameterValueBits) | (value & PARAMETER_VALUE_MASK);
}

inline sign_t GenParameterSign(const Parameter& parameter) {
  return GenParameterSign(parameter.group_, parameter.value_);
}

}  // namespace feature_master
