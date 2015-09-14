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

static SkImageGenerator* my_factory(SkData*) {
    gMyFactoryWasCalled = true;
    return nullptr;
}

static void test_imagegenerator_factory(skiatest::Reporter* reporter) {
    // just need a non-empty data to test things
    SkAutoTUnref<SkData> data(SkData::NewWithCString("test_imagegenerator_factory"));

    gMyFactoryWasCalled = false;

    SkImageGenerator* gen;
    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    gen = SkImageGenerator::NewFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    // Test is racy, in that it hopes no other thread is changing this global...
    SkGraphics::ImageGeneratorFromEncodedFactory prev =
                                    SkGraphics::SetImageGeneratorFromEncodedFactory(my_factory);
    gen = SkImageGenerator::NewFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, gMyFactoryWasCalled);
    SkGraphics::SetImageGeneratorFromEncodedFactory(prev);
}

class MyImageGenerator : public SkImageGenerator {
public:
    MyImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageGenerator, reporter) {
    MyImageGenerator ig;
    SkISize sizes[3];
    sizes[0] = SkISize::Make(200, 200);
    sizes[1] = SkISize::Make(100, 100);
    sizes[2] = SkISize::Make( 50,  50);
    void*   planes[3] = { nullptr };
    size_t  rowBytes[3] = { 0 };
    SkYUVColorSpace colorSpace;

    // Check that the YUV decoding API does not cause any crashes
    ig.getYUV8Planes(sizes, nullptr, nullptr, &colorSpace);
    ig.getYUV8Planes(sizes, nullptr, nullptr, nullptr);
    ig.getYUV8Planes(sizes, planes, nullptr, nullptr);
    ig.getYUV8Planes(sizes, nullptr, rowBytes, nullptr);
    ig.getYUV8Planes(sizes, planes, rowBytes, nullptr);
    ig.getYUV8Planes(sizes, planes, rowBytes, &colorSpace);

    int dummy;
    planes[0] = planes[1] = planes[2] = &dummy;
    rowBytes[0] = rowBytes[1] = rowBytes[2] = 250;

    ig.getYUV8Planes(sizes, planes, rowBytes, &colorSpace);

    // Suppressed due to https://code.google.com/p/skia/issues/detail?id=4339
    if (false) {
        test_imagegenerator_factory(reporter);
    }
}
