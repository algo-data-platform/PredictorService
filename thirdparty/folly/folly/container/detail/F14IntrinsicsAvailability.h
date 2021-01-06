/*
 * Copyright 2018-present Facebook, Inc.
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

#include <folly/Portability.h>

#ifndef FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE

// F14 has been implemented for SSE2 and NEON (so far)
#if FOLLY_SSE >= 2 || FOLLY_NEON
#define FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE 1
#else
#define FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE 0
#pragma message                                                      \
    "Vector intrinsics / F14 support unavailable on this platform, " \
    "falling back to std::unordered_map / set"
#endif

#endif
