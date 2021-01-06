#pragma once

#include "proxygen/lib/http/HTTPMessage.h"

namespace service_framework {
namespace http {

enum class HttpResponseError {
  HTTP_OK,
  HTTP_CONNECT_ERROR,
  HTTP_DNS_ERROR,
  HTTP_ERROR
};

class HttpResponse {
 public:
  using OptionalHttpMessage = folly::Optional<std::unique_ptr<proxygen::HTTPMessage>>;
  using OptionalBody = folly::Optional<std::unique_ptr<folly::IOBuf>>;

  HttpResponse() = default;
  ~HttpResponse() = default;

  void setMessage(std::unique_ptr<proxygen::HTTPMessage> message) { message_ = std::move(message); }

  void setBody(std::unique_ptr<folly::IOBuf> body) {
    const folly::IOBuf* p = body.get();
    do {
      body_.append(reinterpret_cast<const char*>(p->data()), p->length());
      p = p->next();
    } while (p != body.get());
  }

  void setError(const HttpResponseError& error) { error_ = error; }

  OptionalHttpMessage moveMessage() {
    if (message_ && isOk()) {
      return OptionalHttpMessage(std::move(message_));
    }
    return folly::none;
  }

  folly::Optional<folly::fbstring> moveToFbString() {
    if (isOk()) {
      return body_;
    }

    return folly::none;
  }

  const HttpResponseError& getError() const { return error_; }

  bool isOk() { return error_ == HttpResponseError::HTTP_OK; }

 private:
  HttpResponseError error_{HttpResponseError::HTTP_OK};
  folly::fbstring body_;
  std::unique_ptr<proxygen::HTTPMessage> message_;
};
}  // namespace http
}  // namespace service_framework
