/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPath.h"
#include "SkStream.h"
#include "gm.h"

// Test how short paths are stroked with various caps
DEF_SIMPLE_GM(path_stroke_with_zero_length, canvas, 240, 120) {
    SkPath paths[5];
    paths[0].moveTo(30.0f, 0);  // single line segment
    paths[0].rLineTo(30.0f, 0);

    paths[1].moveTo(90.0f, 0);  // single line segment with close
    paths[1].rLineTo(30.0f, 0);
    paths[1].close();

    paths[2].moveTo(150.0f, 0);  // zero-length line
    paths[2].rLineTo(0, 0);

    paths[3].moveTo(180.0f, 0);  // zero-length line with close
    paths[3].rLineTo(0, 0);
    paths[3].close();

    paths[4].moveTo(210.0f, 0);  // close only, no line
    paths[4].close();

    auto drawPaths = [&](const SkPaint& paint) {
        canvas->translate(0, 30.0f);
        for (const SkPath& path : paths) {
            canvas->drawPath(path, paint);
        }
    };
    
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);

    SkPaint butt;
    butt.setStyle(SkPaint::kStroke_Style);
    butt.setStrokeWidth(20.0f);
    butt.setStrokeCap(SkPaint::kButt_Cap);
    drawPaths(butt);

    SkPaint round(butt);
    round.setStrokeCap(SkPaint::kRound_Cap);
    drawPaths(round);

    SkPaint square(butt);
    square.setStrokeCap(SkPaint::kSquare_Cap);
    drawPaths(square);
}
