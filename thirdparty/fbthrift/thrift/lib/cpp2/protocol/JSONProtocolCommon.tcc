/*
 * Copyright 2004-present Facebook, Inc.
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
#pragma once

namespace apache {
namespace thrift {

// Return the hex character representing the integer val. The value is masked
// to make sure it is in the correct range.
uint8_t JSONProtocolWriterCommon::hexChar(uint8_t val) {
  val &= 0x0F;
  if (val < 10) {
    return val + '0';
  } else {
    return val - 10 + 'a';
  }
}

/*
 * Public writing methods
 */

uint32_t JSONProtocolWriterCommon::writeMessageBegin(
    const std::string& name,
    MessageType messageType,
    int32_t seqid) {
  auto ret = beginContext(ContextType::ARRAY);
  ret += writeI32(detail::json::kThriftVersion1);
  ret += writeString(name);
  ret += writeI32(messageType);
  ret += writeI32(seqid);
  return ret;
}

uint32_t JSONProtocolWriterCommon::writeMessageEnd() {
  return endContext();
}

uint32_t JSONProtocolWriterCommon::writeByte(int8_t byte) {
  auto ret = writeContext();
  return ret + writeJSONInt(byte);
}

uint32_t JSONProtocolWriterCommon::writeI16(int16_t i16) {
  auto ret = writeContext();
  return ret + writeJSONInt(i16);
}

uint32_t JSONProtocolWriterCommon::writeI32(int32_t i32) {
  auto ret = writeContext();
  return ret + writeJSONInt(i32);
}

uint32_t JSONProtocolWriterCommon::writeI64(int64_t i64) {
  auto ret = writeContext();
  return ret + writeJSONInt(i64);
}

uint32_t JSONProtocolWriterCommon::writeDouble(double dub) {
  auto ret = writeContext();
  return ret + writeJSONDouble(dub);
}

uint32_t JSONProtocolWriterCommon::writeFloat(float flt) {
  auto ret = writeContext();
  return ret + writeJSONDouble(flt);
}

uint32_t JSONProtocolWriterCommon::writeString(folly::StringPiece str) {
  auto ret = writeContext();
  return ret + writeJSONString(str);
}

uint32_t JSONProtocolWriterCommon::writeBinary(folly::StringPiece str) {
  return writeBinary(folly::ByteRange(str));
}

uint32_t JSONProtocolWriterCommon::writeBinary(folly::ByteRange v) {
  auto ret = writeContext();
  return ret + writeJSONBase64(v);
}

uint32_t JSONProtocolWriterCommon::writeBinary(
    const std::unique_ptr<folly::IOBuf>& str) {
  DCHECK(str);
  if (!str) {
    auto ret = writeContext();
    return ret + writeJSONString(folly::StringPiece());
  }
  return writeBinary(*str);
}

uint32_t JSONProtocolWriterCommon::writeBinary(const folly::IOBuf& str) {
  auto ret = writeContext();
  return ret + writeJSONBase64(str.clone()->coalesce());
}

uint32_t JSONProtocolWriterCommon::writeSerializedData(
    const std::unique_ptr<folly::IOBuf>& buf) {
  if (!buf) {
    return 0;
  }
  out_.insert(buf->clone());
  return buf->computeChainDataLength();
}

uint32_t JSONProtocolWriterCommon::serializedSizeByte(int8_t /*val*/) const {
  // 3 bytes for serialized, plus it might be a key, plus context
  return 6;
}

uint32_t JSONProtocolWriterCommon::serializedSizeI16(int16_t /*val*/) const {
  return 8;
}

uint32_t JSONProtocolWriterCommon::serializedSizeI32(int32_t /*val*/) const {
  return 13;
}

uint32_t JSONProtocolWriterCommon::serializedSizeI64(int64_t /*val*/) const {
  return 25;
}

uint32_t JSONProtocolWriterCommon::serializedSizeDouble(double /*val*/) const {
  return 25;
}

uint32_t JSONProtocolWriterCommon::serializedSizeFloat(float /*val*/) const {
  return 25;
}

uint32_t JSONProtocolWriterCommon::serializedSizeString(
    folly::StringPiece str) const {
  return str.size() * 6 + 3;
}

uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    folly::StringPiece str) const {
  return serializedSizeBinary(folly::ByteRange(str));
}

uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    folly::ByteRange v) const {
  return v.size() * 6 + 3;
}

uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    std::unique_ptr<folly::IOBuf> const& v) const {
  return (v ? serializedSizeBinary(*v) * 6 : 0) + 3;
}

uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    folly::IOBuf const& v) const {
  size_t size = v.computeChainDataLength();
  if (size > std::numeric_limits<uint32_t>::max() - serializedSizeI32()) {
    TProtocolException::throwExceededSizeLimit();
  }
  return size * 6 + 3;
}

uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    folly::StringPiece str) const {
  return serializedSizeZCBinary(folly::ByteRange(str));
}

uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    folly::ByteRange v) const {
  return serializedSizeBinary(v);
}

uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    std::unique_ptr<folly::IOBuf> const&) const {
  // size only
  return serializedSizeI32();
}

uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    folly::IOBuf const&) const {
  // size only
  return serializedSizeI32();
}

uint32_t JSONProtocolWriterCommon::serializedSizeSerializedData(
    std::unique_ptr<folly::IOBuf> const& /*buf*/) const {
  // writeSerializedData's implementation just chains IOBufs together. Thus
  // we don't expect external buffer space for it.
  return 0;
}

/**
 * Protected writing methods
 */

uint32_t JSONProtocolWriterCommon::beginContext(ContextType type) {
  context.push_back({type, 0});
  switch (type) {
    case ContextType::MAP:
      out_.write(detail::json::kJSONObjectStart);
      return 1;
    case ContextType::ARRAY:
      out_.write(detail::json::kJSONArrayStart);
      return 1;
  }
  CHECK(false);
  return 0;
}

uint32_t JSONProtocolWriterCommon::endContext() {
  DCHECK(!context.empty());
  switch (context.back().type) {
    case ContextType::MAP:
      out_.write(detail::json::kJSONObjectEnd);
      break;
    case ContextType::ARRAY:
      out_.write(detail::json::kJSONArrayEnd);
      break;
  }
  context.pop_back();
  return 1;
}

uint32_t JSONProtocolWriterCommon::writeContext() {
  if (context.empty()) {
    return 0;
  }
  auto& ctx = context.back();
  auto meta = ctx.meta++;
  switch (ctx.type) {
    case ContextType::MAP:
      if (meta == 0) {
        return 0;
      } else if (meta % 2 == 0) {
        out_.write(detail::json::kJSONElemSeparator);
      } else {
        out_.write(detail::json::kJSONPairSeparator);
      }
      return 1;
    case ContextType::ARRAY:
      if (meta != 0) {
        out_.write(detail::json::kJSONElemSeparator);
        return 1;
      }
      return 0;
  }
  CHECK(false);
  return 0;
}

uint32_t JSONProtocolWriterCommon::writeJSONEscapeChar(uint8_t ch) {
  DCHECK(detail::json::kJSONEscapePrefix.size() == 4);
  out_.push((const uint8_t*)detail::json::kJSONEscapePrefix.data(), 4);
  out_.write(hexChar(ch >> 4));
  out_.write(hexChar(ch));
  return 6;
}

uint32_t JSONProtocolWriterCommon::writeJSONChar(uint8_t ch) {
  if (ch >= 0x30) {
    // Only special character >= 0x30 is '\'
    if (ch == detail::json::kJSONBackslash) {
      out_.write(detail::json::kJSONBackslash);
      out_.write(detail::json::kJSONBackslash);
      return 2;
    } else {
      out_.write(ch);
      return 1;
    }
  } else {
    uint8_t outCh = kJSONCharTable[ch];
    // Check if regular character, backslash escaped, or JSON escaped
    if (outCh == 1) {
      out_.write(ch);
      return 1;
    } else if (outCh > 1) {
      out_.write(detail::json::kJSONBackslash);
      out_.write(outCh);
      return 2;
    } else {
      return writeJSONEscapeChar(ch);
    }
  }
}

uint32_t JSONProtocolWriterCommon::writeJSONString(folly::StringPiece str) {
  uint32_t ret = 2;

  out_.write(detail::json::kJSONStringDelimiter);
  for (auto c : str) {
    ret += writeJSONChar(c);
  }
  out_.write(detail::json::kJSONStringDelimiter);

  return ret;
}

uint32_t JSONProtocolWriterCommon::writeJSONBase64(folly::ByteRange v) {
  uint32_t ret = 2;

  out_.write(detail::json::kJSONStringDelimiter);
  auto bytes = v.data();
  auto len = v.size();
  uint8_t b[4];
  while (len >= 3) {
    // Encode 3 bytes at a time
    base64_encode(bytes, 3, b);
    for (int i = 0; i < 4; i++) {
      out_.write(b[i]);
    }
    ret += 4;
    bytes += 3;
    len -= 3;
  }
  if (len) { // Handle remainder
    base64_encode(bytes, len, b);
    for (uint32_t i = 0; i < len + 1; i++) {
      out_.write(b[i]);
    }
    ret += len + 1;
  }
  out_.write(detail::json::kJSONStringDelimiter);

  return ret;
}

uint32_t JSONProtocolWriterCommon::writeJSONBool(bool val) {
  const auto& out = val ? detail::json::kJSONTrue : detail::json::kJSONFalse;
  out_.push((const uint8_t*)out.data(), out.size());
  return out.size();
}

uint32_t JSONProtocolWriterCommon::writeJSONInt(int64_t num) {
  return writeJSONIntInternal(num);
}

template <typename T>
uint32_t JSONProtocolWriterCommon::writeJSONDouble(T dbl) {
  if (dbl == std::numeric_limits<T>::infinity()) {
    return writeJSONString(detail::json::kThriftInfinity);
  } else if (dbl == -std::numeric_limits<T>::infinity()) {
    return writeJSONString(detail::json::kThriftNegativeInfinity);
  } else if (std::isnan(dbl)) {
    return writeJSONString(detail::json::kThriftNan);
  } else {
    return writeJSONDoubleInternal(dbl);
  }
}

/**
 * Public reading functions
 */

uint32_t JSONProtocolReaderCommon::readMessageBegin(
    std::string& name,
    MessageType& messageType,
    int32_t& seqid) {
  auto ret = ensureAndBeginContext(ContextType::ARRAY);
  int64_t tmpVal;
  ret += readI64(tmpVal);
  if (tmpVal != detail::json::kThriftVersion1) {
    throwBadVersion();
  }
  ret += readString(name);
  ret += readI64(tmpVal);
  messageType = (MessageType)tmpVal;
  ret += readI32(seqid);
  return ret;
}

uint32_t JSONProtocolReaderCommon::readMessageEnd() {
  return endContext();
}

uint32_t JSONProtocolReaderCommon::readByte(int8_t& byte) {
  return readInContext<int8_t>(byte);
}

uint32_t JSONProtocolReaderCommon::readI16(int16_t& i16) {
  return readInContext<int16_t>(i16);
}

uint32_t JSONProtocolReaderCommon::readI32(int32_t& i32) {
  return readInContext<int32_t>(i32);
}

uint32_t JSONProtocolReaderCommon::readI64(int64_t& i64) {
  return readInContext<int64_t>(i64);
}

uint32_t JSONProtocolReaderCommon::readDouble(double& dub) {
  return readInContext<double>(dub);
}

uint32_t JSONProtocolReaderCommon::readFloat(float& flt) {
  return readInContext<float>(flt);
}

template <typename StrType>
uint32_t JSONProtocolReaderCommon::readString(StrType& str) {
  return readInContext<StrType>(str);
}

template <typename StrType>
uint32_t JSONProtocolReaderCommon::readBinary(StrType& str) {
  bool keyish;
  auto ret = ensureAndReadContext(keyish);
  return ret + readJSONBase64(str);
}

uint32_t JSONProtocolReaderCommon::readBinary(
    std::unique_ptr<folly::IOBuf>& str) {
  std::string tmp;
  bool keyish;
  auto ret = ensureAndReadContext(keyish);
  ret += readJSONBase64(tmp);
  str = folly::IOBuf::copyBuffer(tmp);
  return ret;
}

uint32_t JSONProtocolReaderCommon::readBinary(folly::IOBuf& str) {
  std::string tmp;
  bool keyish;
  auto ret = ensureAndReadContext(keyish);
  ret += readJSONBase64(tmp);
  str.appendChain(folly::IOBuf::copyBuffer(tmp));
  return ret;
}

uint32_t JSONProtocolReaderCommon::skip(TType /*type*/) {
  bool keyish;
  auto ret = ensureAndReadContext(keyish);
  ret += readWhitespace();
  auto ch = peekCharSafe();
  if (ch == detail::json::kJSONObjectStart) {
    ret += beginContext(ContextType::MAP);
    while (true) {
      skipWhitespace();
      if (peekCharSafe() == detail::json::kJSONObjectEnd) {
        break;
      }
      ret += skip(TType::T_VOID);
      ret += skip(TType::T_VOID);
    }
    return ret + endContext();
  } else if (ch == detail::json::kJSONArrayStart) {
    ret += beginContext(ContextType::ARRAY);
    while (true) {
      skipWhitespace();
      if (peekCharSafe() == detail::json::kJSONArrayEnd) {
        break;
      }
      ret += skip(TType::T_VOID);
    }
    return ret + endContext();
  } else if (ch == detail::json::kJSONStringDelimiter) {
    std::string tmp;
    return ret + readJSONVal(tmp);
  } else if (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9')) {
    double tmp;
    return ret + readJSONVal(tmp);
  } else if (ch == 't' || ch == 'f') {
    bool tmp;
    return ret + readJSONVal(tmp);
  } else if (ch == 'n') {
    return ret + readJSONNull();
  }

  throwInvalidFieldStart(ch);
}

uint32_t JSONProtocolReaderCommon::readFromPositionAndAppend(
    folly::io::Cursor& snapshot,
    std::unique_ptr<folly::IOBuf>& ser) {
  int32_t size = folly::io::Cursor(in_) - snapshot;

  if (ser) {
    std::unique_ptr<folly::IOBuf> newBuf;
    snapshot.clone(newBuf, size);
    // IOBuf are circular, so prependChain called on head is the same as
    // appending the whole chain at the tail.
    ser->prependChain(std::move(newBuf));
  } else {
    // cut a chunk of things directly
    snapshot.clone(ser, size);
  }

  return (uint32_t)size;
}

/**
 * Protected reading functions
 */

void JSONProtocolReaderCommon::skipWhitespace() {
  for (auto peek = in_.peekBytes(); !peek.empty(); peek = in_.peekBytes()) {
    uint32_t size = 0;
    for (char ch : peek) {
      if (ch != detail::json::kJSONSpace && ch != detail::json::kJSONNewline &&
          ch != detail::json::kJSONTab &&
          ch != detail::json::kJSONCarriageReturn) {
        in_.skip(size);
        return;
      }
      ++skippedWhitespace_;
      ++size;
    }
    in_.skip(size);
  }
}

uint32_t JSONProtocolReaderCommon::readWhitespace() {
  skipWhitespace();
  auto ret = skippedWhitespace_;
  skippedWhitespace_ = 0;
  return ret;
}

uint32_t JSONProtocolReaderCommon::ensureCharNoWhitespace(char expected) {
  auto actual = in_.read<int8_t>();
  if (actual != expected) {
    throwUnexpectedChar(actual, expected);
  }
  return 1;
}

uint32_t JSONProtocolReaderCommon::ensureChar(char expected) {
  auto ret = readWhitespace();
  return ret + ensureCharNoWhitespace(expected);
}

void JSONProtocolReaderCommon::ensureAndSkipContext() {
  if (skippedIsUnread_) {
    return;
  }
  skippedIsUnread_ = true;
  keyish_ = false;
  if (!context.empty()) {
    auto meta = context.back().meta++;
    switch (context.back().type) {
      case ContextType::MAP:
        if (meta % 2 == 0) {
          if (meta != 0) {
            skippedChars_ += ensureChar(detail::json::kJSONElemSeparator);
          }
          keyish_ = true;
        } else {
          skippedChars_ += ensureChar(detail::json::kJSONPairSeparator);
        }
        break;
      case ContextType::ARRAY:
        if (meta != 0) {
          skippedChars_ += ensureChar(detail::json::kJSONElemSeparator);
        }
        break;
    }
  }
}

uint32_t JSONProtocolReaderCommon::ensureAndReadContext(bool& keyish) {
  ensureAndSkipContext();
  keyish = keyish_;
  auto ret = skippedChars_;
  skippedChars_ = 0;
  skippedIsUnread_ = false;
  return ret;
}

uint32_t JSONProtocolReaderCommon::ensureAndBeginContext(ContextType type) {
  bool keyish;
  auto ret = ensureAndReadContext(keyish);
  // perhaps handle keyish == true?  I think for backwards compat we want to
  // be able to handle non-string keys, even if it isn't valid JSON
  return ret + beginContext(type);
}

uint32_t JSONProtocolReaderCommon::beginContext(ContextType type) {
  context.push_back({type, 0});
  switch (type) {
    case ContextType::MAP:
      return ensureChar(detail::json::kJSONObjectStart);
    case ContextType::ARRAY:
      return ensureChar(detail::json::kJSONArrayStart);
  }
  CHECK(false);
  return 0;
}

uint32_t JSONProtocolReaderCommon::endContext() {
  DCHECK(!context.empty());

  auto type = context.back().type;
  context.pop_back();
  switch (type) {
    case ContextType::MAP:
      return ensureChar(detail::json::kJSONObjectEnd);
    case ContextType::ARRAY:
      return ensureChar(detail::json::kJSONArrayEnd);
  }
  CHECK(false);
  return 0;
}

template <typename T>
T JSONProtocolReaderCommon::castIntegral(folly::StringPiece val) {
  try {
    return folly::to<T>(val);
  } catch (const std::exception&) {
    throwUnrecognizableAsIntegral(val, typeid(T));
  }
}

template <typename T>
uint32_t JSONProtocolReaderCommon::readInContext(T& val) {
  bool keyish;
  auto ret = ensureAndReadContext(keyish);
  if (keyish) {
    return ret + readJSONKey(val);
  } else {
    return ret + readJSONVal(val);
  }
}

uint32_t JSONProtocolReaderCommon::readJSONKey(std::string& key) {
  return readJSONString(key);
}

uint32_t JSONProtocolReaderCommon::readJSONKey(folly::fbstring& key) {
  return readJSONString(key);
}

uint32_t JSONProtocolReaderCommon::readJSONKey(bool& key) {
  std::string s;
  auto ret = readJSONString(s);
  key = JSONtoBool(s);
  return ret;
}

template <typename T>
uint32_t JSONProtocolReaderCommon::readJSONKey(T& key) {
  std::string s;
  auto ret = readJSONString(s);
  key = castIntegral<T>(s);
  return ret;
}

template <typename T>
uint32_t JSONProtocolReaderCommon::readJSONIntegral(T& val) {
  std::string serialized;
  auto ret = readNumericalChars(serialized);
  val = castIntegral<T>(serialized);
  return ret;
}

uint32_t JSONProtocolReaderCommon::readNumericalChars(std::string& val) {
  return readWhitespace() +
      readWhile(
             [](uint8_t ch) {
               return (ch >= '0' && ch <= '9') || ch == '+' || ch == '-' ||
                   ch == '.' || ch == 'E' || ch == 'e';
             },
             val);
}

uint32_t JSONProtocolReaderCommon::readJSONVal(int8_t& val) {
  return readJSONIntegral<int8_t>(val);
}

uint32_t JSONProtocolReaderCommon::readJSONVal(int16_t& val) {
  return readJSONIntegral<int16_t>(val);
}

uint32_t JSONProtocolReaderCommon::readJSONVal(int32_t& val) {
  return readJSONIntegral<int32_t>(val);
}

uint32_t JSONProtocolReaderCommon::readJSONVal(int64_t& val) {
  return readJSONIntegral<int64_t>(val);
}

uint32_t JSONProtocolReaderCommon::readJSONVal(double& val) {
  auto ret = readWhitespace();
  if (peekCharSafe() == detail::json::kJSONStringDelimiter) {
    std::string str;
    ret += readJSONString(str);
    if (str == detail::json::kThriftNan) {
      val = HUGE_VAL / HUGE_VAL; // generates NaN
    } else if (str == detail::json::kThriftNegativeNan) {
      val = -NAN;
    } else if (str == detail::json::kThriftInfinity) {
      val = HUGE_VAL;
    } else if (str == detail::json::kThriftNegativeInfinity) {
      val = -HUGE_VAL;
    } else {
      throwUnrecognizableAsFloatingPoint(str);
    }
    return ret;
  }
  std::string s;
  ret += readNumericalChars(s);
  try {
    val = folly::to<double>(s);
  } catch (const std::exception&) {
    throwUnrecognizableAsFloatingPoint(s);
  }
  return ret;
}

uint32_t JSONProtocolReaderCommon::readJSONVal(float& val) {
  double d;
  auto ret = readJSONVal(d);
  val = float(d);
  return ret;
}

template <typename Str>
typename std::enable_if<detail::is_string<Str>::value, uint32_t>::type
JSONProtocolReaderCommon::readJSONVal(Str& val) {
  return readJSONString(val);
}

bool JSONProtocolReaderCommon::JSONtoBool(const std::string& s) {
  if (s == "true") {
    return true;
  } else if (s == "false") {
    return false;
  } else {
    throwUnrecognizableAsBoolean(s);
  }
  return false;
}

uint32_t JSONProtocolReaderCommon::readJSONVal(bool& val) {
  std::string s;
  auto ret = readJSONKeyword(s);
  val = JSONtoBool(s);
  return ret;
}

uint32_t JSONProtocolReaderCommon::readJSONNull() {
  std::string s;
  auto ret = readJSONKeyword(s);
  if (s != "null") {
    throwUnrecognizableAsAny(s);
  }
  return ret;
}

uint32_t JSONProtocolReaderCommon::readJSONKeyword(std::string& kw) {
  return readWhitespace() +
      readWhile([](int8_t ch) { return ch >= 'a' && ch <= 'z'; }, kw);
}

uint32_t JSONProtocolReaderCommon::readJSONEscapeChar(uint8_t& out) {
  uint8_t b1, b2;
  ensureCharNoWhitespace(detail::json::kJSONZeroChar);
  ensureCharNoWhitespace(detail::json::kJSONZeroChar);
  b1 = in_.read<uint8_t>();
  b2 = in_.read<uint8_t>();
  out = (hexVal(b1) << 4) + hexVal(b2);
  return 4;
}

template <typename StrType>
uint32_t JSONProtocolReaderCommon::readJSONString(StrType& val) {
  auto ret = ensureChar(detail::json::kJSONStringDelimiter);

  std::string json = "\"";
  val.clear();
  while (true) {
    auto ch = in_.read<uint8_t>();
    ret++;
    if (ch == detail::json::kJSONStringDelimiter) {
      break;
    }
    if (ch == detail::json::kJSONBackslash) {
      ch = in_.read<uint8_t>();
      ret++;
      if (ch == detail::json::kJSONEscapeChar) {
        if (allowDecodeUTF8_) {
          json += "\\u";
          continue;
        } else {
          ret += readJSONEscapeChar(ch);
        }
      } else {
        size_t pos = kEscapeChars.find_first_of(ch);
        if (pos == std::string::npos) {
          throwInvalidEscapeChar(ch);
        }
        if (allowDecodeUTF8_) {
          json += "\\";
          json += kEscapeChars[pos];
          continue;
        } else {
          ch = kEscapeCharVals[pos];
        }
      }
    }

    if (allowDecodeUTF8_) {
      json += ch;
    } else {
      val += ch;
    }
  }

  if (allowDecodeUTF8_) {
    json += "\"";
    try {
      folly::dynamic parsed = folly::parseJson(json);
      val += parsed.getString();
    } catch (const std::exception& e) {
      throwUnrecognizableAsString(json, e);
    }
  }

  return ret;
}

template <typename StrType>
uint32_t JSONProtocolReaderCommon::readJSONBase64(StrType& str) {
  std::string tmp;
  uint32_t ret = readJSONString(tmp);
  uint8_t* b = (uint8_t*)tmp.c_str();
  uint32_t len = tmp.length();
  str.clear();
  while (len >= 4) {
    base64_decode(b, 4);
    str.append((const char*)b, 3);
    b += 4;
    len -= 4;
  }
  // Don't decode if we hit the end or got a single leftover byte (invalid
  // base64 but legal for skip of regular string type)
  if (len > 1) {
    base64_decode(b, len);
    str.append((const char*)b, len - 1);
  }
  return ret;
}

// Return the integer value of a hex character ch.
// Throw a protocol exception if the character is not [0-9a-f].
uint8_t JSONProtocolReaderCommon::hexVal(uint8_t ch) {
  if ((ch >= '0') && (ch <= '9')) {
    return ch - '0';
  } else if ((ch >= 'a') && (ch <= 'f')) {
    return ch - 'a' + 10;
  } else {
    throwInvalidHexChar(ch);
  }
}

template <class Predicate>
uint32_t JSONProtocolReaderCommon::readWhile(
    const Predicate& pred,
    std::string& out) {
  uint32_t ret = 0;
  for (auto peek = in_.peekBytes(); !peek.empty(); peek = in_.peekBytes()) {
    uint32_t size = 0;
    for (uint8_t ch : peek) {
      if (!pred(ch)) {
        out.append(peek.begin(), peek.begin() + size);
        in_.skip(size);
        return ret + size;
      }
      ++size;
    }
    out.append(peek.begin(), peek.end());
    ret += size;
    in_.skip(size);
  }
  return ret;
}

int8_t JSONProtocolReaderCommon::peekCharSafe() {
  auto peek = in_.peekBytes();
  return peek.empty() ? 0 : *peek.data();
}

} // namespace thrift
} // namespace apache
