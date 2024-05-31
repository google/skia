// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(IRect_height64, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect large = { 1, -2147483647, 2, 2147483644 };
    SkDebugf("height: %d height64: %" PRId64 "\n", large.height(), large.height64());
}
}  // END FIDDLE
