#include <fstream>

#include "common/util.h"
#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "service_router/thrift.h"

#include "predictor/util/predictor_util.h"
#include "predictor/server/predictor_service/predictor_service.h"
#include "predictor/server/http_service/http_service.h"
#include "predictor/global_resource/resource_manager.h"

DEFINE_int32(port, 0, "predictor server port");
DEFINE_int32(http_port, 0, "predictor http server port");
DEFINE_string(host, "", "predictor server host");
DEFINE_string(validate_host, "127.0.0.1", "validate server host");
DEFINE_string(service_name, "predictor_service", "predictor service name");
DEFINE_string(model_path, "model_path", "path for the models");
DEFINE_string(example_snapshot_producer_config, "example_snapshot_producer.json",
              "path for the example_snapshot copy  producer config");
DEFINE_string(fea_extract_snapshot_producer_config, "fea_extract_snapshot_producer.json",
              "path for the fea_extract_snapshot copy  producer config");
DEFINE_string(qps, "0", "qa stress test qps");
DEFINE_string(qps_model, "all", "qa stress test model name");
DEFINE_string(trans_model, "", "trans one exist model to new model name");
DEFINE_int32(use_dynamic_weight, 0, "use dynamical server weights config func");
DEFINE_int32(use_model_service, 0, "whether to use model service");
DEFINE_int32(http_cpu_thread_num, 0, "size of http cpu thread pool");
DEFINE_int32(use_dynamic_unregister_service, 0, "use dynamical unregister service and unload model config");
DEFINE_int32(sleep_seconds_before_shutdown, 3, "how many seconds the process sleeps for before shutdown");
DEFINE_int32(thrift_io_thread_amplification_factor, 1, "amplification factor of thrift io thread number");
DEFINE_int32(start_thrift_server, 1, "start thrift server flag");
namespace {

constexpr char LOG_CATEGORY[] = "predictor_server.cc";
// base on the core number of aliyun machine, which has least cpu core number among all machines
constexpr unsigned DEFAULT_SERVER_WEIGHT = 16;

// a trick to use lambda which captures variables as the signal handler
std::function<void(int)> shutdown_handler;
void signal_handler(int signal) { shutdown_handler(signal); }

void cerr_and_flush_log(const std::string &component) {
  std::cerr << "Failed to init " << component << "! Exiting..." << std::endl;
  google::FlushLogFiles(google::INFO);
  google::FlushLogFiles(google::ERROR);
}
}  // namespace

int main(int argc, char** argv) {
  // FLAGS_logtostderr = 1;
  folly::init(&argc, &argv);

  LOG(INFO) << "predictor server started!";
  // hardware cpu core num, get once on start
  predictor::ResourceMgr::initCoreNum();

  // init model_manager
  auto model_manager = std::make_shared<predictor::ModelManager>();
  if (!model_manager->init(FLAGS_model_path)) {
    cerr_and_flush_log("model_manager");
    return -1;
  }
  // init predictor_service
  auto predictor_service = std::make_shared<predictor::PredictorService>(model_manager);
  if (!predictor_service->init()) {
    cerr_and_flush_log("predictor_service");
    return -1;
  }

  if (FLAGS_host == "") {
    std::string local_ip{""};
    common::getLocalIpAddress(&local_ip);
    if (local_ip.empty()) {
      std::cerr << "Failed to get local ip" << std::endl;
      return -1;
    }
    FLAGS_host = local_ip;
  }

  predictor::util::setDummyRegistry();
  // start http server
  auto http_service = std::make_shared<predictor::HttpService>(model_manager);
  if (!http_service->start(FLAGS_host, FLAGS_http_port)) {
    cerr_and_flush_log("http_service");
    return -1;
  }

  if (FLAGS_use_model_service) {
    shutdown_handler = [http_service](int signum) {
      LOG(INFO) << "caught signum=" << signum << ", exiting....";
      // destroy the HttpService object, which will unregister all service in its destructor
      google::FlushLogFiles(google::INFO);
      google::FlushLogFiles(google::ERROR);
      http_service->unregisterAll();
      LOG(INFO) << "sleep " << FLAGS_sleep_seconds_before_shutdown
                << " seconds for all clients to detect the death of this service";
      google::FlushLogFiles(google::INFO);
      std::this_thread::sleep_for(std::chrono::seconds(FLAGS_sleep_seconds_before_shutdown));

      LOG(INFO) << "re-fire the signal to trigger the shutdown of thrift server";
      signal(SIGINT, SIG_DFL);
      signal(SIGTERM, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      google::FlushLogFiles(google::INFO);
      google::FlushLogFiles(google::ERROR);
      raise(signum);
    };
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);

    // will not register service names
    // but wait for http requests to register service name on demand
    LOG(INFO) << "create thriftservice";
    service_router::ServerAddress address;
    address.setHost(FLAGS_host);
    address.setPort(FLAGS_port);
    service_router::ServerOption option;
    option.setServiceName(FLAGS_service_name);
    option.setServerAddress(address);
    while (FLAGS_start_thrift_server) {
      FLAGS_start_thrift_server = 0;
      LOG(INFO) << "create thrift server";
      predictor::ResourceMgr::createNewThriftServer();
      LOG(INFO) << "init thrift server";
      predictor::ResourceMgr::getThriftServer()->init<predictor::PredictorService>(option, predictor_service);
      LOG(INFO) << "ResourceMgr::getThriftServer()->configMutable():"
                << predictor::ResourceMgr::getThriftServer()->configMutable();
      LOG(INFO) << "set thrift server io thread number";
      predictor::ResourceMgr::getThriftServer()->setNumIOWorkerThreads(
        predictor::ResourceMgr::core_num_ * FLAGS_thrift_io_thread_amplification_factor);
      LOG(INFO) << "thrift_server.getNumIOWorkerThreads():"
                << predictor::ResourceMgr::getThriftServer()->getNumIOWorkerThreads();
      LOG(INFO) << "start thrift server";
      predictor::ResourceMgr::getThriftServer()->startServe(true);
    }
  } else {
    int server_weight = predictor::ResourceMgr::core_num_;
    LOG(INFO) << "server weight is " << server_weight;
    // start thrift service and register service names in one go
    auto exitSignalHandler = [](int signum) {
      LOG(INFO) << "signum=" << signum << ", unregistering thrift server";
      auto server = std::make_shared<service_router::Server>();
      server->setServiceName(FLAGS_service_name);
      server->setHost(FLAGS_host);
      server->setPort(FLAGS_port);
      service_router::unregisterServer(*server);
      LOG(INFO) << "re-fire the signal to trigger the shutdown of thrift server";
      signal(SIGINT, SIG_DFL);
      signal(SIGTERM, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      google::FlushLogFiles(google::INFO);
      google::FlushLogFiles(google::ERROR);
      raise(signum);
    };
    signal(SIGINT, exitSignalHandler);
    signal(SIGTERM, exitSignalHandler);
    signal(SIGQUIT, exitSignalHandler);
    service_router::thriftServiceServer<predictor::PredictorService>(
        FLAGS_service_name, FLAGS_host, FLAGS_port, predictor_service, service_router::defaultModifier(), 0,
        service_router::ServerStatus::AVAILABLE, [server_weight](const service_router::Server& server) {
          auto router = service_router::Router::getInstance();
          router->setOtherSettings(server, "qps", FLAGS_qps);
          router->setOtherSettings(server, "qps_model", FLAGS_qps_model);
          router->setOtherSettings(server, "trans_model", FLAGS_trans_model);

          // set server weight
          predictor::util::setServerStaticWeight(server.getServiceName(), server.getHost(), server.getPort(),
                                                 server_weight);
        });
  }

  SCOPE_EXIT {
    // We have to call ResourceMgr::clear before metrics::Metrics::getInstance()->stop because, somehow there is some
    // dependency here between the folly
    // cpu excecutor and folly eventbase. If metrics::Metrics::getInstance()->stop() is called before ResourceMgr
    // destructor, somehow the ResourceMgr destructor will be stuck. Need to figure out the exact details later.
    predictor::ResourceMgr::clear();
    metrics::Metrics::getInstance()->stop();
  };

  return 0;
}
