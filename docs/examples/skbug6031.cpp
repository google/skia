// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skbug6031, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    double dWidth = 1024;
    double dHeight = 1024;

    SkPaint paint;
    canvas->clipRect(SkRect::MakeXYWH(0, 0, dWidth, dHeight), SkRegion::kReplace_Op);
    paint.setColor(SkColorSetARGB(0, 0, 0, 0));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect::MakeXYWH(0, 0, dWidth / 2.0, dHeight / 2.0),
                     SkRegion::kReplace_Op);
    paint.setColor(SkColorSetARGB(255, 127, 0, 0));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect::MakeXYWH(0, dHeight / 2.0, dWidth / 2.0, dHeight),
                     SkRegion::kReplace_Op);
    paint.setColor(SkColorSetARGB(255, 0, 127, 0));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect::MakeXYWH(dWidth / 2.0, 0, dWidth, dHeight / 2.0),
                     SkRegion::kReplace_Op);
    paint.setColor(SkColorSetARGB(255, 0, 0, 127));
    canvas->drawPaint(paint);

    canvas->clipRect(SkRect::MakeXYWH(dWidth / 2.0, dHeight / 2.0, dWidth, dHeight),
                     SkRegion::kReplace_Op);
    paint.setColor(SkColorSetARGB(255, 127, 127, 0));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
