// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3eeef529375d8083ae0d615789d55e89
REG_FIDDLE(Color_Burn, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->clipRect({128, 0, 256, 256});
    canvas->drawColor(SkColorSetARGB(0x80, 0x90, 0x90, 0x90), SkBlendMode::kColorBurn);
}
}  // END FIDDLE
