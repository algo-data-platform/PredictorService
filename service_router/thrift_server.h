#pragma once

#include "thrift/lib/cpp2/server/ThriftServer.h"
#include "service_router/router.h"
#include "thrift/lib/cpp2/transport/core/ThriftProcessor.h"
#include "thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h"
#include "thrift/lib/cpp2/transport/rsocket/server/RSRoutingHandler.h"

namespace service_router {

class ThriftServer : public apache::thrift::ThriftServer {
 public:
  ThriftServer() {}
  ~ThriftServer() { stopServe(); }
  ThriftServer(const ThriftServer&) = delete;
  ThriftServer& operator=(const ThriftServer&) = delete;

  template<typename ServiceHandler>
  void init(const ServerOption& option, std::shared_ptr<ServiceHandler> handler);
  void startServe(bool is_block = false);
  void stopServe();
 private:
  std::unique_ptr<apache::thrift::HTTP2RoutingHandler> createHTTP2RoutingHandler();
 private:
  std::shared_ptr<std::thread> serve_thread_;
};

template <typename ServiceHandler>
void ThriftServer::init(const ServerOption& option, std::shared_ptr<ServiceHandler> handler) {
  auto proc_factory = std::make_shared<apache::thrift::ThriftServerAsyncProcessorFactory<ServiceHandler>>(handler);
  setCPUWorkerThreadName("thrift_cpu_worker");
  setIdleTimeout(std::chrono::milliseconds(FLAGS_idle_timeout));
  setAddress(option.getServerAddress().getHost(), option.getServerAddress().getPort());
  setProcessorFactory(proc_factory);
  addRoutingHandler(std::make_unique<apache::thrift::RSRoutingHandler>());
  addRoutingHandler(createHTTP2RoutingHandler());
  setReusePort(option.getReusePort());
  return;
}

}  // namespace service_router
