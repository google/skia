/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkScaledImageCache.h"

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
        SkScalar scale = 1;
        for (int i = 0; i < CACHE_COUNT; ++i) {
            SkBitmap tmp;
            tmp.allocN32Pixels(1, 1);
            fCache.unlock(fCache.addAndLock(fBM, scale, scale, tmp));
            scale += 1;
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

        SkBitmap tmp;
        // search for a miss (-1 scale)
        for (int i = 0; i < loops; ++i) {
            (void)fCache.findAndLock(fBM, -1, -1, &tmp);
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ImageCacheBench(); )
