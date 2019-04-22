// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8dc0d0fdeab20bbc21cac6874ddbefcd
REG_FIDDLE(Canvas_drawAtlas_3, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
  // sk_sp<SkImage> image = mandrill;
  SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 200, 100 } };
  SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };
  const SkImage* imagePtr = image.get();
  canvas->drawAtlas(imagePtr, xforms, tex, 2, nullptr, nullptr);
}
}  // END FIDDLE
