
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
    bool        fDoPos;
    SkPoint*    fPos;
public:
    TextBench(const char text[], int ps,
              SkColor color, FontQuality fq, bool doPos = false)  {
        fPos = NULL;
        fFQ = fq;
        fDoPos = doPos;
        fText.set(text);

        fPaint.setAntiAlias(kBW != fq);
        fPaint.setLCDRenderText(kLCD == fq);
        fPaint.setTextSize(SkIntToScalar(ps));
        fPaint.setColor(color);

        if (doPos) {
            size_t len = strlen(text);
            SkScalar* adv = new SkScalar[len];
            fPaint.getTextWidths(text, len, adv);
            fPos = new SkPoint[len];
            SkScalar x = 0;
            for (size_t i = 0; i < len; ++i) {
                fPos[i].set(x, SkIntToScalar(50));
                x += adv[i];
            }
            delete[] adv;
        }
    }

    virtual ~TextBench() {
        delete[] fPos;
    }

protected:
    virtual const char* onGetName() {
        fName.printf("text_%g", SkScalarToFloat(fPaint.getTextSize()));
        if (fDoPos) {
            fName.append("_pos");
        }
        fName.appendf("_%s", fontQualityName(fPaint));
        if (SK_ColorBLACK != fPaint.getColor()) {
            fName.appendf("_%02X", fPaint.getAlpha());
        } else {
            fName.append("_BK");
        }
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) {
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

        if (fDoPos) {
            // realistically, the matrix is often at least translated, so we
            // do that since it exercises different code in drawPosText.
            canvas->translate(SK_Scalar1, SK_Scalar1);
        }

        for (int i = 0; i < loops; i++) {
            if (fDoPos) {
                canvas->drawPosText(fText.c_str(), fText.size(), fPos, paint);
            } else {
                SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
                SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
                canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#define STR     "Hamburgefons"

DEF_BENCH( return new TextBench(STR, 16, 0xFF000000, kBW); )
DEF_BENCH( return new TextBench(STR, 16, 0xFFFF0000, kBW); )
DEF_BENCH( return new TextBench(STR, 16, 0x88FF0000, kBW); )

DEF_BENCH( return new TextBench(STR, 16, 0xFF000000, kAA); )
DEF_BENCH( return new TextBench(STR, 16, 0xFFFF0000, kAA); )
DEF_BENCH( return new TextBench(STR, 16, 0x88FF0000, kAA); )

DEF_BENCH( return new TextBench(STR, 16, 0xFF000000, kLCD); )
DEF_BENCH( return new TextBench(STR, 16, 0xFFFF0000, kLCD); )
DEF_BENCH( return new TextBench(STR, 16, 0x88FF0000, kLCD); )

DEF_BENCH( return new TextBench(STR, 16, 0xFF000000, kAA, true); )
