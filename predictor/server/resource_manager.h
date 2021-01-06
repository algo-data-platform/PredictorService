#pragma once

#include "common/util.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/FutureExecutor.h"
#include "predictor/server/kafka_producer.h"

namespace predictor {

/*
 * This class maintains some globel resource objects.
 * For each resource, it provides the interface of initialization and access.
 * It also provides the interface to release all the resources at once.
 *
 * The typical usage of this class is, the user calls the initialization interface
 * for the resource he or she needs to use, BEFORE USING THEM. After that they
 * can just call the access interface when needed.
 *
 * Note that each resource will not be valid to used UNTIL THE INITIALIZTION
 * INTERFACE IS CALLED. And it is the USER's RESPONSIBILITY to make sure that
 * the specific resource is initialized before using it.
 */
class ResourceMgr {
 public:
  NO_COPY_NO_MOVE(ResourceMgr);
  ResourceMgr() = delete;

  // explicitly release all resources
  // note that this function is NOT thread safe. It is expected to be called at exit.
  static void clear();

  // interfaces to initialize and access heavy_tasks_thread_pool_
  static void initHeavyTasksThreadPool();
  static void adjustHeavyTasksThreadPool();
  static folly::FutureExecutor<folly::CPUThreadPoolExecutor>* getHeavyTasksThreadPool() {
    return heavy_tasks_thread_pool_;
  }
  static bool initFeaExtractKafkaProducer(std::string config_file);

  static void setHeavyTaskThreadPool(int max_thread_num);

  static folly::Synchronized<std::map<std::string, std::string>> fea_extract_kafka_msg_map_;
  static KafkaProducer* fea_extract_snapshot_producer_;

  static folly::Synchronized<std::map<std::string, std::string>> global_model_service_map_;

 private:
  // This is the thread pool which will idealy handle most, if not all, of the heavy
  // tasks. Therefore the number of the threads in this pool should not be bigger than the cpu core number.
  static folly::FutureExecutor<folly::CPUThreadPoolExecutor>* heavy_tasks_thread_pool_;
};

}  // namespace predictor
