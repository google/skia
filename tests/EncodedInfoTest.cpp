/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <cstdint>
#include <memory>

DEF_TEST(AlphaEncodedInfo, r) {
    auto codec = SkCodec::MakeFromStream(GetResourceAsStream("images/grayscale.jpg"));
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kGray_8_SkColorType);

    SkBitmap bm;
    bm.allocPixels(codec->getInfo().makeColorType(kAlpha_8_SkColorType).makeColorSpace(nullptr));
    auto result = codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    sk_sp<SkData> data = SkPngEncoder::Encode(bm.pixmap(), {});
    REPORTER_ASSERT(r, data);
    REPORTER_ASSERT(r, data->size() > 0);

    codec = SkCodec::MakeFromData(data);
    REPORTER_ASSERT(r, codec);
    // TODO: Make SkEncodedInfo public and compare to its version of kAlpha_8.
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kAlpha_8_SkColorType);

    SkBitmap bm2;
    bm2.allocPixels(codec->getInfo().makeColorSpace(nullptr));
    result = codec->getPixels(bm2.pixmap());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    REPORTER_ASSERT(r, ToolUtils::equal_pixels(bm.pixmap(), bm2.pixmap()));
}

#if defined(SK_CODEC_ENCODES_PNG_WITH_LIBPNG) && \
        (defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG) || defined(SK_CODEC_DECODES_PNG_WITH_RUST))
DEF_TEST(AlphaEncodedInfo_IncompleteDecode, r) {
    constexpr int kWidth = 16;
    constexpr int kHeight = 64;
    SkBitmap src;
    src.allocPixels(SkImageInfo::MakeA8(kWidth, kHeight));
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            *src.getAddr8(x, y) = static_cast<uint8_t>((x * 17 + y * 31) | 1);
        }
    }

    // Store uncompressed so truncating at half the byte size reliably lands mid-image.
    SkPngEncoder::Options encoderOptions;
    encoderOptions.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
    encoderOptions.fZLibLevel = 0;
    sk_sp<SkData> data = SkPngEncoder::Encode(src.pixmap(), encoderOptions);
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    sk_sp<SkData> truncated = SkData::MakeWithoutCopy(data->bytes(), data->size() / 2);
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(truncated);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kAlpha_8_SkColorType);

    SkBitmap dst;
    dst.allocPixels(codec->getInfo().makeColorSpace(nullptr));
    dst.eraseColor(SK_ColorWHITE);

    SkCodec::Result result = codec->getPixels(dst.pixmap());
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput || result == SkCodec::kErrorInInput);

    int rowsDecoded = 0;
    while (rowsDecoded < kHeight && *dst.getAddr8(0, rowsDecoded) != 0) {
        ++rowsDecoded;
    }
    REPORTER_ASSERT(r, rowsDecoded > 0 && rowsDecoded < kHeight);

    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            const uint8_t expected = (y < rowsDecoded) ? *src.getAddr8(x, y) : 0;
            REPORTER_ASSERT(r, *dst.getAddr8(x, y) == expected);
        }
    }
}
#endif
