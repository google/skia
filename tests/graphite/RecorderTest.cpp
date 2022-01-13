/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/Device.h"

using namespace skgpu;

// Tests to make sure the managing of back pointers between Recorder and Device all work properly.
DEF_GRAPHITE_TEST_FOR_CONTEXTS(RecorderDevicePtrTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkImageInfo info = SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<Device> device1 = Device::Make(recorder.get(), info);

    REPORTER_ASSERT(reporter, device1->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, recorder->deviceIsRegistered(device1.get()));

    Device* devPtr = device1.get();
    device1.reset();
    REPORTER_ASSERT(reporter, !recorder->deviceIsRegistered(devPtr));

    // Test adding multiple devices
    device1 = Device::Make(recorder.get(), info);
    sk_sp<Device> device2 = Device::Make(recorder.get(), info);
    sk_sp<Device> device3 = Device::Make(recorder.get(), info);
    REPORTER_ASSERT(reporter, device1->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device2->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device3->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, recorder->deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->deviceIsRegistered(device2.get()));
    REPORTER_ASSERT(reporter, recorder->deviceIsRegistered(device3.get()));

    // Test freeing a device in the middle.
    devPtr = device2.get();
    device2.reset();
    REPORTER_ASSERT(reporter, recorder->deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, !recorder->deviceIsRegistered(devPtr));
    REPORTER_ASSERT(reporter, recorder->deviceIsRegistered(device3.get()));

    // Delete the recorder and make sure remaining devices not longer have a valid recorder.
    recorder.reset();
    REPORTER_ASSERT(reporter, device1->recorder() == nullptr);
    REPORTER_ASSERT(reporter, device3->recorder() == nullptr);

    // Make sure freeing Devices after recorder doesn't cause any crash. This would get checked
    // naturually when these devices go out of scope, but manually reseting will give us a better
    // stack trace if something does go wrong.
    device1.reset();
    device3.reset();
}
