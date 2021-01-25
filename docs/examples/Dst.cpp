// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=35915a2273be1076f00f2e47998ce808
REG_FIDDLE(Dst, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 125, 128 } };
    SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };
    SkColor colors[] = { 0x7f55aa00, 0x7f3333bf };
    SkSamplingOptions sampling;
    canvas->drawAtlas(image.get(), xforms, tex, colors, 2, SkBlendMode::kSrc,
                      sampling, nullptr, nullptr);
    canvas->translate(128, 0);
    canvas->drawAtlas(image.get(), xforms, tex, colors, 2, SkBlendMode::kDst,
                      sampling, nullptr, nullptr);
}
}  // END FIDDLE
