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

#include <future>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {

// Simple RequestChannel wrapper, which automatically retries requests if they
// fail with a TTransportException.
class RetryingRequestChannel : public apache::thrift::RequestChannel {
 public:
  using Impl = apache::thrift::RequestChannel;
  using ImplPtr = std::shared_ptr<Impl>;
  using UniquePtr = std::
      unique_ptr<RetryingRequestChannel, folly::DelayedDestruction::Destructor>;

  static UniquePtr
  newChannel(folly::EventBase& evb, int numRetries, ImplPtr impl) {
    return {new RetryingRequestChannel(evb, numRetries, std::move(impl)), {}};
  }

  uint32_t sendRequest(
      apache::thrift::RpcOptions& options,
      std::unique_ptr<apache::thrift::RequestCallback> cob,
      std::unique_ptr<apache::thrift::ContextStack> ctx,
      std::unique_ptr<folly::IOBuf> buf,
      std::shared_ptr<apache::thrift::transport::THeader> header) override;

  uint32_t sendOnewayRequest(
      apache::thrift::RpcOptions&,
      std::unique_ptr<apache::thrift::RequestCallback>,
      std::unique_ptr<apache::thrift::ContextStack>,
      std::unique_ptr<folly::IOBuf>,
      std::shared_ptr<apache::thrift::transport::THeader>) override {
    LOG(FATAL) << "Not supported";
  }

  void setCloseCallback(apache::thrift::CloseCallback*) override {
    LOG(FATAL) << "Not supported";
  }

  folly::EventBase* getEventBase() const override {
    return &evb_;
  }

  uint16_t getProtocolId() override {
    return impl_->getProtocolId();
  }

 protected:
  ~RetryingRequestChannel() override = default;

  RetryingRequestChannel(folly::EventBase& evb, int numRetries, ImplPtr impl)
      : impl_(std::move(impl)), numRetries_(numRetries), evb_(evb) {}

  class RequestCallback;

  ImplPtr impl_;
  int numRetries_;
  folly::EventBase& evb_;
};
} // namespace thrift
} // namespace apache
