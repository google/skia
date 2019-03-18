// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8c4499e322f10153dcd9b0b9806233b9
REG_FIDDLE(Canvas_clear, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(256, 128));
    canvas->clear(SkColorSetARGB(0x80, 0xFF, 0x00, 0x00));
    canvas->restore();
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(150, 192));
    canvas->clear(SkColorSetARGB(0x80, 0x00, 0xFF, 0x00));
    canvas->restore();
    canvas->clipRect(SkRect::MakeWH(75, 256));
    canvas->clear(SkColorSetARGB(0x80, 0x00, 0x00, 0xFF));
}
}  // END FIDDLE
