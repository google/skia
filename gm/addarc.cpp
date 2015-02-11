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
    SkString onShortName() SK_OVERRIDE { return SkString("addarc"); }

    SkISize onISize() SK_OVERRIDE { return SkISize::Make(1040, 1040); }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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
            paint.setColor(rand.nextU() | (0xFF << 24));
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

    bool onAnimate(const SkAnimTimer& timer) SK_OVERRIDE {
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
    SkString onShortName() SK_OVERRIDE { return SkString("addarc_meas"); }

    SkISize onISize() SK_OVERRIDE { return SkISize::Make(2*R + 40, 2*R + 40); }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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
            if (meas.getPosTan(arcLen, &pos, NULL)) {
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
    SkString onShortName() SK_OVERRIDE { return SkString("strokecircle"); }
    
    SkISize onISize() SK_OVERRIDE { return SkISize::Make(520, 520); }
    
    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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

            paint.setColor(rand.nextU() | (0xFF << 24));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(const SkAnimTimer& timer) SK_OVERRIDE {
        fRotate = timer.scaled(60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new StrokeCircleGM; )
