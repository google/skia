// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a48698975d236573cef512f94a7e360b
REG_FIDDLE(Saturation, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->drawColor(0xFF00FF00, SkBlendMode::kSaturation);
}
}  // END FIDDLE
