// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1d4e0632c97e42692775d834fe10aa99
REG_FIDDLE(Canvas_clipRect_3, 256, 133, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0x8055aaff);
    SkRect clipRect = { 0, 0, 87.4f, 87.4f };
    for (auto alias: { false, true } ) {
        canvas->save();
        canvas->clipRect(clipRect, SkClipOp::kIntersect, alias);
        canvas->drawCircle(67, 67, 60, paint);
        canvas->restore();
        canvas->save();
        canvas->clipRect(clipRect, SkClipOp::kDifference, alias);
        canvas->drawCircle(67, 67, 60, paint);
        canvas->restore();
        canvas->translate(120, 0);
    }
}
}  // END FIDDLE
