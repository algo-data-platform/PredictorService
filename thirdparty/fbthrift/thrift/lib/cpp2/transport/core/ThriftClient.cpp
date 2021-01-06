/*
 * Copyright 2017-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/core/ThriftClient.h>

#include <glog/logging.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/Request.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

using apache::thrift::async::TAsyncTransport;
using apache::thrift::protocol::PROTOCOL_TYPES;
using apache::thrift::transport::THeader;
using apache::thrift::transport::TTransportException;
using folly::EventBase;
using folly::IOBuf;
using folly::RequestContext;

namespace {

/**
 * Used as the callback for sendRequestSync.  It delegates to the
 * wrapped callback and posts on the baton object to allow the
 * synchronous call to continue.
 */
class WaitableRequestCallback final : public RequestCallback {
 public:
  WaitableRequestCallback(
      std::unique_ptr<RequestCallback> cb,
      folly::Baton<>& baton,
      bool oneway)
      : cb_(std::move(cb)), baton_(baton), oneway_(oneway) {}

  void requestSent() override {
    cb_->requestSent();
    if (oneway_) {
      baton_.post();
    }
  }

  void replyReceived(ClientReceiveState&& rs) override {
    DCHECK(!oneway_);
    cb_->replyReceived(std::move(rs));
    baton_.post();
  }

  void requestError(ClientReceiveState&& rs) override {
    DCHECK(rs.isException());
    cb_->requestError(std::move(rs));
    baton_.post();
  }

 private:
  std::unique_ptr<RequestCallback> cb_;
  folly::Baton<>& baton_;
  bool oneway_;
};

} // namespace

const std::chrono::milliseconds ThriftClient::kDefaultRpcTimeout =
    std::chrono::milliseconds(500);

ThriftClient::ThriftClient(
    std::shared_ptr<ClientConnectionIf> connection,
    folly::EventBase* callbackEvb)
    : connection_(connection), callbackEvb_(callbackEvb) {}

ThriftClient::ThriftClient(std::shared_ptr<ClientConnectionIf> connection)
    : ThriftClient(connection, connection->getEventBase()) {}

ThriftClient::~ThriftClient() {
  connection_->setCloseCallback(this, nullptr);
}

void ThriftClient::setProtocolId(uint16_t protocolId) {
  protocolId_ = protocolId;
}

void ThriftClient::setHTTPHost(const std::string& host) {
  httpHost_ = host;
}

void ThriftClient::setHTTPUrl(const std::string& url) {
  httpUrl_ = url;
}

uint32_t ThriftClient::sendRequestSync(
    RpcOptions& rpcOptions,
    std::unique_ptr<RequestCallback> cb,
    std::unique_ptr<ContextStack> ctx,
    std::unique_ptr<IOBuf> buf,
    std::shared_ptr<THeader> header) {
  // Synchronous calls may be made from any thread except the one used
  // by the underlying connection.  That thread is used to handle the
  // callback and to release this thread that will be waiting on
  // "baton".
  EventBase* connectionEvb = connection_->getEventBase();
  DCHECK(!connectionEvb->inRunningEventBaseThread());
  folly::Baton<> baton;
  DCHECK(typeid(ClientSyncCallback) == typeid(*cb));
  auto kind = static_cast<ClientSyncCallback&>(*cb).rpcKind();
  bool oneway = kind == RpcKind::SINGLE_REQUEST_NO_RESPONSE;
  auto scb =
      std::make_unique<WaitableRequestCallback>(std::move(cb), baton, oneway);
  int result = sendRequestHelper(
      rpcOptions,
      kind,
      std::move(scb),
      std::move(ctx),
      std::move(buf),
      std::move(header),
      connectionEvb);
  baton.wait();
  return result;
}

uint32_t ThriftClient::sendRequest(
    RpcOptions& rpcOptions,
    std::unique_ptr<RequestCallback> cb,
    std::unique_ptr<ContextStack> ctx,
    std::unique_ptr<IOBuf> buf,
    std::shared_ptr<THeader> header) {
  return sendRequestHelper(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      std::move(cb),
      std::move(ctx),
      std::move(buf),
      std::move(header),
      callbackEvb_);
}

uint32_t ThriftClient::sendOnewayRequest(
    RpcOptions& rpcOptions,
    std::unique_ptr<RequestCallback> cb,
    std::unique_ptr<ContextStack> ctx,
    std::unique_ptr<IOBuf> buf,
    std::shared_ptr<THeader> header) {
  sendRequestHelper(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      std::move(cb),
      std::move(ctx),
      std::move(buf),
      std::move(header),
      callbackEvb_);
  return ResponseChannel::ONEWAY_REQUEST_ID;
}

uint32_t ThriftClient::sendStreamRequest(
    RpcOptions& rpcOptions,
    std::unique_ptr<RequestCallback> cb,
    std::unique_ptr<apache::thrift::ContextStack> ctx,
    std::unique_ptr<folly::IOBuf> buf,
    std::shared_ptr<apache::thrift::transport::THeader> header) {
  return sendRequestHelper(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE,
      std::move(cb),
      std::move(ctx),
      std::move(buf),
      std::move(header),
      callbackEvb_);
}

std::unique_ptr<RequestRpcMetadata> ThriftClient::createRequestRpcMetadata(
    RpcOptions& rpcOptions,
    RpcKind kind,
    apache::thrift::ProtocolId protocolId,
    THeader* header) {
  auto metadata = std::make_unique<RequestRpcMetadata>();
  metadata->protocol = protocolId;
  metadata->__isset.protocol = true;
  metadata->kind = kind;
  metadata->__isset.kind = true;
  if (!httpHost_.empty()) {
    metadata->host = httpHost_;
    metadata->__isset.host = true;
  }
  if (!httpUrl_.empty()) {
    metadata->url = httpUrl_;
    metadata->__isset.url = true;
  }
  if (rpcOptions.getTimeout() > std::chrono::milliseconds(0)) {
    metadata->clientTimeoutMs = rpcOptions.getTimeout().count();
  } else {
    metadata->clientTimeoutMs = kDefaultRpcTimeout.count();
  }
  metadata->__isset.clientTimeoutMs = true;
  if (rpcOptions.getQueueTimeout() > std::chrono::milliseconds(0)) {
    metadata->queueTimeoutMs = rpcOptions.getQueueTimeout().count();
    metadata->__isset.queueTimeoutMs = true;
  }
  if (rpcOptions.getPriority() < concurrency::N_PRIORITIES) {
    metadata->priority = static_cast<RpcPriority>(rpcOptions.getPriority());
    metadata->__isset.priority = true;
  }
  metadata->otherMetadata = header->releaseWriteHeaders();
  auto* eh = header->getExtraWriteHeaders();
  if (eh) {
    metadata->otherMetadata.insert(eh->begin(), eh->end());
  }
  auto& pwh = getPersistentWriteHeaders();
  metadata->otherMetadata.insert(pwh.begin(), pwh.end());
  if (!metadata->otherMetadata.empty()) {
    metadata->__isset.otherMetadata = true;
  }
  return metadata;
}

uint32_t ThriftClient::sendRequestHelper(
    RpcOptions& rpcOptions,
    RpcKind kind,
    std::unique_ptr<RequestCallback> cb,
    std::unique_ptr<ContextStack> ctx,
    std::unique_ptr<IOBuf> buf,
    std::shared_ptr<THeader> header,
    EventBase* callbackEvb) noexcept {
  DestructorGuard dg(this);
  cb->context_ = RequestContext::saveContext();
  auto metadata = createRequestRpcMetadata(
      rpcOptions,
      kind,
      static_cast<apache::thrift::ProtocolId>(protocolId_),
      header.get());
  auto callback = std::make_unique<ThriftClientCallback>(
      callbackEvb,
      std::move(cb),
      std::move(ctx),
      isSecurityActive(),
      protocolId_,
      std::chrono::milliseconds(metadata->clientTimeoutMs));
  auto conn = connection_;
  connection_->getEventBase()->runInEventBaseThread([conn = std::move(conn),
                                                     metadata =
                                                         std::move(metadata),
                                                     buf = std::move(buf),
                                                     callback = std::move(
                                                         callback)]() mutable {
    getChannelAndSendThriftRequest(
        conn.get(), std::move(metadata), std::move(buf), std::move(callback));
  });
  return 0;
}

void ThriftClient::getChannelAndSendThriftRequest(
    ClientConnectionIf* connection,
    std::unique_ptr<RequestRpcMetadata> metadata,
    std::unique_ptr<IOBuf> payload,
    std::unique_ptr<ThriftClientCallback> callback) noexcept {
  DCHECK(connection->getEventBase()->isInEventBaseThread());
  try {
    auto channel = connection->getChannel(metadata.get());
    channel->sendThriftRequest(
        std::move(metadata), std::move(payload), std::move(callback));
  } catch (TTransportException& te) {
    // sendThriftRequest is noexcept, so callback will never be nullptr in here
    auto callbackEvb = callback->getEventBase();
    callbackEvb->runInEventBaseThread([callback = std::move(callback),
                                       te = std::move(te)]() mutable {
      callback->onError(
          folly::make_exception_wrapper<TTransportException>(std::move(te)));
    });
  }
}

EventBase* ThriftClient::getEventBase() const {
  return connection_->getEventBase();
}

uint16_t ThriftClient::getProtocolId() {
  return protocolId_;
}

void ThriftClient::setCloseCallback(CloseCallback* cb) {
  connection_->setCloseCallback(this, cb);
}

TAsyncTransport* ThriftClient::getTransport() {
  return connection_->getTransport();
}

bool ThriftClient::good() {
  return connection_->good();
}

ClientChannel::SaturationStatus ThriftClient::getSaturationStatus() {
  return connection_->getSaturationStatus();
}

void ThriftClient::attachEventBase(folly::EventBase* eventBase) {
  connection_->attachEventBase(eventBase);
}

void ThriftClient::detachEventBase() {
  connection_->detachEventBase();
}

bool ThriftClient::isDetachable() {
  return connection_->isDetachable();
}

bool ThriftClient::isSecurityActive() {
  return connection_->isSecurityActive();
}

uint32_t ThriftClient::getTimeout() {
  return connection_->getTimeout();
}

void ThriftClient::setTimeout(uint32_t ms) {
  connection_->setTimeout(ms);
}

void ThriftClient::closeNow() {
  connection_->closeNow();
}

CLIENT_TYPE ThriftClient::getClientType() {
  return connection_->getClientType();
}

} // namespace thrift
} // namespace apache
