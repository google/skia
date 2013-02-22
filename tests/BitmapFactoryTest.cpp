/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef SK_DEBUG

#include "SkBitmap.h"
#include "SkBitmapFactory.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkLazyPixelRef.h"
#include "SkLruImageCache.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "Test.h"

#ifdef SK_BUILD_FOR_ANDROID
#include "SkAshmemImageCache.h"
#endif

static SkBitmap* create_bitmap() {
    SkBitmap* bm = SkNEW(SkBitmap);
    const int W = 100, H = 100;
    bm->setConfig(SkBitmap::kARGB_8888_Config, W, H);
    bm->allocPixels();
    bm->eraseColor(SK_ColorBLACK);
    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas.drawRectCoords(0, 0, SkIntToScalar(W/2), SkIntToScalar(H/2), paint);
    return bm;
}

static SkData* create_data_from_bitmap(const SkBitmap& bm) {
    SkDynamicMemoryWStream stream;
    if (SkImageEncoder::EncodeStream(&stream, bm, SkImageEncoder::kPNG_Type, 100)) {
        return stream.copyToData();
    }
    return NULL;
}

static void assert_bounds_equal(skiatest::Reporter* reporter, const SkBitmap& bm1,
                                const SkBitmap& bm2) {
    REPORTER_ASSERT(reporter, bm1.width() == bm2.width());
    REPORTER_ASSERT(reporter, bm1.height() == bm2.height());
}

static void test_cache(skiatest::Reporter* reporter, SkImageCache* cache, SkData* encodedData,
                       const SkBitmap& origBitmap) {
    SkBitmapFactory factory(&SkImageDecoder::DecodeMemoryToTarget);
    factory.setImageCache(cache);
    SkAutoTDelete<SkBitmap> bitmapFromFactory(SkNEW(SkBitmap));
    bool success = factory.installPixelRef(encodedData, bitmapFromFactory.get());
    // This assumes that if the encoder worked, the decoder should also work, so the above call
    // should not fail.
    REPORTER_ASSERT(reporter, success);
    assert_bounds_equal(reporter, origBitmap, *bitmapFromFactory.get());

    SkPixelRef* pixelRef = bitmapFromFactory->pixelRef();
    REPORTER_ASSERT(reporter, pixelRef != NULL);
    if (NULL == cache) {
        // This assumes that installPixelRef called lockPixels.
        REPORTER_ASSERT(reporter, bitmapFromFactory->readyToDraw());
    } else {
        // Lazy decoding
        REPORTER_ASSERT(reporter, !bitmapFromFactory->readyToDraw());
        SkLazyPixelRef* lazyRef = static_cast<SkLazyPixelRef*>(pixelRef);
        int32_t cacheID = lazyRef->getCacheId();
        REPORTER_ASSERT(reporter, cache->getCacheStatus(cacheID)
                                  != SkImageCache::kPinned_CacheStatus);
        {
            SkAutoLockPixels alp(*bitmapFromFactory.get());
            REPORTER_ASSERT(reporter, bitmapFromFactory->readyToDraw());
            cacheID = lazyRef->getCacheId();
            REPORTER_ASSERT(reporter, cache->getCacheStatus(cacheID)
                                      == SkImageCache::kPinned_CacheStatus);
        }
        REPORTER_ASSERT(reporter, !bitmapFromFactory->readyToDraw());
        REPORTER_ASSERT(reporter, cache->getCacheStatus(cacheID)
                                  != SkImageCache::kPinned_CacheStatus);
        bitmapFromFactory.free();
        REPORTER_ASSERT(reporter, cache->getCacheStatus(cacheID)
                                  == SkImageCache::kThrownAway_CacheStatus);
    }
}

static void TestBitmapFactory(skiatest::Reporter* reporter) {
    SkAutoTDelete<SkBitmap> bitmap(create_bitmap());
    SkASSERT(bitmap.get() != NULL);

    SkAutoDataUnref encodedBitmap(create_data_from_bitmap(*bitmap.get()));
    if (encodedBitmap.get() == NULL) {
        // Encoding failed.
        return;
    }

    SkAutoTUnref<SkLruImageCache> lruCache(SkNEW_ARGS(SkLruImageCache, (1024 * 1024)));
    test_cache(reporter, lruCache, encodedBitmap, *bitmap.get());
    test_cache(reporter, NULL, encodedBitmap, *bitmap.get());
#ifdef SK_BUILD_FOR_ANDROID
    test_cache(reporter, SkAshmemImageCache::GetAshmemImageCache(), encodedBitmap, *bitmap.get());
#endif
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BitmapFactory", TestBitmapFactoryClass, TestBitmapFactory)

#endif // SK_DEBUG
