// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=63977f97999bbd6eecfdcc7575d75492
REG_FIDDLE(IRect_width64, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect large = { -2147483647, 1, 2147483644, 2 };
    SkDebugf("width: %d width64: %" PRId64 "\n", large.width(), large.width64());
}
}  // END FIDDLE
