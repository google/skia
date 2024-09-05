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
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Uniform.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PipelineDataCacheTest, reporter, context,
                                   CtsEnforcement::kApiLevel_V) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto cache = recorder->priv().uniformDataCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // UniformDataBlocks can only be created via a PipelineDataGatherer, but for this test the
    // layout and contents don't matter other than their bits being the same or different.
    [[maybe_unused]] static constexpr Uniform kUniforms[] = {{"data", SkSLType::kFloat4}};

    // Add a new unique UDB
    UniformDataBlock id1;
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)

        gatherer.write(SkV4{7.f, 8.f, 9.f, 10.f});
        UniformDataBlock udb1 = gatherer.finishUniformDataBlock();

        id1 = cache->insert(udb1);
        REPORTER_ASSERT(reporter, SkToBool(id1));
        REPORTER_ASSERT(reporter, id1.data() != udb1.data());  // must be a separate address
        REPORTER_ASSERT(reporter, id1 == udb1);  // but equal contents

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate UDB
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)

        gatherer.write(SkV4{7.f, 8.f, 9.f, 10.f});
        UniformDataBlock udb2 = gatherer.finishUniformDataBlock();
        REPORTER_ASSERT(reporter, udb2 == id1); // contents are in fact duplicated

        UniformDataBlock id2 = cache->insert(udb2);
        REPORTER_ASSERT(reporter, id2.data() == id1.data()); // original clone's address

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique UDB
    {
        PipelineDataGatherer gatherer{Layout::kStd430};
        SkDEBUGCODE(UniformExpectationsValidator uev(&gatherer, kUniforms);)

        gatherer.write(SkV4{11.f, 12.f, 13.f, 14.f});
        UniformDataBlock udb3 = gatherer.finishUniformDataBlock();

        UniformDataBlock id3 = cache->insert(udb3);
        REPORTER_ASSERT(reporter, SkToBool(id3));
        REPORTER_ASSERT(reporter, id3 != id1);
        REPORTER_ASSERT(reporter, id3 == udb3);

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the UDB comparison failure modes
}
