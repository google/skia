#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6bad83b64de9266e323c29d550e04188
REG_FIDDLE(Matrix_MakeAll, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setTextSize(64);
    for (SkScalar sx : { -1, 1 } ) {
        for (SkScalar sy : { -1, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            SkMatrix m = SkMatrix::MakeAll(sx, 1, 128,    0, sy, 128,   0, 0, 1);
            canvas->concat(m);
            canvas->drawString("K", 0, 0, p);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
