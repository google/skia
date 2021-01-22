/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"

using namespace sk_gpu_test;

namespace {
struct SubmittedInfo {
    int* fCount;
    bool* fSuccess;
};
}  // namespace

static void testing_submitted_proc(void* ctx, bool success) {
    SubmittedInfo* info = (SubmittedInfo*)ctx;
    *info->fCount += 1;
    *info->fSuccess = success;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FlushSubmittedProcTest, reporter, ctxInfo) {
    auto ctx = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SK_ColorGREEN);

    int submittedCount = 0;
    bool submittedSuccess = false;
    SubmittedInfo submittedInfo = { &submittedCount, &submittedSuccess };

    GrFlushInfo flushInfo;
    flushInfo.fSubmittedProc = testing_submitted_proc;
    flushInfo.fSubmittedContext = &submittedInfo;

    ctx->flush(flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 0);

    ctx->submit();
    REPORTER_ASSERT(reporter, submittedCount == 1);
    REPORTER_ASSERT(reporter, submittedSuccess);

    // There should be no work so if we flush again the submittedProc should be called immediately
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 2);
    REPORTER_ASSERT(reporter, submittedSuccess);

    // However, flushing the context we don't do any checks of work so we still require submit to be
    // called in order for the callback to trigger.
    ctx->flush(flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 2);

    ctx->submit();
    REPORTER_ASSERT(reporter, submittedCount == 3);
    REPORTER_ASSERT(reporter, submittedSuccess);

    // Testing that doing multiple flushes before a submit triggers both submittedProcs to be called
    canvas->clear(SK_ColorBLUE);
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 3);
    canvas->clear(SK_ColorRED);
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 3);
    ctx->submit();

    REPORTER_ASSERT(reporter, submittedCount == 5);
    REPORTER_ASSERT(reporter, submittedSuccess);

    // Test an abandoned context to get a failed submit immediately when flush is called
    canvas->clear(SK_ColorCYAN);
    ctx->abandonContext();
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 6);
    REPORTER_ASSERT(reporter, !submittedSuccess);
    ctx->flush(flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 7);
    REPORTER_ASSERT(reporter, !submittedSuccess);
}
