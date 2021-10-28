/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/src/ProgramCache.h"
#include "experimental/graphite/src/Recorder.h"

using namespace skgpu;

DEF_GRAPHITE_TEST_FOR_CONTEXTS(ProgramCacheTest, reporter, context) {
    Recorder recorder(sk_ref_sp(context));

    auto cache = recorder.programCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Add an initial unique program
    sk_sp<ProgramCache::ProgramInfo> pi1;
    {
        Combination c1 { ShaderCombo::ShaderType::kNone, SkTileMode::kRepeat, SkBlendMode::kSrc };
        pi1 = cache->findOrCreateProgram(c1);
        REPORTER_ASSERT(reporter, pi1->id() != ProgramCache::kInvalidProgramID);
        REPORTER_ASSERT(reporter, pi1->combo() == c1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate program
    {
        Combination c2 { ShaderCombo::ShaderType::kNone, SkTileMode::kRepeat, SkBlendMode::kSrc };
        sk_sp<ProgramCache::ProgramInfo> pi2 = cache->findOrCreateProgram(c2);
        REPORTER_ASSERT(reporter, pi2->id() != ProgramCache::kInvalidProgramID);
        REPORTER_ASSERT(reporter, pi2->id() == pi1->id());
        REPORTER_ASSERT(reporter, pi2->combo() == c2);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second unique program
    {
        Combination c3 { ShaderCombo::ShaderType::kLinearGradient,
                         SkTileMode::kRepeat,
                         SkBlendMode::kSrc };

        sk_sp<ProgramCache::ProgramInfo> pi3 = cache->findOrCreateProgram(c3);
        REPORTER_ASSERT(reporter, pi3->id() != ProgramCache::kInvalidProgramID);
        REPORTER_ASSERT(reporter, pi3->id() != pi1->id());
        REPORTER_ASSERT(reporter, pi3->combo() == c3);

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise more program variations

}
