// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Path_085, 256, 120, false, 0);
// HASH=cac84cf68e63a453c2a8b64c91537704
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath oval;
    oval.addOval({20, 20, 160, 80});
    canvas->drawPath(oval, paint);
}
}
