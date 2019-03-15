#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=05be7844e9afdd7b9bfc31c5423a70a2
REG_FIDDLE(Matrix_050, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setTextSize(48);
    SkMatrix m;
    for (SkScalar sx : { -1, 0, 1 } ) {
        for (SkScalar sy : { -1, 0, 1 } ) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            m.setSkew(sx, sy);
            m.postTranslate(96 + 64 * sx, 128 + 48 * sy);
            canvas->concat(m);
            canvas->drawString("K", 0, 0, p);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
