// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(fluttershadow, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    SkPath path;
    path.moveTo(50.0, 50.0);

    // Comment the next 4 lines, and the shadow will draw.
    path.lineTo(60.0, 50.0);
    path.lineTo(60.0, 60.0);
    path.lineTo(70.0, 60.0);
    path.lineTo(70.0, 50.0);

    path.lineTo(150.0, 50.0);
    path.lineTo(150.0, 150.0);
    path.lineTo(50.0, 150.0);
    path.close();
    p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 3));
    canvas->drawPath(path, p);
}
}  // END FIDDLE
