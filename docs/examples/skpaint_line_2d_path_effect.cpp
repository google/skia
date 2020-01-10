// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_line_2d_path_effect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkMatrix lattice;
    lattice.setScale(8.0f, 8.0f);
    lattice.preRotate(30.0f);
    paint.setPathEffect(SkLine2DPathEffect::Make(0.0f, lattice));
    paint.setAntiAlias(true);
    SkRect bounds = SkRect::MakeWH(256, 256);
    bounds.outset(8.0f, 8.0f);
    canvas->clear(SK_ColorWHITE);
    canvas->drawRect(bounds, paint);
}
}  // END FIDDLE
