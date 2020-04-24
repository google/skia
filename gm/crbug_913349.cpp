/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(crbug_913349, canvas, 500, 600) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    // This is a reduction from crbug.com/913349 to 5 verts.
    SkPath path;
    path.moveTo( 349.5,  225.75);
    path.lineTo(  96.5,   74);
    path.lineTo( 500.50, 226);
    path.lineTo( 350,    226);
    path.lineTo( 350,    224);

    canvas->drawPath(path, paint);
}
