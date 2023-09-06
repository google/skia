/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/FenceSync.h"

struct GrContextOptions;

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

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(FlushSubmittedProcTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto ctx = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info);
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
    ctx->flush(surface.get(), SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
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
    ctx->flush(surface.get(), SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 3);
    canvas->clear(SK_ColorRED);
    ctx->flush(surface.get(), SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 3);
    ctx->submit();

    REPORTER_ASSERT(reporter, submittedCount == 5);
    REPORTER_ASSERT(reporter, submittedSuccess);

    // Test an abandoned context to get a failed submit immediately when flush is called
    canvas->clear(SK_ColorCYAN);
    ctx->abandonContext();
    ctx->flush(surface.get(), SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 6);
    REPORTER_ASSERT(reporter, !submittedSuccess);
    ctx->flush(flushInfo);
    REPORTER_ASSERT(reporter, submittedCount == 7);
    REPORTER_ASSERT(reporter, !submittedSuccess);
}
