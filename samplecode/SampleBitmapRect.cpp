/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkOSFile.h"
#include "SkStream.h"

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
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                   SkShader::kClamp_TileMode))->unref();
    canvas.drawCircle(SCALAR_SIZE/2, SCALAR_SIZE/2, SCALAR_SIZE/2, paint);
}

static SkPoint unit_vec(int degrees) {
    SkScalar rad = SkDegreesToRadians(SkIntToScalar(degrees));
    SkScalar s, c;
    s = SkScalarSinCos(rad, &c);
    return SkPoint::Make(c, s);
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

class BitmapRectView : public SampleView {
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
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "BitmapRect");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

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

    bool onAnimate(const SkAnimTimer& timer) override {
        if (timer.isStopped()) {
            this->resetBounce();
        } else if (timer.isRunning()) {
            this->bounce();
        }
        return true;
    }

private:
    typedef SampleView INHERITED;
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

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(SkIntToScalar(BIG_H));

    const int BIG_W = SkScalarRoundToInt(paint.measureText(gText, strlen(gText)));

    bm->allocN32Pixels(BIG_W, BIG_H);
    bm->eraseColor(SK_ColorWHITE);

    SkCanvas canvas(*bm);

    canvas.drawText(gText, strlen(gText), 0, paint.getTextSize()*4/5, paint);
}

class BitmapRectView2 : public SampleView {
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
    BitmapRectView2() {
        make_big_bitmap(&fBitmap);

        this->setBGColor(SK_ColorGRAY);

        this->resetBounce();

        fLimitR.iset(0, 0, fBitmap.width(), fBitmap.height());

        fDstR[0] = SkRect::MakeXYWH(20, 20, 600, 200);
        fDstR[1] = fDstR[0];
        fDstR[1].offset(0, fDstR[0].height() * 5/4);
    }

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "BigBitmapRect");
            return true;
        }
        return this->INHERITED::onQuery(evt);
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

    bool onAnimate(const SkAnimTimer& timer) override {
        if (timer.isStopped()) {
            this->resetBounce();
        } else if (timer.isRunning()) {
            this->bounceMe();
        }
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* F0() { return new BitmapRectView; }
static SkView* F1() { return new BitmapRectView2; }
static SkViewRegister gR0(F0);
static SkViewRegister gR1(F1);
