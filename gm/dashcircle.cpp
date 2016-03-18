/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
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

DEF_SIMPLE_GM(dashcircle, canvas, 900, 1200) {
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
    canvas->translate(radius + 20, radius + 20);
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
            canvas->drawPath(refPath, refPaint);
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
            p.setPathEffect(SkDashPathEffect::Create(intervals, intervalCount, 0))->unref();
            canvas->drawPath(circle, p);
            canvas->translate(0, radius * 2 + 50);
        }
        canvas->restore();
        canvas->translate(radius * 2  + 50, 0);
    }
}
