 /*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkDiscardableMemory.h"
#include "SkScaledImageCache.h"

static void make_bm(SkBitmap* bm, int w, int h) {
    bm->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bm->allocPixels();
}

static const int COUNT = 10;
static const int DIM = 256;

static void test_cache(skiatest::Reporter* reporter, SkScaledImageCache& cache,
                       bool testPurge) {
    SkScaledImageCache::ID* id;

    SkBitmap bm[COUNT];

    const SkScalar scale = 2;
    for (int i = 0; i < COUNT; ++i) {
        make_bm(&bm[i], DIM, DIM);
    }
    
    for (int i = 0; i < COUNT; ++i) {
        SkBitmap tmp;

        SkScaledImageCache::ID* id = cache.findAndLock(bm[i], scale, scale, &tmp);
        REPORTER_ASSERT(reporter, NULL == id);

        make_bm(&tmp, DIM, DIM);
        id = cache.addAndLock(bm[i], scale, scale, tmp);
        REPORTER_ASSERT(reporter, NULL != id);

        SkBitmap tmp2;
        SkScaledImageCache::ID* id2 = cache.findAndLock(bm[i], scale, scale,
                                                        &tmp2);
        REPORTER_ASSERT(reporter, id == id2);
        REPORTER_ASSERT(reporter, tmp.pixelRef() == tmp2.pixelRef());
        REPORTER_ASSERT(reporter, tmp.width() == tmp2.width());
        REPORTER_ASSERT(reporter, tmp.height() == tmp2.height());
        cache.unlock(id2);

        cache.unlock(id);
    }

    if (testPurge) {
        // stress test, should trigger purges
        float incScale = 2;
        for (size_t i = 0; i < COUNT * 100; ++i) {
            incScale += 1;

            SkBitmap tmp;
            make_bm(&tmp, DIM, DIM);

            SkScaledImageCache::ID* id = cache.addAndLock(bm[0], incScale,
                                                          incScale, tmp);
            REPORTER_ASSERT(reporter, NULL != id);
            cache.unlock(id);
        }
    }

    // test the originals after all that purging
    for (int i = 0; i < COUNT; ++i) {
        SkBitmap tmp;
        id = cache.findAndLock(bm[i], scale, scale, &tmp);
        if (id) {
            cache.unlock(id);
        }
    }

    cache.setByteLimit(0);
}

#include "SkDiscardableMemoryPool.h"

static SkDiscardableMemoryPool* gPool;
static SkDiscardableMemory* pool_factory(size_t bytes) {
    return gPool->create(bytes);
}

DEF_TEST(ImageCache, reporter) {
    static const size_t defLimit = DIM * DIM * 4 * COUNT + 1024;    // 1K slop

    {
        SkScaledImageCache cache(defLimit);
        test_cache(reporter, cache, true);
    }
    {
        SkDiscardableMemoryPool pool(defLimit);
        gPool = &pool;
        SkScaledImageCache cache(pool_factory);
        test_cache(reporter, cache, true);
    }
    {
        SkScaledImageCache cache(SkDiscardableMemory::Create);
        test_cache(reporter, cache, false);
    }
}

DEF_TEST(ImageCache_doubleAdd, r) {
    // Adding the same key twice should be safe.
    SkScaledImageCache cache(1024);

    SkBitmap original;
    original.setConfig(SkBitmap::kARGB_8888_Config, 40, 40);
    original.allocPixels();

    SkBitmap scaled;
    scaled.setConfig(SkBitmap::kARGB_8888_Config, 20, 20);
    scaled.allocPixels();

    SkScaledImageCache::ID* id1 = cache.addAndLock(original, 0.5f, 0.5f, scaled);
    SkScaledImageCache::ID* id2 = cache.addAndLock(original, 0.5f, 0.5f, scaled);
    // We don't really care if id1 == id2 as long as unlocking both works.
    cache.unlock(id1);
    cache.unlock(id2);
}
