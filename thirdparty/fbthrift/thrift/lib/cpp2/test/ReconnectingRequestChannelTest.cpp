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
#include <thrift/lib/cpp2/async/ReconnectingRequestChannel.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/ScopedBoundPort.h>
#include <thrift/lib/cpp/async/TAsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;
using namespace apache::thrift;
using namespace apache::thrift::test;
using apache::thrift::async::TAsyncSocket;
using apache::thrift::transport::TTransportException;

class TestServiceServerMock : public TestServiceSvIf {
 public:
  MOCK_METHOD1(echoInt, int32_t(int32_t));
};

class ReconnectingRequestChannelTest : public Test {
 public:
  folly::EventBase* eb{folly::EventBaseManager::get()->getEventBase()};
  folly::ScopedBoundPort bound;
  std::shared_ptr<TestServiceServerMock> handler{
      std::make_shared<TestServiceServerMock>()};
  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> runner{
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler)};

  folly::SocketAddress up_addr{runner->getAddress()};
  folly::SocketAddress dn_addr{bound.getAddress()};
};

TEST_F(ReconnectingRequestChannelTest, reconnect) {
  auto connection_count = 0;
  auto channel = ReconnectingRequestChannel::newChannel(
      *eb, [this, &connection_count](folly::EventBase& eb) mutable {
        connection_count++;
        return HeaderClientChannel::newChannel(
            TAsyncSocket::newSocket(&eb, up_addr));
      });

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, echoInt(_))
      .WillOnce(Return(1))
      .WillOnce(Return(3))
      .WillOnce(Return(4));
  EXPECT_EQ(client.sync_echoInt(1), 1);
  EXPECT_EQ(connection_count, 1);

  // bounce the server
  runner =
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler);
  up_addr = runner->getAddress();

  EXPECT_THROW(client.sync_echoInt(2), TTransportException);
  EXPECT_EQ(client.sync_echoInt(3), 3);
  EXPECT_EQ(connection_count, 2);
  EXPECT_EQ(client.sync_echoInt(4), 4);
  EXPECT_EQ(connection_count, 2);
}
