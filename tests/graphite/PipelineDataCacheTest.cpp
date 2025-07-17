/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/TextureProxy.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PipelineDataCacheTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    const Caps* caps = context->priv().caps();
    ResourceProvider* resourceProvider = context->priv().resourceProvider();

    // Setup caches
    UniformDataCache uCache;
    TextureDataCache tCache;

    REPORTER_ASSERT(reporter, uCache.count() == 0);
    REPORTER_ASSERT(reporter, tCache.bindingCount() == 0);


    // Create testing textures and uniforms
    [[maybe_unused]] static constexpr Uniform kUniforms[] = {{"data", SkSLType::kFloat4}};
    TextureInfo info = caps->getDefaultSampledTextureInfo(kAlpha_8_SkColorType,
                                                          skgpu::Mipmapped::kNo,
                                                          skgpu::Protected::kNo,
                                                          skgpu::Renderable::kYes);
    REPORTER_ASSERT(reporter, info.isValid());

    sk_sp<TextureProxy> proxyA = TextureProxy::Make(caps,
                                                    resourceProvider,
                                                    SkISize::Make(32, 32),
                                                    info,
                                                    "TestDataProxyA",
                                                    skgpu::Budgeted::kYes);
    sk_sp<TextureProxy> proxyB = TextureProxy::Make(caps,
                                                    resourceProvider,
                                                    SkISize::Make(32, 32),
                                                    info,
                                                    "TestDataProxyB",
                                                    skgpu::Budgeted::kYes);
    REPORTER_ASSERT(reporter, proxyA && proxyB);

    // Block A: Add a new, unique set of uniforms and textures
    UniformDataCache::Index uID1;
    TextureDataCache::Index tID1;
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
        gatherer.add(proxyA, {});

        auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

        uID1 = uCache.insert(udb);
        tID1 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uCache.count() == 1);
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 1);
        REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 1);

        // Verify lookup
        REPORTER_ASSERT(reporter, uCache.lookup(uID1).fCpuData == udb);
        REPORTER_ASSERT(reporter, tCache.lookup(tID1) == tdb);
    }

    // Block B: Add the exact same data to test de-duplication
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f}); // Same uniform data
        gatherer.add(proxyA, {});                 // Same texture

        auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

        UniformDataCache::Index uID2 = uCache.insert(udb);
        TextureDataCache::Index tID2 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uID2 == uID1); // Index should be the same
        REPORTER_ASSERT(reporter, tID2 == tID1); // Index should be the same

        REPORTER_ASSERT(reporter, uCache.count() == 1); // Count should NOT increase
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 1); // Count should NOT increase
    }

    // Block C: Add new unique uniforms but the same texture
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{5.f, 6.f, 7.f, 8.f}); // Different uniform data
        gatherer.add(proxyA, {});                 // Same texture

        auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

        UniformDataCache::Index uID3 = uCache.insert(udb);
        TextureDataCache::Index tID3 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uID3 != uID1); // New unique uniform index
        REPORTER_ASSERT(reporter, tID3 == tID1); // Same texture binding index

        REPORTER_ASSERT(reporter, uCache.count() == 2);        // Uniform count increases
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 1); // Texture count does not
    }

    // Block D: Add the same uniforms but a new unique texture
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f}); // Same uniform data as Block A
        gatherer.add(proxyB, {});                 // Different texture

        auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

        UniformDataCache::Index uID4 = uCache.insert(udb);
        TextureDataCache::Index tID4 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uID4 == uID1); // Same uniform index
        REPORTER_ASSERT(reporter, tID4 != tID1); // New unique texture index

        REPORTER_ASSERT(reporter, uCache.count() == 2);        // Uniform count does not increase
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 2); // Texture count increases
        REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2);
    }
}
