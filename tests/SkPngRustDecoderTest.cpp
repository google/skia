/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustDecoder.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>
#include <utility>

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    actualResult == SkCodec::kSuccess,           \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkPngRustDecoder::Decode`.
std::unique_ptr<SkCodec> SkPngRustDecoderDecode(skiatest::Reporter* r, const char* path) {
    sk_sp<SkData> data(GetResourceAsData(path));
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

void AssertGreenPixel(skiatest::Reporter* r, const SkImage& image, int x, int y) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);

    REPORTER_ASSERT(r, x < image.width(), "x=%d >= width=%d", x, image.width());
    REPORTER_ASSERT(r, y < image.height(), "y=%d >= height=%d", y, image.height());
    REPORTER_ASSERT(r,
                    kN32_SkColorType == image.colorType(),
                    "kN32_SkColorType != image.ColorType()=%d",
                    image.colorType());

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image.peekPixels(&pixmap));

    SkColor actualColor = pixmap.getColor(x, y);
    constexpr SkColor kGreenColor = SkColorSetRGB(0x00, 0xFF, 0x00);
    REPORTER_ASSERT(r,
                    actualColor == kGreenColor,
                    "actualColor=0x%08X != kGreenColor=0x%08X",
                    actualColor,
                    kGreenColor);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-ignoring-default-image
//
// The input file contains the following PNG chunks: IHDR, acTL, IDAT, fcTL,
// fdAT, IEND.  Presence of acTL chunk + no fcTL chunk before IDAT means that
// the IDAT chunk is *not* part of the animation (i.e. APNG-aware decoders
// should ignore IDAT frame and start with fdAT frame).  This test will fail
// with non-APNG-aware decoders (e.g. with `SkPngCodec`), because the `IDAT`
// chunk represents a red image (the `fdAT` chunk represents a green image).
DEF_TEST(Codec_apng_basic_ignoring_default_image, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--basic--ignoring-default-image.png");
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    REPORTER_ASSERT(r, codec->getRepetitionCount() == 0);

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 128, "width %d != 128", image->width());
    REPORTER_ASSERT(r, image->height() == 64, "height %d != 64", image->height());
    AssertGreenPixel(r, *image, 0, 0);
}
