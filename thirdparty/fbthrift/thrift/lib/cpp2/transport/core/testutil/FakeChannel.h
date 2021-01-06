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

#include <glog/logging.h>

#include <folly/Executor.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>

namespace apache {
namespace thrift {

/**
 * A simple channel that collects the response and makes it available
 * to test code.
 */
class FakeChannel : public ThriftChannelIf {
 public:
  explicit FakeChannel(folly::EventBase* evb) : evb_(getKeepAliveToken(evb)) {}
  ~FakeChannel() override = default;

  void sendThriftResponse(
      std::unique_ptr<ResponseRpcMetadata> metadata,
      std::unique_ptr<folly::IOBuf> payload) noexcept override {
    metadata_ = std::move(metadata);
    payload_ = std::move(payload);
    // Tests that use this class are expected to be done at this point.
    // So we shut down the event base.
    evb_.reset();
  }

  void sendThriftRequest(
      std::unique_ptr<RequestRpcMetadata> /*metadata*/,
      std::unique_ptr<folly::IOBuf> /*payload*/,
      std::unique_ptr<ThriftClientCallback> /*callback*/) noexcept override {
    LOG(FATAL) << "sendThriftRequest() unused in this fake object.";
  }

  void sendStreamThriftResponse(
      std::unique_ptr<ResponseRpcMetadata> /*metadata*/,
      std::unique_ptr<folly::IOBuf> /*response*/,
      apache::thrift::SemiStream<
          std::unique_ptr<folly::IOBuf>> /*stream*/) noexcept override {
    LOG(FATAL) << "sendStreamThriftResponse() unused in this fake object.";
  }

  folly::EventBase* getEventBase() noexcept override {
    return evb_.get();
  }

  ResponseRpcMetadata* getMetadata() {
    return metadata_.get();
  }

  folly::IOBuf* getPayloadBuf() {
    return payload_.get();
  }

 private:
  std::unique_ptr<ResponseRpcMetadata> metadata_;
  std::unique_ptr<folly::IOBuf> payload_;
  folly::Executor::KeepAlive<folly::EventBase> evb_;
};

} // namespace thrift
} // namespace apache
