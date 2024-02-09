/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_TestType_DEFINED
#define skiatest_TestType_DEFINED

#include <cstdint>

namespace skiatest {

enum class TestType : uint8_t { kCPU, kGanesh, kGraphite };

}

#endif
