/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

/*
 * Canvas example. Expected large blue stroked circle, white middle, small red circle.
 * GPU-accelerated canvas produces large blue stroked circle, white middle, NO red circle.
 *
 * 1:  var c = document.getElementById("myCanvas");
 * 2:  var ctx = c.getContext("2d");
 * 3:  ctx.beginPath();
 * 4:  ctx.scale(203.20, 203.20);
 * 5:  ctx.translate(-14.55, -711.51);
 * 6:  ctx.fillStyle = "red";
 * 7:  ctx.strokeStyle = "blue";
 * 8:  //ctx.lineWidth = 1/203.20;
 * 9:  ctx.arc(19.221, 720-6.76,0.0295275590551181,0,2*Math.PI);
 * 10: ctx.stroke();
 * 11: ctx.fill();
 * 12: ctx.closePath();
*/
DEF_SIMPLE_GM_BG(crbug_996140, canvas, 300, 300, SK_ColorWHITE) {
    // Specific parameters taken from the canvas minimum working example
    SkScalar cx = 19.221f;
    SkScalar cy = 720-6.76f;
    SkScalar radius = 0.0295275590551181f;

    SkScalar s = 203.20f;
    SkScalar tx = -14.55f;
    SkScalar ty = -711.51f;

    // 0: The test canvas was 1920x574 and the circle was located in the bottom left, but that's
    // not necessary to reproduce the problem, so translate to make a smaller GM.
    canvas->translate(-800, -200);

    // 3: ctx.beginPath();
    SkPath path;

    // 4: ctx.scale(203.20, 203.20);
    canvas->scale(s, s);
    // 5: ctx.translate(-14.55, -711.51);
    canvas->translate(tx, ty);

    // 6: ctx.fillStyle = "red";
    SkPaint fill;
    fill.setColor(SK_ColorRED);
    fill.setStyle(SkPaint::kFill_Style);
    fill.setAntiAlias(true);

    // 7: ctx.strokeStyle = "blue";
    SkPaint stroke;
    stroke.setColor(SK_ColorBLUE);
    stroke.setStrokeWidth(1.f);
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setAntiAlias(true);

    // 9: ctx.arc(19.221, 720-6.76,0.0295275590551181,0,2*Math.PI);
    // This matches how Canvas prepares an arc(x, y, radius, 0, 2pi) call
    SkRect boundingBox = SkRect::MakeLTRB(cx - radius, cy - radius, cx + radius, cy + radius);
    path.arcTo(boundingBox, 0, 180.f, false);
    path.arcTo(boundingBox, 180.f, 180.f, false);

    // 12: ctx.closePath();
    // path.close();

    // 10: ctx.stroke(); (NOT NEEDED TO REPRODUCE FAILING RED CIRCLE)
    canvas->drawPath(path, stroke);
    // 11: ctx.fill()
    canvas->drawPath(path, fill);
}
