/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"

#if GR_GPU_STATS
#if defined(GPU_TEST_UTILS)
#include "include/core/SkString.h"

using namespace skia_private;

using Stats = GrThreadSafePipelineBuilder::Stats;

static const char* cache_result_to_str(int i) {
    const char* kCacheResultStrings[Stats::kNumProgramCacheResults] = {
        "hits",
        "misses",
        "partials"
    };
    static_assert(0 == (int) Stats::ProgramCacheResult::kHit);
    static_assert(1 == (int) Stats::ProgramCacheResult::kMiss);
    static_assert(2 == (int) Stats::ProgramCacheResult::kPartial);
    static_assert(Stats::kNumProgramCacheResults == 3);
    return kCacheResultStrings[i];
}

void GrThreadSafePipelineBuilder::Stats::dump(SkString* out) {
    out->appendf("Shader Compilations: %d\n", fShaderCompilations.load());

    SkASSERT(fNumInlineCompilationFailures == 0);
    out->appendf("Number of Inline compile failures %d\n", fNumInlineCompilationFailures.load());
    for (int i = 0; i < Stats::kNumProgramCacheResults-1; ++i) {
        out->appendf("Inline Program Cache %s %d\n", cache_result_to_str(i),
                     fInlineProgramCacheStats[i].load());
    }

    SkASSERT(fNumPreCompilationFailures == 0);
    out->appendf("Number of precompile failures %d\n", fNumPreCompilationFailures.load());
    for (int i = 0; i < Stats::kNumProgramCacheResults-1; ++i) {
        out->appendf("Precompile Program Cache %s %d\n", cache_result_to_str(i),
                     fPreProgramCacheStats[i].load());
    }

    SkASSERT(fNumCompilationFailures == 0);
    out->appendf("Total number of compilation failures %d\n", fNumCompilationFailures.load());
    out->appendf("Total number of partial compilation successes %d\n",
                 fNumPartialCompilationSuccesses.load());
    out->appendf("Total number of compilation successes %d\n", fNumCompilationSuccesses.load());
}

void GrThreadSafePipelineBuilder::Stats::dumpKeyValuePairs(TArray<SkString>* keys,
                                                           TArray<double>* values) {
    keys->push_back(SkString("shader_compilations")); values->push_back(fShaderCompilations);
}

#endif // defined(GPU_TEST_UTILS)
#endif // GR_GPU_STATS
