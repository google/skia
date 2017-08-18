/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"

DEF_SIMPLE_GM(stroketext, canvas, 200, 480) {

    SkPoint loc = { 20, 460 };

    SkPaint p;
    p.setTextSize(25);
    canvas->drawString("P", loc.fX, loc.fY - 225, p);
    canvas->drawPosText("P", 1, &loc, p);
}
