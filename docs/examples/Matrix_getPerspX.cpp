#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a0f5bf4b55e8c33bfda29bf67e34306f
REG_FIDDLE(Matrix_getPerspX, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix m;
    m.setIdentity();
    m.set(SkMatrix::kMPersp0, -0.004f);
    SkAutoCanvasRestore autoRestore(canvas, true);
    canvas->translate(22, 144);
    SkPaint black;
    black.setAntiAlias(true);
    black.setTextSize(24);
    SkPaint gray = black;
    gray.setColor(0xFF9f9f9f);
    SkString string;
    string.appendScalar(m.getPerspX());
    canvas->drawString(string, 0, -72, gray);
    canvas->concat(m);
    canvas->drawString(string, 0, 0, black);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
