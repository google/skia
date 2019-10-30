/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

class DrawLatticeBench : public Benchmark {
public:
    DrawLatticeBench(int* xDivs, int xCount, int* yDivs, int yCount, const SkISize& srcSize,
                     const SkRect& dst, const char* desc)
        : fSrcSize(srcSize)
        , fDst(dst)
    {
        fLattice.fXDivs = xDivs;
        fLattice.fXCount = xCount;
        fLattice.fYDivs = yDivs;
        fLattice.fYCount = yCount;
        fLattice.fRectTypes = nullptr;
        fLattice.fBounds = nullptr;
        fLattice.fColors = nullptr;

        fName = SkStringPrintf("DrawLattice_%s", desc);
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(1000, 1000);
    }

    bool isSuitableFor(Backend backend) override {
        return kRaster_Backend == backend || kGPU_Backend == backend;
    }

    void onDelayedSetup() override {
        fBitmap.allocN32Pixels(fSrcSize.width(), fSrcSize.height());
        fBitmap.eraseColor(0x880000FF);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            canvas->drawBitmapLattice(fBitmap, fLattice, fDst);
        }
    }

private:
    SkISize           fSrcSize;
    SkCanvas::Lattice fLattice;
    SkRect            fDst;
    SkString          fName;
    SkBitmap          fBitmap;

    typedef Benchmark INHERITED;
};

static int gDivs9[2] = { 25, 75, };
DEF_BENCH(return new DrawLatticeBench(gDivs9, 2, gDivs9, 2, SkISize::Make(100, 100),
                                      SkRect::MakeWH(250.0f, 250.0f), "Src100_Dst250_Rects9");)
DEF_BENCH(return new DrawLatticeBench(gDivs9, 2, gDivs9, 2, SkISize::Make(100, 100),
                                      SkRect::MakeWH(500.0f, 500.0f), "Src100_Dst500_Rects9");)
DEF_BENCH(return new DrawLatticeBench(gDivs9, 2, gDivs9, 2, SkISize::Make(100, 100),
                                      SkRect::MakeWH(1000.0f, 1000.0f), "Src100_Dst1000_Rects9");)
static int gDivs15[4] = { 15, 45, 55, 85, };
DEF_BENCH(return new DrawLatticeBench(gDivs15, 4, gDivs15, 4, SkISize::Make(100, 100),
                                      SkRect::MakeWH(250.0f, 250.0f), "Src100_Dst250_Rects15");)
DEF_BENCH(return new DrawLatticeBench(gDivs15, 4, gDivs15, 4, SkISize::Make(100, 100),
                                      SkRect::MakeWH(500.0f, 500.0f), "Src100_Dst500_Rects15");)
DEF_BENCH(return new DrawLatticeBench(gDivs15, 4, gDivs15, 4, SkISize::Make(100, 100),
                                      SkRect::MakeWH(1000.0f, 1000.0f), "Src100_Dst1000_Rects15");)
