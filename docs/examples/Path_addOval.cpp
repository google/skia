// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_addOval, 256, 120, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath oval;
    oval.addOval({20, 20, 160, 80});
    canvas->drawPath(oval, paint);
}
}  // END FIDDLE
