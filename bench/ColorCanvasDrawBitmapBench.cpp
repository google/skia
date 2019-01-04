/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkString.h"

class ColorCanvasDrawBitmap : public Benchmark {
public:
    ColorCanvasDrawBitmap(sk_sp<SkColorSpace> src, sk_sp<SkColorSpace> dst, const char* name)
        : fDst(dst)
        , fName(SkStringPrintf("ColorCanvasDrawBitmap_%s", name))
    {
        fBitmap.allocPixels(SkImageInfo::MakeN32(100, 100, kOpaque_SkAlphaType, src));
        fBitmap.eraseColor(SK_ColorBLUE);
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(100, 100);
    }

    bool isSuitableFor(Backend backend) override {
        return kRaster_Backend == backend || kGPU_Backend == backend;
    }

    void onDraw(int n, SkCanvas* canvas) override {
        // This simulates an Android app that draws bitmaps to a software canvas.
        // Chrome and the Android framework both use SkImages exclusively.
        std::unique_ptr<SkCanvas> colorCanvas = SkCreateColorSpaceXformCanvas(canvas, fDst);
        for (int i = 0; i < n; i++) {
            colorCanvas->drawBitmap(fBitmap, 0, 0, nullptr);
        }
    }

private:
    sk_sp<SkColorSpace> fDst;
    SkString            fName;
    SkBitmap            fBitmap;

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ColorCanvasDrawBitmap(nullptr, SkColorSpace::MakeSRGB(), "null_to_sRGB");)
DEF_BENCH(return new ColorCanvasDrawBitmap(SkColorSpace::MakeSRGB(), SkColorSpace::MakeSRGB(),
        "sRGB_to_sRGB");)
DEF_BENCH(return new ColorCanvasDrawBitmap(
        SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma, SkColorSpace::kAdobeRGB_Gamut),
        SkColorSpace::MakeSRGB(), "AdobeRGB_to_sRGB");)
