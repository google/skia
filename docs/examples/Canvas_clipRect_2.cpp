// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=13bbc5fa5597a6cd4d704b419dbc66d9
REG_FIDDLE(Canvas_clipRect_2, 280, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    for (SkClipOp op: { SkClipOp::kIntersect, SkClipOp::kDifference } ) {
        canvas->save();
        canvas->clipRect(SkRect::MakeWH(90, 120), op, false);
        canvas->drawCircle(100, 100, 60, paint);
        canvas->restore();
        canvas->translate(80, 0);
    }
}
}  // END FIDDLE
