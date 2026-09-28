/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/RecorderPriv.h"

using namespace skgpu::graphite;
using Mipmapped = skgpu::Mipmapped;

// Tests to make sure the managing of back pointers between Recorder and Device all work properly.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderDevicePtrTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkImageInfo info = SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // Add multiple devices to later test different patterns of destruction.
    sk_sp<Device> device1 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    sk_sp<Device> device2 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    sk_sp<Device> device3 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    sk_sp<Device> device4 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    REPORTER_ASSERT(reporter, device1->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device2->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device3->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device4->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device2.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device3.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device4.get()));

    // Test freeing a device in the middle, marking it as immutable as ~Surface() our FilterResult
    // would when done with the device.
    device2->setImmutable();
    REPORTER_ASSERT(reporter, device2->recorder() == nullptr);
    REPORTER_ASSERT(reporter, device2->unique()); // Only the test holds a ref now
    REPORTER_ASSERT(reporter, !recorder->priv().deviceIsRegistered(device2.get()));
    device2.reset();

    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device3.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device4.get()));

    // Test freeing a device that wasn't marked as immutable, which should have its ref dropped
    // automatically when the recorder flushes.
    Device* dev4Ptr = device4.get();
    device4.reset();
    REPORTER_ASSERT(reporter, dev4Ptr->unique()); // The recorder holds a ref still
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(dev4Ptr));
    recorder->priv().flushTrackedDevices(
            SK_DUMP_TASKS_CODE("RecorderTest")); // should delete device4 now
    REPORTER_ASSERT(reporter, !recorder->priv().deviceIsRegistered(dev4Ptr));

    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device3.get()));

    // Delete the recorder and make sure remaining devices no longer have a valid recorder.
    recorder.reset();
    REPORTER_ASSERT(reporter, device1->recorder() == nullptr);
    REPORTER_ASSERT(reporter, device3->recorder() == nullptr);

    // Make sure freeing Devices after recorder doesn't cause any crash. This would get checked
    // naturually when these devices go out of scope, but manually reseting will give us a better
    // stack trace if something does go wrong.
    device1.reset();
    device3.reset();
}

// Tests to make sure the Recorders can override the ordering requirement.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderOrderingTest, reporter, context,
                                   CtsEnforcement::kNever) {
    std::unique_ptr<Recorder> orderedRecorder, unorderedRecorder;

    if (context->priv().caps()->requireOrderedRecordings()) {
        orderedRecorder = context->makeRecorder();
        RecorderOptions opts;
        opts.fRequireOrderedRecordings = false;
        unorderedRecorder = context->makeRecorder(opts);
    } else {
        RecorderOptions opts;
        opts.fRequireOrderedRecordings = true;
        orderedRecorder = context->makeRecorder(opts);
        unorderedRecorder = context->makeRecorder();
    }

    auto insert = [context](Recording* r) {
        InsertRecordingInfo info;
        info.fRecording = r;
        return context->insertRecording(info);
    };

    std::unique_ptr<Recording> o1 = orderedRecorder->snap();
    std::unique_ptr<Recording> o2 = orderedRecorder->snap();
    std::unique_ptr<Recording> o3 = orderedRecorder->snap();

    std::unique_ptr<Recording> u1 = unorderedRecorder->snap();
    std::unique_ptr<Recording> u2 = unorderedRecorder->snap();

    // Unordered insertion of an unordered Recorder succeeds.
    // NOTE: These Recordings are all out-of-order with respect to orderedRecorder, which had been
    // snapped multiple times before unorderedRecorder. That is always allowed.
    REPORTER_ASSERT(reporter, insert(u2.get()));
    REPORTER_ASSERT(reporter, insert(u1.get()));

    // Unordered insertion of an ordered Recorder fails
    REPORTER_ASSERT(reporter, insert(o1.get())); // succeeds (first insertion)
    REPORTER_ASSERT(reporter, !insert(o3.get())); // fails for out of order
    REPORTER_ASSERT(reporter, insert(o2.get())); // succeeds and recovers
    REPORTER_ASSERT(reporter, insert(o3.get())); // now in order success
}

// Test to make sure inserting a null Recording (e.g. Recorder::snap() failed and the client didn't
// check it) returns the correct invalid status.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(NullRecordingInsertTest, reporter, context,
                                   CtsEnforcement::kNever) {
    struct FinishProcContext {
        skiatest::Reporter* fReporter;
        bool fFinishProcCalled = false;
    } finishProcContext{reporter};

    InsertRecordingInfo info;
    info.fRecording = nullptr; // Set to null to simulate a Recorder::snap() failure.
    info.fFinishedContext = &finishProcContext;
    info.fFinishedProc = [](void* ctx, skgpu::CallbackResult result) {
        FinishProcContext* finishProcContext = static_cast<FinishProcContext*>(ctx);
        REPORTER_ASSERT(finishProcContext->fReporter, result == skgpu::CallbackResult::kFailed);
        finishProcContext->fFinishProcCalled = true;
    };

    InsertStatus status = context->insertRecording(info); // Should not crash on the null Recording!
    REPORTER_ASSERT(reporter, status == InsertStatus::kInvalidRecording);
    REPORTER_ASSERT(reporter, finishProcContext.fFinishProcCalled);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InsertRecordingPendingCountsTest, reporter, context,
                                   CtsEnforcement::kNever) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // 1. Inserting an empty recording first should return zero pending commands and passes.
    std::unique_ptr<Recording> emptyRecording1 = recorder->snap();
    REPORTER_ASSERT(reporter, emptyRecording1);

    InsertStatus status0 = context->insertRecording({emptyRecording1.get()});
    REPORTER_ASSERT(reporter, status0 == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status0.numPendingCommands() == 0);
    REPORTER_ASSERT(reporter, status0.numPendingPasses() == 0);

    // 2. Create a non-empty recording by drawing to a surface.
    SkImageInfo info = SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);
    surface->getCanvas()->clear(SkColors::kRed);

    std::unique_ptr<Recording> nonEmptyRecording = recorder->snap();
    REPORTER_ASSERT(reporter, nonEmptyRecording);

    InsertStatus status1 = context->insertRecording({nonEmptyRecording.get()});
    REPORTER_ASSERT(reporter, status1 == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status1.numPendingCommands() > 0);
    REPORTER_ASSERT(reporter, status1.numPendingPasses() > 0);

    int pendingCommands = status1.numPendingCommands();
    int pendingPasses = status1.numPendingPasses();

    // 3. Inserting an empty recording afterwards should not change the pending commands/passes.
    std::unique_ptr<Recording> emptyRecording2 = recorder->snap();
    REPORTER_ASSERT(reporter, emptyRecording2);

    InsertStatus status2 = context->insertRecording({emptyRecording2.get()});
    REPORTER_ASSERT(reporter, status2 == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status2.numPendingCommands() == pendingCommands);
    REPORTER_ASSERT(reporter, status2.numPendingPasses() == pendingPasses);
}

