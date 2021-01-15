// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(upscale_checkerboard, 512, 512, false, 0) {
void draw(SkCanvas* canvas) {
    SkPMColor p[] = {0xFFFFFFFF, 0xFF000000, 0xFF000000, 0xFFFFFFFF};
    const auto info = SkImageInfo::MakeN32Premul(2, 2);
    auto img = SkImage::MakeRasterCopy({info, p, 8});

    SkPaint paint;
    paint.setShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                    SkSamplingOptions(SkFilterMode::kLinear)));

    canvas->translate(20, 20);
    canvas->scale(20, 20);
    canvas->drawRect({0, 0, 20, 20}, paint);
}
}  // END FIDDLE
