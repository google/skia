// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Color_Dodge, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->clipRect({128, 0, 256, 256});
    canvas->drawColor(SkColorSetARGB(0x80, 0x90, 0x90, 0x90), SkBlendMode::kColorDodge);
}
}  // END FIDDLE
