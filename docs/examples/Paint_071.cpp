// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=73092d4d06faecea3c204d852a4dd8a8
REG_FIDDLE(Paint_071, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint normal, blender;
    normal.setColor(0xFF58a889);
    blender.setColor(0xFF8958a8);
    canvas->clear(0);
    for (SkBlendMode m : { SkBlendMode::kSrcOver, SkBlendMode::kSrcIn, SkBlendMode::kSrcOut } ) {
        normal.setBlendMode(SkBlendMode::kSrcOver);
        canvas->drawOval(SkRect::MakeXYWH(30, 30, 30, 80), normal);
        blender.setBlendMode(m);
        canvas->drawOval(SkRect::MakeXYWH(10, 50, 80, 30), blender);
        canvas->translate(70, 70);
    }
}
}  // END FIDDLE
