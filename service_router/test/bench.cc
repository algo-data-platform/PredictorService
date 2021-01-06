#include "folly/init/Init.h"
#include "folly/portability/GFlags.h"
#include "folly/io/async/EventBase.h"
#include "folly/executors/IOThreadPoolExecutor.h"
#include "thrift/perf/cpp2/util/QPSStats.h"

#include "service_router/service_router_entity.h"
#include "service_router/router.h"
#include "service_router/http.h"
#include "common/metrics/metrics.h"

DEFINE_string(service_name, "test-registry", "test service name");
DEFINE_int32(pull_interval, 2, "test service pull config and service list interval");
DEFINE_int32(node_number, 10, "test service server numbers");
DEFINE_int32(replicator_num, 10, "test service server numbers");
DEFINE_int32(client_nums, 10, "test service client thread numbers");
DEFINE_int32(numbers, 10, "test service client thread numbers");
DEFINE_int32(diff_range, 256, "Address diff range");
DEFINE_string(load_balance_method, "random",
              "request load balance method `random/roundrobin/localfirst/configurable_weight`");
DEFINE_string(host, "127.0.0.1", "current service host address");
DEFINE_int32(http_port, 10011, "current service host address");

void createCluster() {
  uint32_t cip = 10;
  for (uint32_t i = 0; i < FLAGS_replicator_num; i++) {
    uint32_t dip = 10;
    std::map<uint32_t, std::vector<uint32_t>> shards;
    for (uint32_t k = 0; k < 1024; k++) {
      uint32_t mod = k % FLAGS_node_number;
      std::vector<uint32_t> ids;
      if (shards.find(mod) == shards.end()) {
        shards[mod] = {};
      }
      shards[mod].push_back(k);
    }
    for (uint32_t j = 0; j < FLAGS_node_number; j++) {
      service_router::Server server;
      std::string ip = folly::to<std::string>("10.85.", cip + i, ".", dip + j);
      server.setHost(ip);
      server.setPort(9000);
      server.setProtocol(service_router::ServerProtocol::THRIFT);
      server.setServiceName(FLAGS_service_name);
      server.setFollowerAvailableShardList(shards[j]);
      service_router::registerServer(server);
    }
  }
}

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  FLAGS_logtostderr = true;

  createCluster();

  std::vector<std::thread> threads;
  auto metrics = metrics::Metrics::getInstance();
  auto timers = metrics->buildTimers("test", "discover", 1, 0, 1000);
  for (int i = 0; i < FLAGS_client_nums; ++i) {
    threads.push_back(std::thread([&]() {
        auto method = service_router::stringToLoadBalanceMethod(FLAGS_load_balance_method);
        service_router::LoadBalanceMethod load_method = service_router::LoadBalanceMethod::RANDOM;
        if (!method) {
          method = load_method;
        }
        for (int i = 0; i < FLAGS_numbers; i++) {
          metrics::Timer timer(timers.get());
          service_router::ClientOption option;
          option.setServiceName(FLAGS_service_name);
          option.setProtocol(service_router::ServerProtocol::THRIFT);
          option.setLoadBalance(method.value());
          service_router::BalanceLocalFirstConfig local_first;
          local_first.setLocalIp(FLAGS_host);
          local_first.setDiffRange(FLAGS_diff_range);
          option.setLocalFirstConfig(local_first);
          service_router::ServerAddress address;
          option.setShardType(service_router::ShardType::FOLLOWER);
          option.setShardId(i % 1024);
          if (service_router::serviceClient(&address, option)) {
            // LOG(INFO) << address;
          }
        }
        LOG(INFO) << "Complete..";
    }));
  }

  service_router::httpServiceServer(FLAGS_service_name, FLAGS_host, FLAGS_http_port);
  while (1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  for (auto& thr : threads) {
    thr.join();
  }
  service_framework::http::stop();
  return 0;
}
