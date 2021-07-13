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
#include "include/core/SkPathBuilder.h"
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

static SkPath scale(const SkPath& path, SkScalar scale) {
    SkMatrix m;
    m.setScale(scale, scale);
    return path.makeTransform(m);
}

static void one_d_pe(SkPaint* paint) {
    SkPathBuilder b;
    b.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2) {
        b.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    }
    b.close().offset(SkIntToScalar(-6), 0);
    SkPath path = scale(b.detach(), 1.5f);

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

    return SkPath2DPathEffect::Make(m, SkPath::Circle(0,0,5));
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

        SkPath path = SkPath::Polygon({
            {20, 20},
            {70, 120},
            {120, 30},
            {170, 80},
            {240, 50},
        }, false);

        canvas->save();
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPE); i++) {
            gPE[i](&paint);
            canvas->drawPath(path, paint);
            canvas->translate(0, 75);
        }
        canvas->restore();

        path.reset();
        SkRect r = { 0, 0, 250, 120 };
        path = SkPathBuilder().addOval(r, SkPathDirection::kCW)
                              .addRect(r.makeInset(50, 50), SkPathDirection::kCCW)
                              .detach();

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
    using INHERITED = GM;
};

DEF_GM( return new PathEffectGM; )

}  // namespace skiagm

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
        SkPath path0 = SkPath::Circle(100, 100, 60),
               path1 = SkPathBuilder().moveTo(20, 20)
                                      .cubicTo(20, 180, 140, 0, 140, 140)
                                      .detach();

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

        for (const SkPath& path : { path0, path1 }) {
            canvas->save();
            for (const sk_sp<SkPathEffect>& pe : effects) {
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
    using INHERITED = GM;
};
DEF_GM(return new ComboPathEfectsGM;)

#include "include/effects/SkStrokeAndFillPathEffect.h"

// Test that we can replicate SkPaint::kStrokeAndFill_Style
// with a patheffect. We expect the 2nd and 3rd columns to draw the same.
DEF_SIMPLE_GM(stroke_and_fill_patheffect, canvas, 900, 450) {
    const float kStrokeWidth = 20;

    typedef SkPath (*Maker)();
    const Maker makers[] = {
        []() { return SkPath::Oval({0, 0, 100, 100}, SkPathDirection::kCW); },
        []() { return SkPath::Oval({0, 0, 100, 100}, SkPathDirection::kCCW); },
        []() {
            const SkPoint pts[] = {
                {0, 0}, {100, 100}, {0, 100}, {100, 0},
            };
            return SkPath::Polygon(pts, SK_ARRAY_COUNT(pts), true);
        },
    };

    const struct {
        SkPaint::Style  fStyle;
        float           fWidth;
        bool            fUsePE;
        bool            fExpectStrokeAndFill;
    } rec[] = {
        { SkPaint::kStroke_Style,                   0, false, false },
        { SkPaint::kFill_Style,                     0,  true, false },
        { SkPaint::kStroke_Style,                   0,  true, false },
        { SkPaint::kStrokeAndFill_Style, kStrokeWidth, false, true  },
        { SkPaint::kStroke_Style,        kStrokeWidth,  true, true  },
        { SkPaint::kStrokeAndFill_Style, kStrokeWidth,  true, true  },
    };

    SkPaint paint;
    canvas->translate(20, 20);
    for (auto maker : makers) {
        const SkPath path = maker();
        canvas->save();
        for (const auto& r : rec) {
            paint.setStyle(r.fStyle);
            paint.setStrokeWidth(r.fWidth);
            paint.setPathEffect(r.fUsePE ? SkStrokeAndFillPathEffect::Make() : nullptr);
            paint.setColor(r.fExpectStrokeAndFill ? SK_ColorGRAY : SK_ColorBLACK);

            canvas->drawPath(path, paint);
            canvas->translate(150, 0);
        }
        canvas->restore();

        canvas->translate(0, 150);
    }
}

//////////////////////////////////////////////////////////////////////////////

#include "include/core/SkStrokeRec.h"
#include "src/core/SkPathEffectBase.h"

namespace {
/**
 * Example path effect using CTM. This "strokes" a single line segment with some stroke width,
 * and then inflates the result by some number of pixels.
 */
class StrokeLineInflated : public SkPathEffectBase {
public:
    StrokeLineInflated(float strokeWidth, float pxInflate)
            : fRadius(strokeWidth / 2.f), fPxInflate(pxInflate) {}

    bool onNeedsCTM() const final { return true; }

    bool onFilterPath(SkPath* dst,
                      const SkPath& src,
                      SkStrokeRec* rec,
                      const SkRect* cullR,
                      const SkMatrix& ctm) const final {
        SkASSERT(src.countPoints() == 2);
        const SkPoint pts[2] = {src.getPoint(0), src.getPoint(1)};

        SkMatrix invCtm;
        if (!ctm.invert(&invCtm)) {
            return false;
        }

        // For a line segment, we can just map the (scaled) normal vector to pixel-space,
        // increase its length by the desired number of pixels, and then map back to canvas space.
        SkPoint n = {pts[0].fY - pts[1].fY, pts[1].fX - pts[0].fX};
        if (!n.setLength(fRadius)) {
            return false;
        }

        SkPoint mappedN = ctm.mapVector(n.fX, n.fY);
        if (!mappedN.setLength(mappedN.length() + fPxInflate)) {
            return false;
        }
        n = invCtm.mapVector(mappedN.fX, mappedN.fY);

        dst->moveTo(pts[0] + n);
        dst->lineTo(pts[1] + n);
        dst->lineTo(pts[1] - n);
        dst->lineTo(pts[0] - n);
        dst->close();

        rec->setFillStyle();

        return true;
    }

protected:
    void flatten(SkWriteBuffer&) const final {}

private:
    SK_FLATTENABLE_HOOKS(StrokeLineInflated)

    bool computeFastBounds(SkRect* bounds) const final { return false; }

    const float fRadius;
    const float fPxInflate;
};

sk_sp<SkFlattenable> StrokeLineInflated::CreateProc(SkReadBuffer&) { return nullptr; }

}  // namespace

class CTMPathEffectGM : public skiagm::GM {
protected:
    SkString onShortName() override { return SkString("ctmpatheffect"); }

    SkISize onISize() override { return SkISize::Make(800, 600); }

    // TODO: ctm-aware path effects are currently CPU only
    DrawResult onGpuSetup(GrDirectContext* dctx, SkString*) override {
        return dctx == nullptr ? DrawResult::kOk : DrawResult::kSkip;
    }

    void onDraw(SkCanvas* canvas) override {
        const float strokeWidth = 16;
        const float pxInflate = 0.5f;
        sk_sp<SkPathEffect> pathEffect(new StrokeLineInflated(strokeWidth, pxInflate));

        SkPath path;
        path.moveTo(100, 100);
        path.lineTo(200, 200);

        // Draw the inflated path, and a scaled version, in blue.
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SkColorSetA(SK_ColorBLUE, 0xff));
        paint.setPathEffect(pathEffect);
        canvas->drawPath(path, paint);
        canvas->save();
        canvas->translate(150, 0);
        canvas->scale(2.5, 0.5f);
        canvas->drawPath(path, paint);
        canvas->restore();

        // Draw the regular stroked version on top in green.
        // The inflated version should be visible underneath as a blue "border".
        paint.setPathEffect(nullptr);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(strokeWidth);
        paint.setColor(SkColorSetA(SK_ColorGREEN, 0xff));
        canvas->drawPath(path, paint);
        canvas->save();
        canvas->translate(150, 0);
        canvas->scale(2.5, 0.5f);
        canvas->drawPath(path, paint);
        canvas->restore();
    }

private:
    using INHERITED = GM;
};
DEF_GM(return new CTMPathEffectGM;)
