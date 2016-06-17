/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkTemplates.h"

#define STR     "Hamburgefons"

enum FontQuality {
    kBW,
    kAA,
    kLCD
};

static const char* fontQualityName(const SkPaint& paint) {
    if (!paint.isAntiAlias()) {
        return "BW";
    }
    if (paint.isLCDRenderText()) {
        return "LCD";
    }
    return "AA";
}

class ShaderMaskBench : public Benchmark {
    SkPaint     fPaint;
    SkString    fText;
    SkString    fName;
    FontQuality fFQ;
public:
    ShaderMaskBench(bool isOpaque, FontQuality fq)  {
        fFQ = fq;
        fText.set(STR);

        fPaint.setAntiAlias(kBW != fq);
        fPaint.setLCDRenderText(kLCD == fq);
        fPaint.setShader(SkShader::MakeColorShader(isOpaque ? 0xFFFFFFFF : 0x80808080));
    }

protected:
    virtual const char* onGetName() {
        fName.printf("shadermask");
        fName.appendf("_%s", fontQualityName(fPaint));
        fName.appendf("_%02X", fPaint.getAlpha());
        return fName.c_str();
    }

    virtual void onDraw(int loops, SkCanvas* canvas) {
        const SkIPoint dim = this->getSize();
        SkRandom rand;

        SkPaint paint(fPaint);
        this->setupPaint(&paint);
        // explicitly need these
        paint.setAlpha(fPaint.getAlpha());
        paint.setAntiAlias(kBW != fFQ);
        paint.setLCDRenderText(kLCD == fFQ);

        const SkScalar x0 = SkIntToScalar(-10);
        const SkScalar y0 = SkIntToScalar(-10);

        paint.setTextSize(SkIntToScalar(12));
        for (int i = 0; i < loops; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
        }

        paint.setTextSize(SkIntToScalar(48));
        for (int i = 0; i < loops / 4 ; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ShaderMaskBench(true,  kBW); )
DEF_BENCH( return new ShaderMaskBench(false, kBW); )
DEF_BENCH( return new ShaderMaskBench(true,  kAA); )
DEF_BENCH( return new ShaderMaskBench(false, kAA); )
DEF_BENCH( return new ShaderMaskBench(true,  kLCD); )
DEF_BENCH( return new ShaderMaskBench(false, kLCD); )
