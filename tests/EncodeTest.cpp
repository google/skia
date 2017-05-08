/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "Test.h"

#include "SkBitmap.h"
#include "SkEncodedImageFormat.h"
#include "SkImage.h"
#include "SkJpegEncoder.h"
#include "SkPngEncoder.h"
#include "SkStream.h"

static bool encode(SkEncodedImageFormat format, SkWStream* dst, const SkPixmap& src) {
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            return SkJpegEncoder::Encode(dst, src, SkJpegEncoder::Options());
        case SkEncodedImageFormat::kPNG:
            return SkPngEncoder::Encode(dst, src, SkPngEncoder::Options());
        default:
            return false;
    }
}

static std::unique_ptr<SkEncoder> make(SkEncodedImageFormat format, SkWStream* dst,
                                       const SkPixmap& src) {
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            return SkJpegEncoder::Make(dst, src, SkJpegEncoder::Options());
        case SkEncodedImageFormat::kPNG:
            return SkPngEncoder::Make(dst, src, SkPngEncoder::Options());
        default:
            return nullptr;
    }
}

static void test_encode(skiatest::Reporter* r, SkEncodedImageFormat format) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst0, dst1, dst2, dst3;
    success = encode(format, &dst0, src);
    REPORTER_ASSERT(r, success);

    auto encoder1 = make(format, &dst1, src);
    for (int i = 0; i < src.height(); i++) {
        success = encoder1->encodeRows(1);
        REPORTER_ASSERT(r, success);
    }

    auto encoder2 = make(format, &dst2, src);
    for (int i = 0; i < src.height(); i+=3) {
        success = encoder2->encodeRows(3);
        REPORTER_ASSERT(r, success);
    }

    auto encoder3 = make(format, &dst3, src);
    success = encoder3->encodeRows(200);
    REPORTER_ASSERT(r, success);

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    sk_sp<SkData> data3 = dst3.detachAsData();
    REPORTER_ASSERT(r, data0->equals(data1.get()));
    REPORTER_ASSERT(r, data0->equals(data2.get()));
    REPORTER_ASSERT(r, data0->equals(data3.get()));
}

DEF_TEST(Encode, r) {
    test_encode(r, SkEncodedImageFormat::kJPEG);
    test_encode(r, SkEncodedImageFormat::kPNG);
}

static inline bool equals(const SkBitmap& a, const SkBitmap& b) {
    if (a.info() != b.info()) {
        return false;
    }

    SkASSERT(kN32_SkColorType == a.colorType());
    for (int y = 0; y < a.height(); y++) {
        for (int x = 0; x < a.width(); x++) {
            if (*a.getAddr32(x, y) != *b.getAddr32(x, y)) {
                return false;
            }
        }
    }

    return true;
}

DEF_TEST(Encode_PngOptions, r) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst0, dst1, dst2;
    SkPngEncoder::Options options;
    success = SkPngEncoder::Encode(&dst0, src, options);
    REPORTER_ASSERT(r, success);

    options.fFilterFlags = SkPngEncoder::FilterFlag::kUp;
    success = SkPngEncoder::Encode(&dst1, src, options);
    REPORTER_ASSERT(r, success);

    options.fZLibLevel = 3;
    success = SkPngEncoder::Encode(&dst2, src, options);
    REPORTER_ASSERT(r, success);

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    REPORTER_ASSERT(r, data0->size() < data1->size());
    REPORTER_ASSERT(r, data1->size() < data2->size());

    SkBitmap bm0, bm1, bm2;
    SkImage::MakeFromEncoded(data0)->asLegacyBitmap(&bm0, SkImage::kRO_LegacyBitmapMode);
    SkImage::MakeFromEncoded(data1)->asLegacyBitmap(&bm1, SkImage::kRO_LegacyBitmapMode);
    SkImage::MakeFromEncoded(data2)->asLegacyBitmap(&bm2, SkImage::kRO_LegacyBitmapMode);
    REPORTER_ASSERT(r, equals(bm0, bm1));
    REPORTER_ASSERT(r, equals(bm0, bm2));
}
