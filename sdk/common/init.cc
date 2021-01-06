#include "folly/init/Init.h"
#include "folly/logging/Init.h"
#include "folly/portability/Config.h"
#include "folly/Singleton.h"
#include "folly/executors/GlobalExecutor.h"
#include "service_router/http.h"
#include "service_router/connection_pool.h"
#include "common/metrics/metrics.h"

#include "sdk/common/init.h"

DEFINE_string(host, "127.0.0.1", "Application run host ip");
DEFINE_int32(http_port, 0, "Application http server port, default 0");
DECLARE_string(logging);

namespace predictor {

static void initFolly(const std::string& app_name, const std::vector<std::string>& args) {
  std::vector<std::string> params = args;
  params.insert(params.begin(), app_name);
  char** argv = new char* [params.size()];
  for (int i = 0; i < params.size(); i++) {
    argv[i] = new char[params[i].size() + 1];
    memcpy(argv[i], params[i].data(), params[i].size());
    argv[i][params[i].size()] = '\0';
    LOG(INFO) << "Init ads core sdk params:" << argv[i];
  }

  int argc = params.size();
  // folly::init(&argc, &argv, false);
  // copied from folly::init, with installFatalSignalHandler removed
  int* argc_ptr = &argc;
  char*** argv_ptr = &argv;
  folly::SingletonVault::singleton()->registrationComplete();
  gflags::ParseCommandLineFlags(argc_ptr, argv_ptr, false);
  folly::initLoggingOrDie(FLAGS_logging);
  auto programName = argc_ptr && argv_ptr && *argc_ptr > 0 ? (*argv_ptr)[0] : "unknown";
  google::InitGoogleLogging(programName);

  // Somehow if we delete argv here, the glog behavior will be messed up. I guess that's because normally the life cycle
  // of argv is assumed to be as long as the main function. So let's just leave it there.
  LOG(INFO) << "folly::init done!";
  // 由于静态变量析构的顺序问题，所以需要所有的单例启动是创建出来，保证在 stop 后析构
  folly::SingletonVault::singleton()->doEagerInit();
  // 公共线程池需要提前创建出来，防止析构顺序不一致
  folly::getIOExecutor();
  folly::getCPUExecutor();
}

Init::Init(const std::string& app_name, const std::string& args_str) {
  std::vector<std::string> args;
  folly::split(" ", args_str, args, true);
  initFolly(app_name, args);
  initHttpServer(app_name);
}

Init::Init(const std::string& app_name, const std::vector<std::string>& args) {
  initFolly(app_name, args);
  initHttpServer(app_name);
}

void Init::initHttpServer(const std::string& app_name) {
  service_router::ServerAddress address;
  address.setHost(FLAGS_host);
  address.setPort(FLAGS_http_port);
  service_router::ServerOption server_option;
  server_option.setServiceName(app_name);
  server_option.setServerAddress(address);

  http_server_thread_ = std::make_shared<std::thread>([this, server_option]() {
    this->http_server_ = service_router::createAndRegisterHttpServer(server_option);
    this->http_server_->start();
  });
  LOG(INFO) << "initHttpServer done ";
  google::FlushLogFiles(google::INFO);
  google::FlushLogFiles(google::ERROR);
}

void Init::stop() {
  LOG(ERROR) << "stop sdk init env";
  // sleep 50ms 为了防止 http server 没来及的启动导致 启动线程 this 空悬
  // 为了 Init 类通用没有办法采用 weak_ptr 方式，另外可以使用线程同步的方式解决这个问题，
  // 不过太过麻烦，这块只有进程 stop 的时候才会调用
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  http_server_->stop();
  http_server_.reset();
  http_server_thread_->join();
  service_router::unregisterAll();
  service_router::stop();
  service_framework::http::stop();
  metrics::Metrics::getInstance()->stop();
  folly::SingletonVault::singleton()->destroyInstances();
}
}  // namespace predictor
