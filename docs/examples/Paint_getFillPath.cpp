// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=cedd6233848198e1fca4d1e14816baaf
REG_FIDDLE(Paint_getFillPath, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(.1f);
    SkPath strokePath;
    strokePath.moveTo(.08f, .08f);
    strokePath.quadTo(.09f, .08f, .17f, .17f);
    SkPath fillPath;
    SkPaint outlinePaint(strokePaint);
    outlinePaint.setStrokeWidth(2);
    SkMatrix scale = SkMatrix::MakeScale(300, 300);
    for (SkScalar precision : { 0.01f, .1f, 1.f, 10.f, 100.f } ) {
        strokePaint.getFillPath(strokePath, &fillPath, nullptr, precision);
        fillPath.transform(scale);
        canvas->drawPath(fillPath, outlinePaint);
        canvas->translate(60, 0);
        if (1.f == precision) canvas->translate(-180, 100);
    }
    strokePath.transform(scale);
    strokePaint.setStrokeWidth(30);
    canvas->drawPath(strokePath, strokePaint);
}
}  // END FIDDLE
