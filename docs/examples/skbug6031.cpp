// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skbug6031, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    constexpr float kW = 512;
    constexpr float kH = 512;

    SkPaint paint;
    static constexpr SkClipOp replaceClipOp = static_cast<SkClipOp>(5);

    canvas->clipRect(SkRect{0, 0, 2 * kW, 2 * kH}, replaceClipOp);
    paint.setColor(SkColorSetARGB(0, 0, 0, 0));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect{0, 0, kW, kH}, replaceClipOp);
    paint.setColor(SkColorSetARGB(255, 127, 0, 0));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect{0, kH, kW, 3 * kH}, replaceClipOp);
    paint.setColor(SkColorSetARGB(255, 0, 127, 0));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect{kW, 0, 3 * kW, kH}, replaceClipOp);
    paint.setColor(SkColorSetARGB(255, 0, 0, 127));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect{kW, kH, 3 * kW, 3 * kH}, replaceClipOp);
    paint.setColor(SkColorSetARGB(255, 127, 127, 0));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
