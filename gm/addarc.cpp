/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"

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
            paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
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

    bool onAnimate(const SkAnimTimer& timer) override {
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

class AddArcMeasGM : public skiagm::GM {
public:
    AddArcMeasGM() {}

protected:
    SkString onShortName() override { return SkString("addarc_meas"); }

    SkISize onISize() override { return SkISize::Make(2*R + 40, 2*R + 40); }

    void onDraw(SkCanvas* canvas) override {
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
                canvas->drawLine(0, 0, pos.x(), pos.y(), measPaint);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new AddArcMeasGM; )

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

            paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fRotate = timer.scaled(60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new StrokeCircleGM; )

//////////////////////

static void html_canvas_arc(SkPath* path, SkScalar x, SkScalar y, SkScalar r, SkScalar start,
                            SkScalar end, bool ccw) {
    SkRect bounds = { x - r, y - r, x + r, y + r };
    SkScalar sweep = ccw ? end - start : start - end;
    path->arcTo(bounds, start, sweep, false);
}

// Lifted from canvas-arc-circumference-fill-diffs.html
class ManyArcsGM : public skiagm::GM {
public:
    ManyArcsGM() {}
    
protected:
    SkString onShortName() override { return SkString("manyarcs"); }
    
    SkISize onISize() override { return SkISize::Make(620, 330); }
    
    void onDraw(SkCanvas* canvas) override {
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
                                anticlockwise);
                path.lineTo(0, 28);
                canvas->drawPath(path, paint);
                canvas->translate(30, 0);
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
    }
    
private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ManyArcsGM; )

