/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

/**
 * Repro case for https://bugs.chromium.org/p/chromium/issues/detail?id=913223
 *
 * The original bug was filed against square caps, but here we also draw the labyrinth using round
 * and butt caps.
 *
 * Square and round caps expose over-coverage on overlaps when using coverage counting.
 *
 * Butt caps expose under-coverage on abutted strokes when using a 'max()' coverage function.
 */
static void draw_labyrinth(SkCanvas* canvas, SkPaint::Cap cap) {
    constexpr static bool kRows[11][12] = {
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {0,1,0,1,0,1,0,0,0,0,1,1},
        {0,0,0,0,1,0,0,0,0,1,1,1},
        {1,0,1,0,0,0,0,1,0,0,0,0},
        {0,1,1,0,0,0,0,0,0,1,1,1},
        {1,0,0,1,0,0,0,0,1,1,1,0},
        {0,1,0,1,1,1,0,0,1,1,1,0},
        {1,0,1,0,1,1,1,1,0,1,1,1},
        {0,0,1,0,0,1,0,0,0,0,0,1},
        {0,1,1,1,0,0,1,1,1,1,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1},
    };

    constexpr static bool kCols[13][10] = {
        {1,1,1,1,0,1,1,1,1,1},
        {0,0,1,0,0,0,1,1,1,0},
        {0,1,1,0,1,1,1,0,0,1},
        {1,1,0,0,0,0,1,0,1,0},
        {0,0,1,0,1,0,0,0,0,1},
        {0,0,1,1,1,0,0,0,1,0},
        {0,1,0,1,1,1,0,0,0,0},
        {1,1,1,0,1,1,1,0,1,0},
        {1,1,0,1,1,0,0,0,1,0},
        {0,0,1,0,0,0,0,0,0,1},
        {0,0,1,1,0,0,0,0,1,0},
        {0,0,0,0,0,0,1,0,0,1},
        {1,1,1,1,1,1,0,1,1,1},
    };

    SkPath maze;
    for (size_t y = 0; y < SK_ARRAY_COUNT(kRows); ++y) {
        for (size_t x = 0; x < SK_ARRAY_COUNT(kRows[0]); ++x) {
            if (kRows[y][x]) {
                maze.moveTo(x, y);
                maze.lineTo(x+1, y);
            }
        }
    }
    for (size_t x = 0; x < SK_ARRAY_COUNT(kCols); ++x) {
        for (size_t y = 0; y < SK_ARRAY_COUNT(kCols[0]); ++y) {
            if (kCols[x][y]) {
                maze.moveTo(x, y);
                maze.lineTo(x, y+1);
            }
        }
    }

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(.1f);
    paint.setColor(0xff406060);
    paint.setAntiAlias(true);
    paint.setStrokeCap(cap);

    canvas->translate(10.5, 10.5);
    canvas->scale(40, 40);
    canvas->drawPath(maze, paint);
}

constexpr static int kWidth = 500;
constexpr static int kHeight = 420;

DEF_SIMPLE_GM(labyrinth_square, canvas, kWidth, kHeight) {
    draw_labyrinth(canvas, SkPaint::kSquare_Cap);
}

DEF_SIMPLE_GM(labyrinth_round, canvas, kWidth, kHeight) {
    draw_labyrinth(canvas, SkPaint::kRound_Cap);
}

DEF_SIMPLE_GM(labyrinth_butt, canvas, kWidth, kHeight) {
    draw_labyrinth(canvas, SkPaint::kButt_Cap);
}
