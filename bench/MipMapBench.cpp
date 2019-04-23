/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "src/core/SkMipMap.h"

class MipMapBench: public Benchmark {
    SkBitmap fBitmap;
    SkString fName;
    const int fW, fH;
    bool fHalfFoat;

public:
    MipMapBench(int w, int h, bool halfFloat = false)
        : fW(w), fH(h), fHalfFoat(halfFloat)
    {
        fName.printf("mipmap_build_%dx%d", w, h);
        if (halfFloat) {
            fName.append("_f16");
        }
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return kNonRendering_Backend == backend;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        SkColorType ct = fHalfFoat ? kRGBA_F16_SkColorType : kN32_SkColorType;
        SkImageInfo info = SkImageInfo::Make(fW, fH, ct, kPremul_SkAlphaType,
                                             SkColorSpace::MakeSRGB());
        fBitmap.allocPixels(info);
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

DEF_BENCH( return new MipMapBench(512, 512, true); )
DEF_BENCH( return new MipMapBench(511, 511, true); )

DEF_BENCH( return new MipMapBench(2048, 2048); )
DEF_BENCH( return new MipMapBench(2047, 2047); )
DEF_BENCH( return new MipMapBench(2048, 2047); )
DEF_BENCH( return new MipMapBench(2047, 2048); )
