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
#include "tools/timer/AnimTimer.h"

#include "include/core/SkStream.h"
#include "src/core/SkOSFile.h"

#define INT_SIZE        64
#define SCALAR_SIZE     SkIntToScalar(INT_SIZE)

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

static SkPoint unit_vec(int degrees) {
    SkScalar rad = SkDegreesToRadians(SkIntToScalar(degrees));
    return SkPoint::Make(SkScalarCos(rad), SkScalarSin(rad));
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
    SkPoint fSrcPts[2];
    SkPoint fSrcVec[2];
    SkRect  fSrcLimit;
    SkRect  fDstR[2];

    void bounce() {
        bounce_pt(&fSrcPts[0], &fSrcVec[0], fSrcLimit);
        bounce_pt(&fSrcPts[1], &fSrcVec[1], fSrcLimit);
    }

    void resetBounce() {
        fSrcPts[0].set(0, 0);
        fSrcPts[1].set(SCALAR_SIZE, SCALAR_SIZE);

        fSrcVec[0] = unit_vec(30);
        fSrcVec[1] = unit_vec(107);
    }

public:
    BitmapRectView() {
        this->setBGColor(SK_ColorGRAY);

        this->resetBounce();

        fSrcLimit.set(-SCALAR_SIZE/4, -SCALAR_SIZE/4,
                      SCALAR_SIZE*5/4, SCALAR_SIZE*5/4);

        fDstR[0] = SkRect::MakeXYWH(SkIntToScalar(10), SkIntToScalar(100),
                                       SkIntToScalar(250), SkIntToScalar(300));
        fDstR[1] = fDstR[0];
        fDstR[1].offset(fDstR[0].width() * 5/4, 0);

        fSrcPts[0].set(32, 32);
        fSrcPts[1].set(90, 90);
    }

protected:
    SkString name() override { return SkString("BitmapRect"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkRect srcR;
        srcR.set(fSrcPts[0], fSrcPts[1]);
        srcR = SkRect::MakeXYWH(fSrcPts[0].fX, fSrcPts[0].fY, 32, 32);
        srcR.offset(-srcR.width()/2, -srcR.height()/2);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorYELLOW);

        SkBitmap bitmap;
        make_bitmap(&bitmap);

        canvas->translate(20, 20);

        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->drawRect(srcR, paint);

        for (int i = 0; i < 2; ++i) {
            paint.setFilterQuality(1 == i ? kLow_SkFilterQuality : kNone_SkFilterQuality);
            canvas->drawBitmapRect(bitmap, srcR, fDstR[i], &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
            canvas->drawRect(fDstR[i], paint);
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        if (timer.isStopped()) {
            this->resetBounce();
        } else if (timer.isRunning()) {
            this->bounce();
        }
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static void make_big_bitmap(SkBitmap* bm) {
    static const char gText[] =
        "We the people, in order to form a more perfect union, establish justice,"
        " ensure domestic tranquility, provide for the common defense, promote the"
        " general welfare and ensure the blessings of liberty to ourselves and our"
        " posterity, do ordain and establish this constitution for the United"
        " States of America.";

    const int BIG_H = 120;

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

    SkRect  fSrcR;
    SkRect  fLimitR;
    SkScalar fDX;
    SkRect  fDstR[2];

    void bounceMe() {
        SkScalar width = fSrcR.width();
        bounce(&fSrcR.fLeft, &fDX, fLimitR.fLeft, fLimitR.fRight - width);
        fSrcR.fRight = fSrcR.fLeft + width;
    }

    void resetBounce() {
        fSrcR.iset(0, 0, fBitmap.height() * 3, fBitmap.height());
        fDX = SK_Scalar1;
    }

public:
    BitmapRectView2() { }

protected:
    SkString name() override { return SkString("BigBitmapRect"); }

    void onOnceBeforeDraw() override {
        make_big_bitmap(&fBitmap);

        this->setBGColor(SK_ColorGRAY);

        this->resetBounce();

        fLimitR.iset(0, 0, fBitmap.width(), fBitmap.height());

        fDstR[0] = SkRect::MakeXYWH(20, 20, 600, 200);
        fDstR[1] = fDstR[0];
        fDstR[1].offset(0, fDstR[0].height() * 5/4);
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

    bool onAnimate(const AnimTimer& timer) override {
        if (timer.isStopped()) {
            this->resetBounce();
        } else if (timer.isRunning()) {
            this->bounceMe();
        }
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new BitmapRectView(); )
DEF_SAMPLE( return new BitmapRectView2(); )
