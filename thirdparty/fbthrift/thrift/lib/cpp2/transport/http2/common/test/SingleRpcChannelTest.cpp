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

#include <memory>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/transport/core/testutil/CoreTestFixture.h>
#include <thrift/lib/cpp2/transport/core/testutil/ServerConfigsMock.h>
#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>
#include <thrift/lib/cpp2/transport/http2/common/testutil/ChannelTestFixture.h>
#include <thrift/lib/cpp2/transport/http2/common/testutil/FakeProcessors.h>

namespace apache {
namespace thrift {

using folly::IOBuf;
using std::string;
using std::unordered_map;

class SingleRpcChannelTest
    : public ChannelTestFixture,
      public testing::WithParamInterface<string::size_type> {};

TEST_P(SingleRpcChannelTest, VaryingChunkSizes) {
  apache::thrift::server::ServerConfigsMock server;
  EchoProcessor processor(
      server, "extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> inputHeaders;
  inputHeaders["key1"] = "value1";
  inputHeaders["key2"] = "value2";
  string inputPayload = "single stream payload";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor,
      inputHeaders,
      inputPayload,
      GetParam(),
      outputHeaders,
      outputPayload);
  EXPECT_EQ(3, outputHeaders->size());
  EXPECT_EQ("value1", outputHeaders->at("key1"));
  EXPECT_EQ("value2", outputHeaders->at("key2"));
  EXPECT_EQ("extravalue", outputHeaders->at("extrakey"));
  EXPECT_EQ("single stream payload<eom>", toString(outputPayload));
}

INSTANTIATE_TEST_CASE_P(
    AllChunkSizes,
    SingleRpcChannelTest,
    testing::Values(0, 1, 2, 4, 10));

TEST_F(ChannelTestFixture, SingleRpcChannelErrorEmptyBody) {
  apache::thrift::server::ServerConfigsMock server;
  EchoProcessor processor(
      server, "extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> inputHeaders;
  inputHeaders["key1"] = "value1";
  string inputPayload = "";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor,
      inputHeaders,
      inputPayload,
      0,
      outputHeaders,
      outputPayload,
      true);
  EXPECT_EQ(0, outputHeaders->size());
  TApplicationException tae;
  EXPECT_TRUE(CoreTestFixture::deserializeException(outputPayload, &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN, tae.getType());
  EXPECT_EQ("Proxygen stream has no body", tae.getMessage());
}

TEST_F(ChannelTestFixture, SingleRpcChannelErrorNoEnvelope) {
  apache::thrift::server::ServerConfigsMock server;
  EchoProcessor processor(
      server, "extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> inputHeaders;
  inputHeaders["key1"] = "value1";
  string inputPayload = "notempty";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor,
      inputHeaders,
      inputPayload,
      0,
      outputHeaders,
      outputPayload,
      true);
  EXPECT_EQ(0, outputHeaders->size());
  TApplicationException tae;
  EXPECT_TRUE(CoreTestFixture::deserializeException(outputPayload, &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN, tae.getType());
  EXPECT_EQ("Invalid envelope: see logs for error", tae.getMessage());
}

} // namespace thrift
} // namespace apache
