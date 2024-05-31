// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Mask_Filter_Methods, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kSolid_SkBlurStyle, 3));
    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);
}
}  // END FIDDLE
