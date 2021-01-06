#include "predictor_data.h"

namespace predictor {
std::string ModelRecord::stateStr() const {
  switch (state) {
  case State::not_loaded:
    return "not_loaded";
  case State::loading:
    return "loading";
  case State::loaded:
    return "loaded";
  case State::failed:
    return "failed";
  default:
    return "UNKNOWN";
  }
}
folly::dynamic ModelRecord::toDynamic() const {
  return folly::dynamic::object
    ("name", name)
    ("timestamp", timestamp)
    ("fullName", full_name)
    ("configName", config_name)
    ("md5", md5)
    ("state", stateStr())
    ("success_time", success_time);
}
}  // namespace predictor
