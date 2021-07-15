/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"

/*
 * Canvas example from https://crbug.com/1229486
 *
 *    ctx.shadowOffsetX = 20;
 *    ctx.shadowOffsetY = 20;
 *    ctx.shadowBlur = 40;
 *    ctx.shadowColor = 'blue'
 *    ctx.lineWidth = 50;
 *    ctx.rotate(Math.PI / 5);
 *    ctx.scale(1.2, 1.2);
 *    ctx.strokeRect(100, 0, 1, 1);
 */
DEF_SIMPLE_GM_BG(crbug_1229486, canvas, 200, 200, SK_ColorWHITE) {
    SkPaint stroke;
    stroke.setColor(SK_ColorBLACK);
    stroke.setStrokeWidth(50);
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setAntiAlias(true);

    canvas->rotate(180 / 5);
    canvas->scale(1.2f, 1.2f);

    canvas->drawRect(SkRect::MakeXYWH(100, 1, 1, 1), stroke);
}
