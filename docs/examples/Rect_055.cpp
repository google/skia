// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Rect_055, 256, 256, true, 0) {
// HASH=26500032494cf93c5fa3423110fe82af
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 20, 15, 25};
    rect.join({50, 60, 55, 65});
    SkDebugf("join: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}
