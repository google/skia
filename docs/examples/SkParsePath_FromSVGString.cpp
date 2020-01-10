// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkParsePath_FromSVGString, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const char pathString[] =
            "M 243 128 "
            "L 24 178 "
            "L 200 38 "
            "L 102 240 "
            "L 102 16 "
            "L 200 218 "
            "L 24 78 "
            "Z";
    SkPath path;
    SkParsePath::FromSVGString(pathString, &path);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(2);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
