// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(draw_image_nine_blur_mask, 256, 256, false, 6) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6, false));
    canvas->clear(0xFFFF4444);
    canvas->drawImageNine(image.get(), {16, 16, 48, 48}, {8, 8, 248, 248},
                          SkFilterMode::kNearest, &paint);
}
}  // END FIDDLE
