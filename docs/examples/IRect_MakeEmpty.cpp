// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0ade3971c1d2616564992e286966ec8a
REG_FIDDLE(IRect_MakeEmpty, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = SkIRect::MakeEmpty();
    SkDebugf("MakeEmpty isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
    rect.offset(10, 10);
    SkDebugf("offset rect isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
    rect.inset(10, 10);
    SkDebugf("inset rect isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
    rect.outset(20, 20);
    SkDebugf("outset rect isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
