#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "folly/Synchronized.h"
#include "folly/futures/Future.h"
#include "folly/io/async/ScopedEventBaseThread.h"
#include "folly/SpinLock.h"
#include "folly/synchronization/Baton.h"
#include "folly/portability/GFlags.h"
#include "proxygen/lib/http/HTTPMessage.h"

#include "service_framework/http/http_connection.h"
#include "service_framework/http/http_response.h"

namespace service_framework {
namespace http {

class HttpClientManager {
 public:
  static std::shared_ptr<HttpClientManager> getInstance();

  HttpClientManager() = default;

  ~HttpClientManager();

  folly::Future<std::shared_ptr<HttpResponse>> call(const proxygen::HTTPMessage& request,
                                                    std::chrono::milliseconds connectTimeout,
                                                    std::chrono::milliseconds timeout);
  folly::Future<std::shared_ptr<HttpResponse>> call(const proxygen::HTTPMessage& request,
                                                    std::unique_ptr<folly::IOBuf> data,
                                                    std::chrono::milliseconds connectTimeout,
                                                    std::chrono::milliseconds timeout);
  void detach(HttpConnection* client);
  void stop();

 private:
  std::atomic<uint64_t> unique_conn_id_{0};
  uint64_t genUniqueConnId();
  void checkStop();
  folly::Synchronized<std::unordered_map<uint64_t, std::shared_ptr<HttpConnection>>> connections_;
  std::atomic<bool> ready_stop_{false};
  folly::SaturatingSemaphore<true> stop_;
  folly::SpinLock spinlock_;
};

class HttpOption {
 public:
  HttpOption();
  HttpOption(int connect_timeout, int socket_timeout);
  const std::chrono::milliseconds& getConnectTimeout() const { return connect_timeout_; }
  const std::chrono::milliseconds& getSocketTimeout() const { return socket_timeout_; }
  void setConnectTimeout(int connect_timeout) { connect_timeout_ = std::chrono::milliseconds(connect_timeout); }
  void setSocketTimeout(int socket_timeout) { socket_timeout_ = std::chrono::milliseconds(socket_timeout); }
  ~HttpOption() = default;

 private:
  std::chrono::milliseconds connect_timeout_;
  std::chrono::milliseconds socket_timeout_;
};

// HttpClientManager stop warp
void stop();

// GET warp
folly::Future<std::shared_ptr<HttpResponse>> makeGetRequest(const std::string& url);
folly::Future<std::shared_ptr<HttpResponse>> makeGetRequest(const std::string& url, const HttpOption& option);
folly::Future<std::shared_ptr<HttpResponse>> makeGetRequest(
    const std::string& url, const HttpOption& option, folly::Function<void(proxygen::HTTPMessage&)> request_modifier);

// DELETE warp
folly::Future<std::shared_ptr<HttpResponse>> makeDeleteRequest(const std::string& url);
folly::Future<std::shared_ptr<HttpResponse>> makeDeleteRequest(const std::string& url, const HttpOption& option);
folly::Future<std::shared_ptr<HttpResponse>> makeDeleteRequest(
    const std::string& url, const HttpOption& option, folly::Function<void(proxygen::HTTPMessage&)> request_modifier);

// POST warp
folly::Future<std::shared_ptr<HttpResponse>> makePostRequest(const std::string& url, const std::string& data);
folly::Future<std::shared_ptr<HttpResponse>> makePostRequest(const std::string& url, const std::string& data,
                                                             const HttpOption& option);
folly::Future<std::shared_ptr<HttpResponse>> makePostRequest(
    const std::string& url, const std::string& data, const HttpOption& option,
    folly::Function<void(proxygen::HTTPMessage&)> request_modifier);
folly::Future<std::shared_ptr<HttpResponse>> makePostRequest(const std::string& url,
                                                             std::unique_ptr<folly::IOBuf> data);
folly::Future<std::shared_ptr<HttpResponse>> makePostRequest(const std::string& url, std::unique_ptr<folly::IOBuf> data,
                                                             const HttpOption& option);
folly::Future<std::shared_ptr<HttpResponse>> makePostRequest(
    const std::string& url, std::unique_ptr<folly::IOBuf> data, const HttpOption& option,
    folly::Function<void(proxygen::HTTPMessage&)> request_modifier);
// PUT warp
folly::Future<std::shared_ptr<HttpResponse>> makePutRequest(const std::string& url, const std::string& data);
folly::Future<std::shared_ptr<HttpResponse>> makePutRequest(const std::string& url, const std::string& data,
                                                            const HttpOption& option);
folly::Future<std::shared_ptr<HttpResponse>> makePutRequest(
    const std::string& url, const std::string& data, const HttpOption& option,
    folly::Function<void(proxygen::HTTPMessage&)> request_modifier);
folly::Future<std::shared_ptr<HttpResponse>> makePutRequest(const std::string& url, std::unique_ptr<folly::IOBuf> data);
folly::Future<std::shared_ptr<HttpResponse>> makePutRequest(const std::string& url, std::unique_ptr<folly::IOBuf> data,
                                                            const HttpOption& option);
folly::Future<std::shared_ptr<HttpResponse>> makePutRequest(
    const std::string& url, std::unique_ptr<folly::IOBuf> data, const HttpOption& option,
    folly::Function<void(proxygen::HTTPMessage&)> request_modifier);
}  // namespace http
}  // namespace service_framework
