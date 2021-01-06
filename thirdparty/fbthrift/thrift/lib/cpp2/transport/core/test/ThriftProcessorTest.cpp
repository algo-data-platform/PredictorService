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

#include <thrift/lib/cpp2/transport/core/ThriftProcessor.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/transport/core/testutil/CoreTestFixture.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeChannel.h>
#include <thrift/lib/cpp2/transport/core/testutil/TestServiceMock.h>

namespace apache {
namespace thrift {

using namespace testing;
using namespace testutil::testservice;

TEST_F(CoreTestFixture, SumTwoNumbers) {
  int32_t x = 5;
  int32_t y = 10;
  int32_t expected_result = x + y;

  EXPECT_CALL(service_, sumTwoNumbers_(x, y));

  runInEventBaseThread([&]() mutable {
    auto metadata = std::make_unique<RequestRpcMetadata>();
    folly::IOBufQueue request;
    serializeSumTwoNumbers(x, y, false, &request, metadata.get());
    auto channel = std::shared_ptr<ThriftChannelIf>(channel_);
    processor_.onThriftRequest(std::move(metadata), request.move(), channel);
  });

  // Receive Response and compare result
  auto result = deserializeSumTwoNumbers(channel_->getPayloadBuf());
  EXPECT_EQ(expected_result, result);
}

TEST_F(CoreTestFixture, BadName) {
  runInEventBaseThread([&]() mutable {
    auto metadata = makeMetadata("foobar");
    auto payload = folly::IOBuf::copyBuffer("dummy payload");
    auto channel = std::shared_ptr<ThriftChannelIf>(channel_);
    processor_.onThriftRequest(
        std::move(metadata), std::move(payload), channel);
  });

  TApplicationException tae;
  EXPECT_TRUE(deserializeException(channel_->getPayloadBuf(), &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN_METHOD, tae.getType());
}

TEST_F(CoreTestFixture, BadPayload) {
  runInEventBaseThread([&]() mutable {
    // Set function name to some valid function in ThriftService.thrift
    auto metadata = makeMetadata("headers");
    auto payload = folly::IOBuf::copyBuffer("bad payload");
    auto channel = std::shared_ptr<ThriftChannelIf>(channel_);
    processor_.onThriftRequest(
        std::move(metadata), std::move(payload), channel);
  });

  TApplicationException tae;
  EXPECT_TRUE(deserializeException(channel_->getPayloadBuf(), &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN, tae.getType());
}

TEST_F(CoreTestFixture, BadMetadata) {
  runInEventBaseThread([&]() mutable {
    auto metadata = makeMetadata("headers");
    folly::IOBufQueue request;
    serializeSumTwoNumbers(5, 10, false, &request, metadata.get());
    auto channel = std::shared_ptr<ThriftChannelIf>(channel_);

    metadata->__isset.kind = false; // make sure there is an error

    processor_.onThriftRequest(std::move(metadata), request.move(), channel);
  });

  TApplicationException tae;
  EXPECT_TRUE(deserializeException(channel_->getPayloadBuf(), &tae));
  EXPECT_EQ(TApplicationException::UNSUPPORTED_CLIENT_TYPE, tae.getType());
}

// Forces calling sendErrorWrapped()
TEST_F(CoreTestFixture, SendErrorWrapped) {
  threadManager_->setThrowOnAdd(true);
  runInEventBaseThread([&]() mutable {
    auto metadata = std::make_unique<RequestRpcMetadata>();
    folly::IOBufQueue request;
    serializeSumTwoNumbers(5, 10, false, &request, metadata.get());
    auto channel = std::shared_ptr<ThriftChannelIf>(channel_);
    processor_.onThriftRequest(std::move(metadata), request.move(), channel);
  });

  ResponseRpcMetadata* metadata = channel_->getMetadata();
  auto iter = metadata->otherMetadata.find("ex");
  EXPECT_NE(metadata->otherMetadata.end(), iter);
  EXPECT_EQ(kQueueOverloadedErrorCode, iter->second);

  TApplicationException tae;
  EXPECT_TRUE(deserializeException(channel_->getPayloadBuf(), &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN, tae.getType());
}

} // namespace thrift
} // namespace apache
