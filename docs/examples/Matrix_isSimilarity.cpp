#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8b37f4ae7fec1756433c0f984175fb14
REG_FIDDLE(Matrix_isSimilarity, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    SkMatrix m;
    int below = 175;
    for (SkScalar sx : { -1, 1 } ) {
        for (SkScalar sy : { -1, 1 } ) {
            m.setAll(sx, 1, 128,    1, sy, 32,   0, 0, 1);
            bool isSimilarity = m.isSimilarity();
            SkString str;
            str.printf("sx: %g sy: %g sim: %s", sx, sy, isSimilarity ? "true" : "false");
            {
                SkAutoCanvasRestore autoRestore(canvas, true);
                canvas->concat(m);
                canvas->drawString(str, 0, 0, p);
            }
            if (!isSimilarity) {
                canvas->drawString(str, 40, below, p);
                below += 20;
            }
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
