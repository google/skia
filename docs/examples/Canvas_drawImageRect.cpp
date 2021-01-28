// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bfd18e9cac896cdf94c9f154ccf94be8
REG_FIDDLE(Canvas_drawImageRect, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t pixels[][4] = {
            { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000 },
            { 0xFFFF0000, 0xFF000000, 0xFFFFFFFF, 0xFFFF0000 },
            { 0xFFFF0000, 0xFFFFFFFF, 0xFF000000, 0xFFFF0000 },
            { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000 } };
    SkBitmap redBorder;
    redBorder.installPixels(SkImageInfo::MakeN32Premul(4, 4),
            (void*) pixels, sizeof(pixels[0]));
    sk_sp<SkImage> image = redBorder.asImage();
    SkSamplingOptions sampling;
    for (auto constraint : {
            SkCanvas::kFast_SrcRectConstraint,
            SkCanvas::kStrict_SrcRectConstraint,
            SkCanvas::kFast_SrcRectConstraint } ) {
        canvas->drawImageRect(image.get(), SkRect::MakeLTRB(1, 1, 3, 3),
                SkRect::MakeLTRB(16, 16, 48, 48), sampling, nullptr, constraint);
        sampling = SkSamplingOptions(SkFilterMode::kLinear);
        canvas->translate(80, 0);
    }
}
}  // END FIDDLE
