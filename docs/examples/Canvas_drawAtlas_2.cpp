// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0e66a8f230a8d531bcef9f5ebdc5aac1
REG_FIDDLE(Canvas_drawAtlas_2, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
  // SkBitmap source = mandrill;
  SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 200, 100 } };
  SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };
  SkColor colors[] = { 0x7f55aa00, 0x7f3333bf };
  SkPaint paint;
  paint.setAlpha(127);
  SkSamplingOptions sampling;
  canvas->drawAtlas(image.get(), xforms, tex, colors, 2, SkBlendMode::kPlus,
                    sampling, nullptr, &paint);
}
}  // END FIDDLE
