/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/RecorderPriv.h"

using namespace skgpu::graphite;
using Mipmapped = skgpu::Mipmapped;

// Tests to make sure the managing of back pointers between Recorder and Device all work properly.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderDevicePtrTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
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
    recorder->priv().flushTrackedDevices(); // should delete device4 now
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
