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

#include <thrift/lib/cpp2/transport/rsocket/test/util/TestServiceMock.h>

#include <rsocket/Payload.h>
#include <thrift/lib/cpp2/transport/rsocket/YarplStreamImpl.h>

namespace testutil {
namespace testservice {

using namespace apache::thrift;
using namespace yarpl::flowable;

class LeakDetector {
 public:
  class InternalClass {};

  LeakDetector()
      : internal_(std::make_shared<testing::StrictMock<InternalClass>>()) {
    ++instanceCount();
  }

  LeakDetector(const LeakDetector& oth) : internal_(oth.internal_) {
    ++instanceCount();
  }

  LeakDetector& operator=(const LeakDetector& oth) {
    internal_ = oth.internal_;
    return *this;
  }

  virtual ~LeakDetector() {
    --instanceCount();
  }

  std::shared_ptr<testing::StrictMock<InternalClass>> internal_;

  static int32_t getInstanceCount() {
    return instanceCount();
  }

 protected:
  static std::atomic_int& instanceCount() {
    static std::atomic_int instanceCount{0};
    return instanceCount;
  }
};

Stream<int32_t> TestServiceMock::range(int32_t from, int32_t to) {
  return createStreamGenerator([from, to]() mutable -> folly::Optional<int> {
    if (from >= to) {
      return folly::none;
    }
    return from++;
  });
}

ResponseAndStream<int32_t, int32_t> TestServiceMock::leakCheck(
    int32_t from,
    int32_t to) {
  return {LeakDetector::getInstanceCount(),
          toStream(
              Flowable<>::range(from, to)->map(
                  [detector = LeakDetector()](auto i) { return (int32_t)i; }),
              &executor_)};
}

int32_t TestServiceMock::instanceCount() {
  return LeakDetector::getInstanceCount();
}

Stream<Message> TestServiceMock::returnNullptr() {
  return {};
}

ResponseAndStream<int, Message> TestServiceMock::throwError() {
  throw Error();
}

apache::thrift::ResponseAndStream<int32_t, int32_t>
TestServiceMock::sleepWithResponse(int32_t timeMs) {
  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
  return {1,
          toStream(
              Flowable<>::range(1, 1)->map([](auto i) { return (int32_t)i; }),
              &executor_)};
}

apache::thrift::Stream<int32_t> TestServiceMock::sleepWithoutResponse(
    int32_t timeMs) {
  return std::move(sleepWithResponse(timeMs).stream);
}

apache::thrift::ResponseAndStream<int32_t, int32_t>
TestServiceMock::streamNever() {
  return {1, toStream(Flowable<int32_t>::never(), &executor_)};
}

void TestServiceMock::sendMessage(
    int32_t messageId,
    bool complete,
    bool error) {
  if (!messages_) {
    throw std::runtime_error("First call registerToMessages");
  }
  if (messageId > 0) {
    messages_->next(messageId);
  }
  if (complete) {
    std::move(*messages_).complete();
    messages_.reset();
  } else if (error) {
    std::move(*messages_).complete({std::runtime_error("error")});
    messages_.reset();
  }
}

apache::thrift::Stream<int32_t> TestServiceMock::registerToMessages() {
  auto streamAndPublisher = createStreamPublisher<int32_t>([] {});
  messages_ = std::make_unique<apache::thrift::StreamPublisher<int32_t>>(
      std::move(streamAndPublisher.second));
  return std::move(streamAndPublisher.first);
}

apache::thrift::Stream<Message> TestServiceMock::streamThrows(int32_t whichEx) {
  if (whichEx == 0) {
    SecondEx ex;
    ex.set_errCode(0);
    throw ex;
  }

  return createStreamGenerator([whichEx]() -> folly::Optional<Message> {
    if (whichEx == 1) {
      FirstEx ex;
      ex.set_errMsg("FirstEx");
      ex.set_errCode(1);
      throw ex;
    } else if (whichEx == 2) {
      SecondEx ex;
      ex.set_errCode(2);
      throw ex;
    } else {
      throw std::runtime_error("random error");
    }
  });
}

apache::thrift::ResponseAndStream<int32_t, Message>
TestServiceMock::responseAndStreamThrows(int32_t whichEx) {
  return {1, streamThrows(whichEx)};
}

} // namespace testservice
} // namespace testutil
