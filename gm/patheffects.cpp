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
    SkPathEffect* corner = new SkCornerPathEffect(25);
    SkPathEffect* compose;
    if (pe) {
        compose = new SkComposePathEffect(pe, corner);
        corner->unref();
    } else {
        compose = corner;
    }
    paint->setPathEffect(compose)->unref();
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
    paint->setPathEffect(new SkDashPathEffect(inter, SK_ARRAY_COUNT(inter),
                                              0))->unref();
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
    scale(&path, 1.5);
    
    paint->setPathEffect(new SkPath1DPathEffect(path, SkIntToScalar(21), 0,
                                SkPath1DPathEffect::kRotate_Style))->unref();
    compose_pe(paint);
}

typedef void (*PE_Proc)(SkPaint*);
static const PE_Proc gPE[] = { hair_pe, hair2_pe, stroke_pe, dash_pe, one_d_pe };

static void fill_pe(SkPaint* paint) {
    paint->setStyle(SkPaint::kFill_Style);
    paint->setPathEffect(NULL);
}

static void discrete_pe(SkPaint* paint) {
    paint->setPathEffect(new SkDiscretePathEffect(10, 4))->unref();
}

static SkPathEffect* MakeTileEffect() {
    SkMatrix m;
    m.setScale(SkIntToScalar(12), SkIntToScalar(12));

    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(5));
    
    return new SkPath2DPathEffect(m, path);
}

static void tile_pe(SkPaint* paint) {
    paint->setPathEffect(MakeTileEffect())->unref();
}

static const PE_Proc gPE2[] = { fill_pe, discrete_pe, tile_pe };

class PathEffectGM : public GM {
public:
    PathEffectGM() {}

protected:
    SkString onShortName() {
        return SkString("patheffect");
    }

    SkISize onISize() { return make_isize(800, 600); }

    virtual void onDraw(SkCanvas* canvas) {
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
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* PathEffectFactory(void*) { return new PathEffectGM; }
static GMRegistry regPathEffect(PathEffectFactory);

}
