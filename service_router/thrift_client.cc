// @ref fbthrift/thrift/lib/cpp2/transport/core/ThriftClient.cpp
#include "service_router/thrift_client.h"

#include "glog/logging.h"
#include "folly/io/async/Request.h"
#include "thrift/lib/cpp2/async/ResponseChannel.h"
#include "thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h"

namespace service_router {

DEFINE_string(router_thrift_transport, "header", "Transport to use: header, rsocket, http2");

class WaitableRequestCallback final : public apache::thrift::RequestCallback {
 public:
  WaitableRequestCallback(std::unique_ptr<apache::thrift::RequestCallback> cb, folly::Baton<>& baton,  // NOLINT
                          bool oneway)
      : cb_(std::move(cb)), baton_(baton), oneway_(oneway) {}

  void requestSent() override {
    cb_->requestSent();
    if (oneway_) {
      baton_.post();
    }
  }
  void replyReceived(apache::thrift::ClientReceiveState&& rs) override {
    DCHECK(!oneway_);
    cb_->replyReceived(std::move(rs));
    baton_.post();
  }

  void requestError(apache::thrift::ClientReceiveState&& rs) override {
    DCHECK(rs.isException());
    cb_->requestError(std::move(rs));
    baton_.post();
  }

 private:
  std::unique_ptr<apache::thrift::RequestCallback> cb_;
  folly::Baton<>& baton_;
  bool oneway_;
};

ThriftClient::ThriftClient(const std::shared_ptr<apache::thrift::ClientChannel>& channel, folly::EventBase* callbackEvb)
    : channel_(channel), callbackEvb_(callbackEvb) {}

ThriftClient::ThriftClient(const std::shared_ptr<apache::thrift::ClientChannel>& channel)
    : ThriftClient(channel, channel->getEventBase()) {}

ThriftClient::~ThriftClient() { setCloseCallback(nullptr); }

// 此方法必须要重写，因为默认实现时采用 event->loopForever 实现的等待，这样会出现一个 event 多次loop
uint32_t ThriftClient::sendRequestSync(apache::thrift::RpcOptions& rpcOptions,
                                       std::unique_ptr<apache::thrift::RequestCallback> cb,
                                       std::unique_ptr<apache::thrift::ContextStack> ctx,
                                       std::unique_ptr<folly::IOBuf> buf,
                                       std::shared_ptr<apache::thrift::transport::THeader> header) {
  DCHECK(typeid(apache::thrift::ClientSyncCallback) == typeid(*cb));

  DCHECK(!channel_->getEventBase()->inRunningEventBaseThread());
  folly::Baton<> baton;
  auto kind = static_cast<apache::thrift::ClientSyncCallback&>(*cb).rpcKind();
  bool oneway = kind == apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE;
  auto scb = std::make_unique<WaitableRequestCallback>(std::move(cb), baton, oneway);

  switch (kind) {
    case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE:
      sendOnewayRequest(rpcOptions, std::move(scb), std::move(ctx), std::move(buf), std::move(header));
      break;
    case apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
      sendRequest(rpcOptions, std::move(scb), std::move(ctx), std::move(buf), std::move(header));
      break;
    case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
      sendStreamRequest(rpcOptions, std::move(scb), std::move(ctx), std::move(buf), std::move(header));
      break;
    default:
      scb->requestError(apache::thrift::ClientReceiveState(
          folly::make_exception_wrapper<apache::thrift::transport::TTransportException>("Unsupported RpcKind value"),
          std::move(ctx), false));
      break;
  }

  baton.wait();
  return 0;
}

uint32_t ThriftClient::sendRequest(apache::thrift::RpcOptions& rpcOptions,
                                   std::unique_ptr<apache::thrift::RequestCallback> cb,
                                   std::unique_ptr<apache::thrift::ContextStack> ctx, std::unique_ptr<folly::IOBuf> buf,
                                   std::shared_ptr<apache::thrift::transport::THeader> header) {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([
    channel = std::move(channel),
    rpcOptions,
    cb = std::move(cb),
    ctx = std::move(ctx),
    buf = std::move(buf),
    header = std::move(header)
  ]() mutable { channel->sendRequest(rpcOptions, std::move(cb), std::move(ctx), std::move(buf), std::move(header)); });
  return 0;
}

uint32_t ThriftClient::sendOnewayRequest(apache::thrift::RpcOptions& rpcOptions,
                                         std::unique_ptr<apache::thrift::RequestCallback> cb,
                                         std::unique_ptr<apache::thrift::ContextStack> ctx,
                                         std::unique_ptr<folly::IOBuf> buf,
                                         std::shared_ptr<apache::thrift::transport::THeader> header) {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([
    channel = std::move(channel),
    rpcOptions,
    cb = std::move(cb),
    ctx = std::move(ctx),
    buf = std::move(buf),
    header = std::move(header)
  ]() mutable { channel->sendOnewayRequest(rpcOptions, std::move(cb), std::move(ctx), std::move(buf),
                                           std::move(header)); });
  return apache::thrift::ResponseChannel::ONEWAY_REQUEST_ID;
}

uint32_t ThriftClient::sendStreamRequest(apache::thrift::RpcOptions& rpcOptions,
                                         std::unique_ptr<apache::thrift::RequestCallback> cb,
                                         std::unique_ptr<apache::thrift::ContextStack> ctx,
                                         std::unique_ptr<folly::IOBuf> buf,
                                         std::shared_ptr<apache::thrift::transport::THeader> header) {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([
    channel = std::move(channel),
    rpcOptions,
    cb = std::move(cb),
    ctx = std::move(ctx),
    buf = std::move(buf),
    header = std::move(header)
  ]() mutable { channel->sendStreamRequest(rpcOptions, std::move(cb), std::move(ctx), std::move(buf),
                                           std::move(header)); });
  return 0;
}

folly::EventBase* ThriftClient::getEventBase() const { return channel_->getEventBase(); }

uint16_t ThriftClient::getProtocolId() { return channel_->getProtocolId(); }

void ThriftClient::setCloseCallback(apache::thrift::CloseCallback* cb) {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([
    channel = std::move(channel),
    cb
  ]() { channel->setCloseCallback(cb); });
}

apache::thrift::async::TAsyncTransport* ThriftClient::getTransport() { return channel_->getTransport(); }

bool ThriftClient::good() { return channel_->good(); }

apache::thrift::ClientChannel::SaturationStatus ThriftClient::getSaturationStatus() {
  return channel_->getSaturationStatus();
}

void ThriftClient::attachEventBase(folly::EventBase* eventBase) {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([
    channel = std::move(channel),
    eventBase
  ]() { channel->attachEventBase(eventBase); });
}

void ThriftClient::detachEventBase() {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([channel = std::move(channel)]() { channel->detachEventBase(); });
}

bool ThriftClient::isDetachable() { return channel_->isDetachable(); }

bool ThriftClient::isSecurityActive() { return channel_->isSecurityActive(); }

uint32_t ThriftClient::getTimeout() { return channel_->getTimeout(); }

void ThriftClient::setTimeout(uint32_t ms) {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([
    channel = std::move(channel),
    ms
  ]() { channel->setTimeout(ms); });
}

void ThriftClient::closeNow() {
  auto channel = channel_;
  channel_->getEventBase()->runInEventBaseThread([channel = std::move(channel)]() { channel->closeNow(); });
}

CLIENT_TYPE ThriftClient::getClientType() { return channel_->getClientType(); }

}  // namespace service_router
