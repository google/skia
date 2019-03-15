// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6a614faa0fbcf19958b5559c19b02d0f
REG_FIDDLE(Canvas_049, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->rotate(10);
    SkPaint paint;
    paint.setAntiAlias(true);
    for (auto alias: { false, true } ) {
        canvas->save();
        canvas->clipRect(SkRect::MakeWH(90, 80), SkClipOp::kIntersect, alias);
        canvas->drawCircle(100, 60, 60, paint);
        canvas->restore();
        canvas->translate(80, 0);
    }
}
}  // END FIDDLE
