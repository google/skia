/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkScaledImageCache.h"

namespace {
static void* gGlobalAddress;
class TestKey : public SkScaledImageCache::Key {
public:
    void*    fPtr;
    intptr_t fValue;

    TestKey(intptr_t value) : fPtr(&gGlobalAddress), fValue(value) {
        this->init(sizeof(fPtr) + sizeof(fValue));
    }
};
}

class ImageCacheBench : public Benchmark {
    SkScaledImageCache  fCache;
    SkBitmap            fBM;

    enum {
        DIM = 1,
        CACHE_COUNT = 500
    };
public:
    ImageCacheBench()  : fCache(CACHE_COUNT * 100) {
        fBM.allocN32Pixels(DIM, DIM);
    }

    void populateCache() {
        for (int i = 0; i < CACHE_COUNT; ++i) {
            TestKey key(i);
            SkBitmap tmp;
            tmp.allocN32Pixels(1, 1);
            fCache.unlock(fCache.addAndLock(key, tmp));
        }
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "imagecache";
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        if (fCache.getTotalBytesUsed() == 0) {
            this->populateCache();
        }

        TestKey key(-1);
        SkBitmap tmp;
        // search for a miss (-1 scale)
        for (int i = 0; i < loops; ++i) {
            SkDEBUGCODE(SkScaledImageCache::ID* id =) fCache.findAndLock(key, &tmp);
            SkASSERT(NULL == id);
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ImageCacheBench(); )
