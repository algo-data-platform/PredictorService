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

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/transport/http2/common/H2Channel.h>

namespace apache {
namespace thrift {

class MultiRpcChannel : public H2Channel {
 public:
  MultiRpcChannel(
      proxygen::ResponseHandler* toHttp2,
      ThriftProcessor* processor);

  explicit MultiRpcChannel(H2ClientConnection* toHttp2);

  virtual ~MultiRpcChannel() override;

  // Initializes the client side channel by creating a transaction and
  // calling sendHeaders().  Cannot be performed in the constructor.
  void initialize(std::chrono::milliseconds timeout);

  void sendThriftResponse(
      std::unique_ptr<ResponseRpcMetadata> metadata,
      std::unique_ptr<folly::IOBuf> payload) noexcept override;

  virtual void sendThriftRequest(
      std::unique_ptr<RequestRpcMetadata> metadata,
      std::unique_ptr<folly::IOBuf> payload,
      std::unique_ptr<ThriftClientCallback> callback) noexcept override;

  folly::EventBase* getEventBase() noexcept override;

  void onH2StreamBegin(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onH2BodyFrame(std::unique_ptr<folly::IOBuf> contents) noexcept override;

  void onH2StreamEnd() noexcept override;

  void onH2StreamClosed(proxygen::ProxygenError) noexcept override;

  // Returns true if this channel can process more RPCs.
  bool canDoRpcs() noexcept;

  // Closes the outgoing stream on the client side by calling sendEOM().
  // If we force close, channel is marked as closed and sendEOM is not sent.
  void closeClientSide(bool forceClose = false) noexcept;

  bool hasOutstandingRPCs();

 private:
  // Server side handling when we have a complete Thrift request.
  virtual void onThriftRequest() noexcept;

  // Client side handling when we have a complete Thrift response.
  void onThriftResponse() noexcept;

  // Combines the three arguments into a single IOBuf chain.
  std::unique_ptr<folly::IOBuf> combine(
      uint32_t size,
      std::unique_ptr<folly::IOBuf>&& metadata,
      std::unique_ptr<folly::IOBuf>&& payload) noexcept;

  // Add the IOBuf object to payload_.
  void addToPayload(std::unique_ptr<folly::IOBuf> buf) noexcept;

  void performErrorCallbacks(proxygen::ProxygenError error) noexcept;

  // Called from onThriftRequest() to send an error response.
  void sendThriftErrorResponse(
      const std::string& message,
      ProtocolId protoId = ProtocolId::COMPACT) noexcept;

  // The thrift processor used to execute RPCs (server side only).
  // Owned by H2ThriftServer.
  ThriftProcessor* processor_{nullptr};

  // Event base on which all methods in this object must be invoked.
  folly::EventBase* evb_;

  // Transaction object for use on client side to communicate with the
  // Proxygen layer.
  proxygen::HTTPTransaction* httpTransaction_;

  // Callbacks for client side indexed by sequence id.
  std::vector<std::unique_ptr<ThriftClientCallback>> callbacks_;

  // The data from Proxygen for each RPC may arrive via multiple calls
  // to onH2BodyFrame(), and also the same call to onH2BodyFrame() may
  // contain data for multiple RPCs.  Therefore, we need to maintain
  // the current state of the data in the fields below.

  // The payload for each RPC starts with a 4 byte size field.  This
  // specifies how many size bytes are yet to be read for the current
  // RPC.
  uint32_t sizeBytesRemaining_{4};

  // Once the size bytes have been read, we need to read an additional
  // number of bytes as specified in the size bytes.  This specifies
  // how many bytes remain to be read.
  uint32_t payloadBytesRemaining_{0};

  // The part of the RPC payload that has been read so far.
  std::unique_ptr<folly::IOBuf> payload_;

  // If true, no more rpcs may be initiated.
  bool isClosed_{false};

  uint32_t rpcsInitiated_{0};
  uint32_t rpcsCompleted_{0};
};

} // namespace thrift
} // namespace apache
