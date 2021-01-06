#include "service_router/thrift_server.h"

namespace service_router {

void ThriftServer::startServe(bool is_block) {
  if (is_block) {
    serve();
  } else {
    serve_thread_ = std::make_shared<std::thread>(
                      std::bind([this]() {
                        this->serve();
    }));
  }
  return;
}

void ThriftServer::stopServe() {
  stop();
  if (serve_thread_) {
    serve_thread_->join();
    serve_thread_ = nullptr;
  }
  return;
}

std::unique_ptr<apache::thrift::HTTP2RoutingHandler> ThriftServer::createHTTP2RoutingHandler() {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(getNumIOWorkerThreads());
  h2_options->idleTimeout = getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<apache::thrift::HTTP2RoutingHandler>(std::move(h2_options), getThriftProcessor(),
                                                               *this);
}

}  // namespace service_router
