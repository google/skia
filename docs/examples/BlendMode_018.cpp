// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=280ad6267a7d2d77b6d2c4531c6fc0bf
REG_FIDDLE(Color_Dodge, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->clipRect({128, 0, 256, 256});
    canvas->drawColor(SkColorSetARGB(0x80, 0x90, 0x90, 0x90), SkBlendMode::kColorDodge);
}
}  // END FIDDLE
