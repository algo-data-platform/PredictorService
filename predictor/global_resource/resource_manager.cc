#include "resource_manager.h"
#include "predictor/util/predictor_util.h"
#include "service_router/thrift.h"
#include "folly/synchronization/CallOnce.h"
#include <math.h>
#include <thread>  // NOLINT

DEFINE_int64(heavy_tasks_thread_num, 0,
             "num of concurrency threads of heavy_tasks_thread_pool_. default value is the number of cpu cores");
DEFINE_int64(tf_thread_num, 0, "num of concurrency threads of tensorflow_tasks_thread_pool_.");
DEFINE_bool(monitor_thread_task_stats, false, "true: monitor all cpu thread pool tasks");
DEFINE_int64(feature_extract_tasks_num, 0,
             "num of concurrency tasks of features extraction. default value is the half of cpu cores");

namespace {
constexpr char LOG_CATEGORY[] = "resource_manager.cc";
}
namespace predictor {
folly::FutureExecutor<folly::CPUThreadPoolExecutor>* ResourceMgr::heavy_tasks_thread_pool_ = nullptr;
folly::Synchronized<std::map<std::string, std::string>>  ResourceMgr::fea_extract_kafka_msg_map_;
KafkaProducer* ResourceMgr::fea_extract_snapshot_producer_ = nullptr;
folly::atomic_shared_ptr<ModelServiceMap> ResourceMgr::global_model_service_map_;
std::shared_ptr<service_router::ThriftServer> ResourceMgr::thrift_server_ = nullptr;
unsigned int ResourceMgr::core_num_ = 16;

void ResourceMgr::initHeavyTasksThreadPool() {
  static auto init_heavy_tasks_threadpool_once = []() {
    // setting up thread num based on core num
    if (FLAGS_heavy_tasks_thread_num == 0) {
      FLAGS_heavy_tasks_thread_num = ResourceMgr::core_num_;
    }
    if (FLAGS_tf_thread_num == 0) {
      auto half = ceil(FLAGS_heavy_tasks_thread_num / 2);
      FLAGS_heavy_tasks_thread_num = half;
      FLAGS_tf_thread_num = half;
    }
    if (FLAGS_feature_extract_tasks_num == 0) {
      FLAGS_feature_extract_tasks_num = FLAGS_heavy_tasks_thread_num;
    }
    // create thread pool
    auto threadFactory = std::make_shared<folly::NamedThreadFactory>("heavy_tasks_thread_pool");
    heavy_tasks_thread_pool_ = new folly::FutureExecutor<folly::CPUThreadPoolExecutor>(
        std::make_pair(FLAGS_heavy_tasks_thread_num /*max thread num*/,
                       1 /*min thread num*/),
        threadFactory);
    if (FLAGS_monitor_thread_task_stats) {
      util::monitorThreadTaskStats(heavy_tasks_thread_pool_, HEAVY_TASK_CONSUMING);
    }
    LOG(INFO) << "FLAGS_heavy_tasks_thread_num="  << FLAGS_heavy_tasks_thread_num
          << " FLAGS_feature_extract_tasks_num="  << FLAGS_feature_extract_tasks_num
          << " FLAGS_tf_thread_num="              << FLAGS_tf_thread_num;
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

std::shared_ptr<service_router::ThriftServer> ResourceMgr::getThriftServer() {
  return thrift_server_;
}

void ResourceMgr::createNewThriftServer() {
  thrift_server_ = std::make_shared<service_router::ThriftServer>();
}

void ResourceMgr::initCoreNum() {
  unsigned int num = std::thread::hardware_concurrency();
  core_num_ = num > 0 ? num : 16;  // 16 as the default
  LOG(ERROR) << "inited core_num_=" << core_num_;
}

void ResourceMgr::clear() {
  if (nullptr != heavy_tasks_thread_pool_) {
    delete heavy_tasks_thread_pool_;
  }
}

}  // namespace predictor
