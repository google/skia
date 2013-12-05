/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkImageDecoder.h"
#include "SkImagePriv.h"
#include "SkLazyPixelRef.h"
#include "SkCachingPixelRef.h"
#include "SkScaledImageCache.h"
#include "SkStream.h"

#include "Test.h"
#include "TestClassDef.h"

/**
 * Fill this bitmap with some color.
 */
static void make_test_image(SkBitmap* bm) {
    static const int W = 50, H = 50;
    static const SkBitmap::Config config = SkBitmap::kARGB_8888_Config;
    bm->setConfig(config, W, H);
    bm->allocPixels();
    bm->eraseColor(SK_ColorBLACK);
    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas.drawRectCoords(0, 0, SkIntToScalar(W/2),
                          SkIntToScalar(H/2), paint);
    paint.setColor(SK_ColorWHITE);
    canvas.drawRectCoords(SkIntToScalar(W/2), SkIntToScalar(H/2),
                          SkIntToScalar(W), SkIntToScalar(H), paint);
}

/**
 * encode this bitmap into some data via SkImageEncoder
 */
static SkData* create_data_from_bitmap(const SkBitmap& bm,
                                       SkImageEncoder::Type type) {
    SkDynamicMemoryWStream stream;
    if (SkImageEncoder::EncodeStream(&stream, bm, type, 100)) {
        return stream.copyToData();
    }
    return NULL;
}

/**
 *  A simplified version of SkBitmapFactory
 */
static bool simple_bitmap_factory(SkBitmapFactory::DecodeProc proc,
                                  SkData* data,
                                  SkBitmap* dst) {
    SkImageInfo info;
    if (!proc(data->data(), data->size(), &info, NULL)) {
        return false;
    }
    dst->setConfig(SkImageInfoToBitmapConfig(info), info.fWidth,
                    info.fHeight, 0, info.fAlphaType);
    SkAutoTUnref<SkLazyPixelRef> ref(SkNEW_ARGS(SkLazyPixelRef,
                                                (data, proc, NULL)));
    dst->setPixelRef(ref);
    return true;
}

static void compare_bitmaps(skiatest::Reporter* reporter,
                            const SkBitmap& b1, const SkBitmap& b2,
                            bool pixelPerfect = true) {
    REPORTER_ASSERT(reporter, b1.empty() == b2.empty());
    REPORTER_ASSERT(reporter, b1.width() == b2.width());
    REPORTER_ASSERT(reporter, b1.height() == b2.height());
    REPORTER_ASSERT(reporter, b1.isNull() == b2.isNull());
    SkAutoLockPixels autoLockPixels1(b1);
    SkAutoLockPixels autoLockPixels2(b2);
    REPORTER_ASSERT(reporter, b1.isNull() == b2.isNull());
    if (b1.isNull() || b1.empty()) {
        return;
    }
    REPORTER_ASSERT(reporter, NULL != b1.getPixels());
    REPORTER_ASSERT(reporter, NULL != b2.getPixels());
    if ((!(b1.getPixels())) || (!(b2.getPixels()))) {
        return;
    }
    if ((b1.width() != b2.width()) ||
        (b1.height() != b2.height())) {
        return;
    }
    if (!pixelPerfect) {
        return;
    }

    int pixelErrors = 0;
    for (int y = 0; y < b2.height(); ++y) {
        for (int x = 0; x < b2.width(); ++x) {
            if (b1.getColor(x, y) != b2.getColor(x, y)) {
                ++pixelErrors;
            }
        }
    }
    REPORTER_ASSERT(reporter, 0 == pixelErrors);
}


typedef bool (*InstallEncoded)(SkData* encoded, SkBitmap* dst);

/**
   This function tests three differently encoded images against the
   original bitmap */
static void test_three_encodings(skiatest::Reporter* reporter,
                                 InstallEncoded install) {
    SkBitmap original;
    make_test_image(&original);
    REPORTER_ASSERT(reporter, !original.empty());
    REPORTER_ASSERT(reporter, !original.isNull());
    if (original.empty() || original.isNull()) {
        return;
    }
    static const SkImageEncoder::Type types[] = {
        SkImageEncoder::kPNG_Type,
        SkImageEncoder::kJPEG_Type,
        SkImageEncoder::kWEBP_Type
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(types); i++) {
        SkImageEncoder::Type type = types[i];
        SkAutoDataUnref encoded(create_data_from_bitmap(original, type));
        REPORTER_ASSERT(reporter, encoded.get() != NULL);
        if (NULL == encoded.get()) {
            continue;
        }
        SkBitmap lazy;
        bool installSuccess = install(encoded.get(), &lazy);
        REPORTER_ASSERT(reporter, installSuccess);
        if (!installSuccess) {
            continue;
        }
        REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
        {
            SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
            REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
            if (NULL == lazy.getPixels()) {
                continue;
            }
        }
        // pixels should be gone!
        REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
        {
            SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
            REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
            if (NULL == lazy.getPixels()) {
                continue;
            }
        }
        bool comparePixels = (SkImageEncoder::kPNG_Type == type);
        compare_bitmaps(reporter, original, lazy, comparePixels);
    }
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  This checks to see that a SkLazyPixelRef works as advertised.
 */
static bool install_skLazyPixelRef(SkData* encoded, SkBitmap* dst) {
    static const SkBitmapFactory::DecodeProc decoder =
        &(SkImageDecoder::DecodeMemoryToTarget);
    return simple_bitmap_factory(decoder, encoded, dst);
}
DEF_TEST(LazyPixelRef, reporter) {
    test_three_encodings(reporter, install_skLazyPixelRef);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  This checks to see that a SkCachingPixelRef works as advertised.
 */
static bool install_skCachingPixelRef(SkData* encoded, SkBitmap* dst) {
    return SkCachingPixelRef::Install(
        SkNEW_ARGS(SkDecodingImageGenerator, (encoded)), dst);
}
DEF_TEST(CachingPixelRef, reporter) {
    test_three_encodings(reporter, install_skCachingPixelRef);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  This checks to see that a SkDecodingImageGenerator works as advertised.
 */
DEF_TEST(DecodingImageGenerator, reporter) {
    test_three_encodings(reporter, SkDecodingImageGenerator::Install);
}
