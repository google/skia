// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_clipRegion, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkIRect iRect = {30, 40, 120, 130 };
    SkRegion region(iRect);
    canvas->rotate(10);
    canvas->save();
    canvas->clipRegion(region, SkClipOp::kIntersect);
    canvas->drawCircle(50, 50, 45, paint);
    canvas->restore();
    canvas->translate(100, 100);
    canvas->clipRect(SkRect::Make(iRect), SkClipOp::kIntersect);
    canvas->drawCircle(50, 50, 45, paint);
}
}  // END FIDDLE
