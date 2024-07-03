// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_ColorSpaces, 512, 256, false, 3) {
void draw(SkCanvas* canvas) {
  auto rec2020 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                       SkNamedGamut::kRec2020);
  canvas->drawImage(image, 0, 0);
  canvas->drawImage(image->reinterpretColorSpace(rec2020), 256, 0);
}
}  // END FIDDLE
