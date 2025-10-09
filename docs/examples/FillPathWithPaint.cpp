// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(FillPathWithPaint, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(.1f);
    SkPath strokePath = SkPathBuilder()
                        .moveTo(.08f, .08f)
                        .quadTo(.09f, .08f, .17f, .17f)
                        .detach();
    SkPathBuilder fillPath;
    SkPaint outlinePaint(strokePaint);
    outlinePaint.setStrokeWidth(2);
    SkMatrix scale = SkMatrix::Scale(300, 300);
    for (SkScalar precision : { 0.01f, .1f, 1.f, 10.f, 100.f } ) {
        skpathutils::FillPathWithPaint(strokePath, strokePaint, &fillPath, nullptr,
                                       SkMatrix::Scale(precision, precision));
        fillPath.transform(scale);
        canvas->drawPath(fillPath.detach(), outlinePaint);
        canvas->translate(60, 0);
        if (1.f == precision) canvas->translate(-180, 100);
    }
    strokePath = strokePath.makeTransform(scale);
    strokePaint.setStrokeWidth(30);
    canvas->drawPath(strokePath, strokePaint);
}
}  // END FIDDLE
