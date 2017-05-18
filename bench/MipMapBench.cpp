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
    SkDestinationSurfaceColorMode fColorMode;
    bool fHalfFoat;

public:
    MipMapBench(int w, int h, SkDestinationSurfaceColorMode colorMode, bool halfFloat = false)
        : fW(w), fH(h), fColorMode(colorMode), fHalfFoat(halfFloat)
    {
        fName.printf("mipmap_build_%dx%d_%d_gamma", w, h, static_cast<int>(colorMode));
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
        SkImageInfo info = fHalfFoat ? SkImageInfo::Make(fW, fH, kRGBA_F16_SkColorType,
                                                         kPremul_SkAlphaType,
                                                         SkColorSpace::MakeSRGBLinear())
                                     : SkImageInfo::MakeS32(fW, fH, kPremul_SkAlphaType);
        fBitmap.allocPixels(info);
        fBitmap.eraseColor(SK_ColorWHITE);  // so we don't read uninitialized memory
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops * 4; i++) {
            SkMipMap::Build(fBitmap, fColorMode, nullptr)->unref();
        }
    }

private:
    typedef Benchmark INHERITED;
};

// Build variants that exercise the width and heights being even or odd at each level, as the
// impl specializes on each of these.
//
DEF_BENCH( return new MipMapBench(511, 511, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(512, 511, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(511, 512, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(512, 512, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(512, 512,
                                  SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware); )
DEF_BENCH( return new MipMapBench(511, 511,
                                  SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware); )
DEF_BENCH( return new MipMapBench(512, 512, SkDestinationSurfaceColorMode::kLegacy, true); )
DEF_BENCH( return new MipMapBench(511, 511, SkDestinationSurfaceColorMode::kLegacy, true); )
DEF_BENCH( return new MipMapBench(2048, 2048, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(2048, 2048,
                                  SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware); )
DEF_BENCH( return new MipMapBench(2047, 2047, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(2047, 2047,
                                  SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware); )
DEF_BENCH( return new MipMapBench(2048, 2047, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(2048, 2047,
                                  SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware); )
DEF_BENCH( return new MipMapBench(2047, 2048, SkDestinationSurfaceColorMode::kLegacy); )
DEF_BENCH( return new MipMapBench(2047, 2048,
                                  SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware); )
