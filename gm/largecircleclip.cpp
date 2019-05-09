/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/utils/SkRandom.h"

// Tests clipping against circles with large coordinates, both aa and non-aa.
DEF_SIMPLE_GM(large_circle_clip, canvas, 2048, 2048) {
    SkRandom random;
    auto circle = SkRRect::MakeOval(SkRect::MakeWH(2048, 2048));
    bool aa = true;
    do {
        SkColor4f color;
        color.fR = random.nextRangeF(0, 1);
        color.fG = random.nextRangeF(0, 1);
        color.fB = random.nextRangeF(0, 1);
        color.fA = 1.f;
        SkPaint paint;
        paint.setColor4f(color, nullptr);
        canvas->save();
        canvas->clipRRect(circle, aa);
        canvas->drawPaint(paint);
        canvas->restore();
        circle.setOval(circle.getBounds().makeOffset(20, 20).makeInset(25.f, 25.f));
        aa = !aa;
    } while (circle.getBounds().width() > 25.f);
}
