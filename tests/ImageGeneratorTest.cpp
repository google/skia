/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkYUVAIndex.h"
#include "include/private/SkImageInfoPriv.h"
#include "tests/Test.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include "include/ports/SkImageGeneratorCG.h"
#elif defined(SK_BUILD_FOR_WIN)
    #include "include/ports/SkImageGeneratorWIC.h"
#endif

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

    // This just verifies that the signatures match.
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(SkImageGeneratorCG::MakeFromEncodedCG);
#elif defined(SK_BUILD_FOR_WIN)
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(SkImageGeneratorWIC::MakeFromEncodedWIC);
#endif

    SkGraphics::SetImageGeneratorFromEncodedDataFactory(prev);
}

class MyImageGenerator : public SkImageGenerator {
public:
    MyImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageGenerator, reporter) {
    MyImageGenerator ig;
    SkYUVASizeInfo sizeInfo;
    sizeInfo.fSizes[0] = SkISize::Make(200, 200);
    sizeInfo.fSizes[1] = SkISize::Make(100, 100);
    sizeInfo.fSizes[2] = SkISize::Make( 50,  50);
    sizeInfo.fSizes[3] = SkISize::Make( 25,  25);
    sizeInfo.fWidthBytes[0] = 0;
    sizeInfo.fWidthBytes[1] = 0;
    sizeInfo.fWidthBytes[2] = 0;
    sizeInfo.fWidthBytes[3] = 0;
    void* planes[4] = { nullptr };
    SkYUVAIndex yuvaIndices[4];
    SkYUVColorSpace colorSpace;

    // Check that the YUV decoding API does not cause any crashes
    ig.queryYUVA8(&sizeInfo, yuvaIndices, nullptr);
    ig.queryYUVA8(&sizeInfo, yuvaIndices, &colorSpace);
    sizeInfo.fWidthBytes[0] = 250;
    sizeInfo.fWidthBytes[1] = 250;
    sizeInfo.fWidthBytes[2] = 250;
    sizeInfo.fWidthBytes[3] = 250;
    yuvaIndices[0] = { 0, SkColorChannel::kR };
    yuvaIndices[1] = { 1, SkColorChannel::kR };
    yuvaIndices[2] = { 2, SkColorChannel::kR };
    yuvaIndices[3] = { 3, SkColorChannel::kR };
    int dummy;
    planes[0] = planes[1] = planes[2] = planes[3] = &dummy;
    ig.getYUVA8Planes(sizeInfo, yuvaIndices, planes);

    // Suppressed due to https://code.google.com/p/skia/issues/detail?id=4339
    if (false) {
        test_imagegenerator_factory(reporter);
    }
}

#include "include/core/SkPictureRecorder.h"
#include "src/core/SkAutoMalloc.h"

static sk_sp<SkPicture> make_picture() {
    SkPictureRecorder recorder;
    recorder.beginRecording(100, 100)->drawColor(SK_ColorRED);
    return recorder.finishRecordingAsPicture();
}

DEF_TEST(PictureImageGenerator, reporter) {
    const struct {
        SkColorType fColorType;
        SkAlphaType fAlphaType;
    } recs[] = {
        { kRGBA_8888_SkColorType, kPremul_SkAlphaType },
        { kBGRA_8888_SkColorType, kPremul_SkAlphaType },
        { kRGBA_F16_SkColorType,  kPremul_SkAlphaType },
        { kRGBA_F32_SkColorType,  kPremul_SkAlphaType },
        { kRGBA_1010102_SkColorType, kPremul_SkAlphaType },

        { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType },
        { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType },
        { kRGBA_F16_SkColorType,  kUnpremul_SkAlphaType },
        { kRGBA_F32_SkColorType,  kUnpremul_SkAlphaType },
        { kRGBA_1010102_SkColorType, kUnpremul_SkAlphaType },
    };

    auto colorspace = SkColorSpace::MakeSRGB();
    auto picture = make_picture();
    auto gen = SkImageGenerator::MakeFromPicture({100, 100}, picture, nullptr, nullptr,
                                                 SkImage::BitDepth::kU8, colorspace);

    // worst case for all requests
    SkAutoMalloc storage(100 * 100 * SkColorTypeBytesPerPixel(kRGBA_F32_SkColorType));

    for (const auto& rec : recs) {
        SkImageInfo info = SkImageInfo::Make(100, 100, rec.fColorType, rec.fAlphaType, colorspace);
        REPORTER_ASSERT(reporter, gen->getPixels(info, storage.get(), info.minRowBytes()));
    }
}

