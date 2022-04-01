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

std::unique_ptr<SkUniformDataBlock> make_udb(int seed, int dataSize) {
    sk_sp<SkUniformData> ud = SkUniformData::Make(dataSize);
    for (int i = 0; i < dataSize; ++i) {
        ud->data()[i] = (seed+i) % 255;
    }

    return std::make_unique<SkUniformDataBlock>(std::move(ud));
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(PipelineDataCacheTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto cache = recorder->priv().uniformDataCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Nullptr should already be in the cache
    {
        UniformDataCache::Index invalid;
        REPORTER_ASSERT(reporter, !invalid.isValid());

        SkUniformDataBlock* lookup = cache->lookup(invalid);
        REPORTER_ASSERT(reporter, !lookup);
    }

    // Add a new unique UDB
    std::unique_ptr<SkUniformDataBlock> udb1 = make_udb(7, 16);
    UniformDataCache::Index id1;
    {
        id1 = cache->insert(*udb1);
        REPORTER_ASSERT(reporter, id1.isValid());
        SkUniformDataBlock* lookup = cache->lookup(id1);
        REPORTER_ASSERT(reporter, *lookup == *udb1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate UDB
    {
        std::unique_ptr<SkUniformDataBlock> udb2 = make_udb(7, 16);
        UniformDataCache::Index id2 = cache->insert(*udb2);
        REPORTER_ASSERT(reporter, id2.isValid());
        REPORTER_ASSERT(reporter, id2 == id1);
        SkUniformDataBlock* lookup = cache->lookup(id2);
        REPORTER_ASSERT(reporter, *lookup == *udb1);
        REPORTER_ASSERT(reporter, *lookup == *udb2);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique UDB
    {
        std::unique_ptr<SkUniformDataBlock> udb3 = make_udb(13, 16);
        UniformDataCache::Index id3 = cache->insert(*udb3);
        REPORTER_ASSERT(reporter, id3.isValid());
        REPORTER_ASSERT(reporter, id3 != id1);
        SkUniformDataBlock* lookup = cache->lookup(id3);
        REPORTER_ASSERT(reporter, *lookup == *udb3);
        REPORTER_ASSERT(reporter, *lookup != *udb1);

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the UDB comparison failure modes
}
