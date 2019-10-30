/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkOpPathEffect.h"
#include "include/pathops/SkPathOps.h"

#include <initializer_list>

namespace skiagm {

static void compose_pe(SkPaint* paint) {
    SkPathEffect* pe = paint->getPathEffect();
    sk_sp<SkPathEffect> corner = SkCornerPathEffect::Make(25);
    sk_sp<SkPathEffect> compose;
    if (pe) {
        compose = SkPathEffect::MakeCompose(sk_ref_sp(pe), corner);
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

constexpr int gXY[] = {
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
constexpr PE_Proc gPE[] = { hair_pe, hair2_pe, stroke_pe, dash_pe, one_d_pe };

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

constexpr PE_Proc gPE2[] = { fill_pe, discrete_pe, tile_pe };

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

        canvas->save();
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPE); i++) {
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
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPE2); i++) {
            gPE2[i](&paint);
            canvas->drawPath(path, paint);
            canvas->translate(0, 160);
        }

        const SkIRect rect = SkIRect::MakeXYWH(20, 20, 60, 60);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPE); i++) {
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

DEF_GM( return new PathEffectGM; )

}

//////////////////////////////////////////////////////////////////////////////

class ComboPathEfectsGM : public skiagm::GM {
public:
    ComboPathEfectsGM() {}

protected:

    SkString onShortName() override {
        return SkString("combo-patheffects");
    }

    SkISize onISize() override { return SkISize::Make(360, 630); }

    void onDraw(SkCanvas* canvas) override {
        SkPath path0, path1, path2;
        path0.addCircle(100, 100, 60);
        path1.moveTo(20, 20); path1.cubicTo(20, 180, 140, 0, 140, 140);

        sk_sp<SkPathEffect> effects[] = {
            nullptr,
            SkStrokePathEffect::Make(20, SkPaint::kRound_Join, SkPaint::kRound_Cap, 0),
            SkMergePathEffect::Make(nullptr,
                                    SkStrokePathEffect::Make(20, SkPaint::kRound_Join,
                                                             SkPaint::kRound_Cap, 0),
                                    kDifference_SkPathOp),
            SkMergePathEffect::Make(SkMatrixPathEffect::MakeTranslate(50, 30),
                                    SkStrokePathEffect::Make(20, SkPaint::kRound_Join,
                                                             SkPaint::kRound_Cap, 0),
                                    kReverseDifference_SkPathOp),
        };

        SkPaint wireframe;
        wireframe.setStyle(SkPaint::kStroke_Style);
        wireframe.setAntiAlias(true);

        SkPaint paint;
        paint.setColor(0xFF8888FF);
        paint.setAntiAlias(true);

        for (auto& path : { path0, path1 }) {
            canvas->save();
            for (auto pe : effects) {
                paint.setPathEffect(pe);
                canvas->drawPath(path, paint);
                canvas->drawPath(path, wireframe);

                canvas->translate(0, 150);
            }
            canvas->restore();
            canvas->translate(180, 0);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM(return new ComboPathEfectsGM;)

