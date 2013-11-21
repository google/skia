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
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
#include "SkImagePriv.h"
#include "SkLazyCachingPixelRef.h"
#include "SkLazyPixelRef.h"
#include "SkScaledImageCache.h"
#include "SkStream.h"

#include "Test.h"
#include "TestClassDef.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

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


typedef void(*CompareEncodedToOriginal)(skiatest::Reporter* reporter,
                                        SkData* encoded,
                                        const SkBitmap& original,
                                        bool pixelPerfect);
/**
   this function tests three differently encoded images against the
   original bitmap  */
static void test_three_encodings(skiatest::Reporter* reporter,
                                 CompareEncodedToOriginal comp) {
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
        if (NULL != encoded.get()) {
            bool comparePixels = (SkImageEncoder::kPNG_Type == type);
            comp(reporter, encoded, original, comparePixels);
        }
    }
}

/**
 *  This checks to see that a SkLazyPixelRef works as advertised.
 */
static void compare_with_skLazyPixelRef(skiatest::Reporter* reporter,
                                        SkData* encoded,
                                        const SkBitmap& original,
                                        bool comparePixels) {
    SkBitmap lazy;
    static const SkBitmapFactory::DecodeProc decoder =
        &(SkImageDecoder::DecodeMemoryToTarget);
    bool success = simple_bitmap_factory(decoder, encoded, &lazy);
    REPORTER_ASSERT(reporter, success);

    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
    {
        SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
        REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
    }
    // pixels should be gone!
    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
    {
        SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
        REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
    }
    compare_bitmaps(reporter, original, lazy, comparePixels);
}
DEF_TEST(LazyPixelRef, reporter) {
    test_three_encodings(reporter, compare_with_skLazyPixelRef);
}



/**
 *  This checks to see that a SkLazyCachedPixelRef works as advertised.
 */

static void compare_with_skLazyCachedPixelRef(skiatest::Reporter* reporter,
                                              SkData* encoded,
                                              const SkBitmap& original,
                                              bool comparePixels) {
    SkBitmap lazy;
    static const SkBitmapFactory::DecodeProc decoder =
        &(SkImageDecoder::DecodeMemoryToTarget);
    bool success = SkLazyCachingPixelRef::Install(decoder, encoded, &lazy);
    REPORTER_ASSERT(reporter, success);

    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
    {
        SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
        REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
    }
    // pixels should be gone!
    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
    {
        SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
        REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
    }
    compare_bitmaps(reporter, original, lazy, comparePixels);
}
DEF_TEST(LazyCachedPixelRef, reporter) {
    test_three_encodings(reporter, compare_with_skLazyCachedPixelRef);
}

class TestPixelRef : public SkCachingPixelRef {
public:
    TestPixelRef(int x) : fX(x) { }
    virtual ~TestPixelRef() { }
    static bool Install(SkBitmap* destination, int x) {
        SkAutoTUnref<TestPixelRef> ref(SkNEW_ARGS(TestPixelRef, (x)));
        return ref->configure(destination) && destination->setPixelRef(ref);
    }
    SK_DECLARE_UNFLATTENABLE_OBJECT()
protected:
    virtual bool onDecodeInfo(SkImageInfo* info) SK_OVERRIDE {
        if (fX == 0) {
            return false;
        }
        SkASSERT(info);
        info->fWidth = 10;
        info->fHeight = 10;
        info->fColorType = kRGBA_8888_SkColorType;
        info->fAlphaType = kOpaque_SkAlphaType;
        return true;
    }
    virtual bool onDecodePixels(const SkImageInfo& info,
                                void* pixels,
                                size_t rowBytes) SK_OVERRIDE {
        return false;
    }
private:
    int fX;  // controls where the failure happens
    typedef SkCachingPixelRef INHERITED;
};

DEF_TEST(CachingPixelRef, reporter) {
    SkBitmap lazy;
    // test the error handling
    REPORTER_ASSERT(reporter, !TestPixelRef::Install(&lazy, 0));
    // onDecodeInfo should succeed, allowing installation
    REPORTER_ASSERT(reporter, TestPixelRef::Install(&lazy, 1));
    SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
    // onDecodePixels should fail, so getting pixels will fail.
    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
}

static void compare_with_SkDecodingImageGenerator(skiatest::Reporter* reporter,
                                                  SkData* encoded,
                                                  const SkBitmap& original,
                                                  bool comparePixels) {

    SkBitmap lazy;
    bool success = SkDecodingImageGenerator::Install(encoded, &lazy);
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }

    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
    {
        SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
        REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
        if (NULL == lazy.getPixels()) {
            return;
        }
    }
    // pixels should be gone!
    REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
    {
        SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
        REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
    }
    compare_bitmaps(reporter, original, lazy, comparePixels);
}
DEF_TEST(DecodingImageGenerator, reporter) {
    test_three_encodings(reporter, compare_with_SkDecodingImageGenerator);
}
