/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/PipelineDataCache.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkUniform.h"

using namespace skgpu;

namespace {

std::unique_ptr<SkPipelineData> make_pd(int numUniforms, int dataSize) {
    static constexpr int kMaxUniforms = 3;
    static constexpr SkUniform kUniforms[kMaxUniforms] {
        {"point0",   SkSLType::kFloat2 },
        {"point1",   SkSLType::kFloat2 },
        {"point2",   SkSLType::kFloat2 },
    };

    SkASSERT(numUniforms <= kMaxUniforms);

    sk_sp<SkUniformData> ud = SkUniformData::Make(SkSpan<const SkUniform>(kUniforms, numUniforms),
                                                  dataSize);
    for (int i = 0; i < numUniforms; ++i) {
        ud->offsets()[i] = i;
    }
    for (int i = 0; i < dataSize; ++i) {
        ud->data()[i] = i % 255;
    }

    return std::make_unique<SkPipelineData>(std::move(ud));
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(PipelineDataCacheTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto cache = recorder->priv().pipelineDataCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Nullptr should already be in the cache and return kInvalidUniformID
    {
        uint32_t result0 = cache->insert(nullptr);
        REPORTER_ASSERT(reporter, result0 == PipelineDataCache::kInvalidUniformID);
        REPORTER_ASSERT(reporter, cache->count() == 0);
    }

    // Add a new unique PD
    SkPipelineData* danglingPD1 = nullptr;
    uint32_t result1;
    {
        std::unique_ptr<SkPipelineData> pd1 = make_pd(2, 16);
        danglingPD1 = pd1.get();
        result1 = cache->insert(std::move(pd1));
        REPORTER_ASSERT(reporter, result1 != PipelineDataCache::kInvalidUniformID);
        SkPipelineData* lookup = cache->lookup(result1);
        REPORTER_ASSERT(reporter, lookup == danglingPD1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate PD
    {
        std::unique_ptr<SkPipelineData> pd2 = make_pd(2, 16);
        SkPipelineData* danglingPD2 = pd2.get();
        uint32_t result2 = cache->insert(std::move(pd2));
        REPORTER_ASSERT(reporter, result2 != PipelineDataCache::kInvalidUniformID);
        REPORTER_ASSERT(reporter, result2 == result1);
        SkPipelineData* lookup = cache->lookup(result2);
        REPORTER_ASSERT(reporter, lookup != danglingPD2);
        REPORTER_ASSERT(reporter, lookup == danglingPD1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique PD
    {
        std::unique_ptr<SkPipelineData> pd3 = make_pd(3, 16);
        SkPipelineData* danglingPD3 = pd3.get();
        uint32_t result3 = cache->insert(std::move(pd3));
        REPORTER_ASSERT(reporter, result3 != PipelineDataCache::kInvalidUniformID);
        REPORTER_ASSERT(reporter, result3 != result1);
        SkPipelineData* lookup = cache->lookup(result3);
        REPORTER_ASSERT(reporter, lookup == danglingPD3);
        REPORTER_ASSERT(reporter, lookup != danglingPD1);

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the PD comparison failure modes
}
