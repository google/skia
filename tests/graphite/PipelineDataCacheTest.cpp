/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Uniform.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PipelineDataCacheTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto cache = recorder->priv().uniformDataCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    static const int kSize = 16;

    // Add a new unique UDB
    static const char kMemory1[kSize] = {
            7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
    };
    UniformDataBlock udb1(SkSpan(kMemory1, kSize));
    const UniformDataBlock* id1;
    {
        id1 = cache->insert(udb1);
        REPORTER_ASSERT(reporter, SkToBool(id1));
        REPORTER_ASSERT(reporter, id1 != &udb1);  // must be a separate address
        REPORTER_ASSERT(reporter, *id1 == udb1);  // but equal contents

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate UDB
    {
        static const char kMemory2[kSize] = {
                7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
        };
        UniformDataBlock udb2(SkSpan(kMemory2, kSize));
        const UniformDataBlock* id2 = cache->insert(udb2);
        REPORTER_ASSERT(reporter, id2 == id1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique UDB
    {
        static const char kMemory3[kSize] = {
                6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
        };
        UniformDataBlock udb3(SkSpan(kMemory3, kSize));
        const UniformDataBlock* id3 = cache->insert(udb3);
        REPORTER_ASSERT(reporter, SkToBool(id3));
        REPORTER_ASSERT(reporter, id3 != id1);
        REPORTER_ASSERT(reporter, *id3 == udb3);

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the UDB comparison failure modes
}
