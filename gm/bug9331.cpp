/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkDashPathEffect.h"

// Reproduces skbug.com/9331, drawing differently in debug and release builds.
DEF_SIMPLE_GM(bug9331, canvas, 256, 256) {
    SkRect clip = {0, 0, 200, 150};
    {
        SkPaint paint;
        paint.setColor(0x44FF0000);
        canvas->drawRect(clip, paint);
    }

    auto draw = [&](SkColor color, SkRect clip) {
        SkScalar intervals[] = { 13, 17 };
        SkScalar phase = 9;

        SkPaint paint;
        paint.setColor(color);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(10);
        paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), phase));

        canvas->save();
            canvas->clipRect(clip);
            canvas->drawRect({50,50, 150,150}, paint);
        canvas->restore();
    };

    draw(0xFF000000, clip);
    draw(0xFF0000FF, clip.makeOffset(0,150));
}
