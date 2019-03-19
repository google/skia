// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c093c2b14bd3e6171ede7cd4049d9b57
REG_FIDDLE(Canvas_drawAtlas_4, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
  // sk_sp<SkImage> image = mandrill;
  SkRSXform xforms[] = { { 1, 0, 0, 0 }, {0, 1, 300, 100 } };
  SkRect tex[] = { { 0, 0, 200, 200 }, { 200, 0, 400, 200 } };
  canvas->drawAtlas(image, xforms, tex, 2, nullptr, nullptr);
}
}  // END FIDDLE
