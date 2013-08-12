/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkScaledImageCache.h"

static void make_bm(SkBitmap* bm, int w, int h) {
    bm->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bm->allocPixels();
}

static void TestImageCache(skiatest::Reporter* reporter) {
    static const int COUNT = 10;
    static const int DIM = 256;
    static const size_t defLimit = DIM * DIM * 4 * COUNT + 1024;    // 1K slop
    SkScaledImageCache cache(defLimit);
    SkScaledImageCache::ID* id;

    SkBitmap bm[COUNT];

    SkScalar scale = 2;
    for (int i = 0; i < COUNT; ++i) {
        SkBitmap tmp;

        make_bm(&bm[i], DIM, DIM);
        id = cache.findAndLock(bm[i], scale, scale, &tmp);
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

    // stress test, should trigger purges
    for (size_t i = 0; i < COUNT * 100; ++i) {
        scale += 1;

        SkBitmap tmp;

        make_bm(&tmp, DIM, DIM);
        id = cache.addAndLock(bm[0], scale, scale, tmp);
        REPORTER_ASSERT(reporter, NULL != id);
        cache.unlock(id);
    }

    cache.setByteLimit(0);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ImageCache", TestImageCacheClass, TestImageCache)
