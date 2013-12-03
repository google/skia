/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkScaledImageCache.h"

class ImageCacheBench : public SkBenchmark {
    SkScaledImageCache  fCache;
    SkBitmap            fBM;

    enum {
        DIM = 1,
        CACHE_COUNT = 500
    };
public:
    ImageCacheBench()  : fCache(CACHE_COUNT * 100) {
        fBM.setConfig(SkBitmap::kARGB_8888_Config, DIM, DIM);
        fBM.allocPixels();
    }

    void populateCache() {
        SkScalar scale = 1;
        for (int i = 0; i < CACHE_COUNT; ++i) {
            SkBitmap tmp;
            tmp.setConfig(SkBitmap::kARGB_8888_Config, 1, 1);
            tmp.allocPixels();
            fCache.unlock(fCache.addAndLock(fBM, scale, scale, tmp));
            scale += 1;
        }
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "imagecache";
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        if (fCache.getBytesUsed() == 0) {
            this->populateCache();
        }

        SkBitmap tmp;
        // search for a miss (-1 scale)
        for (int i = 0; i < loops; ++i) {
            (void)fCache.findAndLock(fBM, -1, -1, &tmp);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ImageCacheBench(); )
