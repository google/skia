// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ef6ae2eaae6761130ce38065d0364abd
REG_FIDDLE(Canvas_053, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(0x8055aaff);
    auto oval = SkRRect::MakeOval({10, 20, 90, 100});
    canvas->clipRRect(oval, SkClipOp::kIntersect);
    canvas->drawCircle(70, 100, 60, paint);
}
}  // END FIDDLE
