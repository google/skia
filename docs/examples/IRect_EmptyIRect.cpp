// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=65e0b9b52e907902630577941fb3ed6d
REG_FIDDLE(IRect_EmptyIRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const SkIRect& rect = SkIRect::EmptyIRect();
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
