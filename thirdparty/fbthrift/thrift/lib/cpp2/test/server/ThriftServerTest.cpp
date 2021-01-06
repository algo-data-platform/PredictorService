/*
 * Copyright 2015-present Facebook, Inc.
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

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/Memory.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/io/GlobalShutdownSocketSet.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/EventBase.h>
#include <wangle/acceptor/ServerSocketConfig.h>

#include <proxygen/httpserver/HTTPServerOptions.h>
#include <thrift/lib/cpp/async/TAsyncSocket.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/StubSaslClient.h>
#include <thrift/lib/cpp2/async/StubSaslServer.h>
#include <thrift/lib/cpp2/server/Cpp2Connection.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/util/TestHeaderClientChannelFactory.h>
#include <thrift/lib/cpp2/test/util/TestInterface.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;

DECLARE_int32(thrift_cpp2_protocol_reader_string_limit);

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer& server) {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server.getNumIOWorkerThreads());
  h2_options->idleTimeout = server.getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server.getThriftProcessor(), server);
}

TEST(ThriftServer, H2ClientAddressTest) {
  class EchoClientAddrTestInterface : public TestServiceSvIf {
    void sendResponse(std::string& _return, int64_t /* size */) override {
      _return = getConnectionContext()->getPeerAddress()->describe();
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<EchoClientAddrTestInterface>());
  auto& thriftServer = dynamic_cast<ThriftServer&>(runner.getThriftServer());
  thriftServer.addRoutingHandler(createHTTP2RoutingHandler(thriftServer));

  folly::EventBase base;
  TAsyncSocket::UniquePtr socket(new TAsyncSocket(&base, runner.getAddress()));
  TestServiceAsyncClient client(
      HTTPClientChannel::newHTTP2Channel(std::move(socket)));
  auto channel =
      boost::polymorphic_downcast<HTTPClientChannel*>(client.getChannel());

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, channel->getTransport()->getLocalAddress().describe());
}

TEST(ThriftServer, OnewayClientConnectionCloseTest) {
  static std::atomic<bool> done(false);

  class OnewayTestInterface : public TestServiceSvIf {
    void noResponse(int64_t size) override {
      usleep(size);
      done = true;
    }
  };

  TestThriftServerFactory<OnewayTestInterface> factory2;
  ScopedServerThread st(factory2.create());

  {
    folly::EventBase base;
    std::shared_ptr<TAsyncSocket> socket(
        TAsyncSocket::newSocket(&base, *st.getAddress()));
    TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

    client.sync_noResponse(10000);
  } // client out of scope

  usleep(50000);
  EXPECT_TRUE(done);
}

TEST(ThriftServer, OnewayDeferredHandlerTest) {
  class OnewayTestInterface : public TestServiceSvIf {
   public:
    folly::Baton<> done;

    folly::Future<folly::Unit> future_noResponse(int64_t size) override {
      auto tm = getThreadManager();
      auto ctx = getConnectionContext();
      return folly::futures::sleep(std::chrono::milliseconds(size))
          .via(tm)
          .then([ctx] { EXPECT_EQ("noResponse", ctx->getMethodName()); })
          .then([this] { done.post(); });
    }
  };

  auto handler = std::make_shared<OnewayTestInterface>();
  ScopedServerInterfaceThread runner(handler);

  folly::EventBase eb;
  handler->done.reset();
  auto client = runner.newClient<TestServiceAsyncClient>(eb);
  client->sync_noResponse(100);
  ASSERT_TRUE(handler->done.try_wait_for(std::chrono::seconds(1)));
}

TEST(ThriftServer, CompressionClientTest) {
  TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  auto channel =
      boost::polymorphic_downcast<HeaderClientChannel*>(client.getChannel());
  channel->setTransform(apache::thrift::transport::THeader::ZLIB_TRANSFORM);
  channel->setMinCompressBytes(1);

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");

  auto trans = channel->getWriteTransforms();
  EXPECT_EQ(trans.size(), 1);
  for (auto& tran : trans) {
    EXPECT_EQ(tran, apache::thrift::transport::THeader::ZLIB_TRANSFORM);
  }
}

TEST(ThriftServer, ResponseTooBigTest) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  runner.getThriftServer().setMaxResponseSize(4096);
  folly::EventBase eb;
  auto client = runner.newClient<TestServiceAsyncClient>(eb);

  std::string request(4096, 'a');
  std::string response;
  try {
    client->sync_echoRequest(response, request);
    ADD_FAILURE() << "should throw";
  } catch (const TApplicationException& tae) {
    EXPECT_EQ(
        tae.getType(),
        TApplicationException::TApplicationExceptionType::INTERNAL_ERROR);
  } catch (...) {
    ADD_FAILURE() << "unexpected exception thrown";
  }
}

class ConnCallback : public TAsyncSocket::ConnectCallback {
 public:
  void connectSuccess() noexcept override {
  }

  void connectError(
      const transport::TTransportException& ex) noexcept override {
    exception.reset(new transport::TTransportException(ex));
  }

  std::unique_ptr<transport::TTransportException> exception;
};

TEST(ThriftServer, SSLClientOnPlaintextServerTest) {
  TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  auto sslCtx = std::make_shared<SSLContext>();
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSSLSocket::newSocket(sslCtx, &base));
  ConnCallback cb;
  socket->connect(&cb, *sst.getAddress());
  base.loop();
  ASSERT_TRUE(cb.exception);
  auto msg = cb.exception->what();
  EXPECT_NE(nullptr, strstr(msg, "unexpected message"));
}

TEST(ThriftServer, CompressionServerTest) {
  /* This tests the boundary condition of uncompressed value being larger
     than minCompressBytes and compressed value being smaller. We want to ensure
     this case does not cause corruption */
  TestThriftServerFactory<TestInterface> factory;
  factory.minCompressBytes(100);
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  auto channel =
    boost::polymorphic_downcast<HeaderClientChannel*>(client.getChannel());
  channel->setTransform(apache::thrift::transport::THeader::ZLIB_TRANSFORM);

  std::string request(55, 'a');
  std::string response;
  // The response is slightly more than 100 bytes before compression
  // and less than 100 bytes after compression
  client.sync_echoRequest(response, request);
  EXPECT_EQ(response.size(), 100);
}

TEST(ThriftServer, DefaultCompressionTest) {
  /* Tests the functionality of default transforms, ensuring the server properly
     applies them even if the client does not apply any transforms. */
  class Callback : public RequestCallback {
   public:
    explicit Callback(bool compressionExpected, uint16_t expectedTransform)
        : compressionExpected_(compressionExpected),
          expectedTransform_(expectedTransform) {}

   private:
    void requestSent() override {}

    void replyReceived(ClientReceiveState&& state) override {
      auto trans = state.header()->getTransforms();
      if (compressionExpected_) {
        EXPECT_EQ(trans.size(), 1);
        for (auto& tran : trans) {
          EXPECT_EQ(tran, expectedTransform_);
        }
      } else {
        EXPECT_EQ(trans.size(), 0);
      }
    }
    void requestError(ClientReceiveState&& state) override {
      state.exception().throw_exception();
    }
    bool compressionExpected_;
    uint16_t expectedTransform_;
  };

  TestThriftServerFactory<TestInterface> factory;
  factory.minCompressBytes(1);
  factory.defaultWriteTransform(
    apache::thrift::transport::THeader::ZLIB_TRANSFORM);
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  ScopedServerThread sst(server);
  folly::EventBase base;

  // First, with minCompressBytes set low, ensure we compress even though the
  // client did not compress
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));
  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));
  client.sendResponse(
    std::make_unique<Callback>(
      true, apache::thrift::transport::THeader::ZLIB_TRANSFORM
    ),
    64
  );
  base.loop();

  // Ensure that client transforms take precedence
  auto channel =
    boost::polymorphic_downcast<HeaderClientChannel*>(client.getChannel());
  channel->setTransform(apache::thrift::transport::THeader::SNAPPY_TRANSFORM);
  client.sendResponse(
    std::make_unique<Callback>(
      true, apache::thrift::transport::THeader::SNAPPY_TRANSFORM
    ),
    64
  );
  base.loop();

  // Ensure that minCompressBytes still works with default transforms. We
  // Do not expect compression
  server->setMinCompressBytes(1000);
  std::shared_ptr<TAsyncSocket> socket2(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));
  TestServiceAsyncClient client2(HeaderClientChannel::newChannel(socket2));
  client2.sendResponse(std::make_unique<Callback>(false, 0), 64);
  base.loop();

}

TEST(ThriftServer, HeaderTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto serv = factory.create();
  ScopedServerThread sst(serv);
  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
    TAsyncSocket::newSocket(&base, *sst.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  RpcOptions options;
  // Set it as a header directly so the client channel won't set a
  // timeout and the test won't throw TTransportException
  options.setWriteHeader(
      apache::thrift::transport::THeader::CLIENT_TIMEOUT_HEADER,
      folly::to<std::string>(10));
  try {
    client.sync_processHeader(options);
    ADD_FAILURE() << "should timeout";
  } catch (const TApplicationException& e) {
    EXPECT_EQ(e.getType(),
              TApplicationException::TApplicationExceptionType::TIMEOUT);
  }
}

TEST(ThriftServer, LoadHeaderTest) {
  class Callback : public RequestCallback {
   public:
    explicit Callback(bool isLoadExpected)
        : isLoadExpected_(isLoadExpected) {}

   private:
    void requestSent() override {}

    void replyReceived(ClientReceiveState&& state) override {
      const auto& headers = state.header()->getHeaders();
      auto loadIter = headers.find(Cpp2Connection::loadHeader);
      ASSERT_EQ(isLoadExpected_, loadIter != headers.end());
      if (isLoadExpected_) {
        auto load = loadIter->second;
        EXPECT_NE("", load);
      }
    }
    void requestError(ClientReceiveState&&) override {
      ADD_FAILURE() << "The response should not be an error";
    }
    bool isLoadExpected_;
  };

  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  folly::EventBase base;
  auto client = runner.newClient<TestServiceAsyncClient>(&base);

  {
    LOG(ERROR) << "========= no load header ==========";
    client->voidResponse(std::make_unique<Callback>(false));
  }

  {
    LOG(ERROR) << "========= empty load header ==========";
    RpcOptions emptyLoadOptions;
    emptyLoadOptions.setWriteHeader(Cpp2Connection::loadHeader, "");
    client->voidResponse(emptyLoadOptions, std::make_unique<Callback>(true));
  }

  {
    LOG(ERROR) << "========= foo load header ==========";
    RpcOptions customLoadOptions;
    customLoadOptions.setWriteHeader(Cpp2Connection::loadHeader, "foo");
    client->voidResponse(customLoadOptions, std::make_unique<Callback>(true));
  }

  {
    LOG(ERROR) << "========= server overloaded ==========";
    RpcOptions customLoadOptions;
    // force overloaded
    runner.getThriftServer().setIsOverloaded(
        [](const THeader*) { return true; });
    runner.getThriftServer().setGetLoad([](const std::string&) { return 1; });
    customLoadOptions.setWriteHeader(Cpp2Connection::loadHeader, "foo");
    client->voidResponse(customLoadOptions, std::make_unique<Callback>(true));
  }

  base.loop();
}

TEST(ThriftServer, ClientTimeoutTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  ScopedServerThread sst(server);
  folly::EventBase base;

  auto getClient = [&base, &sst]() {
    std::shared_ptr<TAsyncSocket> socket(
        TAsyncSocket::newSocket(&base, *sst.getAddress()));

    return std::make_shared<TestServiceAsyncClient>(
        HeaderClientChannel::newChannel(socket));
  };

  int cbCtor = 0;
  int cbCall = 0;

  auto callback = [&cbCall, &cbCtor](
      std::shared_ptr<TestServiceAsyncClient> client, bool& timeout) {
    cbCtor++;
    return std::unique_ptr<RequestCallback>(new FunctionReplyCallback(
        [&cbCall, client, &timeout](ClientReceiveState&& state) {
          cbCall++;
          if (state.exception()) {
            timeout = true;
            auto ex = state.exception().get_exception();
            auto& e = dynamic_cast<TTransportException const&>(*ex);
            EXPECT_EQ(TTransportException::TIMED_OUT, e.getType());
            return;
          }
          try {
            std::string resp;
            client->recv_sendResponse(resp, state);
          } catch (const TApplicationException& e) {
            timeout = true;
            EXPECT_EQ(TApplicationException::TIMEOUT, e.getType());
            EXPECT_TRUE(state.header()->getFlags() &
                HEADER_FLAG_SUPPORT_OUT_OF_ORDER);
            return;
          }
          timeout = false;
        }));
  };

  // Set the timeout to be 5 milliseconds, but the call will take 10 ms.
  // The server should send a timeout after 5 milliseconds
  RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(5));
  auto client1 = getClient();
  bool timeout1;
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  EXPECT_TRUE(timeout1);
  usleep(10000);

  // This time we set the timeout to be 100 millseconds.  The server
  // should not time out
  options.setTimeout(std::chrono::milliseconds(100));
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  EXPECT_FALSE(timeout1);
  usleep(10000);

  // This time we set server timeout to be 5 millseconds.  However, the
  // task should start processing within that millisecond, so we should
  // not see an exception because the client timeout should be used after
  // processing is started
  server->setTaskExpireTime(std::chrono::milliseconds(5));
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  usleep(10000);

  // The server timeout stays at 5 ms, but we put the client timeout at
  // 5 ms.  We should timeout even though the server starts processing within
  // 5ms.
  options.setTimeout(std::chrono::milliseconds(5));
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  EXPECT_TRUE(timeout1);
  usleep(50000);

  // And finally, with the server timeout at 50 ms, we send 2 requests at
  // once.  Because the first request will take more than 50 ms to finish
  // processing (the server only has 1 worker thread), the second request
  // won't start processing until after 50ms, and will timeout, despite the
  // very high client timeout.
  // We don't know which one will timeout (race conditions) so we just check
  // the xor
  auto client2 = getClient();
  bool timeout2;
  server->setTaskExpireTime(std::chrono::milliseconds(50));
  options.setTimeout(std::chrono::milliseconds(110));
  client1->sendResponse(options, callback(client1, timeout1), 100000);
  client2->sendResponse(options, callback(client2, timeout2), 100000);
  base.loop();
  EXPECT_TRUE(timeout1 || timeout2);
  EXPECT_FALSE(timeout1 && timeout2);

  EXPECT_EQ(cbCall, cbCtor);
}

TEST(ThriftServer, ConnectionIdleTimeoutTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  server->setIdleTimeout(std::chrono::milliseconds(20));
  apache::thrift::util::ScopedServerThread st(server);

  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *st.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  std::string response;
  client.sync_sendResponse(response, 200);
  EXPECT_EQ(response, "test200");
  base.loop();
}

namespace {
class Callback : public RequestCallback {
  void requestSent() override { ADD_FAILURE(); }
  void replyReceived(ClientReceiveState&&) override {
    ADD_FAILURE();
  }
  void requestError(ClientReceiveState&& state) override {
    EXPECT_TRUE(state.exception());
    auto ex =
        state.exception()
            .get_exception<apache::thrift::transport::TTransportException>();
    ASSERT_TRUE(ex);
    EXPECT_THAT(
        ex->what(), testing::StartsWith("transport is closed in write()"));
  }
};
}

TEST(ThriftServer, BadSendTest) {
  TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  client.sendResponse(std::unique_ptr<RequestCallback>(new Callback), 64);

  socket->shutdownWriteNow();
  base.loop();

  std::string response;
  EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
}

TEST(ThriftServer, ResetStateTest) {
  folly::EventBase base;

  // Create a server socket and bind, don't listen.  This gets us a
  // port to test with which is guaranteed to fail.
  auto ssock = std::unique_ptr<folly::AsyncServerSocket,
                               folly::DelayedDestruction::Destructor>(
      new folly::AsyncServerSocket);
  ssock->bind(0);
  EXPECT_FALSE(ssock->getAddresses().empty());

  // We do this loop a bunch of times, because the bug which caused
  // the assertion failure was a lost race, which doesn't happen
  // reliably.
  for (int i = 0; i < 1000; ++i) {
    std::shared_ptr<TAsyncSocket> socket(
        TAsyncSocket::newSocket(&base, ssock->getAddresses()[0]));

    // Create a client.
    TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

    std::string response;
    // This will fail, because there's no server.
    EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
    // On a failed client object, this should also throw an exception.
    // In the past, this would generate an assertion failure and
    // crash.
    EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
  }
}

TEST(ThriftServer, FailureInjection) {
  enum ExpectedFailure { NONE = 0, ERROR, TIMEOUT, DISCONNECT, END };

  std::atomic<ExpectedFailure> expected(NONE);

  using apache::thrift::transport::TTransportException;

  class Callback : public RequestCallback {
   public:
    explicit Callback(const std::atomic<ExpectedFailure>* expected)
        : expected_(expected) {}

   private:
    void requestSent() override {}

    void replyReceived(ClientReceiveState&& state) override {
      std::string response;
      try {
        TestServiceAsyncClient::recv_sendResponse(response, state);
        EXPECT_EQ(NONE, *expected_);
      } catch (const apache::thrift::TApplicationException& ex) {
        const auto& headers = state.header()->getHeaders();
        EXPECT_TRUE(headers.find("ex") != headers.end() &&
                    headers.find("ex")->second == kInjectedFailureErrorCode);
        EXPECT_EQ(ERROR, *expected_);
      } catch (...) {
        ADD_FAILURE() << "Unexpected exception thrown";
      }

      // Now do it again with exception_wrappers.
      auto ew =
          TestServiceAsyncClient::recv_wrapped_sendResponse(response, state);
      if (ew) {
        EXPECT_TRUE(
            ew.is_compatible_with<apache::thrift::TApplicationException>());
        EXPECT_EQ(ERROR, *expected_);
      } else {
        EXPECT_EQ(NONE, *expected_);
      }
    }

    void requestError(ClientReceiveState&& state) override {
      ASSERT_TRUE(state.exception());
      auto ex_ = state.exception().get_exception();
      auto& ex = dynamic_cast<TTransportException const&>(*ex_);
      if (ex.getType() == TTransportException::TIMED_OUT) {
        EXPECT_EQ(TIMEOUT, *expected_);
      } else {
        EXPECT_EQ(DISCONNECT, *expected_);
      }
    }

    const std::atomic<ExpectedFailure>* expected_;
  };

  TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  auto server = std::dynamic_pointer_cast<ThriftServer>(sst.getServer().lock());
  CHECK(server);
  SCOPE_EXIT { server->setFailureInjection(ThriftServer::FailureInjection()); };

  RpcOptions rpcOptions;
  rpcOptions.setTimeout(std::chrono::milliseconds(100));
  for (int i = 0; i < END; ++i) {
    auto exp = static_cast<ExpectedFailure>(i);
    ThriftServer::FailureInjection fi;

    switch (exp) {
      case NONE:
        break;
      case ERROR:
        fi.errorFraction = 1;
        break;
      case TIMEOUT:
        fi.dropFraction = 1;
        break;
      case DISCONNECT:
        fi.disconnectFraction = 1;
        break;
      case END:
        LOG(FATAL) << "unreached";
    }

    server->setFailureInjection(std::move(fi));

    expected = exp;

    auto callback = std::make_unique<Callback>(&expected);
    client.sendResponse(rpcOptions, std::move(callback), 1);
    base.loop();
  }
}

TEST(ThriftServer, useExistingSocketAndExit) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  folly::AsyncServerSocket::UniquePtr serverSocket(
      new folly::AsyncServerSocket);
  serverSocket->bind(0);
  server->useExistingSocket(std::move(serverSocket));
  // In the past, this would cause a SEGV
}

TEST(ThriftServer, useExistingSocketAndConnectionIdleTimeout) {
  // This is ConnectionIdleTimeoutTest, but with an existing socket
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  folly::AsyncServerSocket::UniquePtr serverSocket(
      new folly::AsyncServerSocket);
  serverSocket->bind(0);
  server->useExistingSocket(std::move(serverSocket));

  server->setIdleTimeout(std::chrono::milliseconds(20));
  apache::thrift::util::ScopedServerThread st(server);

  folly::EventBase base;
  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *st.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  std::string response;
  client.sync_sendResponse(response, 200);
  EXPECT_EQ(response, "test200");
  base.loop();
}

namespace {
class ReadCallbackTest : public TAsyncTransport::ReadCallback {
 public:
  void getReadBuffer(void**, size_t*) override {}
  void readDataAvailable(size_t) noexcept override {}
  void readEOF() noexcept override { eof = true; }

  void readError(const transport::TTransportException&) noexcept override {
    eof = true;
  }

  bool eof = false;
};
}

TEST(ThriftServer, ShutdownSocketSetTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  ScopedServerThread sst(server);
  folly::EventBase base;
  ReadCallbackTest cb;

  std::shared_ptr<TAsyncSocket> socket2(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));
  socket2->setReadCallback(&cb);

  base.tryRunAfterDelay(
      [&]() { folly::tryGetShutdownSocketSet()->shutdownAll(); }, 10);
  base.tryRunAfterDelay([&]() { base.terminateLoopSoon(); }, 30);
  base.loopForever();
  EXPECT_EQ(cb.eof, true);
}

TEST(ThriftServer, ShutdownDegenarateServer) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  server->setMaxRequests(1);
  server->setNumIOWorkerThreads(1);
  ScopedServerThread sst(server);
}

TEST(ThriftServer, ModifyingIOThreadCountLive) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  auto iothreadpool = std::make_shared<folly::IOThreadPoolExecutor>(0);
  server->setIOThreadPool(iothreadpool);

  ScopedServerThread sst(server);
  // If there are no worker threads, generally the server event base
  // will stop loop()ing.  Create a timeout event to make sure
  // it continues to loop for the duration of the test.
  server->getServeEventBase()->runInEventBaseThread(
      [&]() { server->getServeEventBase()->tryRunAfterDelay([]() {}, 5000); });

  server->getServeEventBase()->runInEventBaseThreadAndWait(
      [=]() { iothreadpool->setNumThreads(0); });

  folly::EventBase base;

  std::shared_ptr<TAsyncSocket> socket(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));

  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));

  std::string response;

  boost::polymorphic_downcast<HeaderClientChannel*>(client.getChannel())
      ->setTimeout(100);

  // This should fail as soon as it connects:
  // since AsyncServerSocket has no accept callbacks installed,
  // it should close the connection right away.
  ASSERT_ANY_THROW(client.sync_sendResponse(response, 64));

  server->getServeEventBase()->runInEventBaseThreadAndWait(
      [=]() { iothreadpool->setNumThreads(30); });

  std::shared_ptr<TAsyncSocket> socket2(
      TAsyncSocket::newSocket(&base, *sst.getAddress()));

  // Can't reuse client since the channel has gone bad
  TestServiceAsyncClient client2(HeaderClientChannel::newChannel(socket2));

  client2.sync_sendResponse(response, 64);
}

TEST(ThriftServer, setIOThreadPool) {
  auto exe = std::make_shared<folly::IOThreadPoolExecutor>(1);
  TestThriftServerFactory<TestInterface> factory;
  factory.useSimpleThreadManager(false);
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());

  // Set the exe, this used to trip various calls like
  // CHECK(ioThreadPool->numThreads() == 0).
  server->setIOThreadPool(exe);
  EXPECT_EQ(1, server->getNumIOWorkerThreads());
}

namespace {
class ExtendedTestServiceAsyncProcessor : public TestServiceAsyncProcessor {
 public:
  explicit ExtendedTestServiceAsyncProcessor(TestServiceSvIf* serviceInterface)
      : TestServiceAsyncProcessor(serviceInterface) {}

  folly::Optional<std::string> getCacheKeyTest() {
    folly::IOBuf emptyBuffer;
    return getCacheKey(
        &emptyBuffer,
        apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL);
  }
};
}

TEST(ThriftServer, CacheAnnotation) {
  // We aren't parsing anything just want this to compile
  auto testInterface = std::unique_ptr<TestInterface>(new TestInterface);
  ExtendedTestServiceAsyncProcessor processor(testInterface.get());
  EXPECT_FALSE(processor.getCacheKeyTest().hasValue());
}

TEST(ThriftServer, IdleServerTimeout) {
  TestThriftServerFactory<TestInterface> factory;

  auto server = factory.create();
  auto thriftServer = dynamic_cast<ThriftServer *>(server.get());
  thriftServer->setIdleServerTimeout(std::chrono::milliseconds(50));

  ScopedServerThread scopedServer(server);
  scopedServer.join();
}

TEST(ThriftServer, LocalIPCheck) {
  folly::SocketAddress empty;
  folly::SocketAddress v4Local("127.0.0.1", 1);
  folly::SocketAddress v4NonLocal1("128.0.0.1", 2);
  folly::SocketAddress v4NonLocal2("128.0.0.2", 3);
  folly::SocketAddress v6Local("::1", 4);
  folly::SocketAddress v6NonLocal1("::2", 5);
  folly::SocketAddress v6NonLocal2("::3", 6);

  // expect true for client loopback regardless of server
  EXPECT_TRUE(Cpp2Connection::isClientLocal(v4Local, empty));
  EXPECT_TRUE(Cpp2Connection::isClientLocal(v6Local, empty));

  // expect false for any empty address
  EXPECT_FALSE(Cpp2Connection::isClientLocal(empty, v4NonLocal1));
  EXPECT_FALSE(Cpp2Connection::isClientLocal(v4NonLocal1, empty));
  EXPECT_FALSE(Cpp2Connection::isClientLocal(empty, v6NonLocal1));
  EXPECT_FALSE(Cpp2Connection::isClientLocal(v6NonLocal1, empty));

  // expect false for non matching addrs
  EXPECT_FALSE(Cpp2Connection::isClientLocal(v4NonLocal1, v4NonLocal2));
  EXPECT_FALSE(Cpp2Connection::isClientLocal(v6NonLocal1, v6NonLocal2));

  // expect true for matches
  EXPECT_TRUE(Cpp2Connection::isClientLocal(v4NonLocal1, v4NonLocal1));
  EXPECT_TRUE(Cpp2Connection::isClientLocal(v6NonLocal1, v6NonLocal1));
}

TEST(ThriftServer, ServerConfigTest) {
  ThriftServer server;

  wangle::ServerSocketConfig defaultConfig;
  // If nothing is set, expect defaults
  auto serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(serverConfig.sslHandshakeTimeout,
            defaultConfig.sslHandshakeTimeout);

  // Idle timeout of 0 with no SSL handshake set, expect it to be 0.
  server.setIdleTimeout(std::chrono::milliseconds::zero());
  serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(serverConfig.sslHandshakeTimeout,
            std::chrono::milliseconds::zero());

  // Expect the explicit to always win
  server.setSSLHandshakeTimeout(std::chrono::milliseconds(100));
  serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(serverConfig.sslHandshakeTimeout, std::chrono::milliseconds(100));

  // Clear it and expect it to be zero again (due to idle timeout = 0)
  server.setSSLHandshakeTimeout(folly::none);
  serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(serverConfig.sslHandshakeTimeout,
            std::chrono::milliseconds::zero());
}

TEST(ThriftServer, ClientIdentityHook) {
  /* Tests that the server calls the client identity hook when creating a new
     connection context */

  std::atomic<bool> flag{false};
  auto hook = [&flag](
                  const folly::AsyncTransportWrapper* /* unused */,
                  const X509* /* unused */,
                  const SaslServer* /* unused */,
                  const folly::SocketAddress& /* unused */) {
    flag = true;
    return std::unique_ptr<void, void (*)(void*)>(nullptr, [](void *){});
  };

  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  server->setClientIdentityHook(hook);
  apache::thrift::util::ScopedServerThread st(server);

  folly::EventBase base;
  auto socket = TAsyncSocket::newSocket(&base, *st.getAddress());
  TestServiceAsyncClient client(HeaderClientChannel::newChannel(socket));
  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_TRUE(flag);
}

TEST(ThriftServer, SaslThreadCount) {
  auto server = std::make_shared<ThriftServer>();
  server->setNumIOWorkerThreads(10);
  EXPECT_EQ(server->getNumSaslThreadsToRun(), 10);

  server->setNumCPUWorkerThreads(20);
  EXPECT_EQ(server->getNumSaslThreadsToRun(), 20);

  server->setNSaslPoolThreads(30);
  EXPECT_EQ(server->getNumSaslThreadsToRun(), 30);
}
