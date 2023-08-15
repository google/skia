/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
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
#include "include/effects/SkDashPathEffect.h"
#include "tools/timer/TimeUtils.h"

int dash1[] = { 1, 1 };
int dash2[] = { 1, 3 };
int dash3[] = { 1, 1, 3, 3 };
int dash4[] = { 1, 3, 2, 4 };

struct DashExample {
    int* pattern;
    int length;
} dashExamples[] = {
    { dash1, std::size(dash1) },
    { dash2, std::size(dash2) },
    { dash3, std::size(dash3) },
    { dash4, std::size(dash4) }
};


class DashCircleGM : public skiagm::GM {
public:
    DashCircleGM() : fRotation(0) { }

protected:
    SkString getName() const override { return SkString("dashcircle"); }

    SkISize getISize() override { return SkISize::Make(900, 1200); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint refPaint;
        refPaint.setAntiAlias(true);
        refPaint.setColor(0xFFbf3f7f);
        refPaint.setStroke(true);
        refPaint.setStrokeWidth(1);
        const SkScalar radius = 125;
        SkRect oval = SkRect::MakeLTRB(-radius - 20, -radius - 20, radius + 20, radius + 20);
        SkPath circle = SkPath::Circle(0, 0, radius);
        SkScalar circumference = radius * SK_ScalarPI * 2;
        int wedges[] = { 6, 12, 36 };
        canvas->translate(radius+20, radius+20);
        for (int wedge : wedges) {
            SkScalar arcLength = 360.f / wedge;
            canvas->save();
            for (const DashExample& dashExample : dashExamples) {
                SkPathBuilder refPath;
                int dashUnits = 0;
                for (int index = 0; index < dashExample.length; ++index) {
                    dashUnits += dashExample.pattern[index];
                }
                SkScalar unitLength = arcLength / dashUnits;
                SkScalar angle = 0;
                for (int index = 0; index < wedge; ++index) {
                    for (int i2 = 0; i2 < dashExample.length; i2 += 2) {
                        SkScalar span = dashExample.pattern[i2] * unitLength;
                        refPath.moveTo(0, 0);
                        refPath.arcTo(oval, angle, span, false);
                        refPath.close();
                        angle += span + (dashExample.pattern[i2 + 1]) * unitLength;
                    }
                }
                canvas->save();
                canvas->rotate(fRotation);
                canvas->drawPath(refPath.detach(), refPaint);
                canvas->restore();
                SkPaint p;
                p.setAntiAlias(true);
                p.setStroke(true);
                p.setStrokeWidth(10);
                SkScalar intervals[4];
                int intervalCount = dashExample.length;
                SkScalar dashLength = circumference / wedge / dashUnits;
                for (int index = 0; index < dashExample.length; ++index) {
                    intervals[index] = dashExample.pattern[index] * dashLength;
                }
                p.setPathEffect(SkDashPathEffect::Make(intervals, intervalCount, 0));
                canvas->save();
                canvas->rotate(fRotation);
                canvas->drawPath(circle, p);
                canvas->restore();
                canvas->translate(0, radius * 2 + 50);
            }
            canvas->restore();
            canvas->translate(radius * 2 + 50, 0);
        }
    }

    bool onAnimate(double nanos) override {
        constexpr SkScalar kDesiredDurationSecs = 100.0f;

        fRotation = TimeUtils::Scaled(1e-9 * nanos, 360.0f/kDesiredDurationSecs, 360.0f);
        return true;
    }

private:
    SkScalar fRotation;

    using INHERITED = GM;
};

DEF_GM(return new DashCircleGM; )

class DashCircle2GM : public skiagm::GM {
public:
    DashCircle2GM() {}

protected:
    SkString getName() const override { return SkString("dashcircle2"); }

    SkISize getISize() override { return SkISize::Make(635, 900); }

    void onDraw(SkCanvas* canvas) override {
        // These intervals are defined relative to tau.
        static constexpr SkScalar kIntervals[][2]{
                {0.333f, 0.333f},
                {0.015f, 0.015f},
                {0.01f , 0.09f },
                {0.097f, 0.003f},
                {0.02f , 0.04f },
                {0.1f  , 0.2f  },
                {0.25f , 0.25f },
                {0.6f  , 0.7f  }, // adds to > 1
                {1.2f  , 0.8f  }, // on is > 1
                {0.1f  , 1.1f  }, // off is > 1*/
        };

        static constexpr int kN = std::size(kIntervals);
        static constexpr SkScalar kRadius = 20.f;
        static constexpr SkScalar kStrokeWidth = 15.f;
        static constexpr SkScalar kPad = 5.f;
        static constexpr SkRect kCircle = {-kRadius, -kRadius, kRadius, kRadius};

        static constexpr SkScalar kThinRadius = kRadius * 1.5;
        static constexpr SkRect kThinCircle = {-kThinRadius, -kThinRadius,
                                                kThinRadius,  kThinRadius};
        static constexpr SkScalar kThinStrokeWidth = 0.4f;

        sk_sp<SkPathEffect> deffects[std::size(kIntervals)];
        sk_sp<SkPathEffect> thinDEffects[std::size(kIntervals)];
        for (int i = 0; i < kN; ++i) {
            static constexpr SkScalar kTau = 2 * SK_ScalarPI;
            static constexpr SkScalar kCircumference = kRadius * kTau;
            SkScalar scaledIntervals[2] = {kCircumference * kIntervals[i][0],
                                           kCircumference * kIntervals[i][1]};
            deffects[i] = SkDashPathEffect::Make(
                    scaledIntervals, 2, kCircumference * fPhaseDegrees * kTau / 360.f);
            static constexpr SkScalar kThinCircumference = kThinRadius * kTau;
            scaledIntervals[0] = kThinCircumference * kIntervals[i][0];
            scaledIntervals[1] = kThinCircumference * kIntervals[i][1];
            thinDEffects[i] = SkDashPathEffect::Make(
                    scaledIntervals, 2, kThinCircumference * fPhaseDegrees * kTau / 360.f);
        }

        SkMatrix rotate;
        rotate.setRotate(25.f);
        const SkMatrix kMatrices[]{
                SkMatrix::I(),
            SkMatrix::Scale(1.2f, 1.2f),
                SkMatrix::MakeAll(1, 0, 0, 0, -1, 0, 0, 0, 1),  // y flipper
                SkMatrix::MakeAll(-1, 0, 0, 0, 1, 0, 0, 0, 1),  // x flipper
            SkMatrix::Scale(0.7f, 0.7f),
                rotate,
                SkMatrix::Concat(
                        SkMatrix::Concat(SkMatrix::MakeAll(-1, 0, 0, 0, 1, 0, 0, 0, 1), rotate),
                        rotate)
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(kStrokeWidth);
        paint.setStroke(true);

        // Compute the union of bounds of all of our test cases.
        SkRect bounds = SkRect::MakeEmpty();
        const SkRect kBounds = kThinCircle.makeOutset(kThinStrokeWidth / 2.f,
                                                      kThinStrokeWidth / 2.f);
        for (const auto& m : kMatrices) {
            SkRect devBounds;
            m.mapRect(&devBounds, kBounds);
            bounds.join(devBounds);
        }

        canvas->save();
        canvas->translate(-bounds.fLeft + kPad, -bounds.fTop + kPad);
        for (size_t i = 0; i < std::size(deffects); ++i) {
            canvas->save();
            for (const auto& m : kMatrices) {
                canvas->save();
                canvas->concat(m);

                paint.setPathEffect(deffects[i]);
                paint.setStrokeWidth(kStrokeWidth);
                canvas->drawOval(kCircle, paint);

                paint.setPathEffect(thinDEffects[i]);
                paint.setStrokeWidth(kThinStrokeWidth);
                canvas->drawOval(kThinCircle, paint);

                canvas->restore();
                canvas->translate(bounds.width() + kPad, 0);
            }
            canvas->restore();
            canvas->translate(0, bounds.height() + kPad);
        }
        canvas->restore();
    }

protected:
    bool onAnimate(double nanos) override {
        fPhaseDegrees = 1e-9 * nanos;
        return true;
    }

    // Init with a non-zero phase for when run as a non-animating GM.
    SkScalar fPhaseDegrees = 12.f;
};

DEF_GM(return new DashCircle2GM;)

DEF_SIMPLE_GM(maddash, canvas, 1600, 1600) {
    canvas->drawRect({0, 0, 1600, 1600}, SkPaint());
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStroke(true);
    p.setStrokeWidth(380);

    SkScalar intvls[] = { 2.5, 10 /* 1200 */ };
    p.setPathEffect(SkDashPathEffect::Make(intvls, 2, 0));

    canvas->drawCircle(400, 400, 200, p);

    SkPathBuilder path;
    path.moveTo(800, 400);
    path.quadTo(1000, 400, 1000, 600);
    path.quadTo(1000, 800, 800, 800);
    path.quadTo(600, 800, 600, 600);
    path.quadTo(600, 400, 800, 400);
    path.close();
    canvas->translate(350, 150);
    p.setStrokeWidth(320);
    canvas->drawPath(path.detach(), p);

    path.moveTo(800, 400);
    path.cubicTo(900, 400, 1000, 500, 1000, 600);
    path.cubicTo(1000, 700, 900, 800, 800, 800);
    path.cubicTo(700, 800, 600, 700, 600, 600);
    path.cubicTo(600, 500, 700, 400, 800, 400);
    path.close();
    canvas->translate(-550, 500);
    p.setStrokeWidth(300);
    canvas->drawPath(path.detach(), p);
}
