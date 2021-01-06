#pragma once

#include <string>
#include <vector>

#include "folly/dynamic.h"

namespace predictor {
struct ModelRecord {
  enum class State { not_loaded, loading, loaded, failed };

  std::string name;
  std::string timestamp;
  std::string full_name;
  std::string config_name;
  std::string md5;
  std::string business_line;
  State state;
  std::string success_time;

  ModelRecord() {}
  ModelRecord(const std::string &_name, const std::string &_timestamp,
              const std::string &_full_name, const std::string &_config_name,
              const std::string &_md5, const std::string &_business_line,
              State _state = State::not_loaded)
  : name(_name), timestamp(_timestamp), full_name(_full_name),
    config_name(_config_name), md5(_md5), business_line(_business_line), state(_state) {}

  ModelRecord(const ModelRecord& rhs)            = default;
  ModelRecord(ModelRecord&& rhs)                 = default;
  ModelRecord& operator=(const ModelRecord& rhs) = default;
  ModelRecord& operator=(ModelRecord&& rhs)      = default;

  inline bool operator<(const ModelRecord& rhs) const { return full_name < rhs.full_name; }
  std::string stateStr() const;
  folly::dynamic toDynamic() const;
};
}  // namespace predictor
