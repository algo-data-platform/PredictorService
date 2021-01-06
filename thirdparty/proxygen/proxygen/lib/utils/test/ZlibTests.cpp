/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <proxygen/lib/utils/ZlibStreamCompressor.h>
#include <proxygen/lib/utils/ZlibStreamDecompressor.h>

using namespace folly;
using namespace proxygen;
using namespace std;
using namespace testing;

class ZlibTests : public testing::Test {};

std::unique_ptr<folly::IOBuf> makeBuf(uint32_t size) {
  auto out = folly::IOBuf::create(size);
  out->append(size);
  // fill with random junk
  folly::io::RWPrivateCursor cursor(out.get());
  while (cursor.length() >= 8) {
    cursor.write<uint64_t>(folly::Random::rand64());
  }
  while (cursor.length()) {
    cursor.write<uint8_t>((uint8_t)folly::Random::rand32());
  }
  return out;
}

void verify(ZlibCompressionType type,
            std::unique_ptr<IOBuf> original,
            std::unique_ptr<IOBuf> compressed) {
  auto zd = std::make_unique<ZlibStreamDecompressor>(type);

  auto decompressed = zd->decompress(compressed.get());
  ASSERT_FALSE(zd->hasError()) << "Decompression error. r=" << zd->getStatus();

  IOBufEqualTo eq;
  ASSERT_TRUE(eq(original, decompressed));
}

void compressThenDecompress(ZlibCompressionType type,
                            int level,
                            unique_ptr<IOBuf> buf) {

  unique_ptr<IOBuf> compressed;
  unique_ptr<IOBuf> decompressed;

  unique_ptr<ZlibStreamCompressor> zc(new ZlibStreamCompressor(type, level));

  compressed = zc->compress(buf.get(), true);
  ASSERT_FALSE(zc->hasError()) << "Compression error. r=" << zc->getStatus();

  verify(type, std::move(buf), std::move(compressed));
}

// Try many different sizes because we've hit truncation problems before
TEST_F(ZlibTests, compress_decompress_gzip_5000) {
  ASSERT_NO_FATAL_FAILURE(
      { compressThenDecompress(ZlibCompressionType::GZIP, 6, makeBuf(5000)); });
}

TEST_F(ZlibTests, compress_decompress_gzip_2000) {
  ASSERT_NO_FATAL_FAILURE(
      { compressThenDecompress(ZlibCompressionType::GZIP, 6, makeBuf(2000)); });
}

TEST_F(ZlibTests, compress_decompress_gzip_1024) {
  ASSERT_NO_FATAL_FAILURE(
      { compressThenDecompress(ZlibCompressionType::GZIP, 6, makeBuf(1024)); });
}

TEST_F(ZlibTests, compress_decompress_gzip_500) {
  ASSERT_NO_FATAL_FAILURE(
      { compressThenDecompress(ZlibCompressionType::GZIP, 6, makeBuf(500)); });
}

TEST_F(ZlibTests, compress_decompress_gzip_50) {
  ASSERT_NO_FATAL_FAILURE(
      { compressThenDecompress(ZlibCompressionType::GZIP, 6, makeBuf(50)); });
}

TEST_F(ZlibTests, compress_decompress_deflate) {
  ASSERT_NO_FATAL_FAILURE({
    compressThenDecompress(ZlibCompressionType::DEFLATE, 6, makeBuf(500));
  });
}

TEST_F(ZlibTests, compress_decompress_empty) {
  ASSERT_NO_FATAL_FAILURE(
      { compressThenDecompress(ZlibCompressionType::GZIP, 4, makeBuf(0)); });
}

TEST_F(ZlibTests, compress_decompress_chain) {
  ASSERT_NO_FATAL_FAILURE({
    auto buf = makeBuf(4);
    buf->appendChain(makeBuf(38));
    buf->appendChain(makeBuf(12));
    buf->appendChain(makeBuf(0));
    compressThenDecompress(ZlibCompressionType::GZIP, 6, std::move(buf));
  });
}

TEST_F(ZlibTests, compress_decompress_streaming) {
  ASSERT_NO_FATAL_FAILURE({
    auto compressor =
        std::make_unique<ZlibStreamCompressor>(ZlibCompressionType::GZIP, 6);

    auto first = makeBuf(38);
    auto out = compressor->compress(first.get(), false);

    auto second = makeBuf(12);
    out->prev()->appendChain(compressor->compress(second.get(), false));
    first->prev()->appendChain(std::move(second));

    auto third = makeBuf(4096); // Larger than buffer size
    out->prev()->appendChain(compressor->compress(third.get(), false));
    first->prev()->appendChain(std::move(third));

    auto empty = IOBuf::create(0);
    out->prev()->appendChain(compressor->compress(empty.get(), true));

    verify(ZlibCompressionType::GZIP, std::move(first), std::move(out));
  });
}

TEST_F(ZlibTests, compress_decompress_small_buffer) {
  ASSERT_NO_FATAL_FAILURE({
    auto oldFlag = FLAGS_zlib_compressor_buffer_growth;
    auto guard = folly::makeGuard([&] {
      FLAGS_zlib_compressor_buffer_growth = oldFlag;
    });
    // NB: This is picked intentionally so we don't generate multiple
    // zlib flush markers as ZlibStreamDecompressor fatals on them.
    FLAGS_zlib_compressor_buffer_growth = 10;
    compressThenDecompress(ZlibCompressionType::GZIP, 4, makeBuf(127));
  });
}
