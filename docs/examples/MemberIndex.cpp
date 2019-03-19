#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3bbf75f4748420810aa2586e3c8548d9
REG_FIDDLE(MemberIndex, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint black;
    black.setAntiAlias(true);
    black.setTextSize(48);
    SkPaint gray = black;
    gray.setColor(0xFF9f9f9f);
    SkScalar offset[] = { 1.5f, 1.5f, 20,   1.5f, 1.5f, 20,   .03f, .01f, 2 };
    for (int i : { SkMatrix::kMScaleX, SkMatrix::kMSkewX,  SkMatrix::kMTransX,
                   SkMatrix::kMSkewY,  SkMatrix::kMScaleY, SkMatrix::kMTransY,
                   SkMatrix::kMPersp0, SkMatrix::kMPersp1, SkMatrix::kMPersp2 } ) {
        SkMatrix m;
        m.setIdentity();
        m.set(i, offset[i]);
        SkAutoCanvasRestore autoRestore(canvas, true);
        canvas->translate(22 + (i % 3) * 88, 44 + (i / 3) * 88);
        canvas->drawString("&", 0, 0, gray);
        canvas->concat(m);
        canvas->drawString("&", 0, 0, black);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
