/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"

DEF_SIMPLE_GM(crbug_847759, canvas, 500, 500) {
    // This path exposed an issue in AAHairlinePathRenderer. When converting from cubics to quads
    // we produced quads where the previously vertical tangents at the left and right tips of the
    // squashed oval-like path became slightly non-vertical. This caused a missed pixel of AA just
    // outside each tip.
    SkPathBuilder path;
    path.moveTo(97,374.5f);
    path.cubicTo(97,359.8644528f,155.8745488f,348,228.5f,348);
    path.cubicTo(301.1254512f,348,360,359.8644528f,360,374.5f);
    path.cubicTo(360,389.1355472f,301.1254512f,401,228.5f,401);
    path.cubicTo(155.8745488f,401,97,389.1355472f,97,374.5f);
    path.close();
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(0);
    paint.setStrokeMiter(1.5);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->translate(-80, -330);
    canvas->drawPath(path.detach(), paint);
}
