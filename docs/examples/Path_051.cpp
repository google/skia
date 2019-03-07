// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=78ad51fa1cd33eb84a6f99061e56e067
REG_FIDDLE(Path_051, 256, 110, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPoint quadPts[] = {{20, 90}, {120, 10}, {220, 90}};
    canvas->drawLine(quadPts[0], quadPts[1], paint);
    canvas->drawLine(quadPts[1], quadPts[2], paint);
    SkPath path;
    path.moveTo(quadPts[0]);
    path.quadTo(quadPts[1], quadPts[2]);
    paint.setStrokeWidth(3);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
