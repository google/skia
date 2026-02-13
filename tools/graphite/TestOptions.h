/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_TestOptions_DEFINED
#define skiatest_graphite_TestOptions_DEFINED

#include "include/gpu/graphite/ContextOptions.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"

namespace skiatest::graphite {

struct TestOptions {
    TestOptions() {
        fContextOptions.fOptionsPriv = &fOptionsPriv;
    }
    TestOptions(const TestOptions& other) {
        *this = other;
    }
    TestOptions(TestOptions&&) = delete;
    TestOptions& operator=(const TestOptions& other) {
        fContextOptions = other.fContextOptions;
        fOptionsPriv = other.fOptionsPriv;
        fContextOptions.fOptionsPriv = &fOptionsPriv;
#if defined(SK_DAWN)
        fDisableTintSymbolRenaming = other.fDisableTintSymbolRenaming;
        fNeverYieldToWebGPU = other.fNeverYieldToWebGPU;
        fUseWGPUTextureView = other.fUseWGPUTextureView;
#endif
        return *this;
    }
    TestOptions& operator=(TestOptions&&) = delete;

    bool hasDawnOptions() const {
#if defined(SK_DAWN)
        return fDisableTintSymbolRenaming ||
               fNeverYieldToWebGPU ||
               fUseWGPUTextureView;
#else
        return false;
#endif
    }

    skgpu::graphite::ContextOptions fContextOptions = {};
    skgpu::graphite::ContextOptionsPriv fOptionsPriv;

#if defined(SK_DAWN)
    bool fDisableTintSymbolRenaming = false;
    bool fNeverYieldToWebGPU = false;
    bool fUseWGPUTextureView = false;
#endif
};

}  // namespace skiatest::graphite

#endif
