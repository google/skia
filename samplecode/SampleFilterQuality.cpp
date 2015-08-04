/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkInterpolator.h"
#include "SkPath.h"
#include "SkSurface.h"
#include "SkRandom.h"
#include "SkTime.h"

static SkSurface* make_surface(SkCanvas* canvas, const SkImageInfo& info) {
    SkSurface* surface = canvas->newSurface(info);
    if (!surface) {
        surface = SkSurface::NewRaster(info);
    }
    return surface;
}

#define N   128
#define ANGLE_DELTA 3
#define SCALE_DELTA (SK_Scalar1 / 32)

static SkImage* make_image() {
    SkImageInfo info = SkImageInfo::MakeN32(N, N, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorWHITE);

    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);

    path.addRect(SkRect::MakeWH(N/2, N));
    path.addRect(SkRect::MakeWH(N, N/2));
    path.moveTo(0, 0); path.lineTo(N, 0); path.lineTo(0, N); path.close();

    canvas->drawPath(path, SkPaint());
    return surface->newImageSnapshot();
}

static SkImage* zoom_up(SkImage* orig) {
    const SkScalar S = 8;    // amount to scale up
    const int D = 2;    // dimension scaling for the offscreen
    // since we only view the center, don't need to produce the entire thing
    
    SkImageInfo info = SkImageInfo::MakeN32(orig->width() * D, orig->height() * D,
                                            kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(orig->newSurface(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorWHITE);
    canvas->scale(S, S);
    canvas->translate(-SkScalarHalf(orig->width()) * (S - D) / S,
                      -SkScalarHalf(orig->height()) * (S - D) / S);
    canvas->drawImage(orig, 0, 0, NULL);
    
    if (S > 3) {
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        for (int i = 1; i < orig->height(); ++i) {
            SkScalar y = SkIntToScalar(i);
            canvas->drawLine(0, y, SkIntToScalar(orig->width()), y, paint);
        }
        for (int i = 1; i < orig->width(); ++i) {
            SkScalar x = SkIntToScalar(i);
            canvas->drawLine(x, 0, x, SkIntToScalar(orig->height()), paint);
        }
    }
    return surface->newImageSnapshot();
}

struct AnimValue {
    SkScalar fValue;
    SkScalar fMin;
    SkScalar fMax;
    SkScalar fMod;

    operator SkScalar() const { return fValue; }

    void set(SkScalar value, SkScalar min, SkScalar max) {
        fValue = value;
        fMin = min;
        fMax = max;
        fMod = 0;
    }

    void setMod(SkScalar value, SkScalar mod) {
        fValue = value;
        fMin = 0;
        fMax = 0;
        fMod = mod;
    }

    SkScalar inc(SkScalar delta) {
        fValue += delta;
        return this->fixUp();
    }

    SkScalar fixUp() {
        if (fMod) {
            fValue = SkScalarMod(fValue, fMod);
        } else {
            if (fValue > fMax) {
                fValue = fMax;
            } else if (fValue < fMin) {
                fValue = fMin;
            }
        }
        return fValue;
    }
};

static void draw_box_frame(SkCanvas* canvas, int width, int height) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorRED);
    SkRect r = SkRect::MakeIWH(width, height);
    r.inset(0.5f, 0.5f);
    canvas->drawRect(r, p);
    canvas->drawLine(r.left(), r.top(), r.right(), r.bottom(), p);
    canvas->drawLine(r.left(), r.bottom(), r.right(), r.top(), p);
}

class FilterQualityView : public SampleView {
    SkAutoTUnref<SkImage> fImage;
    AnimValue             fScale, fAngle;
    SkSize              fCell;
    SkInterpolator      fTrans;
    SkMSec              fCurrTime;
    bool                fShowFatBits;

public:
    FilterQualityView() : fImage(make_image()), fTrans(2, 2), fShowFatBits(true) {
        fCell.set(256, 256);

        fScale.set(1, SK_Scalar1 / 8, 1);
        fAngle.setMod(0, 360);

        SkScalar values[2];
        fTrans.setMirror(true);
        fTrans.setReset(true);

        fCurrTime = 0;

        fTrans.setRepeatCount(999);
        values[0] = values[1] = 0;
        fTrans.setKeyFrame(0, fCurrTime, values);
        values[0] = values[1] = 1;
        fTrans.setKeyFrame(1, fCurrTime + 2000, values);
    }

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FilterQuality");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case '1': fAngle.inc(-ANGLE_DELTA); this->inval(NULL); return true;
                case '2': fAngle.inc( ANGLE_DELTA); this->inval(NULL); return true;
                case '3': fScale.inc(-SCALE_DELTA); this->inval(NULL); return true;
                case '4': fScale.inc( SCALE_DELTA); this->inval(NULL); return true;
                case '5': fShowFatBits = !fShowFatBits; this->inval(NULL); return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawTheImage(SkCanvas* canvas, const SkISize& size, SkFilterQuality filter,
                      SkScalar dx, SkScalar dy) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterQuality(filter);

        SkAutoCanvasRestore acr(canvas, true);

        canvas->translate(dx, dy);

        canvas->translate(SkScalarHalf(size.width()), SkScalarHalf(size.height()));
        canvas->scale(fScale, fScale);
        canvas->rotate(fAngle);
        canvas->drawImage(fImage, -SkScalarHalf(fImage->width()), -SkScalarHalf(fImage->height()),
                          &paint);

        if (false) {
            acr.restore();
            draw_box_frame(canvas, size.width(), size.height());
        }
    }

    void drawHere(SkCanvas* canvas, SkFilterQuality filter, SkScalar dx, SkScalar dy) {
        SkCanvas* origCanvas = canvas;
        SkAutoCanvasRestore acr(canvas, true);

        SkISize size = SkISize::Make(fImage->width(), fImage->height());

        SkAutoTUnref<SkSurface> surface;
        if (fShowFatBits) {
            // scale up so we don't clip rotations
            SkImageInfo info = SkImageInfo::MakeN32(fImage->width() * 2, fImage->height() * 2,
                                                    kOpaque_SkAlphaType);
            surface.reset(make_surface(canvas, info));
            canvas = surface->getCanvas();
            canvas->drawColor(SK_ColorWHITE);
            size.set(info.width(), info.height());
        } else {
            canvas->translate(SkScalarHalf(fCell.width() - fImage->width()),
                              SkScalarHalf(fCell.height() - fImage->height()));
        }
        this->drawTheImage(canvas, size, filter, dx, dy);

        if (surface) {
            SkAutoTUnref<SkImage> orig(surface->newImageSnapshot());
            SkAutoTUnref<SkImage> zoomed(zoom_up(orig));
            origCanvas->drawImage(zoomed,
                                  SkScalarHalf(fCell.width() - zoomed->width()),
                                  SkScalarHalf(fCell.height() - zoomed->height()));
        }
    }

    void drawBorders(SkCanvas* canvas) {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        p.setColor(SK_ColorBLUE);

        SkRect r = SkRect::MakeWH(fCell.width() * 2, fCell.height() * 2);
        r.inset(SK_ScalarHalf, SK_ScalarHalf);
        canvas->drawRect(r, p);
        canvas->drawLine(r.left(), r.centerY(), r.right(), r.centerY(), p);
        canvas->drawLine(r.centerX(), r.top(), r.centerX(), r.bottom(), p);
    }

    void onDrawContent(SkCanvas* canvas) override {
        fCell.set(this->height() / 2, this->height() / 2);

        SkScalar trans[2];
        fTrans.timeToValues(fCurrTime, trans);

        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 2; ++x) {
                int index = y * 2 + x;
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(fCell.width() * x, fCell.height() * y);
                SkRect r = SkRect::MakeWH(fCell.width(), fCell.height());
                r.inset(4, 4);
                canvas->clipRect(r);
                this->drawHere(canvas, SkFilterQuality(index), trans[0], trans[1]);
            }
        }

        this->drawBorders(canvas);

        const SkScalar textX = fCell.width() * 2 + 30;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(36);
        SkString str;
        str.appendScalar(fScale);
        canvas->drawText(str.c_str(), str.size(), textX, 100, paint);
        str.reset(); str.appendScalar(fAngle);
        canvas->drawText(str.c_str(), str.size(), textX, 150, paint);

        str.reset(); str.appendScalar(trans[0]);
        canvas->drawText(str.c_str(), str.size(), textX, 200, paint);
        str.reset(); str.appendScalar(trans[1]);
        canvas->drawText(str.c_str(), str.size(), textX, 250, paint);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fCurrTime = timer.msec();
        return true;
    }

    virtual bool handleKey(SkKey key) {
        this->inval(NULL);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FilterQualityView; }
static SkViewRegister reg(MyFactory);
