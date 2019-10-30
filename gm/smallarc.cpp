/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

// this draws a small arc scaled up
// see https://code.google.com/p/chromium/issues/detail?id=102411
// and https://code.google.com/p/skia/issues/detail?id=2769
DEF_SIMPLE_GM(smallarc, canvas, 762, 762) {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(120);

        SkPath path;
        path.moveTo(75, 0);
        path.cubicTo(33.5, 0, 0, 33.5, 0, 75);

        canvas->translate(-400, -400);
        canvas->scale(8, 8);
        canvas->drawPath(path, p);
}
