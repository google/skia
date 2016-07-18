/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

DEF_SIMPLE_GM(bug615686, canvas, 250, 250) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(20);
    SkPath path;
    path.moveTo(0, 0);
    path.cubicTo(200, 200, 0, 200, 200, 0);
    canvas->drawPath(path, p);
}
