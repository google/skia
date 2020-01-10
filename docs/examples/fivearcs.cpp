// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(fivearcs, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255, 255, 255, 255));
    SkPaint ovalPaint;
    ovalPaint.setAntiAlias(true);
    ovalPaint.setStyle(SkPaint::kStroke_Style);
    SkPaint arcPaint(ovalPaint);
    arcPaint.setStrokeWidth(5);
    arcPaint.setColor(SK_ColorBLUE);
    SkRect oval = {8, 8, 56, 56};

    canvas->drawOval(oval, ovalPaint);
    SkPath arcPath;
    arcPath.arcTo(oval, 0, 90, false);
    canvas->drawPath(arcPath, arcPaint);

    canvas->translate(64, 0);
    canvas->drawOval(oval, ovalPaint);
    canvas->drawArc(oval, 0, 90, false, arcPaint);

    canvas->translate(64, 0);
    canvas->drawOval(oval, ovalPaint);
    arcPath.reset();
    arcPath.addArc(oval, 0, 90);
    canvas->drawPath(arcPath, arcPaint);

    canvas->translate(-96, 64);
    canvas->drawOval(oval, ovalPaint);
    arcPath.reset();
    arcPath.moveTo({56, 32});
    arcPath.arcTo({56, 56}, {32, 56}, 24);
    canvas->drawPath(arcPath, arcPaint);

    canvas->translate(64, 0);
    canvas->drawOval(oval, ovalPaint);
    arcPath.reset();
    arcPath.moveTo({56, 32});
    arcPath.arcTo({24, 24}, 0, SkPath::kSmall_ArcSize, SkPathDirection::kCW, {32, 56});
    canvas->drawPath(arcPath, arcPaint);
}
}  // END FIDDLE
