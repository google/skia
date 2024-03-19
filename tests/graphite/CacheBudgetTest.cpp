/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"

static constexpr size_t kContextBudget = 1234567;

static void set_context_budget(skgpu::graphite::ContextOptions* options) {
    options->fGpuBudgetInBytes = kContextBudget;
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(CacheBudgetTest,
                                           nullptr,
                                           reporter,
                                           context,
                                           testContext,
                                           set_context_budget,
                                           true,
                                           CtsEnforcement::kNextRelease) {
    REPORTER_ASSERT(reporter, context->maxBudgetedBytes() == kContextBudget);

    static constexpr size_t kRecorderBudget = 7654321;
    skgpu::graphite::RecorderOptions recorderOptions;
    recorderOptions.fGpuBudgetInBytes = kRecorderBudget;

    auto recorder = context->makeRecorder(recorderOptions);
    if (!recorder) {
        ERRORF(reporter, "Could not create recorder.");
        return;
    }

    REPORTER_ASSERT(reporter, recorder->maxBudgetedBytes() == kRecorderBudget);
}
