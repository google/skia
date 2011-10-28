
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkSfntUtils.h"
#include "SkString.h"
#include "SkTemplates.h"

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

/*  Some considerations for performance:
        short -vs- long strings (measuring overhead)
        tiny -vs- large pointsize (measure blit -vs- overhead)
        1 -vs- many point sizes (measure cache lookup)
        normal -vs- subpixel -vs- lineartext (minor)
        force purge after each draw to measure scaler
        textencoding?
        text -vs- postext - pathtext
 */
class TextBench : public SkBenchmark {
    SkPaint     fPaint;
    SkString    fText;
    SkString    fName;
    FontQuality fFQ;
    enum { N = SkBENCHLOOP(800) };
public:
    TextBench(void* param, const char text[], int ps,
              SkColor color, FontQuality fq) : INHERITED(param) {
        fFQ = fq;
        fText.set(text);

        fPaint.setAntiAlias(kBW != fq);
        fPaint.setLCDRenderText(kLCD == fq);
        fPaint.setTextSize(SkIntToScalar(ps));
        fPaint.setColor(color);
    }

protected:
    virtual const char* onGetName() {
        fName.printf("text_%g", SkScalarToFloat(fPaint.getTextSize()));
        fName.appendf("_%s", fontQualityName(fPaint));
        if (SK_ColorBLACK != fPaint.getColor()) {
            fName.appendf("_%02X", fPaint.getAlpha());
        } else {
            fName.appendf("_BK");
        }
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        const SkIPoint dim = this->getSize();
        SkRandom rand;

        SkPaint paint(fPaint);
        this->setupPaint(&paint);
        // explicitly need these
        paint.setColor(fPaint.getColor());
        paint.setAntiAlias(kBW != fFQ);
        paint.setLCDRenderText(kLCD == fFQ);

        const SkScalar x0 = SkIntToScalar(-10);
        const SkScalar y0 = SkIntToScalar(-10);

        for (int i = 0; i < N; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#define STR     "Hamburgefons"

static SkBenchmark* Fact01(void* p) { return new TextBench(p, STR, 16, 0xFF000000, kBW); }
static SkBenchmark* Fact02(void* p) { return new TextBench(p, STR, 16, 0xFFFF0000, kBW); }
static SkBenchmark* Fact03(void* p) { return new TextBench(p, STR, 16, 0x88FF0000, kBW); }

static SkBenchmark* Fact11(void* p) { return new TextBench(p, STR, 16, 0xFF000000, kAA); }
static SkBenchmark* Fact12(void* p) { return new TextBench(p, STR, 16, 0xFFFF0000, kAA); }
static SkBenchmark* Fact13(void* p) { return new TextBench(p, STR, 16, 0x88FF0000, kAA); }

static SkBenchmark* Fact21(void* p) { return new TextBench(p, STR, 16, 0xFF000000, kLCD); }
static SkBenchmark* Fact22(void* p) { return new TextBench(p, STR, 16, 0xFFFF0000, kLCD); }
static SkBenchmark* Fact23(void* p) { return new TextBench(p, STR, 16, 0x88FF0000, kLCD); }

static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);
static BenchRegistry gReg03(Fact03);

static BenchRegistry gReg11(Fact11);
static BenchRegistry gReg12(Fact12);
static BenchRegistry gReg13(Fact13);

static BenchRegistry gReg21(Fact21);
static BenchRegistry gReg22(Fact22);
static BenchRegistry gReg23(Fact23);

