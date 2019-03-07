// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Canvas_076, 256, 256, false, 0);
// HASH=80309e0deca0f8add616cec7bec634ca
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

}
