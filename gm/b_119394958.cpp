/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

DEF_SIMPLE_GM(b_119394958, canvas, 100, 100) {
    // The root cause of this bug was that a stroked arc with round caps was batched with a filled
    // circle. The circle op code would choose a GeometryProcessor configuration that expected round
    // cap centers as vertex attributes. However, the tessellation code for the filled circle would
    // not put in zero-width round cap centers and then didn't advance the pointer into which
    // vertex data was being written by the expected vertex stride.
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);
    canvas->drawCircle(50, 50, 45, paint);
    paint.setColor(SK_ColorGREEN);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);
    canvas->drawCircle(50, 50, 35, paint);
    paint.setColor(SK_ColorRED);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    canvas->drawArc(SkRect::MakeLTRB(30, 30, 70, 70), 0, 110, false, paint);
}
