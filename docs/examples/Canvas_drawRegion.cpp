// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=80309e0deca0f8add616cec7bec634ca
REG_FIDDLE(Canvas_drawRegion, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    region.op( 10, 10, 50, 50, SkRegion::kUnion_Op);
    region.op( 10, 50, 90, 90, SkRegion::kUnion_Op);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawRegion(region, paint);
}
}  // END FIDDLE
