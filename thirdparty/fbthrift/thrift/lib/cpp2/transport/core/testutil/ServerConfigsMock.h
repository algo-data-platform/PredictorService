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

#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeServerObserver.h>

namespace apache {
namespace thrift {
namespace server {

// Use instance of this class, instead of ThriftServer, in the unit tests of
// ThriftProcessor.
class ServerConfigsMock : public ServerConfigs {
 public:
  uint64_t getMaxResponseSize() const {
    return maxResponseSize_;
  }

  /**
   * @see BaseThriftServer::getTaskExpireTimeForRequest function.
   */
  bool getTaskExpireTimeForRequest(
      std::chrono::milliseconds,
      std::chrono::milliseconds,
      std::chrono::milliseconds& queueTimeout,
      std::chrono::milliseconds& taskTimeout) const override {
    queueTimeout = queueTimeout_;
    taskTimeout = taskTimeout_;
    return queueTimeout == taskTimeout;
  }

  const std::shared_ptr<apache::thrift::server::TServerObserver>& getObserver()
      const override {
    return observer_;
  }

  void setNumIOWorkerThreads(size_t numIOWorkerThreads) override {
    numIOWorkerThreads_ = numIOWorkerThreads;
  }

  size_t getNumIOWorkerThreads() const override {
    return numIOWorkerThreads_;
  }

  std::chrono::milliseconds getStreamExpireTime() const override {
    return streamExpireTime_;
  }

 public:
  uint64_t maxResponseSize_{0};
  std::chrono::milliseconds queueTimeout_{std::chrono::milliseconds(500)};
  std::chrono::milliseconds taskTimeout_{std::chrono::milliseconds(500)};
  std::shared_ptr<apache::thrift::server::TServerObserver> observer_{
      std::make_shared<FakeServerObserver>()};
  size_t numIOWorkerThreads_{10};
  std::chrono::milliseconds streamExpireTime_{std::chrono::minutes(1)};
};

} // namespace server
} // namespace thrift
} // namespace apache
