#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=45b9ea2247b9ca7f10aa22ea29a426f4
REG_FIDDLE(Region_034, 256, 120, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath textPath;
    paint.getTextPath("Q", 1, 0, 110, &textPath);
    SkIRect clipRect = {20, 20, 100, 120};
    SkRegion clipRegion(clipRect);
    SkRegion region;
    region.setPath(textPath, clipRegion);
    canvas->drawRegion(region, SkPaint());
    clipRect.offset(100, 0);
    textPath.offset(100, 0);
    canvas->clipRect(SkRect::Make(clipRect), false);
    canvas->drawPath(textPath, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
