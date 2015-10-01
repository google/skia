/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkMipMap.h"

class MipMapBench: public Benchmark {
    SkBitmap fBitmap;

public:
    MipMapBench() {}

protected:
    bool isSuitableFor(Backend backend) override {
        return kNonRendering_Backend == backend;
    }

    const char* onGetName() override { return "mipmap_build"; }

    void onDelayedSetup() override {
        fBitmap.allocN32Pixels(1000, 1000, true);
        fBitmap.eraseColor(SK_ColorWHITE);  // so we don't read uninitialized memory
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            SkMipMap::Build(fBitmap, nullptr)->unref();
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new MipMapBench; )
