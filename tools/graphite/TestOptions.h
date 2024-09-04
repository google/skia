/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_TestOptions_DEFINED
#define skiatest_graphite_TestOptions_DEFINED

#include "include/gpu/graphite/ContextOptions.h"

namespace skiatest::graphite {

struct TestOptions {
    TestOptions() = default;
    TestOptions(const TestOptions&) = default;
    TestOptions(TestOptions&&) = default;
    TestOptions& operator=(const TestOptions&) = default;
    TestOptions& operator=(TestOptions&&) = default;

    bool hasDawnOptions() const {
        return fNeverYieldToWebGPU || fUseTintIR || fUseWGPUTextureView;
    }

    skgpu::graphite::ContextOptions fContextOptions = {};

    // The following 3 are Dawn-specific flags
    bool fNeverYieldToWebGPU = false;
    bool fUseTintIR = false;
    bool fUseWGPUTextureView = false;
};

}  // namespace skiatest::graphite

#endif
