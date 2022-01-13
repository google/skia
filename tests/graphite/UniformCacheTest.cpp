/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/Uniform.h"
#include "experimental/graphite/src/UniformCache.h"

using namespace skgpu;

namespace {

sk_sp<UniformData> make_ud(int numUniforms, int dataSize) {
    static constexpr int kMaxUniforms = 3;
    static constexpr Uniform kUniforms[kMaxUniforms] {
        {"point0",   SLType::kFloat2 },
        {"point1",   SLType::kFloat2 },
        {"point2",   SLType::kFloat2 },
    };

    SkASSERT(numUniforms <= kMaxUniforms);

    sk_sp<UniformData> ud = UniformData::Make(numUniforms, kUniforms, dataSize);
    for (int i = 0; i < numUniforms; ++i) {
        ud->offsets()[i] = i;
    }
    for (int i = 0; i < dataSize; ++i) {
        ud->data()[i] = i % 255;
    }

    return ud;
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UniformCacheTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto cache = recorder->uniformCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Nullptr should already be in the cache and return kInvalidUniformID
    {
        uint32_t result0 = cache->insert(nullptr);
        REPORTER_ASSERT(reporter, result0 == UniformCache::kInvalidUniformID);
        REPORTER_ASSERT(reporter, cache->count() == 0);
    }

    // Add a new unique UD
    sk_sp<UniformData> ud1;
    uint32_t result1;
    {
        ud1 = make_ud(2, 16);
        result1 = cache->insert(ud1);
        REPORTER_ASSERT(reporter, result1 != UniformCache::kInvalidUniformID);
        sk_sp<UniformData> lookup = cache->lookup(result1);
        REPORTER_ASSERT(reporter, lookup.get() == ud1.get());

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate UD
    {
        sk_sp<UniformData> ud2 = make_ud(2, 16);
        uint32_t result2 = cache->insert(ud2);
        REPORTER_ASSERT(reporter, result2 != UniformCache::kInvalidUniformID);
        REPORTER_ASSERT(reporter, result2 == result1);
        sk_sp<UniformData> lookup = cache->lookup(result2);
        REPORTER_ASSERT(reporter, lookup.get() != ud2.get());
        REPORTER_ASSERT(reporter, lookup.get() == ud1.get());

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique UD
    {
        sk_sp<UniformData> ud3 = make_ud(3, 16);
        uint32_t result3 = cache->insert(ud3);
        REPORTER_ASSERT(reporter, result3 != UniformCache::kInvalidUniformID);
        REPORTER_ASSERT(reporter, result3 != result1);
        sk_sp<UniformData> lookup = cache->lookup(result3);
        REPORTER_ASSERT(reporter, lookup.get() == ud3.get());
        REPORTER_ASSERT(reporter, lookup.get() != ud1.get());

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the UD comparison failure modes
}
