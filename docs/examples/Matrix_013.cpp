#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b9becf0dc24a9f00726e24a81fb72f16
REG_FIDDLE(Matrix_013, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    SkMatrix m;
    int pos = 0;
    for (SkScalar sx : { 1, 2 } ) {
        for (SkScalar kx : { 0, 1 } ) {
            m.setAll(sx, kx, 16,    0, 1, 32,   0, 0, 1);
            bool isSimilarity = m.isSimilarity();
            bool preservesRightAngles = m.preservesRightAngles();
            SkString str;
            str.printf("sx: %g kx: %g %s %s", sx, kx, isSimilarity ? "sim" : "",
                        preservesRightAngles ? "right" : "");
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->concat(m);
            canvas->drawString(str, 0, pos, p);
            pos += 20;
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
