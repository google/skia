/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

// clipRect and drawLine should line up exactly when they use the same point.
// When SkPDF rounds large floats, this doesn't always happen.
DEF_SIMPLE_GM(skbug_4868, canvas, 32, 32) {
    canvas->translate(-68.0f, -3378.0f);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->scale(0.56692914f, 0.56692914f);
    SkRect rc = SkRect::MakeLTRB(158.0f, 5994.80273f, 165.0f, 5998.80225f);
    canvas->clipRect(rc);
    canvas->clear(0xFFCECFCE);
    canvas->drawLine(rc.left(), rc.top(), rc.right(), rc.bottom(), paint);
    canvas->drawLine(rc.right(), rc.top(), rc.left(), rc.bottom(), paint);
}
