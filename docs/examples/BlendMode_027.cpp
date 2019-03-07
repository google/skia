// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5d7c6e23a34ca9bf3ba8cda4cdc94cc4
REG_FIDDLE(BlendMode_027, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->drawColor(0xFF00FF00, SkBlendMode::kColor);
}
}  // END FIDDLE
