/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

// Repro case for http://skbug.com/9453
DEF_SIMPLE_GM(inverseclip, canvas, 400, 400) {
    SkPath clip;
    clip.setFillType(SkPath::kInverseWinding_FillType);
    clip.moveTo(195.448f, 31);
    clip.cubicTo(97.9925f, 31, 18.99f, 105.23f, 18.99f, 196.797f);
    clip.cubicTo(18.99f, 288.365f, 97.9925f, 362.595f, 195.448f, 362.595f);
    clip.cubicTo(292.905f, 362.595f, 371.905f, 288.365f, 371.905f, 196.797f);
    clip.cubicTo(371.905f, 105.23f, 292.905f, 31, 195.448f, 31);
    clip.close();
    canvas->clipPath(clip, true);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeWH(400, 400), paint);
}
