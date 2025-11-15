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
    SkDEBUGCODE(static constexpr Uniform kUniforms[] = {{"data", SkSLType::kFloat4}};)
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

    // Block A: Add a new, unique set of uniforms and textures for a render step
    UniformDataCache::Index uID1;
    TextureDataCache::Index tID1;
    {
        PipelineDataGatherer gatherer{Layout::kStd430};

        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
        gatherer.add(proxyA, {});

        auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);

        uID1 = uCache.insert(udb);
        tID1 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uCache.count() == 1);
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 1);
        REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 1);

        // Verify lookup
        REPORTER_ASSERT(reporter, uCache.lookup(uID1).fCpuData == udb);
        REPORTER_ASSERT(reporter, tCache.lookup(tID1) == tdb);
    }

    // Block B: Add the exact same render step data to test de-duplication
    {
        PipelineDataGatherer gatherer{Layout::kStd430};

        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f}); // Same uniform data
        gatherer.add(proxyA, {});                 // Same texture

        auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);

        UniformDataCache::Index uID2 = uCache.insert(udb);
        TextureDataCache::Index tID2 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uID2 == uID1); // Index should be the same
        REPORTER_ASSERT(reporter, tID2 == tID1); // Index should be the same

        REPORTER_ASSERT(reporter, uCache.count() == 1);        // Count should NOT increase
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 1); // Count should NOT increase
    }

    // Block C: Add new unique render step uniforms but the same texture
    UniformDataCache::Index uID3;
    {
        PipelineDataGatherer gatherer{Layout::kStd430};

        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{5.f, 6.f, 7.f, 8.f}); // Different uniform data
        gatherer.add(proxyA, {});                 // Same texture

        auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);

        uID3 = uCache.insert(udb);
        TextureDataCache::Index tID3 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uID3 != uID1); // New unique uniform index
        REPORTER_ASSERT(reporter, tID3 == tID1); // Same texture binding index

        REPORTER_ASSERT(reporter, uCache.count() == 2);        // Uniform count increases
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 1); // Texture count does not
    }

    // Block D: Add the same render step uniforms but a new unique texture
    {
        PipelineDataGatherer gatherer{Layout::kStd430};

        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f}); // Same uniform data as Block A
        gatherer.add(proxyB, {});                 // Different texture

        auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);

        UniformDataCache::Index uID4 = uCache.insert(udb);
        TextureDataCache::Index tID4 = tCache.insert(tdb);

        REPORTER_ASSERT(reporter, uID4 == uID1); // Same uniform index
        REPORTER_ASSERT(reporter, tID4 != tID1); // New unique texture index

        REPORTER_ASSERT(reporter, uCache.count() == 2);        // Uniform count does not increase
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 2); // Texture count increases
        REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2);
    }

    // Block E: Add a unique paint uniform.
    UniformDataCache::Index uID5;
    {
        PipelineDataGatherer gatherer{Layout::kStd430};

        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{10.f, 20.f, 30.f, 40.f}); // New unique paint uniform data

        auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);
        uID5 = uCache.insert(udb);

        REPORTER_ASSERT(reporter, uID5 != uID1 && uID5 != uID3); // New unique uniform index
        REPORTER_ASSERT(reporter, uCache.count() == 3);          // Uniform count increases
    }

    // Block F: Add the same paint uniform to test de-duplication.
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
        gatherer.write(SkV4{10.f, 20.f, 30.f, 40.f});   // Same paint uniform data as Block E

        auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);
        UniformDataCache::Index uID6 = uCache.insert(udb);

        REPORTER_ASSERT(reporter, uID6 == uID5);        // Index should be the same
        REPORTER_ASSERT(reporter, uCache.count() == 3); // Count should NOT increase
    }

    // Block G: Test paint and render step uniforms together.
    {
        PipelineDataGatherer gatherer{Layout::kStd430};

        {
            SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
            gatherer.write(SkV4{10.f, 20.f, 30.f, 40.f}); // Same paint uniform as Block E
            gatherer.add(proxyA, {});
        }

        {
            SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
            gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f}); // Same render step uniform as Block A
            gatherer.add(proxyB, {});
        }

        auto [combinedUdb, combinedTdb] = gatherer.endCombinedData(/*performsShading=*/true);
        UniformDataCache::Index combinedUID = uCache.insert(combinedUdb);

        // This ID should be new because it combines Block A (RenderStep) and Block E (Paint)
        // into a single block of memory {10, 20, 30, 40, 1, 2, 3, 4}
        REPORTER_ASSERT(reporter, combinedUID != uID1);
        REPORTER_ASSERT(reporter, combinedUID != uID5);
        REPORTER_ASSERT(reporter, uCache.count() == 4); // Count increases

        REPORTER_ASSERT(reporter, combinedTdb.numTextures() == 2);
        TextureDataCache::Index combinedTID = tCache.insert(combinedTdb);

        REPORTER_ASSERT(reporter, combinedTID != tID1);              // New binding
        REPORTER_ASSERT(reporter, tCache.bindingCount() == 3);       // Count increases
        REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2); // Still 2 unique proxies
    }

    // Block H: Simulate multiple recordDraw calls for a single geometry without an resetForDraw().
    {
        SkDEBUGCODE(Uniform kCombinedUniforms[] = {{"step1", SkSLType::kFloat4},
                                                   {"step2", SkSLType::kFloat4}};)
        PipelineDataGatherer gatherer{Layout::kStd430};
        UniformDataCache::Index firstCombinedID;
        UniformDataCache::Index secondCombinedID;

        gatherer.resetForDraw();

        // First recordDraw, paint AND renderstep
        {
            {
                SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)
                gatherer.write(SkV4{11.f, 22.f, 33.f, 44.f});       // First unique paint data
            }

            {
                gatherer.markOffsetAndAlign(/*performsShading=*/true, /*requiredAlignment=*/1);

                SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kCombinedUniforms);)
                gatherer.write(SkV4{111.f, 222.f, 333.f, 444.f});       // Renderstep Uniforms 1
                gatherer.write(SkV4{1111.f, 2222.f, 3333.f, 4444.f});   // Renderstep Uniforms 2

                auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);
                firstCombinedID = uCache.insert(udb);
            }

            REPORTER_ASSERT(reporter, uCache.count() == 5);     // Count increases
        }

        // Second recordDraw
        {
            gatherer.rewindForRenderStep();
            gatherer.markOffsetAndAlign(/*performsShading=*/true, /*requiredAlignment=*/1);

            SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kCombinedUniforms);)
            gatherer.write(SkV4{111.f, 222.f, 333.f, 444.f});       // Renderstep Uniforms 1
            gatherer.write(SkV4{1111.f, 2222.f, 3333.f, 4444.f});   // Renderstep Uniforms 2

            auto [udb, tdb] = gatherer.endCombinedData(/*performsShading=*/true);
            secondCombinedID = uCache.insert(udb);

            REPORTER_ASSERT(reporter, secondCombinedID == firstCombinedID); // Should be same block
            REPORTER_ASSERT(reporter, uCache.count() == 5);                 // Count remains the same
        }
    }

    // Block I: Interleaved Shading and Non-Shading steps with varying alignment.
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        gatherer.resetForDraw();

        // Paint Data
        {
            SkDEBUGCODE(Uniform kPaintUniforms[] = {{"paintVal", SkSLType::kFloat}};)
            SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kPaintUniforms);)
            gatherer.write(1.0f);
        }

        // RenderStep: Non-Shading
        {
            gatherer.markOffsetAndAlign(/*performsShading=*/false, /*requiredAlignment=*/16);

            SkDEBUGCODE(Uniform kStepUniforms[] = {{"stepA", SkSLType::kFloat4}};)
            SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kStepUniforms);)
            gatherer.write(SkV4{10.f, 10.f, 10.f, 10.f});

            auto [udb, _] = gatherer.endCombinedData(/*performsShading=*/false);
            uCache.insert(udb);

            REPORTER_ASSERT(reporter, udb.size() == 16); // 16 (renderStep)
        }

        // RenderStep: Shading
        {
            gatherer.rewindForRenderStep();
            gatherer.markOffsetAndAlign(/*performsShading=*/true, /*requiredAlignment=*/1);

            SkDEBUGCODE(Uniform kStepUniforms[] = {{"stepB", SkSLType::kFloat4}});
            SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kStepUniforms);)
            gatherer.write(SkV4{20.f, 20.f, 20.f, 20.f});

            auto [udb, _] = gatherer.endCombinedData(/*performsShading=*/true);
            uCache.insert(udb);

            REPORTER_ASSERT(reporter, udb.size() == 32); // 4 (paint) + 12 (pad) + 16 (renderStep)
        }

        // RenderStep: Non-Shading
        {
            gatherer.rewindForRenderStep();
            gatherer.markOffsetAndAlign(/*performsShading=*/false, /*requiredAlignment=*/8);

            SkDEBUGCODE(Uniform kStepUniforms[] = {{"stepC", SkSLType::kFloat2}});
            SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kStepUniforms);)
            gatherer.write(SkV2{30.f, 30.f});

            auto [udb, _] = gatherer.endCombinedData(/*performsShading=*/false);
            uCache.insert(udb);

            REPORTER_ASSERT(reporter, udb.size() == 8); // 8 (renderStep)
        }

        REPORTER_ASSERT(reporter, uCache.count() == 8);
    }
}
