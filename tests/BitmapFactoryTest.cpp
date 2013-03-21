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
#include "SkPurgeableImageCache.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "Test.h"

static SkBitmap* create_bitmap() {
    SkBitmap* bm = SkNEW(SkBitmap);
    // Use a large bitmap.
    const int W = 1000, H = 1000;
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

static void test_cache(skiatest::Reporter* reporter, SkImageCache* cache) {
    // Test the cache directly:
    cache->purgeAllUnpinnedCaches();
    intptr_t ID = SkImageCache::UNINITIALIZED_ID;
    const size_t size = 1000;
    char buffer[size];
    sk_bzero((void*) buffer, size);
    void* memory = cache->allocAndPinCache(size, &ID);
    if (memory != NULL) {
        memcpy(memory, (void*)buffer, size);
        REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID) == SkImageCache::kPinned_MemoryStatus);
        cache->releaseCache(ID);
        REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID) != SkImageCache::kPinned_MemoryStatus);
        SkImageCache::DataStatus dataStatus;
        memory = cache->pinCache(ID, &dataStatus);
        if (memory != NULL) {
            REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID)
                                      == SkImageCache::kPinned_MemoryStatus);
            if (SkImageCache::kRetained_DataStatus == dataStatus) {
                REPORTER_ASSERT(reporter, !memcmp(memory, (void*) buffer, size));
            }
            cache->releaseCache(ID);
            REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID)
                                      != SkImageCache::kPinned_MemoryStatus);
            cache->purgeAllUnpinnedCaches();
            REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID)
                                      != SkImageCache::kPinned_MemoryStatus);
            memory = cache->pinCache(ID, &dataStatus);
            if (memory != NULL) {
                // The memory block may or may not have survived the purging (at the
                // memory manager's whim) so we cannot check dataStatus here.
                cache->releaseCache(ID);
                REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID)
                                          != SkImageCache::kPinned_MemoryStatus);
                cache->throwAwayCache(ID);
                REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID)
                                          == SkImageCache::kFreed_MemoryStatus);
            } else {
                REPORTER_ASSERT(reporter, cache->getMemoryStatus(ID)
                                          == SkImageCache::kFreed_MemoryStatus);
            }
        }
    }
}

static void test_factory(skiatest::Reporter* reporter, SkImageCache* cache, SkData* encodedData,
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
        intptr_t cacheID = lazyRef->getCacheId();
        REPORTER_ASSERT(reporter, cache->getMemoryStatus(cacheID)
                                  != SkImageCache::kPinned_MemoryStatus);
        {
            SkAutoLockPixels alp(*bitmapFromFactory.get());
            REPORTER_ASSERT(reporter, bitmapFromFactory->readyToDraw());
            cacheID = lazyRef->getCacheId();
            REPORTER_ASSERT(reporter, cache->getMemoryStatus(cacheID)
                                      == SkImageCache::kPinned_MemoryStatus);
        }
        REPORTER_ASSERT(reporter, !bitmapFromFactory->readyToDraw());
        REPORTER_ASSERT(reporter, cache->getMemoryStatus(cacheID)
                                  != SkImageCache::kPinned_MemoryStatus);
        bitmapFromFactory.free();
        REPORTER_ASSERT(reporter, cache->getMemoryStatus(cacheID)
                                  == SkImageCache::kFreed_MemoryStatus);
    }
}

class ImageCacheHolder : public SkNoncopyable {

public:
    ~ImageCacheHolder() {
        fCaches.safeUnrefAll();
    }

    void addImageCache(SkImageCache* cache) {
        SkSafeRef(cache);
        *fCaches.append() = cache;
    }

    int count() const { return fCaches.count(); }

    SkImageCache* getAt(int i) {
        if (i < 0 || i > fCaches.count()) {
            return NULL;
        }
        return fCaches.getAt(i);
    }

private:
    SkTDArray<SkImageCache*> fCaches;
};

static void TestBitmapFactory(skiatest::Reporter* reporter) {
    SkAutoTDelete<SkBitmap> bitmap(create_bitmap());
    SkASSERT(bitmap.get() != NULL);

    SkAutoDataUnref encodedBitmap(create_data_from_bitmap(*bitmap.get()));
    bool encodeSucceeded = encodedBitmap.get() != NULL;
    SkASSERT(encodeSucceeded);

    ImageCacheHolder cacheHolder;

    SkAutoTUnref<SkLruImageCache> lruCache(SkNEW_ARGS(SkLruImageCache, (1024 * 1024)));
    cacheHolder.addImageCache(lruCache);

    cacheHolder.addImageCache(NULL);

    SkImageCache* purgeableCache = SkPurgeableImageCache::Create();
    if (purgeableCache != NULL) {
        cacheHolder.addImageCache(purgeableCache);
        purgeableCache->unref();
    }

    for (int i = 0; i < cacheHolder.count(); i++) {
        SkImageCache* cache = cacheHolder.getAt(i);
        if (cache != NULL) {
            test_cache(reporter, cache);
        }
        if (encodeSucceeded) {
            test_factory(reporter, cache, encodedBitmap, *bitmap.get());
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BitmapFactory", TestBitmapFactoryClass, TestBitmapFactory)

#endif // SK_DEBUG
