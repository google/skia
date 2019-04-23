// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ece04ee3d3761e3425f37c8f06f054c1
REG_FIDDLE(Image_Filter_Methods, 256, 256, false, 0) {
#include "include/effects/SkBlurImageFilter.h"

void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2);
    SkRegion region;
    region.op( 10, 10, 50, 50, SkRegion::kUnion_Op);
    region.op( 10, 50, 90, 90, SkRegion::kUnion_Op);
    paint.setImageFilter(SkBlurImageFilter::Make(5.0f, 5.0f, nullptr));
    canvas->drawRegion(region, paint);
    paint.setImageFilter(nullptr);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5));
    canvas->translate(100, 100);
    canvas->drawRegion(region, paint);
}
}  // END FIDDLE
