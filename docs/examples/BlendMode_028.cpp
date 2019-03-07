// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7d42fe34ae20dd9e12c39dc3950e9989
REG_FIDDLE(BlendMode_028, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->drawColor(0xFF00FF00, SkBlendMode::kLuminosity);
}
}  // END FIDDLE
