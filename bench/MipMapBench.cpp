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
    SkString fName;
    const int fW, fH;

public:
    MipMapBench(int w, int h) : fW(w), fH(h) {
        fName.printf("mipmap_build_%dx%d", w, h);
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return kNonRendering_Backend == backend;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        fBitmap.allocN32Pixels(fW, fH, true);
        fBitmap.eraseColor(SK_ColorWHITE);  // so we don't read uninitialized memory
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops * 4; i++) {
            SkMipMap::Build(fBitmap, nullptr)->unref();
        }
    }

private:
    typedef Benchmark INHERITED;
};

// Build variants that exercise the width and heights being even or odd at each level, as the
// impl specializes on each of these.
//
DEF_BENCH( return new MipMapBench(511, 511); )
DEF_BENCH( return new MipMapBench(512, 511); )
DEF_BENCH( return new MipMapBench(511, 512); )
DEF_BENCH( return new MipMapBench(512, 512); )
