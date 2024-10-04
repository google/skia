/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/base/SkAutoMalloc.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/codec/SkPixmapUtilsPriv.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include "include/ports/SkImageGeneratorCG.h"
#elif defined(SK_BUILD_FOR_WIN)
    #include "include/ports/SkImageGeneratorWIC.h"
    #include "src/utils/win/SkAutoCoInitialize.h"
#elif defined(SK_ENABLE_NDK_IMAGES)
    #include "include/ports/SkImageGeneratorNDK.h"
#endif

static bool nearly_equal(SkColor4f a, SkColor4f b) {
    float d = SkV3{a.fR - b.fR, a.fG - b.fG, a.fB - b.fB}.length();
    static constexpr float kTol = 0.02f;
    return d <= kTol;
}

DEF_TEST(ImageGeneratorOrientationTest, reporter) {
  #if defined(SK_BUILD_FOR_WIN)
    SkAutoCoInitialize com;
    REPORTER_ASSERT(reporter, com.succeeded());
  #endif

  for (char i = '1'; i <= '8'; i++) {
    SkString path =  SkStringPrintf("images/orientation/%c_444.jpg", i);
    // Get the image as data
    sk_sp<SkData> data(GetResourceAsData(path.c_str()));

    std::unique_ptr<SkImageGenerator> gen;
    #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
      gen = SkImageGeneratorCG::MakeFromEncodedCG(data);
    #elif defined(SK_BUILD_FOR_WIN)
      gen = SkImageGeneratorWIC::MakeFromEncodedWIC(data);
    #elif defined(SK_ENABLE_NDK_IMAGES)
      gen = SkImageGeneratorNDK::MakeFromEncodedNDK(data);
    #else
      gen = SkCodecImageGenerator::MakeFromEncodedCodec(data);
    #endif

    REPORTER_ASSERT(reporter, gen);

    bool success;
    // Get bitmap from image generator.
    SkImageInfo genInfo = gen->getInfo();
    SkBitmap genBitmap;
    success = genBitmap.tryAllocPixels(genInfo);
    SkPixmap genPM = genBitmap.pixmap();
    REPORTER_ASSERT(reporter, genPM.addr());
    success = gen->getPixels(genPM);
    REPORTER_ASSERT(reporter, success);

    // Get bitmap from codec. SkCodec::getPixels does not automatically apply
    // the origin so we rotate it manually.
    SkBitmap codecBitmap;
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path.c_str()));
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(reporter, codec);

    SkImageInfo codecInfo = codec->getInfo();
    SkEncodedOrigin origin = codec->getOrigin();

    success = codecBitmap.tryAllocPixels(codecInfo);
    REPORTER_ASSERT(reporter, success);
    SkCodec::Result result = codec->getPixels(codecInfo, codecBitmap.getPixels(),
                                              codecBitmap.rowBytes());
    REPORTER_ASSERT(reporter, result == SkCodec::kSuccess);


    SkBitmap tmpBitmap;
    if (SkEncodedOriginSwapsWidthHeight(origin)) {
      codecInfo = SkPixmapUtils::SwapWidthHeight(codecInfo);
    }
    success = tmpBitmap.tryAllocPixels(codecInfo);
    REPORTER_ASSERT(reporter, success);

    success = SkPixmapUtils::Orient(tmpBitmap.pixmap(), codecBitmap.pixmap(), origin);
    REPORTER_ASSERT(reporter, success);

    codecBitmap.swap(tmpBitmap);

    // Compare the decoded SkCodec and the bits from the SkImageGenerator.
    // They should be the same because both should respect orientation.
    if (codecBitmap.dimensions() != genInfo.dimensions()) {
      ERRORF(reporter, "Bitmaps do not have the same dimentions.\n"
                        "\tgenBitmap: (%i, %i)\tcodecBitmap: (%i, %i)",
                        codecBitmap.dimensions().fWidth, codecBitmap.dimensions().fHeight,
                        genBitmap.dimensions().fWidth, genBitmap.dimensions().fHeight);
    }
      for (int k = 0; k < codecBitmap.width();  ++k) {
        for (int j = 0; j < codecBitmap.height(); ++j) {
            SkColor4f c1 = genBitmap.getColor4f(k, j);
            SkColor4f c2 = codecBitmap.getColor4f(k, j);
            if (!nearly_equal(c1, c2)) {
                ERRORF(reporter, "Bitmaps for %s do not match starting at position %i, %i\n"
                          "\tgenBitmap: %x\tcodecBitmap: %x", path.c_str(), i, j,
                          c1.toSkColor(), c2.toSkColor());
                return;
            }
        }
      }
  }
}
