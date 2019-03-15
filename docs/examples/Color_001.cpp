// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=dad12dd912197cd5edd789ac0801bf8a
REG_FIDDLE(Color_001, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorRED);
    canvas->clipRect(SkRect::MakeWH(150, 150));
    canvas->drawColor(SkColorSetRGB(0x00, 0xFF, 0x00));
    canvas->clipRect(SkRect::MakeWH(75, 75));
    canvas->drawColor(SkColorSetRGB(0x00, 0x00, 0xFF));
}
}  // END FIDDLE
