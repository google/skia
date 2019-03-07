// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=737c747df07ddf392c05970440de0927
REG_FIDDLE(IRect_024, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 10, 50, 20, 60 };
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect = rect.makeOffset(15, 32);
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
