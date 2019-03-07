// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9fb76971b1a104a2a59816e0392267a7
REG_FIDDLE(Rect_068, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = {6.f / 7, 2.f / 3, 26.f / 10, 42.f / 6};
    rect.dump();
    SkRect copy = SkRect::MakeLTRB(0.857143f, 0.666667f, 2.6f, 7);
    SkDebugf("rect is " "%s" "equal to copy\n", rect == copy ? "" : "not ");
}
}  // END FIDDLE
