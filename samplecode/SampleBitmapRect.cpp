/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

#include "include/core/SkStream.h"
#include "src/core/SkOSFile.h"

static constexpr int INT_SIZE = 64;
static constexpr float SCALAR_SIZE = (float)INT_SIZE;

static void make_bitmap(SkBitmap* bitmap) {
    bitmap->allocN32Pixels(INT_SIZE, INT_SIZE);
    SkCanvas canvas(*bitmap);

    canvas.drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint pts[] = { { 0, 0 }, { SCALAR_SIZE, SCALAR_SIZE } };
    const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
    canvas.drawCircle(SCALAR_SIZE/2, SCALAR_SIZE/2, SCALAR_SIZE/2, paint);
}

static void bounce(SkScalar* value, SkScalar* delta, SkScalar min, SkScalar max) {
    *value += *delta;
    if (*value < min) {
        *value = min;
        *delta = - *delta;
    } else if (*value > max) {
        *value = max;
        *delta = - *delta;
    }
}

static void bounce_pt(SkPoint* pt, SkVector* vec, const SkRect& limit) {
    bounce(&pt->fX, &vec->fX, limit.fLeft, limit.fRight);
    bounce(&pt->fY, &vec->fY, limit.fTop, limit.fBottom);
}

class BitmapRectView : public Sample {
    SkPoint fSrcPt = {0, 0};
    SkPoint fSrcVec = {0.866025f, 0.5f};

    SkRect  fSrcLimit = {-SCALAR_SIZE/4,  -SCALAR_SIZE/4,
                          SCALAR_SIZE*5/4, SCALAR_SIZE*5/4};
    SkRect  fDstR[2] = {{10, 100, 260, 400}, {322.5, 100, 572.5, 400}};
    SkBitmap fBitmap;

    SkString name() override { return SkString("BitmapRect"); }

    void onOnceBeforeDraw() override {
        this->setBGColor(SK_ColorGRAY);
        make_bitmap(&fBitmap);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkRect srcR = {fSrcPt.fX - 16, fSrcPt.fY - 16,
                       fSrcPt.fX + 16, fSrcPt.fY + 16};

        SkPaint paint(SkColors::kYellow);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(20, 20);

        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->drawRect(srcR, paint);

        for (int i = 0; i < 2; ++i) {
            paint.setFilterQuality(1 == i ? kLow_SkFilterQuality : kNone_SkFilterQuality);
            canvas->drawBitmapRect(fBitmap, srcR, fDstR[i], &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
            canvas->drawRect(fDstR[i], paint);
        }
    }

    bool onAnimate(double nanos) override {
        bounce_pt(&fSrcPt, &fSrcVec, fSrcLimit);
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static constexpr int BIG_H = 120;

static void make_big_bitmap(SkBitmap* bm) {
    static const char gText[] =
        "We the people, in order to form a more perfect union, establish justice,"
        " ensure domestic tranquility, provide for the common defense, promote the"
        " general welfare and ensure the blessings of liberty to ourselves and our"
        " posterity, do ordain and establish this constitution for the United"
        " States of America.";

    SkFont font;
    font.setSize(SkIntToScalar(BIG_H));

    const int BIG_W = SkScalarRoundToInt(font.measureText(gText, strlen(gText), SkTextEncoding::kUTF8));

    bm->allocN32Pixels(BIG_W, BIG_H);
    bm->eraseColor(SK_ColorWHITE);

    SkCanvas canvas(*bm);

    canvas.drawSimpleText(gText, strlen(gText), SkTextEncoding::kUTF8, 0, font.getSize()*4/5, font, SkPaint());
}

class BitmapRectView2 : public Sample {
    SkBitmap fBitmap;
    SkRect   fSrcR = {0, 0, 3 * BIG_H, BIG_H};
    SkRect   fLimitR;
    SkScalar fDX = 1;
    SkRect   fDstR[2] = {{20, 20, 620, 220}, {20, 270, 620, 470}};

    SkString name() override { return SkString("BigBitmapRect"); }

    void onOnceBeforeDraw() override {
        this->setBGColor(SK_ColorGRAY);
        make_big_bitmap(&fBitmap);
        fLimitR = SkRect::Make(fBitmap.dimensions());
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorYELLOW);

        for (int i = 0; i < 2; ++i) {
            paint.setFilterQuality(1 == i ? kLow_SkFilterQuality : kNone_SkFilterQuality);
            canvas->drawBitmapRect(fBitmap, fSrcR, fDstR[i], &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
            canvas->drawRect(fDstR[i], paint);
        }
    }

    bool onAnimate(double nanos) override {
        SkScalar width = fSrcR.width();
        bounce(&fSrcR.fLeft, &fDX, fLimitR.fLeft, fLimitR.fRight - width);
        fSrcR.fRight = fSrcR.fLeft + width;
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new BitmapRectView(); )
DEF_SAMPLE( return new BitmapRectView2(); )
