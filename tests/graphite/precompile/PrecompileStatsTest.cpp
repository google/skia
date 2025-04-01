/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"

namespace {

// The cache behavior shouldn't change based on the backend so just pick one for testing
bool is_dawn_metal_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal;
}

} // anonymous namespace


DEF_GRAPHITE_TEST_FOR_CONTEXTS(PrecompileStatsTest, is_dawn_metal_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    using namespace skgpu::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    GlobalCache* cache = precompileContext->priv().globalCache();

    const RenderPassProperties kBGRA_1_D { DepthStencilFlags::kDepth,
                                           kBGRA_8888_SkColorType,
                                           /* fDstCS= */ nullptr,
                                           /* fRequiresMSAA= */ false };

    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    // Epoch 1 -------------------------------------------------------------------------------------
    REPORTER_ASSERT(reporter, cache->getEpoch() == 1);

    Precompile(precompileContext.get(),
               paintOptions,
               DrawTypeFlags::kBitmapText_Mask,
               { &kBGRA_1_D, 1 });


    GlobalCache::PipelineStats stats = cache->getStats();

    uint16_t saved = stats.fPipelineUsesInEpoch;
    REPORTER_ASSERT(reporter, stats.fPipelineUsesInEpoch > 0); // we should see some uses

    precompileContext->reportPipelineStats(PrecompileContext::StatOptions::kPipelineCache);
    stats = cache->getStats();

    REPORTER_ASSERT(reporter, stats.fPipelineUsesInEpoch == 0); // stats should've been reset

    // Epoch 2 -------------------------------------------------------------------------------------
    REPORTER_ASSERT(reporter, cache->getEpoch() == 2);

    Precompile(precompileContext.get(),
               paintOptions,
               DrawTypeFlags::kBitmapText_Mask,
               { &kBGRA_1_D, 1 });

    stats = cache->getStats();
    // It's a new epoch so the cache lookups count as new uses
    REPORTER_ASSERT(reporter, stats.fPipelineUsesInEpoch == saved);

    Precompile(precompileContext.get(),
               paintOptions,
               DrawTypeFlags::kBitmapText_Mask,
               { &kBGRA_1_D, 1 });

    stats = cache->getStats();
    // The epoch hasn't changed so the additional uses don't contribute
    REPORTER_ASSERT(reporter, stats.fPipelineUsesInEpoch == saved);

    // Test epoch overflow - back to Epoch 1 -------------------------------------------------------
    cache->forceNextEpochOverflow();

    precompileContext->reportPipelineStats(PrecompileContext::StatOptions::kPipelineCache);
    stats = cache->getStats();

    REPORTER_ASSERT(reporter, stats.fPipelineUsesInEpoch == 0); // stats should've been reset

    REPORTER_ASSERT(reporter, cache->getEpoch() == 1);

    Precompile(precompileContext.get(),
               paintOptions,
               DrawTypeFlags::kBitmapText_Mask,
               { &kBGRA_1_D, 1 });

    stats = cache->getStats();
    // Usage counting should still work across epoch reset
    REPORTER_ASSERT(reporter, stats.fPipelineUsesInEpoch == saved);
}

#endif // SK_GRAPHITE
