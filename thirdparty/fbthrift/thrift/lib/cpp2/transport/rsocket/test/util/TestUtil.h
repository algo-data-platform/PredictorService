/*
 * Copyright 2018-present Facebook, Inc.
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

#include <gflags/gflags.h>
#include <gmock/gmock.h>

#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp/async/TAsyncSocket.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RSocketClientChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeServerObserver.h>
#include <thrift/lib/cpp2/transport/core/testutil/MockCallback.h>
#include <thrift/lib/cpp2/transport/rsocket/YarplStreamImpl.h>
#include <thrift/lib/cpp2/transport/rsocket/server/RSRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rsocket/test/util/TestServiceMock.h>
#include <thrift/lib/cpp2/transport/util/ConnectionManager.h>
#include <yarpl/flowable/TestSubscriber.h>

namespace apache {
namespace thrift {

using namespace apache::thrift;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;
using namespace testing;
using namespace testutil::testservice;
using testutil::testservice::Message;
using yarpl::flowable::TestSubscriber;

// Event handler to attach to the Thrift server so we know when it is
// ready to serve and also so we can determine the port it is
// listening on.
class TestEventHandler : public server::TServerEventHandler {
 public:
  // This is a callback that is called when the Thrift server has
  // initialized and is ready to serve RPCs.
  void preServe(const folly::SocketAddress* address) override {
    port_ = address->getPort();
    baton_.post();
  }

  int32_t waitForPortAssignment() {
    baton_.wait();
    return port_;
  }

 private:
  folly::Baton<> baton_;
  int32_t port_;
};

class TestSetup : public testing::Test {
 protected:
  virtual std::unique_ptr<ThriftServer> createServer(
      std::shared_ptr<AsyncProcessorFactory>,
      uint16_t& port);

  std::unique_ptr<PooledRequestChannel, folly::DelayedDestruction::Destructor>
  connectToServer(
      uint16_t port,
      folly::Function<void()> onDetachable = nullptr);

 protected:
  std::shared_ptr<FakeServerObserver> observer_;

  int numIOThreads_{10};
  int numWorkerThreads_{10};

  folly::ScopedEventBaseThread evbThread_;
  folly::ScopedEventBaseThread executor_;
};

} // namespace thrift
} // namespace apache
