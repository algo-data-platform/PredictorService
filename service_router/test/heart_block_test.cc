#include "folly/init/Init.h"
#include "folly/portability/GFlags.h"
#include "folly/io/async/EventBase.h"
#include "folly/executors/IOThreadPoolExecutor.h"
#include "thrift/perf/cpp2/util/QPSStats.h"

#include "service_router/service_router_entity.h"
#include "service_router/router.h"

DEFINE_string(service_name, "test-registry", "test service name");
DEFINE_int32(pull_interval, 2, "test service pull config and service list interval");
DEFINE_int32(server_nums, 1, "test service server numbers");
DEFINE_int32(client_nums, 10, "test service client thread numbers");
DEFINE_int32(numbers, 10, "test service client thread numbers");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  FLAGS_logtostderr = true;

  service_router::Server server;
  server.setHost("127.0.0.1");
  server.setPort(9000);
  server.setProtocol(service_router::ServerProtocol::THRIFT);
  server.setServiceName(FLAGS_service_name);
  service_router::registerServer(server);

  auto router = service_router::Router::getInstance();
  router->waitForConfig(FLAGS_service_name);

  router->subscribeConfigItem(FLAGS_service_name, "cluster_info_data",
                          [](const std::string&, const std::string&, const std::string&) {
    LOG(INFO) << "callback cluster_info_data config";
    while (1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  });

  while (1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
