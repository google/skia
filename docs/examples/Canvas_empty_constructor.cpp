// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4a00e6589e862fde5be532f4b6e316ce
REG_FIDDLE(Canvas_empty_constructor, 256, 256, true, 0) {
static void check_for_rotated_ctm(const SkCanvas* canvas) {
    const SkMatrix& matrix = canvas->getTotalMatrix();
    SkDebugf("rect stays rect is %s\n", matrix.rectStaysRect() ? "true" : "false");
}

void draw(SkCanvas* canvas) {
    check_for_rotated_ctm(canvas);
    canvas->rotate(30);
    check_for_rotated_ctm(canvas);
    SkCanvas defaultCanvas;
    check_for_rotated_ctm(&defaultCanvas);
}
}  // END FIDDLE
