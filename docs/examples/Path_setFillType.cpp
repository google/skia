// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b4a91cd7f50b2a0a0d1bec6d0ac823d2
REG_FIDDLE(Path_setFillType, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.setFillType(SkPath::kInverseWinding_FillType);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
