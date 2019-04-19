#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=55e0431adc6c5b1987ebb8123cc10342
REG_FIDDLE(Matrix_setSkew, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setTextSize(48);
    SkMatrix m;
    for (SkScalar sx : { -1, 0, 1 } ) {
        for (SkScalar sy : { -1, 0, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            m.setSkew(sx, sy, 96 + 64 * sx, 128 + 48 * sy);
            canvas->concat(m);
            canvas->drawString("K", 96 + 64 * sx, 128 + 48 * sy, p);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
