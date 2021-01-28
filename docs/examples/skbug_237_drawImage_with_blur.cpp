// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skbug_237_drawImage_with_blur, 256, 256, false, 6) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0f, false));
    canvas->clear(0xFF88FF88);
    canvas->scale(4, 4);
    auto subset = image->makeSubset({16, 16, 48, 48});
    canvas->drawImage(subset, 16, 16, SkSamplingOptions(), &paint);
}
}  // END FIDDLE
