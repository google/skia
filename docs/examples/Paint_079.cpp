// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=55d7b9d482ac8e17a6153f555a8adb8d
REG_FIDDLE(Paint_079, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kSolid_SkBlurStyle, 3));
    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);
}
}  // END FIDDLE
