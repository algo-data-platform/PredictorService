#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <vector>
#include <thread>  // NOLINT
#include <iostream>

// noinline
#ifdef _MSC_VER
#define AD_NOINLINE __declspec(noinline)
#elif defined(__clang__) || defined(__GNUC__)
#define AD_NOINLINE __attribute__((__noinline__))
#else
#define AD_NOINLINE
#endif

namespace folly {
template <class T>
class Future;
template <class T>
class Function;
template <class T>
class Try;
}

namespace proxygen {
class HTTPServer;
}

namespace service_router {
enum class LoadBalanceMethod;
}

namespace predictor {

class Init {
 public:
  // Force ctor & dtor out of line for better stack traces even with LTO.
  AD_NOINLINE Init(const std::string& app_name, const std::string& args);
  AD_NOINLINE Init(const std::string& app_name, const std::vector<std::string>& args);
  // 必须在进程结束的时候显式调用，否则会出现析构乱序的 coredump 问题
  AD_NOINLINE void stop();
  AD_NOINLINE ~Init() = default;

 private:
  std::shared_ptr<proxygen::HTTPServer> http_server_;
  std::shared_ptr<std::thread> http_server_thread_;
  AD_NOINLINE void initHttpServer(const std::string& app_name);
};

}  // namespace predictor
