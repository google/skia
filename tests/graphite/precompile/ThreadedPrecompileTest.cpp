/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

#include <thread>

using namespace::skgpu::graphite;

namespace {

PaintOptions linear() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions radial() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::RadialGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions sweep() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::SweepGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions conical() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::TwoPointConicalGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

void precompile_gradients(Context* context,
                          skiatest::Reporter* reporter,
                          int threadID) {
    constexpr RenderPassProperties kProps = { DepthStencilFlags::kDepth,
                                              kBGRA_8888_SkColorType,
                                              /* requiresMSAA= */ false };

    for (auto createOptionsMtd : { linear, radial, sweep, conical }) {
        PaintOptions paintOptions = createOptionsMtd();
        Precompile(context,
                   paintOptions,
                   DrawTypeFlags::kBitmapText_Mask,
                   { &kProps, 1 });
    }
}

} // anonymous namespace

// This test precompiles all four flavors of gradient sequentially but on multiple
// threads with the goal of creating cache races.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPrecompileTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumThreads = 4;

    std::thread threads[kNumThreads];
    for (int i = 0; i < kNumThreads; ++i) {
        threads[i] = std::thread([context, reporter, i]() {
            precompile_gradients(context, reporter, i);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    // Four types of gradient times three combinations (i.e., 4,8,N) for each one.
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == 12);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses =
                              stats.fGraphicsCacheAdditions + stats.fGraphicsRaces);
}

#endif // SK_GRAPHITE
