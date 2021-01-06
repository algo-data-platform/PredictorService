/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionUtils.h>

#include <proxygen/lib/http/codec/HTTP2Constants.h>

using std::string;
using std::vector;

namespace {

using namespace proxygen;

std::string combineCookieCrumbsSorted(std::vector<std::string> crumbs) {
  std::string retval;
  sort(crumbs.begin(), crumbs.end());
  folly::join("; ", crumbs.begin(), crumbs.end(), retval);
  return retval;
}

bool containsAllHeaders(const HTTPHeaders& h1, const HTTPHeaders& h2) {
  bool allValuesPresent = true;
  bool verifyCookies = false;
  h1.forEachWithCode(
      [&](HTTPHeaderCode code, const string& name, const string& value1) {
        bool h2HasValue =
            h2.forEachValueOfHeader(code, [&value1](const std::string& value2) {
              return (value1 == value2);
            });
        if (!h2HasValue && code == HTTP_HEADER_COOKIE) {
          verifyCookies = true;
          return;
        }
        DCHECK(h2HasValue) << "h2 does not contain name=" << name
                           << " value=" << value1;
        allValuesPresent &= h2HasValue;
      });

  if (verifyCookies) {
    const HTTPHeaders* headers[] = {
        &h1,
        &h2,
    };
    std::string cookies[2] = {
        "",
        "",
    };
    unsigned i;
    for (i = 0; i < 2; ++i) {
      std::vector<std::string> crumbs;
      headers[i]->forEachValueOfHeader(HTTP_HEADER_COOKIE,
                                       [&](const std::string& crumb) {
                                         crumbs.push_back(crumb);
                                         return false;
                                       });
      cookies[i] = combineCookieCrumbsSorted(crumbs);
    }
    if (cookies[0] == cookies[1]) {
      LOG(INFO) << "Cookie crumbs are reordered";
    } else {
      LOG(INFO) << "Cookies are not equal: `" << cookies[0] << "' vs. `"
                << cookies[1] << "'";
      return false;
    }
  }

  return allValuesPresent;
}

}

namespace proxygen {

namespace compress {

std::vector<compress::Header> prepareMessageForCompression(
    const HTTPMessage& msg,
    std::vector<string>& cookies) {
  std::vector<compress::Header> allHeaders;
  // The encode API is pretty bad.  We should just let HPACK directly encode
  // HTTP messages
  allHeaders.emplace_back(HTTP_HEADER_COLON_METHOD, http2::kMethod,
                          msg.getMethodString());
  if (msg.getMethod() != HTTPMethod::CONNECT) {
    allHeaders.emplace_back(HTTP_HEADER_COLON_SCHEME, http2::kScheme,
                            (msg.isSecure() ? http2::kHttps : http2::kHttp));
    allHeaders.emplace_back(HTTP_HEADER_COLON_PATH, http2::kPath, msg.getURL());
  }

  const HTTPHeaders& headers = msg.getHeaders();
  const string& host = headers.getSingleOrEmpty(HTTP_HEADER_HOST);
  if (!host.empty()) {
    allHeaders.emplace_back(
        HTTP_HEADER_COLON_AUTHORITY, http2::kAuthority, host);
  }
  // Cookies are coalesced in the HAR file but need to be added as separate
  // headers to optimize compression ratio
  headers.forEachWithCode(
      [&](HTTPHeaderCode code, const string& name, const string& value) {
        if (code == HTTP_HEADER_COOKIE) {
          vector<folly::StringPiece> cookiePieces;
          folly::split(';', value, cookiePieces);
          cookies.reserve(cookies.size() + cookiePieces.size());
          for (auto cookie : cookiePieces) {
            cookies.push_back(ltrimWhitespace(cookie).str());
            allHeaders.emplace_back(code, name, cookies.back());
          }
        } else if (code != HTTP_HEADER_HOST && name[0] != ':') {
          // HAR files contain actual serialized headers protocol headers like
          // :authority, which we are re-adding above.  Strip them so our
          // equality test works
          allHeaders.emplace_back(code, name, value);
        }
      });
  return allHeaders;
}

} // end compress

bool operator==(const HTTPMessage& msg1, const HTTPMessage& msg2) {
  return (msg1.getMethodString() == msg2.getMethodString() &&
          msg1.getURL() == msg2.getURL() &&
          msg1.isSecure() == msg2.isSecure() &&
          containsAllHeaders(msg1.getHeaders(), msg2.getHeaders()) &&
          containsAllHeaders(msg2.getHeaders(), msg1.getHeaders()));
}

}
