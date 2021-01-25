// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1df575f9b8132306ce0552a2554ed132
REG_FIDDLE(Canvas_drawAtlas, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
  // SkBitmap source = mandrill;
  SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 200, 100 } };
  SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };
  SkColor colors[] = { 0x7f55aa00, 0x7f3333bf };
  const SkImage* imagePtr = image.get();
  SkSamplingOptions sampling;
  canvas->drawAtlas(imagePtr, xforms, tex, colors, 2, SkBlendMode::kSrcOver,
                    sampling, nullptr, nullptr);
}
}  // END FIDDLE
