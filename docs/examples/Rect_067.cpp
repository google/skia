// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cea049ffff702a5923da41fe0ae0763b
REG_FIDDLE(Rect_dump, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = {20, 30, 40, 50};
     for (bool dumpAsHex : { false, true } ) {
         rect.dump(dumpAsHex);
         SkDebugf("\n");
     }
}
}  // END FIDDLE
