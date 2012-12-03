
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

static void drawIntoBitmap(const SkBitmap& bm) {
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
    uint8_t     fAlpha;
    SkString    fName;
    SkRect      fSrcR, fDstR;
    enum { N = SkBENCHLOOP(300) };
public:
    BitmapRectBench(void* param, U8CPU alpha, bool doFilter) : INHERITED(param) {
        fAlpha = SkToU8(alpha);
        fDoFilter = doFilter;

        const int w = 128;
        const int h = 128;

        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        fBitmap.allocPixels();
        fBitmap.setIsOpaque(true);
        fBitmap.eraseColor(SK_ColorBLACK);
        drawIntoBitmap(fBitmap);

        fSrcR.iset(0, 0, w, h);
        fDstR.iset(0, 0, w, h);
    }

protected:
    virtual const char* onGetName() {
        fName.printf("bitmaprect_%02X_%sfilter", fAlpha, fDoFilter ? "" : "no");
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkIPoint dim = this->getSize();
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

DEF_BENCH(return new BitmapRectBench(p, 0xFF, false))
DEF_BENCH(return new BitmapRectBench(p, 0x80, false))
DEF_BENCH(return new BitmapRectBench(p, 0xFF, true))
DEF_BENCH(return new BitmapRectBench(p, 0x80, true))

