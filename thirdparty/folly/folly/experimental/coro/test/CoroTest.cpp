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

#include <folly/Portability.h>

#if FOLLY_HAS_COROUTINES

#include <folly/Chrono.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/Future.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GTest.h>

using namespace folly;

coro::Task<int> task42() {
  co_return 42;
}

TEST(Coro, Basic) {
  ManualExecutor executor;
  auto future = via(&executor, task42());

  EXPECT_FALSE(future.await_ready());

  executor.drive();

  EXPECT_TRUE(future.await_ready());
  EXPECT_EQ(42, future.get());
}

TEST(Coro, BasicFuture) {
  ManualExecutor executor;

  auto future = via(&executor, task42()).toFuture();

  EXPECT_FALSE(future.isReady());

  EXPECT_EQ(42, future.getVia(&executor));
}

coro::Task<void> taskVoid() {
  (void)co_await task42();
  co_return;
}

TEST(Coro, Basic2) {
  ManualExecutor executor;
  auto future = via(&executor, taskVoid());

  EXPECT_FALSE(future.await_ready());

  executor.drive();

  EXPECT_TRUE(future.await_ready());
}

coro::Task<void> taskSleep() {
  (void)co_await futures::sleep(std::chrono::seconds{1});
  co_return;
}

TEST(Coro, Sleep) {
  ScopedEventBaseThread evbThread;

  auto startTime = std::chrono::steady_clock::now();
  auto future = via(evbThread.getEventBase(), taskSleep());

  EXPECT_FALSE(future.await_ready());

  future.wait();

  // The total time should be roughly 1 second. Some builds, especially
  // optimized ones, may result in slightly less than 1 second, so we perform
  // rounding here.
  auto totalTime = std::chrono::steady_clock::now() - startTime;
  EXPECT_GE(
      chrono::round<std::chrono::seconds>(totalTime), std::chrono::seconds{1});

  EXPECT_TRUE(future.await_ready());
}

coro::Task<int> taskException() {
  throw std::runtime_error("Test exception");
  co_return 42;
}

TEST(Coro, Throw) {
  ManualExecutor executor;
  auto future = via(&executor, taskException());

  EXPECT_FALSE(future.await_ready());

  executor.drive();

  EXPECT_TRUE(future.await_ready());
  EXPECT_THROW(future.get(), std::runtime_error);
}

TEST(Coro, FutureThrow) {
  ManualExecutor executor;
  auto future = via(&executor, taskException()).toFuture();

  EXPECT_FALSE(future.isReady());

  executor.drive();

  EXPECT_TRUE(future.isReady());
  EXPECT_THROW(std::move(future).get(), std::runtime_error);
}

coro::Task<int> taskRecursion(int depth) {
  if (depth > 0) {
    EXPECT_EQ(depth - 1, co_await taskRecursion(depth - 1));
  } else {
    (void)co_await futures::sleep(std::chrono::seconds{1});
  }

  co_return depth;
}

TEST(Coro, LargeStack) {
  ScopedEventBaseThread evbThread;
  auto future = via(evbThread.getEventBase(), taskRecursion(5000));

  future.wait();
  EXPECT_EQ(5000, future.get());
}

coro::Task<void> taskThreadNested(std::thread::id threadId) {
  EXPECT_EQ(threadId, std::this_thread::get_id());
  (void)co_await futures::sleep(std::chrono::seconds{1});
  EXPECT_EQ(threadId, std::this_thread::get_id());
  co_return;
}

coro::Task<int> taskThread() {
  auto threadId = std::this_thread::get_id();

  folly::ScopedEventBaseThread evbThread;
  co_await via(
      evbThread.getEventBase(), taskThreadNested(evbThread.getThreadId()));

  EXPECT_EQ(threadId, std::this_thread::get_id());

  co_return 42;
}

TEST(Coro, NestedThreads) {
  ScopedEventBaseThread evbThread;
  auto future = via(evbThread.getEventBase(), taskThread());

  future.wait();
  EXPECT_EQ(42, future.get());
}

coro::Task<int> taskYield(Executor* executor) {
  auto currentExecutor = co_await coro::getCurrentExecutor();
  EXPECT_EQ(executor, currentExecutor);

  auto future = via(currentExecutor, task42());
  EXPECT_FALSE(future.await_ready());

  co_await coro::yield();

  EXPECT_TRUE(future.await_ready());
  co_return future.get();
}

TEST(Coro, CurrentExecutor) {
  ScopedEventBaseThread evbThread;
  auto future =
      via(evbThread.getEventBase(), taskYield(evbThread.getEventBase()));

  future.wait();
  EXPECT_EQ(42, future.get());
}

coro::Task<void> taskTimedWait() {
  auto fastFuture =
      futures::sleep(std::chrono::milliseconds{50}).then([] { return 42; });
  auto fastResult = co_await coro::timed_wait(
      std::move(fastFuture), std::chrono::milliseconds{100});
  EXPECT_TRUE(fastResult);
  EXPECT_EQ(42, *fastResult);

  struct ExpectedException : public std::runtime_error {
    ExpectedException() : std::runtime_error("ExpectedException") {}
  };

  auto throwingFuture = futures::sleep(std::chrono::milliseconds{50}).then([] {
    throw ExpectedException();
  });
  EXPECT_THROW(
      (void)co_await coro::timed_wait(
          std::move(throwingFuture), std::chrono::milliseconds{100}),
      ExpectedException);

  auto slowFuture =
      futures::sleep(std::chrono::milliseconds{200}).then([] { return 42; });
  auto slowResult = co_await coro::timed_wait(
      std::move(slowFuture), std::chrono::milliseconds{100});
  EXPECT_FALSE(slowResult);

  co_return;
}

TEST(Coro, TimedWait) {
  ManualExecutor executor;
  via(&executor, taskTimedWait()).toFuture().getVia(&executor);
}

#endif
