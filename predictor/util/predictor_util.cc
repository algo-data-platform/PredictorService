#include "predictor_util.h"
#include "service_router/router.h"

DEFINE_string(static_file, "", "static_file name");
DECLARE_string(predictor_service_name);
namespace predictor {
namespace util {
namespace {
constexpr char LOG_CATEGORY[] = "predictor_util.cc";
}  // namespace
//
// common functions
//
bool setServerStaticWeight(const std::string& service_name, const std::string& host, int port, int weight) {
  if (weight < 0) {
    LOG(ERROR) << "invalid weight " << weight;
    return false;
  }
  auto router = service_router::Router::getInstance();
  service_router::Server server;
  server.setServiceName(service_name);
  server.setHost(host);
  server.setPort(port);
  // Router::setWeight will only make the change IF the service is registered in this process. So no need to worry
  // about changing the wrong service.
  router->setWeight(server, weight);
  LOG(INFO) << "server weight has been changed to " << weight;
  return true;
}
void monitorThreadTaskStats(folly::FutureExecutor<folly::CPUThreadPoolExecutor> *thread_pool,
                            const std::string &metric_name) {
  if (!thread_pool) return;

  try {
    thread_pool->subscribeToTaskStats([metric_name](folly::ThreadPoolExecutor::TaskStats stats) {
      typedef std::chrono::duration<double, std::milli> double_ms;
      auto wtm = std::chrono::duration_cast<double_ms>(stats.waitTime).count();
      auto rtm = std::chrono::duration_cast<double_ms>(stats.runTime).count();
      markHistogram(metric_name, TAG_TASK, TASK_WAIT, wtm);
      markHistogram(metric_name, TAG_TASK, TASK_RUN, rtm);
    });
  } catch (const std::exception &e) {
    LOG(ERROR) << "failed to monitor thread task stats. ERROR:" << e.what();
  }
}

// service router的默认registry是consul，所以想通过ip+port的直连模式，需要显式调用一下这个函数
void setDummyRegistry() {
  auto router = service_router::Router::getInstance();
  auto registry = std::make_shared<service_router::FileRegistry>(3);
  registry->registerServersFromFile(FLAGS_predictor_service_name, FLAGS_static_file);
  router->setServiceRegistry(FLAGS_predictor_service_name, registry);
}

}  // namespace util
}  // namespace predictor
