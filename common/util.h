#pragma once

#include <iomanip>
#include <ctime>
#include <chrono>  // NOLINT
#include <string>

#include "folly/dynamic.h"
#include "folly/json.h"
#include "folly/GLog.h"

constexpr int DEBUG_LEVEL = 5;

namespace common {

bool fromJson(folly::dynamic* result, const folly::StringPiece& json);

bool toJson(std::string* result, const folly::dynamic& data);

std::string pathJoin(const std::string &a, const std::string &b);
std::string pathJoin(const std::string& a, const std::string& b, const std::string& c);

std::time_t currentTimeInMs();

std::time_t currentTimeInNs();

std::string currentTimeToStr(std::time_t cur_time);

// get current time and format it as requested
std::string currentTimeInFormat(const std::string &format);

uint32_t ipToInt(const std::string& ip);

void getLocalIpAddress(std::string* ip_addr);

class Timer {
 public:
  inline void start() { begin_ = std::chrono::steady_clock::now(); }
  double stop() {
    end_ = std::chrono::steady_clock::now();
    return get();
  }
  double get() {
    std::chrono::duration<double, std::milli> ms = end_ - begin_;
    return ms.count();
  }

 private:
  std::chrono::steady_clock::time_point begin_;
  std::chrono::steady_clock::time_point end_;
};

// range struct for multi-thread processing
struct Range {
  unsigned begin;
  unsigned end;
};
bool split_slice(std::vector<Range>* slice_vec,
                 unsigned total_num,
                 unsigned min_item_num_per_thread,
                 unsigned max_thread_num);

#define NO_COPY_NO_MOVE(ClassName)       \
  ClassName(const ClassName&) = delete;           \
  ClassName operator=(const ClassName&) = delete; \
  ClassName(ClassName&&) = delete;                \
  ClassName& operator=(ClassName&&) = delete

#define NO_COPY(ClassName)     \
  ClassName(const ClassName&) = delete; \
  ClassName operator=(const ClassName&) = delete

#define NO_MOVE(ClassName) \
  ClassName(ClassName&&) = delete;  \
  ClassName& operator=(ClassName&&) = delete

#define AD_VLOG(log_level) \
  VLOG(log_level) << LOG_CATEGORY << ": "

#define AD_LOG(log_level) \
  LOG(log_level) << LOG_CATEGORY << ": "

}  // namespace common
