/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include <chrono>
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"

using namespace sk_gpu_test;

static void testing_finished_proc(void* ctx) {
    int* count = (int*)ctx;
    *count += 1;
}

static void busy_wait_for_callback(int* count, int expectedValue, GrContext* ctx,
                                   skiatest::Reporter* reporter) {
    // Busy waiting should detect that the work is done.
    auto begin = std::chrono::steady_clock::now();
    auto end = begin;
    do {
        ctx->checkAsyncWorkCompletion();
        end = std::chrono::steady_clock::now();
    } while (*count != expectedValue && (end - begin) < std::chrono::seconds(1));
    if (*count != expectedValue) {
        ERRORF(reporter, "Expected count failed to reach %d within 1 second of busy waiting.",
               expectedValue);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FlushFinishedProcTest, reporter, ctxInfo) {
    GrContext* ctx = ctxInfo.grContext();

    SkImageInfo info =
            SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SK_ColorGREEN);
    auto image = surface->makeImageSnapshot();

    GrFlushInfo flushInfoSyncCpu;
    flushInfoSyncCpu.fFlags = kSyncCpu_GrFlushFlag;
    ctx->flush(flushInfoSyncCpu);

    int count = 0;

    GrFlushInfo flushInfoFinishedProc;
    flushInfoFinishedProc.fFinishedProc = testing_finished_proc;
    flushInfoFinishedProc.fFinishedContext = (void*)&count;
    // There is no work on the surface so flushing may immediately call the finished proc.
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfoFinishedProc);
    REPORTER_ASSERT(reporter, count == 0 || count == 1);
    // Busy waiting should detect that the work is done.
    busy_wait_for_callback(&count, 1, ctx, reporter);

    canvas->clear(SK_ColorRED);

    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfoFinishedProc);

    bool expectAsyncCallback =
            ctx->backend() == GrBackendApi::kVulkan ||
            ((ctx->backend() == GrBackendApi::kOpenGL) && ctx->priv().caps()->fenceSyncSupport()) ||
            ((ctx->backend() == GrBackendApi::kMetal) && ctx->priv().caps()->fenceSyncSupport());
    if (expectAsyncCallback) {
        // On Vulkan the command buffer we just submitted may or may not have finished immediately
        // so the finish proc may not have been called.
        REPORTER_ASSERT(reporter, count == 1 || count == 2);
    } else {
        REPORTER_ASSERT(reporter, count == 2);
    }
    ctx->flush(flushInfoSyncCpu);
    REPORTER_ASSERT(reporter, count == 2);

    // Test flushing via the SkImage
    canvas->drawImage(image, 0, 0);
    image->flush(ctx, flushInfoFinishedProc);
    if (expectAsyncCallback) {
        // On Vulkan the command buffer we just submitted may or may not have finished immediately
        // so the finish proc may not have been called.
        REPORTER_ASSERT(reporter, count == 2 || count == 3);
    } else {
        REPORTER_ASSERT(reporter, count == 3);
    }
    ctx->flush(flushInfoSyncCpu);
    REPORTER_ASSERT(reporter, count == 3);

    // Test flushing via the GrContext
    canvas->clear(SK_ColorBLUE);
    ctx->flush(flushInfoFinishedProc);
    if (expectAsyncCallback) {
        // On Vulkan the command buffer we just submitted may or may not have finished immediately
        // so the finish proc may not have been called.
        REPORTER_ASSERT(reporter, count == 3 || count == 4);
    } else {
        REPORTER_ASSERT(reporter, count == 4);
    }
    ctx->flush(flushInfoSyncCpu);
    REPORTER_ASSERT(reporter, count == 4);

    // There is no work on the surface so flushing may immediately call the finished proc.
    ctx->flush(flushInfoFinishedProc);
    REPORTER_ASSERT(reporter, count == 4 || count == 5);
    busy_wait_for_callback(&count, 5, ctx, reporter);

    count = 0;
    int count2 = 0;
    canvas->clear(SK_ColorGREEN);
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfoFinishedProc);
    // There is no work to be flushed here so this will return immediately, but make sure the
    // finished call from this proc isn't called till the previous surface flush also is finished.
    flushInfoFinishedProc.fFinishedContext = (void*)&count2;
    ctx->flush(flushInfoFinishedProc);
    REPORTER_ASSERT(reporter, count <= 1 && count2 <= count);

    ctx->flush(flushInfoSyncCpu);

    REPORTER_ASSERT(reporter, count == 1);
    REPORTER_ASSERT(reporter, count == count2);
}

