// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4acfbe051805940210c8916a94794142
REG_FIDDLE(IRect_011, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted width: %d\n", unsorted.width());
    SkIRect large = { -2147483647, 1, 2147483644, 2 };
    SkDebugf("large width: %d\n", large.width());
}
}  // END FIDDLE
