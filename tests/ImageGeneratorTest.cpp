/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkGraphics.h"
#include "SkImageGenerator.h"
#include "Test.h"

static bool gMyFactoryWasCalled;

static std::unique_ptr<SkImageGenerator> my_factory(sk_sp<SkData>) {
    gMyFactoryWasCalled = true;
    return nullptr;
}

static void test_imagegenerator_factory(skiatest::Reporter* reporter) {
    // just need a non-empty data to test things
    sk_sp<SkData> data(SkData::MakeWithCString("test_imagegenerator_factory"));

    gMyFactoryWasCalled = false;

    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    std::unique_ptr<SkImageGenerator> gen = SkImageGenerator::MakeFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    // Test is racy, in that it hopes no other thread is changing this global...
    auto prev = SkGraphics::SetImageGeneratorFromEncodedDataFactory(my_factory);
    gen = SkImageGenerator::MakeFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, gMyFactoryWasCalled);
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(prev);
}

class MyImageGenerator : public SkImageGenerator {
public:
    MyImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageGenerator, reporter) {
    MyImageGenerator ig;
    SkYUVSizeInfo sizeInfo;
    sizeInfo.fSizes[SkYUVSizeInfo::kY] = SkISize::Make(200, 200);
    sizeInfo.fSizes[SkYUVSizeInfo::kU] = SkISize::Make(100, 100);
    sizeInfo.fSizes[SkYUVSizeInfo::kV] = SkISize::Make( 50,  50);
    sizeInfo.fWidthBytes[SkYUVSizeInfo::kY] = 0;
    sizeInfo.fWidthBytes[SkYUVSizeInfo::kU] = 0;
    sizeInfo.fWidthBytes[SkYUVSizeInfo::kV] = 0;
    void* planes[3] = { nullptr };
    SkYUVColorSpace colorSpace;

    // Check that the YUV decoding API does not cause any crashes
    ig.queryYUV8(&sizeInfo, nullptr);
    ig.queryYUV8(&sizeInfo, &colorSpace);
    sizeInfo.fWidthBytes[SkYUVSizeInfo::kY] = 250;
    sizeInfo.fWidthBytes[SkYUVSizeInfo::kU] = 250;
    sizeInfo.fWidthBytes[SkYUVSizeInfo::kV] = 250;
    int dummy;
    planes[SkYUVSizeInfo::kY] = planes[SkYUVSizeInfo::kU] = planes[SkYUVSizeInfo::kV] = &dummy;
    ig.getYUV8Planes(sizeInfo, planes);

    // Suppressed due to https://code.google.com/p/skia/issues/detail?id=4339
    if (false) {
        test_imagegenerator_factory(reporter);
    }
}
