// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=41e45570d682397d3b8ff2f51bd9c574
REG_FIDDLE(Hue, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->drawColor(0xFF00FF00, SkBlendMode::kHue);
}
}  // END FIDDLE
