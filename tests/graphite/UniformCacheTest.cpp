/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/Recorder.h"
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
    Recorder recorder(sk_ref_sp(context));

    auto cache = recorder.uniformCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Add a new unique UD
    sk_sp<UniformData> result1;
    {
        sk_sp<UniformData> ud1 = make_ud(2, 16);
        result1 = cache->findOrCreate(ud1);
        REPORTER_ASSERT(reporter, result1->id() != UniformData::kInvalidUniformID);
        REPORTER_ASSERT(reporter, ud1->id() == result1->id());

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate UD
    {
        sk_sp<UniformData> ud2 = make_ud(2, 16);
        sk_sp<UniformData> result2 = cache->findOrCreate(ud2);
        REPORTER_ASSERT(reporter, result2->id() != UniformData::kInvalidUniformID);
        REPORTER_ASSERT(reporter, ud2->id() == UniformData::kInvalidUniformID);
        REPORTER_ASSERT(reporter, result2->id() == result1->id());

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique UD
    {
        sk_sp<UniformData> ud3 = make_ud(3, 16);
        sk_sp<UniformData> result3 = cache->findOrCreate(ud3);
        REPORTER_ASSERT(reporter, result3->id() != UniformData::kInvalidUniformID);
        REPORTER_ASSERT(reporter, ud3->id() == result3->id());
        REPORTER_ASSERT(reporter, result3->id() != result1->id());

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the UD comparison failure modes
}
