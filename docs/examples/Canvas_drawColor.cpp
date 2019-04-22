// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9cf94fead1e6b17d836c704b4eac269a
REG_FIDDLE(Canvas_drawColor, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorRED);
    canvas->clipRect(SkRect::MakeWH(150, 150));
    canvas->drawColor(SkColorSetARGB(0x80, 0x00, 0xFF, 0x00), SkBlendMode::kPlus);
    canvas->clipRect(SkRect::MakeWH(75, 75));
    canvas->drawColor(SkColorSetARGB(0x80, 0x00, 0x00, 0xFF), SkBlendMode::kPlus);
}
}  // END FIDDLE
