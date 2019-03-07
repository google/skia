#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4565a0792058178c88e0a129a87272d6
REG_FIDDLE(Matrix_042, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setTextSize(64);
    SkMatrix m;
    for (SkScalar sx : { -1, 1 } ) {
        for (SkScalar sy : { -1, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            m.setScale(sx, sy, 128, 64);
            canvas->concat(m);
            canvas->drawString("%", 128, 64, p);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
