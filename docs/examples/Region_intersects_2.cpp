#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4263d79ac0e7df02e90948fdde9fa965
REG_FIDDLE(Region_intersects_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath hPath, dotPath;
    paint.getTextPath("H", 1, 40, 110, &hPath);
    paint.getTextPath(",", 1, frame * 180, 95, &dotPath);
    SkRegion hRegion, dotRegion;
    hRegion.setPath(hPath, SkRegion({0, 0, 256, 256}));
    dotRegion.setPath(dotPath, SkRegion({0, 0, 256, 256}));
    canvas->drawRegion(hRegion, paint);
    paint.setColor(hRegion.intersects(dotRegion) ? SK_ColorBLUE : SK_ColorRED);
    canvas->drawRegion(dotRegion, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
