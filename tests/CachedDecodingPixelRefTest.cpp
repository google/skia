/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
#include "SkImagePriv.h"
#include "SkLazyPixelRef.h"
#include "SkScaledImageCache.h"
#include "SkStream.h"
#include "Test.h"

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

/**
 *  This checks to see that a SkLazyPixelRef works as advertized.
 */
#include "TestClassDef.h"
DEF_TEST(CachedDecodingPixelRefTest, reporter) {
    SkBitmap original;
    make_test_image(&original);
    const size_t bitmapSize = original.getSize();
    const size_t oldByteLimit = SkScaledImageCache::GetByteLimit();
    REPORTER_ASSERT(reporter, (!(original.empty())) && (!(original.isNull())));

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
        static const SkBitmapFactory::DecodeProc decoder =
            &(SkImageDecoder::DecodeMemoryToTarget);
        bool success = simple_bitmap_factory(decoder, encoded.get(), &lazy);

        REPORTER_ASSERT(reporter, success);

        size_t bytesUsed = SkScaledImageCache::GetBytesUsed();

        if (oldByteLimit < bitmapSize) {
            SkScaledImageCache::SetByteLimit(bitmapSize + oldByteLimit);
        }
        void* lazyPixels = NULL;

        // Since this is lazy, it shouldn't have fPixels yet!
        REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
        {
            SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
            lazyPixels = lazy.getPixels();
            REPORTER_ASSERT(reporter, NULL != lazy.getPixels());
            // first time we lock pixels, we should get bump in the size
            // of the cache by exactly bitmapSize.
            REPORTER_ASSERT(reporter, bytesUsed + bitmapSize
                            == SkScaledImageCache::GetBytesUsed());
            bytesUsed = SkScaledImageCache::GetBytesUsed();
        }
        // pixels should be gone!
        REPORTER_ASSERT(reporter, NULL == lazy.getPixels());
        {
            SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
            REPORTER_ASSERT(reporter, NULL != lazy.getPixels());

            // verify that the same pixels are used this time.
            REPORTER_ASSERT(reporter, lazy.getPixels() == lazyPixels);
        }

        bool comparePixels = (SkImageEncoder::kPNG_Type == type);
        // Only PNG is pixel-perfect.
        compare_bitmaps(reporter, original, lazy, comparePixels);

        // force the cache to clear by making it too small.
        SkScaledImageCache::SetByteLimit(bitmapSize / 2);
        compare_bitmaps(reporter, original, lazy, comparePixels);

        // I'm pretty sure that the logic of the cache should mean
        // that it will clear to zero, regardless of where it started.
        REPORTER_ASSERT(reporter, SkScaledImageCache::GetBytesUsed() == 0);
        // TODO(someone) - write a custom allocator that can verify
        // that the memory where those pixels were cached really did
        // get freed.

        ////////////////////////////////////////////////////////////////////////
        //  The following commented-out code happens to work on my
        //  machine, and indicates to me that the SkLazyPixelRef is
        //  behaving as designed.  But I don't know an easy way to
        //  guarantee that a second allocation of the same size will
        //  give a different address.
        ////////////////////////////////////////////////////////////////////////
        // {
        //     // confuse the heap allocation system
        //     SkAutoMalloc autoMalloc(bitmapSize);
        //     REPORTER_ASSERT(reporter, autoMalloc.get() == lazyPixels);
        //     {
        //         SkAutoLockPixels autoLockPixels(lazy);
        //         // verify that *different* pixels are used this time.
        //         REPORTER_ASSERT(reporter, lazy.getPixels() != lazyPixels);
        //         compare_bitmaps(reporter, original, lazy, comparePixels);
        //     }
        // }

        // restore cache size
        SkScaledImageCache::SetByteLimit(oldByteLimit);
    }
}
