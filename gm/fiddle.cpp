/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    SkPaint p;

    float vals[] = { 13, 17 };
    p.setPathEffect(SkDashPathEffect::Make(vals, 2, 9));

    SkRect r = { 50, 50, 150, 150 };
    SkRect clip = {0, 0, 200, 150};

    p.setColor(0x44FF0000);
    canvas->drawRect(clip, p);

    p.setColor(0xFF000000);
    p.setStrokeWidth(10);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->save();
    canvas->clipRect(clip);
    canvas->drawRect(r, p);
    canvas->restore();

    canvas->save();
    canvas->clipRect(clip.makeOffset(0, 150));
    canvas->drawRect(r, p);
    canvas->restore();
}
