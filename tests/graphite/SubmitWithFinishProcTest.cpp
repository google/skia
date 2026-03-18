/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;

namespace {

struct FinishContext {
    skiatest::Reporter* fReporter;
    bool fCalled = false;
};

void FinishProc(GpuFinishedContext ctx, skgpu::CallbackResult result) {
    FinishContext* fc = static_cast<FinishContext*>(ctx);
    REPORTER_ASSERT(fc->fReporter, result == skgpu::CallbackResult::kSuccess);
    fc->fCalled = true;
}

}  // anonymous namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(SubmitWithFinishProc_PendingCommandBuffer,
                                               reporter, context, testCtx, /*condition=*/true,
                                               CtsEnforcement::kNextRelease) {
    // Ensure the Context is fully idle initially.
    testCtx->syncedSubmit(context);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    if (!surface) {
        ERRORF(reporter, "Failed to create surface");
        return;
    }
    surface->getCanvas()->clear(SkColors::kRed);

    FinishContext recordingFinishContext;
    recordingFinishContext.fReporter = reporter;

    // Add work to the pending command buffer with a finish proc on the recording.
    auto recording = recorder->snap();
    InsertRecordingInfo info;
    info.fRecording = recording.get();
    info.fFinishedProc = FinishProc;
    info.fFinishedContext = &recordingFinishContext;
    context->insertRecording(info);

    FinishContext submitFinishContext;
    submitFinishContext.fReporter = reporter;

    // Submit the pending command buffer with a finish proc attached.
    SubmitInfo submitInfo;
    submitInfo.fFinishedProc = FinishProc;
    submitInfo.fFinishedContext = &submitFinishContext;
    context->submit(submitInfo);

    // Syncing should trigger both callbacks.
    testCtx->syncedSubmit(context);
    REPORTER_ASSERT(reporter, recordingFinishContext.fCalled);
    REPORTER_ASSERT(reporter, submitFinishContext.fCalled);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(SubmitWithFinishProc_NoPendingCommandBuffer,
                                               reporter, context, testCtx, /*condition=*/true,
                                               CtsEnforcement::kNextRelease) {
    // Ensure the Context is fully idle.
    testCtx->syncedSubmit(context);

    FinishContext finishContext;
    finishContext.fReporter = reporter;

    // No pending command buffer and GPU is idle: the proc should be triggered immediately.
    SubmitInfo submitInfo;
    submitInfo.fFinishedProc = FinishProc;
    submitInfo.fFinishedContext = &finishContext;
    context->submit(submitInfo);

    REPORTER_ASSERT(reporter, finishContext.fCalled);
}
