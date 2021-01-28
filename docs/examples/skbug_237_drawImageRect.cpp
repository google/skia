// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skbug_237_drawImageRect, 256, 256, false, 6) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 20.0f, false));
    canvas->clear(0xFF88FF88);
    canvas->drawImageRect(image, SkRect{16, 16, 48, 48}, {64, 64, 192, 192}, SkSamplingOptions(),
                          &paint, SkCanvas::kFast_SrcRectConstraint);
}
}  // END FIDDLE
