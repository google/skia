#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1579d0cc109c26e69f66f73abd35fb0e
REG_FIDDLE(Matrix_setScale_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setTextSize(64);
    SkMatrix m;
    for (SkScalar sx : { -1, 1 } ) {
        for (SkScalar sy : { -1, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            m.setScale(sx, sy);
            m.postTranslate(128, 64);
            canvas->concat(m);
            canvas->drawString("@", 0, 0, p);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
