// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6c4acd8aa203f632b7d85cae672abf4d
REG_FIDDLE(IRect_019, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect test = {2, 2, 0, 0};
    SkIRect sorted = test.makeSorted();
    SkDebugf("test %c= sorted\n", test != sorted ? '!' : '=');
}
}  // END FIDDLE
