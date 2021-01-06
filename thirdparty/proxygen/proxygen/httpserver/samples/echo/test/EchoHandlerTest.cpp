/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/httpserver/Mocks.h>
#include <proxygen/httpserver/samples/echo/EchoHandler.h>
#include <proxygen/httpserver/samples/echo/EchoStats.h>

using namespace EchoService;
using namespace proxygen;
using namespace testing;

class MockEchoStats : public EchoStats {
 public:
  MOCK_METHOD0(recordRequest, void());
  MOCK_METHOD0(getRequestCount, uint64_t());
};

class EchoHandlerFixture : public testing::Test {
 public:
  void SetUp() override {
    handler = new EchoHandler(&stats);
    responseHandler = std::make_unique<MockResponseHandler>(handler);
    handler->setResponseHandler(responseHandler.get());
  }

  void TearDown() override {
    Mock::VerifyAndClear(&stats);
    Mock::VerifyAndClear(responseHandler.get());

    // Since there is no easy way to verify that handler has deleted
    // itself, its advised to run test binary under AddressSanitzer
    // or valgrind to verify that.
  }

 protected:
  EchoHandler* handler{nullptr};
  StrictMock<MockEchoStats> stats;
  std::unique_ptr<MockResponseHandler> responseHandler;
};

TEST_F(EchoHandlerFixture, OnProperRequestSendsResponse) {
  EXPECT_CALL(stats, recordRequest()).WillOnce(Return());
  EXPECT_CALL(stats, getRequestCount()).WillOnce(Return(5));

  HTTPMessage response;
  EXPECT_CALL(*responseHandler, sendHeaders(_)).WillOnce(
      DoAll(SaveArg<0>(&response), Return()));
  EXPECT_CALL(*responseHandler, sendEOM()).WillOnce(Return());

  // Since we know we dont touch request, its ok to pass `nullptr` here.
  handler->onRequest(nullptr);
  handler->onEOM();
  handler->requestComplete();

  EXPECT_EQ("5", response.getHeaders().getSingleOrEmpty("Request-Number"));
  EXPECT_EQ(200, response.getStatusCode());
}

TEST_F(EchoHandlerFixture, ReplaysBodyProperly) {
  EXPECT_CALL(stats, recordRequest()).WillOnce(Return());
  EXPECT_CALL(stats, getRequestCount()).WillOnce(Return(5));

  HTTPMessage response;
  folly::fbstring body;

  EXPECT_CALL(*responseHandler, sendHeaders(_)).WillOnce(
      DoAll(SaveArg<0>(&response), Return()));

  EXPECT_CALL(*responseHandler, sendBody(_)).WillRepeatedly(
      DoAll(
          Invoke([&] (std::shared_ptr<folly::IOBuf> b) {
            body += b->moveToFbString();
          }),
          Return()));

  EXPECT_CALL(*responseHandler, sendEOM()).WillOnce(Return());

  // Since we know we dont touch request, its ok to pass `nullptr` here.
  handler->onRequest(nullptr);
  handler->onBody(folly::IOBuf::copyBuffer("part1"));
  handler->onBody(folly::IOBuf::copyBuffer("part2"));
  handler->onEOM();
  handler->requestComplete();

  EXPECT_EQ("5", response.getHeaders().getSingleOrEmpty("Request-Number"));
  EXPECT_EQ(200, response.getStatusCode());
  EXPECT_EQ("part1part2", body);
}
