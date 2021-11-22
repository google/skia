/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"

// Repro case for skia:7674.  Requires lots of RAM to run, and currently triggers UB:
// //include/private/SkTDArray.h:382:26:
//   runtime error: signed integer overflow: 2147483644 + 4 cannot be represented in type 'int'

static SK_UNUSED void path_measure_explosion(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(false);
    float intervals[] = { 0, 10e9f };
    p.setStyle(SkPaint::kStroke_Style);
    p.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));

    int quadratic_at[] = {
        13, 68, 258, 1053, 1323, 2608, 10018, 15668, 59838, 557493, 696873, 871098, 4153813,
        15845608, 48357008, 118059138, 288230353, 360287948, 562949933, 703687423, 1099511613, 0
    };
    int next_quadratic_at = 0;

    SkPathBuilder path;
    path.moveTo(0, 0);

    int i = 1;
    for (int points = 1; points < 2147483647; ) {
        if (points == quadratic_at[next_quadratic_at]) {
            path.quadTo(i, 0, i, 0);
            next_quadratic_at++;
            points += 2;
        } else {
            path.lineTo(i, 0);
            points += 1;
        }

        i++;

        if (i == 1000000) {
            path.moveTo(0, 0);
            points += 1;
            i = 1;
        }
    }
    canvas->drawPath(path.detach(), p);
}

#if 0
#include "gm/gm.h"
DEF_SIMPLE_GM(PathMeasure_explosion, canvas, 500,500) {
    path_measure_explosion(canvas);
}
#endif
