/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"
#include "SkDashPathEffect.h"

DEF_SIMPLE_GM(bug530095, canvas, 900, 1200) {
    SkPath path1, path2;
    path1.addCircle(200, 200, 124);
    path2.addCircle(2, 2, 1.24f);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(26);
    SkScalar intervals[] = {700, 700 };
    int intervalCount = (int) SK_ARRAY_COUNT(intervals);
    paint.setPathEffect(SkDashPathEffect::Create(intervals, intervalCount, -40))->unref();
    canvas->drawPath(path1, paint);

    paint.setStrokeWidth(0.26f);
    SkScalar smIntervals[] = {7, 7 };
    int smIntervalCount = (int) SK_ARRAY_COUNT(smIntervals);
    paint.setPathEffect(SkDashPathEffect::Create(smIntervals, smIntervalCount, -0.40f))->unref();
    canvas->save();
    canvas->scale(100, 100);
    canvas->translate(4, 0);
    canvas->drawPath(path2, paint);
    canvas->restore();

    paint.setStrokeWidth(26);
    paint.setPathEffect(SkDashPathEffect::Create(intervals, intervalCount, 0))->unref();
    canvas->save();
    canvas->translate(0, 400);
    canvas->drawPath(path1, paint);
    canvas->restore();

    paint.setStrokeWidth(0.26f);
    paint.setPathEffect(SkDashPathEffect::Create(smIntervals, smIntervalCount, 0))->unref();
    canvas->scale(100, 100);
    canvas->translate(4, 4);
    canvas->drawPath(path2, paint);
}
