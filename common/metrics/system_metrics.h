#pragma once

#include "folly/dynamic.h"
#include "folly/String.h"
#include "folly/Singleton.h"

#include "City.h"
#include "common/metrics/metrics.h"

namespace metrics {

class SystemMetrics : public std::enable_shared_from_this<SystemMetrics> {
 public:
  static std::shared_ptr<SystemMetrics> getInstance();
  SystemMetrics() {}
  ~SystemMetrics() = default;
  void init();

 private:
  size_t getProcessFileDescriptorNumber();
  const std::unordered_map<std::string, uint32_t> mallInfo();
  const std::unordered_map<std::string, double> procStats();
  folly::Synchronized<std::unordered_map<std::string, double>> data_;
};

}  // namespace metrics
