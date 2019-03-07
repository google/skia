// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=635d54b4716e226e93dfbc21ad40e77d
REG_FIDDLE(Canvas_069, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    paint.setColor(0x80349a45);
    const SkPoint points[] = {{32, 16}, {48, 48}, {16, 32}};
    const SkPaint::Join join[] = { SkPaint::kRound_Join,
                                   SkPaint::kMiter_Join,
                                   SkPaint::kBevel_Join };
    int joinIndex = 0;
    SkPath path;
    path.addPoly(points, 3, false);
    for (const auto cap : { SkPaint::kRound_Cap, SkPaint::kSquare_Cap, SkPaint::kButt_Cap } ) {
        paint.setStrokeCap(cap);
        paint.setStrokeJoin(join[joinIndex++]);
        for (const auto mode : { SkCanvas::kPoints_PointMode,
                                 SkCanvas::kLines_PointMode,
                                 SkCanvas::kPolygon_PointMode } ) {
            canvas->drawPoints(mode, 3, points, paint);
            canvas->translate(64, 0);
        }
        canvas->drawPath(path, paint);
        canvas->translate(-192, 64);
    }
}
}  // END FIDDLE
