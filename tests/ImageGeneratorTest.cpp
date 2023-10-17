/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "src/base/SkAutoMalloc.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "tests/Test.h"

#include <memory>

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include "include/ports/SkImageGeneratorCG.h"
#elif defined(SK_BUILD_FOR_WIN)
    #include "include/ports/SkImageGeneratorWIC.h"
#endif

static bool gMyFactoryWasCalled;

// NOLINTNEXTLINE(performance-unnecessary-value-param)
static std::unique_ptr<SkImageGenerator> my_factory(sk_sp<SkData>) {
    gMyFactoryWasCalled = true;
    return nullptr;
}

static void test_imagegenerator_factory(skiatest::Reporter* reporter) {
    // just need a non-empty data to test things
    sk_sp<SkData> data(SkData::MakeWithCString("test_imagegenerator_factory"));

    gMyFactoryWasCalled = false;

    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    std::unique_ptr<SkImageGenerator> gen = SkImageGenerators::MakeFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    // Test is racy, in that it hopes no other thread is changing this global...
    auto prev = SkGraphics::SetImageGeneratorFromEncodedDataFactory(my_factory);
    gen = SkImageGenerators::MakeFromEncoded(data);
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
    SkYUVAPixmapInfo yuvaPixmapInfo;

    // Check that the YUV decoding API does not cause any crashes
    ig.queryYUVAInfo(SkYUVAPixmapInfo::SupportedDataTypes::All(), &yuvaPixmapInfo);
    SkYUVAInfo yuvaInfo({250, 250},
                        SkYUVAInfo::PlaneConfig::kY_UV,
                        SkYUVAInfo::Subsampling::k420,
                        kJPEG_Full_SkYUVColorSpace);
    yuvaPixmapInfo = SkYUVAPixmapInfo(yuvaInfo,
                                      SkYUVAPixmapInfo::DataType::kUnorm8,
                                      /*rowBytes[]*/ nullptr);
    SkYUVAPixmaps yuvaPixmaps = SkYUVAPixmaps::Allocate(yuvaPixmapInfo);
    ig.getYUVAPlanes(yuvaPixmaps);

    // Suppressed due to https://code.google.com/p/skia/issues/detail?id=4339
    if ((false)) {
        test_imagegenerator_factory(reporter);
    }
}

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
    auto gen = SkImageGenerators::MakeFromPicture(
            {100, 100}, picture, nullptr, nullptr, SkImages::BitDepth::kU8, colorspace);

    // worst case for all requests
    SkAutoMalloc storage(100 * 100 * SkColorTypeBytesPerPixel(kRGBA_F32_SkColorType));

    for (const auto& rec : recs) {
        SkImageInfo info = SkImageInfo::Make(100, 100, rec.fColorType, rec.fAlphaType, colorspace);
        REPORTER_ASSERT(reporter, gen->getPixels(info, storage.get(), info.minRowBytes()));
    }
}

