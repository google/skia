// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cac84cf68e63a453c2a8b64c91537704
REG_FIDDLE(Path_addOval, 256, 120, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath oval;
    oval.addOval({20, 20, 160, 80});
    canvas->drawPath(oval, paint);
}
}  // END FIDDLE
