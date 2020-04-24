// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=48d963ad4286eddf680f9c511eb6da91
REG_FIDDLE(Paint_setStrokeJoin, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeJoin(SkPaint::kMiter_Join);
    paint.setStrokeJoin((SkPaint::Join) SkPaint::kJoinCount);
    SkDebugf("kMiter_Join %c= paint.getStrokeJoin()\n",
            SkPaint::kMiter_Join == paint.getStrokeJoin() ? '=' : '!');
}
}  // END FIDDLE
