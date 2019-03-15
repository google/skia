// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=466ef576b88e29d7252422db7adeed1c
REG_FIDDLE(AutoCanvasRestore_000, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkFont font(nullptr, 64);
    for (SkScalar sx : { -1, 1 } ) {
        for (SkScalar sy : { -1, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            SkMatrix m = SkMatrix::MakeAll(sx, 1, 96,    0, sy, 64,   0, 0, 1);
            canvas->concat(m);
            canvas->drawString("R", 0, 0, font, paint);
        }
    }
}
}  // END FIDDLE
