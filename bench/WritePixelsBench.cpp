/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"

// Time variants of write-pixels
//  [ colortype ][ alphatype ][ colorspace ]
//  Different combinations can trigger fast or slow paths in the impls
//
class WritePixelsBench : public Benchmark {
public:
    WritePixelsBench(SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs)
        : fColorType(ct)
        , fAlphaType(at)
        , fCS(cs)
    {
        fName.printf("writepix_%s_%s_%s",
                     at == kPremul_SkAlphaType ? "pm" : "um",
                     ct == kRGBA_8888_SkColorType ? "rgba" : "bgra",
                     cs ? "srgb" : "null");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkISize size = canvas->getBaseLayerSize();

        SkImageInfo info = SkImageInfo::Make(size, fColorType, fAlphaType, fCS);
        SkBitmap bmp;
        bmp.allocPixels(info);
        bmp.eraseColor(SK_ColorBLACK);

        for (int loop = 0; loop < loops; ++loop) {
            canvas->writePixels(info, bmp.getPixels(), bmp.rowBytes(), 0, 0);
        }
    }

private:
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    sk_sp<SkColorSpace> fCS;
    SkString    fName;

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);)
DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr);)
DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());)
DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());)

DEF_BENCH(return new WritePixelsBench(kBGRA_8888_SkColorType, kPremul_SkAlphaType, nullptr);)
DEF_BENCH(return new WritePixelsBench(kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr);)
DEF_BENCH(return new WritePixelsBench(kBGRA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());)
DEF_BENCH(return new WritePixelsBench(kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());)
