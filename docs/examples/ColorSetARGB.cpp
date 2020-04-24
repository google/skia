// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=35888f0869e01a6e03b5b93bba563734
REG_FIDDLE(ColorSetARGB, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorRED);
    canvas->clipRect(SkRect::MakeWH(150, 150));
    canvas->drawColor(SkColorSetARGB(0x80, 0x00, 0xFF, 0x00));
    canvas->clipRect(SkRect::MakeWH(75, 75));
    canvas->drawColor(SkColorSetARGB(0x80, 0x00, 0x00, 0xFF));
}
}  // END FIDDLE
