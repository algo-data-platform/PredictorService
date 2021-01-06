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

#include <cstdint>
#include <string>

#include <proxygen/lib/utils/Export.h>

namespace proxygen {

/**
 * Codes (hashes) of common HTTP header names
 */
enum HTTPHeaderCode : uint8_t {
  // code reserved to indicate the absence of an HTTP header
  HTTP_HEADER_NONE = 0,
  // code for any HTTP header name not in the list of common headers
  HTTP_HEADER_OTHER = 1,

  /* the following is a placeholder for the build script to generate a list
   * of enum values from the list in HTTPCommonHeaders.txt
   *
   * enum name of Some-Header is HTTP_HEADER_SOME_HEADER,
   * so an example fragment of the generated list could be:
   * ...
   * HTTP_HEADER_WARNING = 65,
   * HTTP_HEADER_WWW_AUTHENTICATE = 66,
   * HTTP_HEADER_X_BACKEND = 67,
   * HTTP_HEADER_X_BLOCKID = 68,
   * ...
   */
  HTTP_HEADER_ACCEPT_ENCODING = 11,
  HTTP_HEADER_ACCEPT_LANGUAGE = 12,
  HTTP_HEADER_ACCEPT_RANGES = 13,
  HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS = 14,
  HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS = 15,
  HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS = 16,
  HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN = 17,
  HTTP_HEADER_CONTENT_LENGTH = 31,
  HTTP_HEADER_ACCESS_CONTROL_EXPOSE_HEADERS = 18,
  HTTP_HEADER_CONTENT_LOCATION = 32,
  HTTP_HEADER_ACCESS_CONTROL_MAX_AGE = 19,
  HTTP_HEADER_CONTENT_MD5 = 33,
  HTTP_HEADER_ACCESS_CONTROL_REQUEST_HEADERS = 20,
  HTTP_HEADER_CONTENT_RANGE = 34,
  HTTP_HEADER_CONTENT_TYPE = 35,
  HTTP_HEADER_COOKIE = 36,
  HTTP_HEADER_DNT = 37,
  HTTP_HEADER_LAST_MODIFIED = 51,
  HTTP_HEADER_DATE = 38,
  HTTP_HEADER_LINK = 52,
  HTTP_HEADER_ETAG = 39,
  HTTP_HEADER_LOCATION = 53,
  HTTP_HEADER_EXPECT = 40,
  HTTP_HEADER_MAX_FORWARDS = 54,
  HTTP_HEADER_ORIGIN = 55,
  HTTP_HEADER_P3P = 56,
  HTTP_HEADER_PRAGMA = 57,
  HTTP_HEADER_COLON_AUTHORITY = 2,
  HTTP_HEADER_TE = 71,
  HTTP_HEADER_PROXY_AUTHENTICATE = 58,
  HTTP_HEADER_COLON_METHOD = 3,
  HTTP_HEADER_TIMESTAMP = 72,
  HTTP_HEADER_PROXY_AUTHORIZATION = 59,
  HTTP_HEADER_COLON_PATH = 4,
  HTTP_HEADER_TRAILER = 73,
  HTTP_HEADER_PROXY_CONNECTION = 60,
  HTTP_HEADER_COLON_PROTOCOL = 5,
  HTTP_HEADER_TRANSFER_ENCODING = 74,
  HTTP_HEADER_COLON_SCHEME = 6,
  HTTP_HEADER_UPGRADE = 75,
  HTTP_HEADER_COLON_STATUS = 7,
  HTTP_HEADER_USER_AGENT = 76,
  HTTP_HEADER_ACCEPT = 8,
  HTTP_HEADER_VIP = 77,
  HTTP_HEADER_ACCEPT_CHARSET = 9,
  HTTP_HEADER_X_UA_COMPATIBLE = 91,
  HTTP_HEADER_VARY = 78,
  HTTP_HEADER_ACCEPT_DATETIME = 10,
  HTTP_HEADER_X_WAP_PROFILE = 92,
  HTTP_HEADER_VIA = 79,
  HTTP_HEADER_X_XSS_PROTECTION = 93,
  HTTP_HEADER_WWW_AUTHENTICATE = 80,
  HTTP_HEADER_ACCESS_CONTROL_REQUEST_METHOD = 21,
  HTTP_HEADER_AGE = 22,
  HTTP_HEADER_ALLOW = 23,
  HTTP_HEADER_ALT_SVC = 24,
  HTTP_HEADER_AUTHORIZATION = 25,
  HTTP_HEADER_CACHE_CONTROL = 26,
  HTTP_HEADER_CONNECTION = 27,
  HTTP_HEADER_EXPIRES = 41,
  HTTP_HEADER_CONTENT_DISPOSITION = 28,
  HTTP_HEADER_FROM = 42,
  HTTP_HEADER_CONTENT_ENCODING = 29,
  HTTP_HEADER_FRONT_END_HTTPS = 43,
  HTTP_HEADER_CONTENT_LANGUAGE = 30,
  HTTP_HEADER_HOST = 44,
  HTTP_HEADER_IF_MATCH = 45,
  HTTP_HEADER_IF_MODIFIED_SINCE = 46,
  HTTP_HEADER_IF_NONE_MATCH = 47,
  HTTP_HEADER_RANGE = 61,
  HTTP_HEADER_IF_RANGE = 48,
  HTTP_HEADER_REFERER = 62,
  HTTP_HEADER_IF_UNMODIFIED_SINCE = 49,
  HTTP_HEADER_REFRESH = 63,
  HTTP_HEADER_KEEP_ALIVE = 50,
  HTTP_HEADER_RETRY_AFTER = 64,
  HTTP_HEADER_SEC_TOKEN_BINDING = 65,
  HTTP_HEADER_SEC_WEBSOCKET_ACCEPT = 66,
  HTTP_HEADER_SEC_WEBSOCKET_KEY = 67,
  HTTP_HEADER_WARNING = 81,
  HTTP_HEADER_SERVER = 68,
  HTTP_HEADER_X_ACCEL_REDIRECT = 82,
  HTTP_HEADER_SET_COOKIE = 69,
  HTTP_HEADER_X_CONTENT_SECURITY_POLICY_REPORT_ONLY = 83,
  HTTP_HEADER_STRICT_TRANSPORT_SECURITY = 70,
  HTTP_HEADER_X_CONTENT_TYPE_OPTIONS = 84,
  HTTP_HEADER_X_FORWARDED_FOR = 85,
  HTTP_HEADER_X_FORWARDED_PROTO = 86,
  HTTP_HEADER_X_FRAME_OPTIONS = 87,
  HTTP_HEADER_X_POWERED_BY = 88,
  HTTP_HEADER_X_REAL_IP = 89,
  HTTP_HEADER_X_REQUESTED_WITH = 90,

};

const uint8_t HTTPHeaderCodeCommonOffset = 2;

enum HTTPCommonHeaderTableType: uint8_t {
  TABLE_CAMELCASE = 0,
  TABLE_LOWERCASE = 1,
};

class HTTPCommonHeaders {
 public:
  // Perfect hash function to match common HTTP header names
  FB_EXPORT static HTTPHeaderCode hash(const char* name, size_t len);

  FB_EXPORT inline static HTTPHeaderCode hash(const std::string& name) {
    return hash(name.data(), name.length());
  }

  FB_EXPORT static std::string* initHeaderNames(HTTPCommonHeaderTableType type);
  constexpr static uint64_t num_header_codes = 94;

  static const std::string* getPointerToCommonHeaderTable(
    HTTPCommonHeaderTableType type);

  inline static const std::string* getPointerToHeaderName(HTTPHeaderCode code,
      HTTPCommonHeaderTableType type = TABLE_CAMELCASE) {
    return getPointerToCommonHeaderTable(type) + code;
  }

  inline static bool isHeaderNameFromTable(const std::string* headerName,
      HTTPCommonHeaderTableType type) {
    return getHeaderCodeFromTableCommonHeaderName(headerName, type) >=
      HTTPHeaderCodeCommonOffset;
  }

  // This method supplements hash().  If dealing with string pointers, some
  // pointing to entries in the the common header name table and some not, this
  // method can be used in place of hash to reverse map a string from the common
  // header name table to its HTTPHeaderCode
  inline static HTTPHeaderCode getHeaderCodeFromTableCommonHeaderName(
      const std::string* headerName, HTTPCommonHeaderTableType type) {
    if (headerName == nullptr) {
      return HTTP_HEADER_NONE;
    } else {
      auto diff = headerName - getPointerToCommonHeaderTable(type);
      if (diff >= HTTPHeaderCodeCommonOffset && diff < (long)num_header_codes) {
        return static_cast<HTTPHeaderCode>(diff);
      } else {
        return HTTP_HEADER_OTHER;
      }
    }
  }

};

} // proxygen
