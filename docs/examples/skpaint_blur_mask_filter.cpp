// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_blur_mask_filter, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF));
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0f));
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString("Skia", SkFont(nullptr, 120));
    canvas->drawTextBlob(blob.get(), 0, 160, paint);
}
}  // END FIDDLE
