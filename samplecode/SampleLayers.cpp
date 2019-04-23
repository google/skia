/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkCamera.h"
#include "include/utils/SkInterpolator.h"
#include "samplecode/Sample.h"
#include "src/core/SkClipOpPriv.h"
#include "src/utils/SkUTF.h"

static void make_paint(SkPaint* paint, const SkMatrix& localMatrix) {
    SkColor colors[] = { 0, SK_ColorWHITE };
    SkPoint pts[] = { { 0, 0 }, { 0, SK_Scalar1*20 } };
    paint->setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                  SkTileMode::kClamp, 0, &localMatrix));
    paint->setBlendMode(SkBlendMode::kDstIn);
}

// test drawing with strips of fading gradient above and below
static void test_fade(SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, true);

    SkRect r;

    SkPaint p;
    p.setAlpha(0x88);

    SkAutoCanvasRestore ar2(canvas, false);

    // create the layers

    r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));
    canvas->clipRect(r);

    r.fBottom = SkIntToScalar(20);
    canvas->saveLayer(&r, nullptr);

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
    canvas->saveLayer(&r, nullptr);

    // now draw the "content"

    if (true) {
        r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));

        canvas->saveLayerAlpha(&r, 0x80);

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);

        canvas->restore();
    } else {
        r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);
    }

//    return;

    // now apply an effect
    SkMatrix m;
    m.setScale(SK_Scalar1, -SK_Scalar1);
    m.postTranslate(0, SkIntToScalar(100));

    SkPaint paint;
    make_paint(&paint, m);
    r.set(0, 0, SkIntToScalar(100), SkIntToScalar(20));
//    SkDebugf("--------- draw top grad\n");
    canvas->drawRect(r, paint);

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
//    SkDebugf("--------- draw bot grad\n");
    canvas->drawRect(r, paint);
}

class LayersView : public Sample {
public:
    LayersView() {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Layers");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->drawBG(canvas);

        if (true) {
            SkRect r;
            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(120));
            SkPaint p;
            canvas->saveLayer(&r, &p);
            canvas->drawColor(0xFFFF0000);
            p.setAlpha(0);  // or 0
            p.setBlendMode(SkBlendMode::kSrc);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }

        if (false) {
            SkRect r;
            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(120));
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);

            if (true) {
                canvas->saveLayer(&r, &p);
                p.setColor(0xFFFF0000);
                canvas->drawOval(r, p);
                canvas->restore();
            }

            p.setColor(0xFF0000FF);
            r.offset(SkIntToScalar(20), SkIntToScalar(50));
            canvas->drawOval(r, p);
        }

        if (false) {
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);

            canvas->translate(SkIntToScalar(300), 0);

            SkRect r;
            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(60));

            canvas->saveLayer(&r, &p);

            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(120));
            p.setColor(SK_ColorBLUE);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }

        test_fade(canvas);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new LayersView; )

//////////////////////////////////////////////////////////////////////////////

#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkMatrixConvolutionImageFilter.h"
#include "include/effects/SkMorphologyImageFilter.h"

#include "tools/Resources.h"
#include "tools/timer/AnimTimer.h"

class BackdropView : public Sample {
    SkPoint fCenter;
    SkScalar fAngle;
    sk_sp<SkImage> fImage;
    sk_sp<SkImageFilter> fFilter;
public:
    BackdropView() {
        fCenter.set(200, 150);
        fAngle = 0;
        fImage = GetResourceAsImage("images/mandrill_512.png");
        fFilter = SkDilateImageFilter::Make(8, 8, nullptr);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Backdrop");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawImage(fImage.get(), 0, 0, nullptr);

        const SkScalar w = 250;
        const SkScalar h = 150;
        SkPath path;
        path.addOval(SkRect::MakeXYWH(-w/2, -h/2, w, h));
        SkMatrix m;
        m.setRotate(fAngle);
        m.postTranslate(fCenter.x(), fCenter.y());
        path.transform(m);

        canvas->clipPath(path, kIntersect_SkClipOp, true);
        const SkRect bounds = path.getBounds();

        SkPaint paint;
        paint.setAlpha(0xCC);
        canvas->saveLayer({ &bounds, &paint, fFilter.get(), nullptr, nullptr, 0 });

        canvas->restore();
    }

    bool onAnimate(const AnimTimer& timer) override {
        fAngle = SkDoubleToScalar(fmod(timer.secs() * 360 / 5, 360));
        return true;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new Click(this);
    }

    bool onClick(Click* click) override {
        fCenter = click->fCurr;
        return this->INHERITED::onClick(click);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new BackdropView; )
