// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_setFillType, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.setFillType(SkPathFillType::kInverseWinding);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
