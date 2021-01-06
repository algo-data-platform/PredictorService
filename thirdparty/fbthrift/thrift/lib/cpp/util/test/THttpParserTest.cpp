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

#include <thrift/lib/cpp/util/THttpParser.h>
#include <folly/Format.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

class THttpClientParserTest : public testing::Test{};

using HeaderMap = std::map<std::string, std::string>;

void write(
    apache::thrift::util::THttpParser& parser,
    folly::StringPiece text) {
  while (!text.empty()) {
    void* buf = nullptr;
    size_t len = 0;
    parser.getReadBuffer(&buf, &len);
    len = std::min(len, text.size());
    std::memcpy(buf, text.data(), len);
    text.advance(len);
    parser.readDataAvailable(len);
  }
}
} // namespace

TEST_F(THttpClientParserTest, too_many_headers) {
  apache::thrift::util::THttpClientParser parser;
  std::map<std::string, std::string> header;
  for (int i = 0; i < 100; i++) {
    header[folly::sformat("testing_header{}", i)] = "test_header";
  }
  std::map<std::string, std::string> header_persistent;
  auto answer = std::string("{'testing': 'this is a test'}");
  auto pre = folly::IOBuf::copyBuffer(answer.c_str(), answer.size());
  auto buf = parser.constructHeader(
      std::move(pre), header, header_persistent, nullptr);
  auto fbs = buf->moveToFbString();
  std::string output = std::string(fbs.c_str(), fbs.size());
  for (int i = 0; i < 100; i++) {
    std::string s = folly::sformat("testing_header{}: test_header\r\n", i);
    EXPECT_THAT(output, ::testing::HasSubstr(s));
  }
  std::vector<std::string> o;
  folly::split("\r\n", output, o);
  if (o.at(o.size() - 1) != answer) {
    FAIL() << folly::sformat(
        "Final line should be {} not {}", answer, o.at(o.size() - 1));
  }
}

TEST_F(THttpClientParserTest, read_encapsulated_status_line) {
  apache::thrift::util::THttpClientParser parser;
  write(parser, "HTTP/1.1 200 OK\r\n");
  SUCCEED() << "did not crash";
}
