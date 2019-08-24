/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/effects/SkGradientShader.h"

enum ColorPattern {
    kWhite_ColorPattern,
    kBlue_ColorPattern,
    kOpaqueBitmap_ColorPattern,
    kAlphaBitmap_ColorPattern,
};

static const struct ColorPatternData{
    SkColor         fColor;
    bool            fIsBitmap;
    const char*     fName;
} gColorPatterns[] = {
    // Keep this in same order as ColorPattern enum
    { SK_ColorWHITE, false,  "white"        }, // kWhite_ColorPattern
    { SK_ColorBLUE,  false,  "blue"         }, // kBlue_ColorPattern
    { SK_ColorWHITE, true,   "obaqueBitMap" }, // kOpaqueBitmap_ColorPattern
    { 0x10000000,    true,   "alphaBitmap"  }, // kAlphaBitmap_ColorPattern
};

enum DrawType {
    kRect_DrawType,
    kPath_DrawType,
};

static void makebm(SkBitmap* bm, int w, int h) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkScalar    s = SkIntToScalar(SkMin32(w, h));
    static const SkPoint     kPts0[] = { { 0, 0 }, { s, s } };
    static const SkPoint     kPts1[] = { { s/2, 0 }, { s/2, s } };
    static const SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    static const SkColor kColors0[] = {0x80F00080, 0xF0F08000, 0x800080F0 };
    static const SkColor kColors1[] = {0xF08000F0, 0x8080F000, 0xF000F080 };


    SkPaint     paint;

    paint.setShader(SkGradientShader::MakeLinear(kPts0, kColors0, kPos, SK_ARRAY_COUNT(kColors0),
                                                 SkTileMode::kClamp));
    canvas.drawPaint(paint);
    paint.setShader(SkGradientShader::MakeLinear(kPts1, kColors1, kPos, SK_ARRAY_COUNT(kColors1),
                                                 SkTileMode::kClamp));
    canvas.drawPaint(paint);
}

/**
 * This bench draws a grid of either rects or filled paths, with two alternating color patterns.
 * This color patterns are passed in as enums to the class. The options are:
 *   1) solid white color
 *   2) solid blue color
 *   3) opaque bitmap
 *   4) partial alpha bitmap
 * The same color pattern can be set for both arguments to create a uniform pattern on all draws.
 *
 * The bench is used to test a few things. First it can test any optimizations made for a specific
 * color pattern (for example drawing an opaque bitmap versus one with partial alpha). Also it can
 * be used to test the cost of program switching and/or GrDrawOp combining when alternating between
 * different patterns when on the gpu.
 */
class AlternatingColorPatternBench : public Benchmark {
public:
    enum {
        NX = 5,
        NY = 5,
        NUM_DRAWS = NX * NY,
    };
    sk_sp<SkShader> fBmShader;

    SkPath  fPaths[NUM_DRAWS];
    SkRect  fRects[NUM_DRAWS];
    SkColor fColors[NUM_DRAWS];
    sk_sp<SkShader> fShaders[NUM_DRAWS];

    SkString        fName;
    ColorPatternData    fPattern1;
    ColorPatternData    fPattern2;
    DrawType fDrawType;
    SkBitmap fBmp;


    AlternatingColorPatternBench(ColorPattern pattern1, ColorPattern pattern2, DrawType drawType) {
        fPattern1 = gColorPatterns[pattern1];
        fPattern2 = gColorPatterns[pattern2];
        fName.printf("colorPattern_%s_%s_%s",
                     fPattern1.fName, fPattern2.fName,
                     kRect_DrawType == drawType ? "rect" : "path");
        fDrawType = drawType;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        int w = 40;
        int h = 40;
        makebm(&fBmp, w, h);
        fBmShader = fBmp.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat);
        int offset = 2;
        int count = 0;
        for (int j = 0; j < NY; ++j) {
            for (int i = 0; i < NX; ++i) {
                int x = (w + offset) * i;
                int y = (h * offset) * j;
                if (kRect_DrawType == fDrawType) {
                    fRects[count].setXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                          SkIntToScalar(w), SkIntToScalar(h));
                } else {
                    fPaths[count].moveTo(SkIntToScalar(x), SkIntToScalar(y));
                    fPaths[count].rLineTo(SkIntToScalar(w), 0);
                    fPaths[count].rLineTo(0, SkIntToScalar(h));
                    fPaths[count].rLineTo(SkIntToScalar(-w + 1), 0);
                }
                if (0 == count % 2) {
                    fColors[count]  = fPattern1.fColor;
                    fShaders[count] = fPattern1.fIsBitmap ? fBmShader : nullptr;
                } else {
                    fColors[count]  = fPattern2.fColor;
                    fShaders[count] = fPattern2.fIsBitmap ? fBmShader : nullptr;
                }
                ++count;
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(false);
        paint.setFilterQuality(kLow_SkFilterQuality);

        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < NUM_DRAWS; ++j) {
                paint.setColor(fColors[j]);
                paint.setShader(fShaders[j]);
                if (kRect_DrawType == fDrawType) {
                    canvas->drawRect(fRects[j], paint);
                } else {
                    canvas->drawPath(fPaths[j], paint);
                }
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new AlternatingColorPatternBench(kWhite_ColorPattern,
                                                  kWhite_ColorPattern,
                                                  kPath_DrawType);)
DEF_BENCH(return new AlternatingColorPatternBench(kBlue_ColorPattern,
                                                  kBlue_ColorPattern,
                                                  kPath_DrawType);)
DEF_BENCH(return new AlternatingColorPatternBench(kWhite_ColorPattern,
                                                  kBlue_ColorPattern,
                                                  kPath_DrawType);)

DEF_BENCH(return new AlternatingColorPatternBench(kOpaqueBitmap_ColorPattern,
                                                  kOpaqueBitmap_ColorPattern,
                                                  kPath_DrawType);)
DEF_BENCH(return new AlternatingColorPatternBench(kAlphaBitmap_ColorPattern,
                                                  kAlphaBitmap_ColorPattern,
                                                  kPath_DrawType);)
DEF_BENCH(return new AlternatingColorPatternBench(kOpaqueBitmap_ColorPattern,
                                                  kAlphaBitmap_ColorPattern,
                                                  kPath_DrawType);)

DEF_BENCH(return new AlternatingColorPatternBench(kOpaqueBitmap_ColorPattern,
                                                  kOpaqueBitmap_ColorPattern,
                                                  kRect_DrawType);)
DEF_BENCH(return new AlternatingColorPatternBench(kAlphaBitmap_ColorPattern,
                                                  kAlphaBitmap_ColorPattern,
                                                  kRect_DrawType);)
DEF_BENCH(return new AlternatingColorPatternBench(kOpaqueBitmap_ColorPattern,
                                                  kAlphaBitmap_ColorPattern,
                                                  kRect_DrawType);)
