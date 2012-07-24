
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkColorShader.h"
#include "SkFontHost.h"
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

class ShaderMaskBench : public SkBenchmark {
    SkPaint     fPaint;
    SkString    fText;
    SkString    fName;
    FontQuality fFQ;
    enum { N = SkBENCHLOOP(500) };
public:
    ShaderMaskBench(void* param, bool isOpaque, FontQuality fq) : INHERITED(param) {
        fFQ = fq;
        fText.set(STR);

        fPaint.setAntiAlias(kBW != fq);
        fPaint.setLCDRenderText(kLCD == fq);
        fPaint.setAlpha(isOpaque ? 0xFF : 0x80);
        fPaint.setShader(new SkColorShader)->unref();
    }

protected:
    virtual const char* onGetName() {
        fName.printf("shadermask", SkScalarToFloat(fPaint.getTextSize()));
        fName.appendf("_%s", fontQualityName(fPaint));
        fName.appendf("_%02X", fPaint.getAlpha());
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
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
        for (int i = 0; i < N; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
        }
        
        paint.setTextSize(SkIntToScalar(48));
        for (int i = 0; i < N/4; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact00(void* p) { return new ShaderMaskBench(p, true,  kBW); }
static SkBenchmark* Fact01(void* p) { return new ShaderMaskBench(p, false, kBW); }
static SkBenchmark* Fact10(void* p) { return new ShaderMaskBench(p, true,  kAA); }
static SkBenchmark* Fact11(void* p) { return new ShaderMaskBench(p, false, kAA); }
static SkBenchmark* Fact20(void* p) { return new ShaderMaskBench(p, true,  kLCD); }
static SkBenchmark* Fact21(void* p) { return new ShaderMaskBench(p, false, kLCD); }

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg10(Fact10);
static BenchRegistry gReg11(Fact11);
static BenchRegistry gReg20(Fact20);
static BenchRegistry gReg21(Fact21);

