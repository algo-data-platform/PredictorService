/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <memory>
#include <folly/FBString.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/codec/compress/Header.h>
#include <proxygen/lib/http/codec/compress/HeaderPiece.h>
#include <proxygen/lib/utils/Result.h>
#include <vector>

namespace folly {
class IOBuf;
}

namespace folly { namespace io {
class Cursor;
}}

namespace proxygen {

struct HeaderDecodeResult {
  compress::HeaderPieceList& headers;
  uint32_t bytesConsumed;
};

class HeaderCodec {
 public:
  const static uint32_t kMaxUncompressed = 128 * 1024;

  enum class Type : uint8_t {
    GZIP = 0,
    HPACK = 1,
  };

  class Stats {
   public:
    Stats() {}
    virtual ~Stats() {}

    virtual void recordEncode(Type type, HTTPHeaderSize& size) = 0;
    virtual void recordDecode(Type type, HTTPHeaderSize& size) = 0;
    virtual void recordDecodeError(Type type) = 0;
    virtual void recordDecodeTooLarge(Type type) = 0;
  };

  HeaderCodec() {}
  virtual ~HeaderCodec() {}

  /**
   * compressed and uncompressed size of the last encode
   */
  const HTTPHeaderSize& getEncodedSize() {
    return encodedSize_;
  }

  /**
   * amount of space to reserve as a headroom in the encode buffer
   */
  void setEncodeHeadroom(uint32_t headroom) {
    encodeHeadroom_ = headroom;
  }

  virtual void setMaxUncompressed(uint32_t maxUncompressed) {
    maxUncompressed_ = maxUncompressed;
  }

  uint32_t getMaxUncompressed() const {
    return maxUncompressed_;
  }

  /**
   * set the stats object
   */
  void setStats(Stats* stats) {
    stats_ = stats;
  }

 protected:

  HTTPHeaderSize encodedSize_;
  uint32_t encodeHeadroom_{0};
  uint32_t maxUncompressed_{kMaxUncompressed};
  Stats* stats_{nullptr};
};

}
