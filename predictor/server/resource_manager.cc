#include "predictor/server/resource_manager.h"
#include "predictor/util/predictor_util.h"

#include <math.h>

#include "folly/synchronization/CallOnce.h"

DEFINE_int64(heavy_tasks_thread_num, 0,
             "num of concurrency threads of heavy_tasks_thread_pool_. default value is the number of cpu cores");
DEFINE_int64(tf_thread_num, 0, "num of concurrency threads of tensorflow_tasks_thread_pool_.");
DEFINE_int32(adjust_thread_pools, 0,
             "1: adjust thread number for heavry task thread pool and tf thread pool     others: do nothing");
DECLARE_int32(adjust_thread_pools);
DEFINE_bool(monitor_thread_task_stats, false, "true: monitor all cpu thread pool tasks");
DEFINE_int64(feature_extract_tasks_num, 0,
             "num of concurrency tasks of features extraction. default value is the half of cpu cores");

namespace predictor {

folly::FutureExecutor<folly::CPUThreadPoolExecutor>* ResourceMgr::heavy_tasks_thread_pool_ = nullptr;
folly::Synchronized<std::map<std::string, std::string>>  ResourceMgr::fea_extract_kafka_msg_map_;
KafkaProducer* ResourceMgr::fea_extract_snapshot_producer_ = nullptr;
folly::Synchronized<std::map<std::string, std::string>>  ResourceMgr::global_model_service_map_;

void ResourceMgr::initHeavyTasksThreadPool() {
  static auto init_heavy_tasks_threadpool_once = []() {
    // set up cpu thread pool
    if (0 == FLAGS_heavy_tasks_thread_num) {
      unsigned core_num = std::thread::hardware_concurrency();
      // turns out that hardware_concurrency may return 0 if the core number is not well defined. in that case, let's
      // just
      // give it a reasonable random value
      FLAGS_heavy_tasks_thread_num = core_num > 0 ? core_num : 10;
      if (1 == FLAGS_adjust_thread_pools && 0 == FLAGS_tf_thread_num) {
        FLAGS_tf_thread_num = FLAGS_heavy_tasks_thread_num = ceil(FLAGS_heavy_tasks_thread_num / 2);
      }
    }
    if (0 == FLAGS_feature_extract_tasks_num) {
      FLAGS_feature_extract_tasks_num = FLAGS_heavy_tasks_thread_num;
    }
    LOG(INFO) << "FLAGS_heavy_tasks_thread_num=" << FLAGS_heavy_tasks_thread_num
              << " FLAGS_feature_extract_tasks_num=" << FLAGS_feature_extract_tasks_num
              << " FLAGS_tf_thread_num=" << FLAGS_tf_thread_num;

    auto threadFactory = std::make_shared<folly::NamedThreadFactory>("heavy_tasks_thread_pool");
    heavy_tasks_thread_pool_ = new folly::FutureExecutor<folly::CPUThreadPoolExecutor>(
        std::make_pair(FLAGS_heavy_tasks_thread_num /*max thread num*/,
                       1 /*min thread num*/),
        threadFactory);

    if (FLAGS_monitor_thread_task_stats) {
      util::monitorThreadTaskStats(heavy_tasks_thread_pool_, HEAVY_TASK_CONSUMING);
    }
  };

  static folly::once_flag heavy_tasks_thread_pool_flag;
  folly::call_once(heavy_tasks_thread_pool_flag, init_heavy_tasks_threadpool_once);
}

void ResourceMgr::setHeavyTaskThreadPool(int max_thread_num) {
  heavy_tasks_thread_pool_->setNumThreads(max_thread_num);
  LOG(INFO) << "set heavy_tasks_thread_num=" << max_thread_num;
}

bool ResourceMgr::initFeaExtractKafkaProducer(std::string config_file) {
  fea_extract_snapshot_producer_ = new KafkaProducer();
  if (fea_extract_snapshot_producer_->initialize(config_file) != Status::OK) {
    LOG(ERROR) << "fea_extract_snapshot_producer init error";
    return false;
  }
  return true;
}

void ResourceMgr::clear() {
  if (nullptr != heavy_tasks_thread_pool_) {
    delete heavy_tasks_thread_pool_;
  }
}

}  // namespace predictor
