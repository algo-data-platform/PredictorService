#pragma once

#include "predictor_constants.h"
#include "common/metrics/metrics.h"
#include "folly/GLog.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/FutureExecutor.h"
#include "folly/io/IOBuf.h"
#include <memory>

namespace predictor {
namespace util {
//
// Metrics
//
static metrics::Timers* buildTimers(const std::string &metric_name, const MetricTagsMap &tags_map) {
  return metrics::Metrics::getInstance()->buildTimers(
    SERVER_NAME, metric_name, TIMER_BUCKET_SCALE, TIMER_MIN, TIMER_MAX, tags_map).get();
}
static void markMeter(const std::string &metric_name, const MetricTagsMap &tags_map) {
  metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, metric_name, tags_map, MINUTES)->mark();
}
static void markMeter(const std::string &metric_name, const MetricTagsMap &tags_map, double num) {
  metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, metric_name, tags_map, MINUTES)->mark(num);
}
static void markHistogram(const std::string &metric_name, const std::string &tag_key,
                          const std::string &tag_value, const double &num) {
  const MetricTagsMap tags_map{{tag_key, tag_value}};
  metrics::Metrics::getInstance()->buildHistograms(
    SERVER_NAME, metric_name, HISTOGRAMS_BUCKET_SCALE, HISTOGRAMS_MIN, HISTOGRAMS_MAX, tags_map)->addValue(num);
}
static void logAndMetricError(const std::string& err_category, const std::string& req_id = "") {
  const MetricTagsMap err_tag{{TAG_CATEGORY, err_category}};
  metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag)->mark();
  FB_LOG_EVERY_MS(ERROR, 2000) << err_category << ", req_id: " << req_id;
}
//
// common functions
//
bool setServerStaticWeight(const std::string& service_name, const std::string& host, int port, int weight);
void monitorThreadTaskStats(folly::FutureExecutor<folly::CPUThreadPoolExecutor> *thread_pool,
                            const std::string &metric_name);

inline double restoreFromNegativeSampling(double origin, double negative_sampling_ratio) {
  return origin / (origin + (1 - origin) / negative_sampling_ratio);
}
inline double sigmoid(double output) { return 1.0 / (1.0 + std::exp(-output)); }
void setDummyRegistry();
static std::string getModelFrameworkName(const std::string& model_full_name) {
  std::vector<std::string> tmp;
  folly::split('_', model_full_name, tmp);
  if (tmp.size() < 3) {
    logAndMetricError(model_full_name + "-invalid_model_name");
    return "";
  }
  return tmp[0];
}

/*
 * ONLY use this function when 'buf' can be parsed as a string
 */
std::string parseIOBuf(std::unique_ptr<folly::IOBuf> buf);

}  // namespace util
}  // namespace predictor
