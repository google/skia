#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=95ccfc2a89ce593e6b7a9f992a844bc0
REG_FIDDLE(Matrix_setAll, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setTextSize(64);
    SkMatrix m;
    for (SkScalar sx : { -1, 1 } ) {
        for (SkScalar sy : { -1, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            m.setAll(sx, 1, 128,    0, sy, 64,   0, 0, 1);
            canvas->concat(m);
            canvas->drawString("K", 0, 0, p);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
