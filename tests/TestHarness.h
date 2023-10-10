/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestHarness_DEFINED
#define TestHarness_DEFINED

/**
 * Identifies the currently-running test harness.
 */
enum class TestHarness {
    kDM,
    kFM,
    kListGpuUnitTests,
    kSkQP,
    kWasmGMTests,
    kBazelUnitTestRunner,
};
TestHarness CurrentTestHarness();
bool CurrentTestHarnessIsSkQP();

#endif
