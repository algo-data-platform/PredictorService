/*
 * Copyright 2016 Facebook, Inc.
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

/*
 * Platform independent mkdir function
 */
int make_dir(const char* path);

/*
 * Platform independent chmod function
 */
int chmod_to_755(const char* path);

namespace apache {
namespace thrift {
namespace compiler {

/*
 * Boolean to determine during runtime if we are running on a Windows platform
 */
constexpr bool isWindows() {
#ifdef _WIN32
  return true;
#else
  return false;
#endif
}

} // namespace compiler
} // namespace thrift
} // namespace apache
