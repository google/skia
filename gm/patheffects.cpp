/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"

namespace skiagm {

static void compose_pe(SkPaint* paint) {
    SkPathEffect* pe = paint->getPathEffect();
    sk_sp<SkPathEffect> corner = SkCornerPathEffect::Make(25);
    sk_sp<SkPathEffect> compose;
    if (pe) {
        compose = SkComposePathEffect::Make(sk_ref_sp(pe), corner);
    } else {
        compose = corner;
    }
    paint->setPathEffect(compose);
}

static void hair_pe(SkPaint* paint) {
    paint->setStrokeWidth(0);
}

static void hair2_pe(SkPaint* paint) {
    paint->setStrokeWidth(0);
    compose_pe(paint);
}

static void stroke_pe(SkPaint* paint) {
    paint->setStrokeWidth(12);
    compose_pe(paint);
}

static void dash_pe(SkPaint* paint) {
    SkScalar inter[] = { 20, 10, 10, 10 };
    paint->setStrokeWidth(12);
    paint->setPathEffect(SkDashPathEffect::Make(inter, SK_ARRAY_COUNT(inter), 0));
    compose_pe(paint);
}

static const int gXY[] = {
4, 0, 0, -4, 8, -4, 12, 0, 8, 4, 0, 4
};

static void scale(SkPath* path, SkScalar scale) {
    SkMatrix m;
    m.setScale(scale, scale);
    path->transform(m);
}

static void one_d_pe(SkPaint* paint) {
    SkPath  path;
    path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
        path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    path.close();
    path.offset(SkIntToScalar(-6), 0);
    scale(&path, 1.5f);

    paint->setPathEffect(SkPath1DPathEffect::Make(path, SkIntToScalar(21), 0,
                                                  SkPath1DPathEffect::kRotate_Style));
    compose_pe(paint);
}

typedef void (*PE_Proc)(SkPaint*);
static const PE_Proc gPE[] = { hair_pe, hair2_pe, stroke_pe, dash_pe, one_d_pe };

static void fill_pe(SkPaint* paint) {
    paint->setStyle(SkPaint::kFill_Style);
    paint->setPathEffect(nullptr);
}

static void discrete_pe(SkPaint* paint) {
    paint->setPathEffect(SkDiscretePathEffect::Make(10, 4));
}

static sk_sp<SkPathEffect> MakeTileEffect() {
    SkMatrix m;
    m.setScale(SkIntToScalar(12), SkIntToScalar(12));

    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(5));

    return SkPath2DPathEffect::Make(m, path);
}

static void tile_pe(SkPaint* paint) {
    paint->setPathEffect(MakeTileEffect());
}

static const PE_Proc gPE2[] = { fill_pe, discrete_pe, tile_pe };

class PathEffectGM : public GM {
public:
    PathEffectGM() {}

protected:

    SkString onShortName() override {
        return SkString("patheffect");
    }

    SkISize onISize() override { return SkISize::Make(800, 600); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        SkPath path;
        path.moveTo(20, 20);
        path.lineTo(70, 120);
        path.lineTo(120, 30);
        path.lineTo(170, 80);
        path.lineTo(240, 50);

        size_t i;
        canvas->save();
        for (i = 0; i < SK_ARRAY_COUNT(gPE); i++) {
            gPE[i](&paint);
            canvas->drawPath(path, paint);
            canvas->translate(0, 75);
        }
        canvas->restore();

        path.reset();
        SkRect r = { 0, 0, 250, 120 };
        path.addOval(r, SkPath::kCW_Direction);
        r.inset(50, 50);
        path.addRect(r, SkPath::kCCW_Direction);

        canvas->translate(320, 20);
        for (i = 0; i < SK_ARRAY_COUNT(gPE2); i++) {
            gPE2[i](&paint);
            canvas->drawPath(path, paint);
            canvas->translate(0, 160);
        }

        SkIRect rect = SkIRect::MakeXYWH(20, 20, 60, 60);
        for (i = 0; i < SK_ARRAY_COUNT(gPE); i++) {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kFill_Style);
            gPE[i](&p);
            canvas->drawIRect(rect, p);
            canvas->translate(75, 0);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* PathEffectFactory(void*) { return new PathEffectGM; }
static GMRegistry regPathEffect(PathEffectFactory);

}
