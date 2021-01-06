/*
 * Copyright 2004-present Facebook, Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache {
namespace thrift {

[[noreturn]] void CompactProtocolReader::throwBadProtocolIdentifier() {
  throw TProtocolException(
      TProtocolException::BAD_VERSION, "Bad protocol identifier");
}

    [[noreturn]] void CompactProtocolReader::throwBadProtocolVersion() {
  throw TProtocolException(
      TProtocolException::BAD_VERSION, "Bad protocol version");
}

[[noreturn]] void CompactProtocolReader::throwBadType(uint8_t const type) {
  throw TProtocolException("don't know what type: " + std::to_string(type));
}

void CompactProtocolReader::readFieldBeginWithStateMediumSlow(
    StructReadState& state,
    int16_t prevFieldId) {
  auto byte = *in_.data();
  in_.skipNoAdvance(1);

  readFieldBeginWithStateImpl(state, prevFieldId, byte);
}
} // namespace thrift
} // namespace apache
