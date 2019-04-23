/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPathMeasure.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"
#include "tools/timer/AnimTimer.h"

class AddArcGM : public skiagm::GM {
public:
    AddArcGM() : fRotate(0) {}

protected:
    SkString onShortName() override { return SkString("addarc"); }

    SkISize onISize() override { return SkISize::Make(1040, 1040); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        SkRect r = SkRect::MakeWH(1000, 1000);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(15);

        const SkScalar inset = paint.getStrokeWidth() + 4;
        const SkScalar sweepAngle = 345;
        SkRandom rand;

        SkScalar sign = 1;
        while (r.width() > paint.getStrokeWidth() * 3) {
            paint.setColor(ToolUtils::color_to_565(rand.nextU() | (0xFF << 24)));
            SkScalar startAngle = rand.nextUScalar1() * 360;

            SkScalar speed = SkScalarSqrt(16 / r.width()) * 0.5f;
            startAngle += fRotate * 360 * speed * sign;

            SkPath path;
            path.addArc(r, startAngle, sweepAngle);
            canvas->drawPath(path, paint);

            r.inset(inset, inset);
            sign = -sign;
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        fRotate = timer.scaled(1, 360);
        return true;
    }

private:
    SkScalar fRotate;
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new AddArcGM; )

///////////////////////////////////////////////////

#define R   400

DEF_SIMPLE_GM(addarc_meas, canvas, 2*R + 40, 2*R + 40) {
        canvas->translate(R + 20, R + 20);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        SkPaint measPaint;
        measPaint.setAntiAlias(true);
        measPaint.setColor(SK_ColorRED);

        const SkRect oval = SkRect::MakeLTRB(-R, -R, R, R);
        canvas->drawOval(oval, paint);

        for (SkScalar deg = 0; deg < 360; deg += 10) {
            const SkScalar rad = SkDegreesToRadians(deg);
            SkScalar rx = SkScalarCos(rad) * R;
            SkScalar ry = SkScalarSin(rad) * R;

            canvas->drawLine(0, 0, rx, ry, paint);

            SkPath path;
            path.addArc(oval, 0, deg);
            SkPathMeasure meas(path, false);
            SkScalar arcLen = rad * R;
            SkPoint pos;
            if (meas.getPosTan(arcLen, &pos, nullptr)) {
                canvas->drawLine({0, 0}, pos, measPaint);
            }
        }
}

///////////////////////////////////////////////////

// Emphasize drawing a stroked oval (containing conics) and then scaling the results up,
// to ensure that we compute the stroke taking the CTM into account
//
class StrokeCircleGM : public skiagm::GM {
public:
    StrokeCircleGM() : fRotate(0) {}

protected:
    SkString onShortName() override { return SkString("strokecircle"); }

    SkISize onISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(20, 20);
        canvas->translate(13, 13);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1 / 2);

        const SkScalar delta = paint.getStrokeWidth() * 3 / 2;
        SkRect r = SkRect::MakeXYWH(-12, -12, 24, 24);
        SkRandom rand;

        SkScalar sign = 1;
        while (r.width() > paint.getStrokeWidth() * 2) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(fRotate * sign);

            paint.setColor(ToolUtils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        fRotate = timer.scaled(60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new StrokeCircleGM; )

//////////////////////

// Fill circles and rotate them to test our Analytic Anti-Aliasing.
// This test is based on StrokeCircleGM.
class FillCircleGM : public skiagm::GM {
public:
    FillCircleGM() : fRotate(0) {}

protected:
    SkString onShortName() override { return SkString("fillcircle"); }

    SkISize onISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(20, 20);
        canvas->translate(13, 13);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1 / 2);

        const SkScalar strokeWidth = paint.getStrokeWidth();
        const SkScalar delta = strokeWidth * 3 / 2;
        SkRect r = SkRect::MakeXYWH(-12, -12, 24, 24);
        SkRandom rand;

        // Reset style to fill. We only need stroke stype for producing delta and strokeWidth
        paint.setStyle(SkPaint::kFill_Style);

        SkScalar sign = 1;
        while (r.width() > strokeWidth * 2) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(fRotate * sign);
            paint.setColor(ToolUtils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        fRotate = timer.scaled(60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new FillCircleGM; )

//////////////////////

static void html_canvas_arc(SkPath* path, SkScalar x, SkScalar y, SkScalar r, SkScalar start,
                            SkScalar end, bool ccw, bool callArcTo) {
    SkRect bounds = { x - r, y - r, x + r, y + r };
    SkScalar sweep = ccw ? end - start : start - end;
    if (callArcTo)
        path->arcTo(bounds, start, sweep, false);
    else
        path->addArc(bounds, start, sweep);
}

// Lifted from canvas-arc-circumference-fill-diffs.html
DEF_SIMPLE_GM(manyarcs, canvas, 620, 330) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(10, 10);

        // 20 angles.
        SkScalar sweepAngles[] = {
                           -123.7f, -2.3f, -2, -1, -0.3f, -0.000001f, 0, 0.000001f, 0.3f, 0.7f,
                           1, 1.3f, 1.5f, 1.7f, 1.99999f, 2, 2.00001f, 2.3f, 4.3f, 3934723942837.3f
        };
        for (size_t i = 0; i < SK_ARRAY_COUNT(sweepAngles); ++i) {
            sweepAngles[i] *= 180;
        }

        SkScalar startAngles[] = { -1, -0.5f, 0, 0.5f };
        for (size_t i = 0; i < SK_ARRAY_COUNT(startAngles); ++i) {
            startAngles[i] *= 180;
        }

        bool anticlockwise = false;
        SkScalar sign = 1;
        for (size_t i = 0; i < SK_ARRAY_COUNT(startAngles) * 2; ++i) {
            if (i == SK_ARRAY_COUNT(startAngles)) {
                anticlockwise = true;
                sign = -1;
            }
            SkScalar startAngle = startAngles[i % SK_ARRAY_COUNT(startAngles)] * sign;
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(sweepAngles); ++j) {
                SkPath path;
                path.moveTo(0, 2);
                html_canvas_arc(&path, 18, 15, 10, startAngle, startAngle + (sweepAngles[j] * sign),
                                anticlockwise, true);
                path.lineTo(0, 28);
                canvas->drawPath(path, paint);
                canvas->translate(30, 0);
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
}

// Lifted from https://bugs.chromium.org/p/chromium/issues/detail?id=640031
DEF_SIMPLE_GM(tinyanglearcs, canvas, 620, 330) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(50, 50);

        SkScalar outerRadius = 100000.0f;
        SkScalar innerRadius = outerRadius - 20.0f;
        SkScalar centerX = 50;
        SkScalar centerY = outerRadius;
        SkScalar startAngles[] = { 1.5f * SK_ScalarPI , 1.501f * SK_ScalarPI  };
        SkScalar sweepAngle = 10.0f / outerRadius;

        for (size_t i = 0; i < SK_ARRAY_COUNT(startAngles); ++i) {
            SkPath path;
            SkScalar endAngle = startAngles[i] + sweepAngle;
            path.moveTo(centerX + innerRadius * sk_float_cos(startAngles[i]),
                        centerY + innerRadius * sk_float_sin(startAngles[i]));
            path.lineTo(centerX + outerRadius * sk_float_cos(startAngles[i]),
                        centerY + outerRadius * sk_float_sin(startAngles[i]));
            // A combination of tiny sweepAngle + large radius, we should draw a line.
            html_canvas_arc(&path, centerX, outerRadius, outerRadius,
                            startAngles[i] * 180 / SK_ScalarPI, endAngle * 180 / SK_ScalarPI,
                            true, true);
            path.lineTo(centerX + innerRadius * sk_float_cos(endAngle),
                        centerY + innerRadius * sk_float_sin(endAngle));
            html_canvas_arc(&path, centerX, outerRadius, innerRadius,
                            endAngle * 180 / SK_ScalarPI, startAngles[i] * 180 / SK_ScalarPI,
                            true, false);
            canvas->drawPath(path, paint);
            canvas->translate(20, 0);
        }
}
