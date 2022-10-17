/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_CODEC_DECODES_AVIF
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

struct AvifTestCase {
    const char* path;
    int imageWidth;
    int imageHeight;
    int bitmapWidth;
    int bitmapHeight;
    int expectedFrameCount;
    int expectedFrameDuration;
    SkColorType color_type;
    SkAlphaType alpha_type;
};

static void run_avif_test(skiatest::Reporter* r, const AvifTestCase& t) {
    auto data = GetResourceAsData(t.path);
    if (!data) {
        ERRORF(r, "failed to find %s", t.path);
        return;
    }

    auto codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        ERRORF(r, "Could not create codec from %s", t.path);
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == t.expectedFrameCount);
    auto info = codec->getInfo();
    REPORTER_ASSERT(r, info.width() == t.imageWidth);
    REPORTER_ASSERT(r, info.height() == t.imageHeight);

    SkImageInfo imageInfo =
            SkImageInfo::Make(t.bitmapWidth, t.bitmapHeight, t.color_type, t.alpha_type);
    SkBitmap bm;
    bm.allocPixels(imageInfo);

    for (int i = 0; i < codec->getFrameCount(); i++) {
        SkCodec::Options options;
        options.fFrameIndex = i;
        auto result = codec->getPixels(imageInfo, bm.getPixels(), bm.rowBytes(), &options);
        if (result != SkCodec::kSuccess) {
            ERRORF(r,
                   "Failed to decode %s frame %i - error %s",
                   t.path,
                   i,
                   SkCodec::ResultToString(result));
            return;
        }

        if (codec->getFrameCount() > 1) {
            SkCodec::FrameInfo frameInfo;
            if (!codec->getFrameInfo(i, &frameInfo)) {
                ERRORF(r, "Failed to getFrameInfo for %s frame %i", t.path, i);
                return;
            }
            REPORTER_ASSERT(r, frameInfo.fAlphaType == t.alpha_type);
            REPORTER_ASSERT(r, frameInfo.fDuration == t.expectedFrameDuration);
        }
    }
}

DEF_TEST(AvifDecodeBasic, r) {
    AvifTestCase t = {.path = "images/dog.avif",
                      .imageWidth = 180,
                      .imageHeight = 180,
                      .bitmapWidth = 180,
                      .bitmapHeight = 180,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecodeOddDimensions, r) {
    AvifTestCase t = {.path = "images/ducky.avif",
                      .imageWidth = 489,
                      .imageHeight = 537,
                      .bitmapWidth = 489,
                      .bitmapHeight = 537,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecodeScaleDown, r) {
    AvifTestCase t = {.path = "images/dog.avif",
                      .imageWidth = 180,
                      .imageHeight = 180,
                      .bitmapWidth = 20,
                      .bitmapHeight = 20,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecode10BitToRGBA8888Bitmap, r) {
    AvifTestCase t = {.path = "images/example_3_10bit.avif",
                      .imageWidth = 512,
                      .imageHeight = 512,
                      .bitmapWidth = 512,
                      .bitmapHeight = 512,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecode10BitToRGBAF16Bitmap, r) {
    AvifTestCase t = {.path = "images/example_3_10bit.avif",
                      .imageWidth = 512,
                      .imageHeight = 512,
                      .bitmapWidth = 512,
                      .bitmapHeight = 512,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_F16_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecode10BitToRGBAF16BitmapDownscale, r) {
    AvifTestCase t = {.path = "images/example_3_10bit.avif",
                      .imageWidth = 512,
                      .imageHeight = 512,
                      .bitmapWidth = 100,
                      .bitmapHeight = 100,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_F16_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecode12BitToRGBA8888Bitmap, r) {
    AvifTestCase t = {.path = "images/example_3_12bit.avif",
                      .imageWidth = 512,
                      .imageHeight = 512,
                      .bitmapWidth = 512,
                      .bitmapHeight = 512,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecode12BitToRGBAF16Bitmap, r) {
    AvifTestCase t = {.path = "images/example_3_12bit.avif",
                      .imageWidth = 512,
                      .imageHeight = 512,
                      .bitmapWidth = 512,
                      .bitmapHeight = 512,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_F16_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecode12BitToRGBAF16BitmapDownscale, r) {
    AvifTestCase t = {.path = "images/example_3_12bit.avif",
                      .imageWidth = 512,
                      .imageHeight = 512,
                      .bitmapWidth = 100,
                      .bitmapHeight = 100,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_F16_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecodeImageWithAlpha, r) {
    AvifTestCase t = {.path = "images/baby_tux.avif",
                      .imageWidth = 240,
                      .imageHeight = 246,
                      .bitmapWidth = 240,
                      .bitmapHeight = 246,
                      .expectedFrameCount = 1,
                      .expectedFrameDuration = 0,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kUnpremul_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecodeAnimation, r) {
    AvifTestCase t = {.path = "images/alphabetAnim.avif",
                      .imageWidth = 100,
                      .imageHeight = 100,
                      .bitmapWidth = 100,
                      .bitmapHeight = 100,
                      .expectedFrameCount = 13,
                      .expectedFrameDuration = 100,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kOpaque_SkAlphaType};
    run_avif_test(r, t);
}

DEF_TEST(AvifDecodeAnimationWithAlpha, r) {
    AvifTestCase t = {.path = "images/example_1_animated.avif",
                      .imageWidth = 256,
                      .imageHeight = 256,
                      .bitmapWidth = 256,
                      .bitmapHeight = 256,
                      .expectedFrameCount = 8,
                      .expectedFrameDuration = 33,
                      .color_type = kRGBA_8888_SkColorType,
                      .alpha_type = kUnpremul_SkAlphaType};
    run_avif_test(r, t);
}

#endif  // SK_CODEC_DECODES_AVIF
