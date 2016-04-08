/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAnimTimer.h"
#include "SkPath.h"
#include "SkDashPathEffect.h"

int dash1[] = { 1, 1 };
int dash2[] = { 1, 3 };
int dash3[] = { 1, 1, 3, 3 };
int dash4[] = { 1, 3, 2, 4 };

struct DashExample {
    int* pattern;
    int length;
} dashExamples[] = {
    { dash1, SK_ARRAY_COUNT(dash1) },
    { dash2, SK_ARRAY_COUNT(dash2) },
    { dash3, SK_ARRAY_COUNT(dash3) },
    { dash4, SK_ARRAY_COUNT(dash4) }
};


class DashCircleGM : public skiagm::GM {
public:
    DashCircleGM() : fRotation(0) { }

protected:
    SkString onShortName() override { return SkString("dashcircle"); }

    SkISize onISize() override { return SkISize::Make(900, 1200); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint refPaint;
        refPaint.setAntiAlias(true);
        refPaint.setColor(0xFFbf3f7f);
        refPaint.setStyle(SkPaint::kStroke_Style);
        refPaint.setStrokeWidth(1);
        const SkScalar radius = 125;
        SkRect oval = SkRect::MakeLTRB(-radius - 20, -radius - 20, radius + 20, radius + 20);
        SkPath circle;
        circle.addCircle(0, 0, radius);
        SkScalar circumference = radius * SK_ScalarPI * 2;
        int wedges[] = { 6, 12, 36 };
        canvas->translate(radius+20, radius+20);
        for (int wedge : wedges) {
            SkScalar arcLength = 360.f / wedge;
            canvas->save();
            for (const DashExample& dashExample : dashExamples) {
                SkPath refPath;
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
                canvas->drawPath(refPath, refPaint);
                canvas->restore();
                SkPaint p;
                p.setAntiAlias(true);
                p.setStyle(SkPaint::kStroke_Style);
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

    bool onAnimate(const SkAnimTimer& timer) override {
        static const SkScalar kDesiredDurationSecs = 100.0f;

        fRotation = timer.scaled(360.0f/kDesiredDurationSecs, 360.0f);
        return true;
    }

private:
    SkScalar fRotation;

    typedef GM INHERITED;
};

DEF_GM(return new DashCircleGM; )
