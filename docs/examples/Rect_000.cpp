// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2e262d0ac4b8ef51695e0525fc3ecdf6
REG_FIDDLE(Rect_000, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = SkRect::MakeEmpty();
    SkDebugf("MakeEmpty isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
    rect.offset(10, 10);
    SkDebugf("offset rect isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
    rect.inset(10, 10);
    SkDebugf("inset rect isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
    rect.outset(20, 20);
    SkDebugf("outset rect isEmpty: %s\n", rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
