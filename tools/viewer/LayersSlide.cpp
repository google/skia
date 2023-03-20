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
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkCamera.h"
#include "src/base/SkUTF.h"
#include "tools/viewer/ClickHandlerSlide.h"
#include "tools/viewer/Slide.h"

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

    SkAutoCanvasRestore ar2(canvas, false);

    // create the layers

    r.setWH(100, 100);
    canvas->clipRect(r);

    r.fBottom = SkIntToScalar(20);
    canvas->saveLayer(&r, nullptr);

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
    canvas->saveLayer(&r, nullptr);

    // now draw the "content"

    if ((true)) {
        r.setWH(100, 100);

        canvas->saveLayerAlpha(&r, 0x80);

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);

        canvas->restore();
    } else {
        r.setWH(100, 100);

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
    r.setWH(100, 20);
//    SkDebugf("--------- draw top grad\n");
    canvas->drawRect(r, paint);

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
//    SkDebugf("--------- draw bot grad\n");
    canvas->drawRect(r, paint);
}

class LayersSlide : public Slide {
public:
    LayersSlide() { fName = "Layers"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorGRAY);

        if ((true)) {
            SkRect r;
            r.setWH(220, 120);
            SkPaint p;
            canvas->saveLayer(&r, &p);
            canvas->drawColor(0xFFFF0000);
            p.setAlpha(0);  // or 0
            p.setBlendMode(SkBlendMode::kSrc);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }

        if ((false)) {
            SkRect r;
            r.setWH(220, 120);
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

        if ((false)) {
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);

            canvas->translate(SkIntToScalar(300), 0);

            SkRect r;
            r.setWH(220, 60);

            canvas->saveLayer(&r, &p);

            r.setWH(220, 120);
            p.setColor(SK_ColorBLUE);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }

        test_fade(canvas);
    }
};
DEF_SLIDE( return new LayersSlide; )

//////////////////////////////////////////////////////////////////////////////

#include "include/effects/SkImageFilters.h"

#include "tools/Resources.h"

class BackdropSlide : public ClickHandlerSlide {
    SkPoint fCenter;
    SkScalar fAngle;
    sk_sp<SkImage> fImage;
    sk_sp<SkImageFilter> fFilter;

public:
    BackdropSlide() {
        fCenter.set(200, 150);
        fAngle = 0;
        fImage = GetResourceAsImage("images/mandrill_512.png");
        fFilter = SkImageFilters::Dilate(8, 8, nullptr);
        fName = "Backdrop";
    }

    void draw(SkCanvas* canvas) override {
        canvas->drawImage(fImage.get(), 0, 0);

        const SkScalar w = 250;
        const SkScalar h = 150;
        SkPath path;
        path.addOval(SkRect::MakeXYWH(-w/2, -h/2, w, h));
        SkMatrix m;
        m.setRotate(fAngle);
        m.postTranslate(fCenter.x(), fCenter.y());
        path.transform(m);

        canvas->clipPath(path, SkClipOp::kIntersect, true);
        const SkRect bounds = path.getBounds();

        SkPaint paint;
        paint.setAlpha(0xCC);
        canvas->saveLayer(SkCanvas::SaveLayerRec(&bounds, &paint, fFilter.get(), 0));

        canvas->restore();
    }

    bool animate(double nanos) override {
        fAngle = SkDoubleToScalar(fmod(1e-9 * nanos * 360 / 5, 360));
        return true;
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return new Click();
    }

    bool onClick(Click* click) override {
        fCenter = click->fCurr;
        return true;
    }
};

DEF_SLIDE( return new BackdropSlide; )
