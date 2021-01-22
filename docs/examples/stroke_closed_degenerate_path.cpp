// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(stroke_closed_degenerate_path, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.addRect({50.f, 50.f, 50.f, 50.f});

    SkPaint joinStroke;
    joinStroke.setColor(SK_ColorGREEN);
    joinStroke.setStrokeWidth(10.f);
    joinStroke.setStyle(SkPaint::kStroke_Style);
    joinStroke.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawPath(path, joinStroke);

    canvas->translate(100.f, 0);

    SkPaint capStroke;
    capStroke.setColor(SK_ColorRED);
    capStroke.setStrokeWidth(10.f);
    capStroke.setStyle(SkPaint::kStroke_Style);
    capStroke.setStrokeCap(SkPaint::kRound_Cap);
    canvas->drawPath(path, capStroke);
}
}  // END FIDDLE
