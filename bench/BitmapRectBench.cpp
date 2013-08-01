
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkRandom.h"
#include "SkString.h"

static void draw_into_bitmap(const SkBitmap& bm) {
    const int w = bm.width();
    const int h = bm.height();

    SkCanvas canvas(bm);
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorRED);
    canvas.drawCircle(SkIntToScalar(w)/2, SkIntToScalar(h)/2,
                      SkIntToScalar(SkMin32(w, h))*3/8, p);

    SkRect r;
    r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SkIntToScalar(4));
    p.setColor(SK_ColorBLUE);
    canvas.drawRect(r, p);
}

/*  Variants for bitmaprect
    src : entire bitmap, subset, fractional subset
    dst : same size as src, diff size
    paint : filter-p
 */

class BitmapRectBench : public SkBenchmark {
    SkBitmap    fBitmap;
    bool        fDoFilter;
    bool        fSlightMatrix;
    uint8_t     fAlpha;
    SkString    fName;
    SkRect      fSrcR, fDstR;
    static const int kWidth = 128;
    static const int kHeight = 128;
    enum { N = SkBENCHLOOP(300) };
public:
    BitmapRectBench(void* param, U8CPU alpha, bool doFilter, bool slightMatrix) : INHERITED(param) {
        fAlpha = SkToU8(alpha);
        fDoFilter = doFilter;
        fSlightMatrix = slightMatrix;

        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, kWidth, kHeight);
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        fName.printf("bitmaprect_%02X_%sfilter_%s",
                     fAlpha, fDoFilter ? "" : "no",
                     fSlightMatrix ? "trans" : "identity");
        return fName.c_str();
    }

    virtual void onPreDraw() SK_OVERRIDE {
        fBitmap.allocPixels();
        fBitmap.setIsOpaque(true);
        fBitmap.eraseColor(SK_ColorBLACK);
        draw_into_bitmap(fBitmap);

        fSrcR.iset(0, 0, kWidth, kHeight);
        fDstR.iset(0, 0, kWidth, kHeight);

        if (fSlightMatrix) {
            // want fractional translate
            fDstR.offset(SK_Scalar1 / 3, SK_Scalar1 * 5 / 7);
            // want enough to create a scale matrix, but not enough to scare
            // off our sniffer which tries to see if the matrix is "effectively"
            // translate-only.
            fDstR.fRight += SK_Scalar1 / (kWidth * 60);
        }
    }


    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkRandom rand;

        SkPaint paint;
        this->setupPaint(&paint);
        paint.setFilterBitmap(fDoFilter);
        paint.setAlpha(fAlpha);

        for (int i = 0; i < N; i++) {
            canvas->drawBitmapRectToRect(fBitmap, &fSrcR, fDstR, &paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH(return new BitmapRectBench(p, 0xFF, false, false))
DEF_BENCH(return new BitmapRectBench(p, 0x80, false, false))
DEF_BENCH(return new BitmapRectBench(p, 0xFF, true, false))
DEF_BENCH(return new BitmapRectBench(p, 0x80, true, false))

DEF_BENCH(return new BitmapRectBench(p, 0xFF, false, true))
DEF_BENCH(return new BitmapRectBench(p, 0xFF, true, true))
