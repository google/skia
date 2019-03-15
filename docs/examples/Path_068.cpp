#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=386000684073fccabc224d7d6dc81cd9
REG_FIDDLE(Path_068, 256, 226, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint tangentPaint;
    tangentPaint.setAntiAlias(true);
    SkPaint textPaint(tangentPaint);
    tangentPaint.setStyle(SkPaint::kStroke_Style);
    tangentPaint.setColor(SK_ColorGRAY);
    SkPaint arcPaint(tangentPaint);
    arcPaint.setStrokeWidth(5);
    arcPaint.setColor(SK_ColorBLUE);
    SkPath path;
    SkPoint pts[] = { {56, 20}, {200, 20}, {90, 190} };
    SkScalar radius = 50;
    path.moveTo(pts[0]);
    path.arcTo(pts[1], pts[2], radius);
    canvas->drawLine(pts[0], pts[1], tangentPaint);
    canvas->drawLine(pts[1], pts[2], tangentPaint);
    SkPoint lastPt;
    (void) path.getLastPt(&lastPt);
    SkVector radial = pts[2] - pts[1];
    radial.setLength(radius);
    SkPoint center = { lastPt.fX - radial.fY, lastPt.fY + radial.fX };
    canvas->drawCircle(center, radius, tangentPaint);
    canvas->drawLine(lastPt, center, tangentPaint);
    radial = pts[1] - pts[0];
    radial.setLength(radius);
    SkPoint arcStart = { center.fX + radial.fY, center.fY - radial.fX };
    canvas->drawLine(center, arcStart, tangentPaint);
    canvas->drawPath(path, arcPaint);
    canvas->drawString("(x0, y0)", pts[0].fX - 5, pts[0].fY, textPaint);
    canvas->drawString("(x1, y1)", pts[1].fX + 5, pts[1].fY, textPaint);
    canvas->drawString("(x2, y2)", pts[2].fX, pts[2].fY + 15, textPaint);
    canvas->drawString("radius", center.fX + 15, center.fY + 25, textPaint);
    canvas->drawString("radius", center.fX - 3, center.fY - 16, textPaint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
