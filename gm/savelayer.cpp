/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

static void saveLayer(SkCanvas* canvas, SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    uint32_t flag = 1U << 31;
    SkRect rect = SkRect::MakeLTRB(l, t, r, b);
    canvas->saveLayer({ &rect, nullptr, nullptr, flag });
}

static void do_draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRandom rand;

    SkAutoCanvasRestore acr(canvas, true);
    for (int i = 0; i < 20; ++i) {
        paint.setColor(rand.nextU() | (0xFF << 24));
        canvas->drawRect({ 15, 15, 290, 40 }, paint);
        canvas->translate(0, 30);
    }
}

DEF_SIMPLE_GM(savelayer_unclipped, canvas, 320, 640) {
    const SkScalar L = 10;
    const SkScalar T = 10;
    const SkScalar R = 310;
    const SkScalar B = 630;

    canvas->clipRect({ L, T, R, B });

    for (int i = 0; i < 100; ++i) {
        SkAutoCanvasRestore acr(canvas, true);
        saveLayer(canvas, L, T, R, T + 20);
        saveLayer(canvas, L, B - 20, R, B);
        do_draw(canvas);
    }
}
