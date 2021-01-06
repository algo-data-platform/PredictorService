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
DEFINE_int32(diff_range, 256, "Address diff range");
DEFINE_string(host, "127.0.0.1", "current service host address");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  FLAGS_logtostderr = true;
  auto io_pool = std::make_shared<folly::IOThreadPoolExecutor>(
      std::thread::hardware_concurrency(), std::make_shared<folly::NamedThreadFactory>("IOThreadPool"));
  folly::setIOExecutor(io_pool);

  // for (int i = 0; i < FLAGS_server_nums; i++) {
  //  service_router::Server server;
  //  server.setHost("127.0.0.1");
  //  server.setPort(9000 + i);
  //  server.setProtocol(service_router::ServerProtocol::THRIFT);
  //  server.setServiceName(FLAGS_service_name);
  //  service_router::registerServer(server);
  //}

  facebook::thrift::benchmarks::QPSStats qps_stats_;
  std::string service_router_succ_ = "service_router_succ";
  std::string service_router_err_ = "service_router_err";
  qps_stats_.registerCounter(service_router_succ_);
  qps_stats_.registerCounter(service_router_err_);

  std::vector<std::thread> threads;
  for (int i = 0; i < FLAGS_client_nums; ++i) {
    threads.push_back(std::thread([&]() {
      for (int i = 0; i < FLAGS_numbers; i++) {
        service_router::ClientOption option;
        option.setServiceName(FLAGS_service_name);
        option.setProtocol(service_router::ServerProtocol::THRIFT);
        option.setLoadBalance(service_router::LoadBalanceMethod::LOCALFIRST);
        service_router::BalanceLocalFirstConfig local_first;
        local_first.setLocalIp(FLAGS_host);
        local_first.setDiffRange(FLAGS_diff_range);
        option.setLocalFirstConfig(local_first);
        service_router::ServerAddress address;
        if (service_router::serviceClient(&address, option)) {
          LOG(INFO) << address;
          qps_stats_.add(service_router_succ_);
        } else {
          qps_stats_.add(service_router_err_);
        }
      }
      LOG(INFO) << "Complete..";
    }));
  }

  while (1) {
    qps_stats_.printStats(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  for (auto& thr : threads) {
    thr.join();
  }

  service_framework::http::stop();
  io_pool->stop();
  return 0;
}
