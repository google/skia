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

// kCPU tests run in parallel; kCPUSerial do not and are executed before any
// other parallel work starts. GPU tests always run serially with respect to
// each other, but in parallel with kCPU tests.
enum class TestType : uint8_t { kCPU, kCPUSerial, kGanesh, kGraphite };

}

#endif
