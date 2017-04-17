/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"

class FontScalerBench : public Benchmark {
    SkString fName;
    SkString fText;
    bool     fDoLCD;
public:
    FontScalerBench(bool doLCD)  {
        fName.printf("fontscaler_%s", doLCD ? "lcd" : "aa");
        fText.set("abcdefghijklmnopqrstuvwxyz01234567890");
        fDoLCD = doLCD;
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }
    virtual void onDraw(int loops, SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setLCDRenderText(fDoLCD);

        for (int i = 0; i < loops; i++) {
            // this is critical - we want to time the creation process, so we
            // explicitly flush our cache before each run
            SkGraphics::PurgeFontCache();

            for (int ps = 9; ps <= 24; ps += 2) {
                paint.setTextSize(SkIntToScalar(ps));
                canvas->drawString(fText,
                        0, SkIntToScalar(20), paint);
            }
        }
    }
private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new FontScalerBench(false);)
DEF_BENCH(return new FontScalerBench(true);)
