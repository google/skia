/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkInterpolator.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"

static sk_sp<SkSurface> make_surface(SkCanvas* canvas, const SkImageInfo& info) {
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRaster(info);
    }
    return surface;
}

static sk_sp<SkShader> make_shader(const SkRect& bounds) {
    sk_sp<SkImage> image(GetResourceAsImage("images/mandrill_128.png"));
    return image ? image->makeShader() : nullptr;
}

#define N   128
#define ANGLE_DELTA 3
#define SCALE_DELTA (SK_Scalar1 / 32)

static sk_sp<SkImage> make_image() {
    SkImageInfo info = SkImageInfo::MakeN32(N, N, kOpaque_SkAlphaType);
    auto surface(SkSurface::MakeRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorWHITE);

    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);

    path.addRect(SkRect::MakeWH(N/2, N));
    path.addRect(SkRect::MakeWH(N, N/2));
    path.moveTo(0, 0); path.lineTo(N, 0); path.lineTo(0, N); path.close();

    SkPaint paint;
    paint.setShader(make_shader(SkRect::MakeWH(N, N)));

    canvas->drawPath(path, paint);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> zoom_up(SkSurface* origSurf, SkImage* orig) {
    const SkScalar S = 16;    // amount to scale up
    const int D = 2;    // dimension scaling for the offscreen
    // since we only view the center, don't need to produce the entire thing

    SkImageInfo info = SkImageInfo::MakeN32(orig->width() * D, orig->height() * D,
                                            kOpaque_SkAlphaType);
    auto surface(origSurf->makeSurface(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorWHITE);
    canvas->scale(S, S);
    canvas->translate(-SkScalarHalf(orig->width()) * (S - D) / S,
                      -SkScalarHalf(orig->height()) * (S - D) / S);
    canvas->drawImage(orig, 0, 0, nullptr);

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
    return surface->makeImageSnapshot();
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

class FilterQualityView : public Sample {
    sk_sp<SkImage>  fImage;
    AnimValue       fScale, fAngle;
    SkSize          fCell;
    SkInterpolator  fTrans;
    SkMSec          fCurrTime;
    bool            fShowFatBits;

public:
    FilterQualityView() : fTrans(2, 2), fShowFatBits(true) {
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
    SkString name() override { return SkString("FilterQuality"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case '1': fAngle.inc(-ANGLE_DELTA); return true;
                case '2': fAngle.inc( ANGLE_DELTA); return true;
                case '3': fScale.inc(-SCALE_DELTA); return true;
                case '4': fScale.inc( SCALE_DELTA); return true;
                case '5': fShowFatBits = !fShowFatBits; return true;
                default: break;
            }
            return false;
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
        canvas->drawImage(fImage.get(), -SkScalarHalf(fImage->width()), -SkScalarHalf(fImage->height()),
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

        sk_sp<SkSurface> surface;
        if (fShowFatBits) {
            // scale up so we don't clip rotations
            SkImageInfo info = SkImageInfo::MakeN32(fImage->width() * 2, fImage->height() * 2,
                                                    kOpaque_SkAlphaType);
            surface = make_surface(canvas, info);
            canvas = surface->getCanvas();
            canvas->drawColor(SK_ColorWHITE);
            size.set(info.width(), info.height());
        } else {
            canvas->translate(SkScalarHalf(fCell.width() - fImage->width()),
                              SkScalarHalf(fCell.height() - fImage->height()));
        }
        this->drawTheImage(canvas, size, filter, dx, dy);

        if (surface) {
            sk_sp<SkImage> orig(surface->makeImageSnapshot());
            sk_sp<SkImage> zoomed(zoom_up(surface.get(), orig.get()));
            origCanvas->drawImage(zoomed.get(),
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

    void onOnceBeforeDraw() override {
        fImage = make_image();
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

        SkFont font(nullptr, 36);
        SkPaint paint;
        canvas->drawString(SkStringPrintf("%.8g", (float)fScale), textX, 100, font, paint);
        canvas->drawString(SkStringPrintf("%.8g", (float)fAngle), textX, 150, font, paint);
        canvas->drawString(SkStringPrintf("%.8g", trans[0]     ), textX, 200, font, paint);
        canvas->drawString(SkStringPrintf("%.8g", trans[1]     ), textX, 250, font, paint);
    }

    bool onAnimate(double nanos) override {
        fCurrTime = TimeUtils::NanosToMSec(nanos);
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new FilterQualityView(); )
