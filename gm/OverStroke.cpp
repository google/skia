/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*
 * This GM exercises stroking of paths with large stroke lengths, which is
 * referred to as "overstroke" for brevity. In Skia as of 8/2016 we offset
 * each part of the curve the request amount even if it makes the offsets
 * overlap and create holes. There is not a really great algorithm for this
 * and several other 2D graphics engines have the same bug.
 *
 * See crbug.com/589769 skbug.com/5405 skbug.com/5406
 */


#include "gm.h"
#include "SkPaint.h"
#include "SkPath.h"

//////// path and paint builders

SkPaint make_overstroke_paint() {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(500);

    return p;
}

SkPath quad_path() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(100, 0);
    path.quadTo(50, -40,
                0, 0);

    return path;
}

SkPath cubic_path() {
    SkPath path;
    path.moveTo(0, 0);
    path.cubicTo(25, 75,
                 75, -50,
                 100, 0);

    return path;
}

SkPath oval_path() {
    SkRect oval = SkRect::MakeXYWH(0, -25, 100, 50);

    SkPath path;
    path.arcTo(oval, 0, 359, true);
    path.close();

    return path;
}

///////// quads

void draw_small_quad(SkCanvas *canvas) {
    // scaled so it's visible
    canvas->scale(8, 8);

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(3);

    SkPath path = quad_path();

    canvas->drawPath(path, p);
}

void draw_large_quad(SkCanvas *canvas) {
    SkPaint p = make_overstroke_paint();
    SkPath path = quad_path();

    canvas->drawPath(path, p);
}

void draw_quad_fillpath(SkCanvas *canvas) {
    SkPath path = quad_path();
    SkPaint p = make_overstroke_paint();

    SkPaint fillp;
    fillp.setAntiAlias(true);
    fillp.setStyle(SkPaint::kStroke_Style);
    fillp.setColor(SK_ColorMAGENTA);

    SkPath fillpath;
    p.getFillPath(path, &fillpath);

    canvas->drawPath(fillpath, fillp);
}

void draw_stroked_quad(SkCanvas *canvas) {
    canvas->translate(200, 0);
    draw_large_quad(canvas);
    draw_quad_fillpath(canvas);
}

////////// cubics

void draw_small_cubic(SkCanvas *canvas) {
    // scaled so it's visible
    canvas->scale(8, 8);

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(3);

    SkPath path = cubic_path();

    canvas->drawPath(path, p);
}

void draw_large_cubic(SkCanvas *canvas) {
    SkPaint p = make_overstroke_paint();
    SkPath path = cubic_path();

    canvas->drawPath(path, p);
}

void draw_cubic_fillpath(SkCanvas *canvas) {
    SkPath path = cubic_path();
    SkPaint p = make_overstroke_paint();

    SkPaint fillp;
    fillp.setAntiAlias(true);
    fillp.setStyle(SkPaint::kStroke_Style);
    fillp.setColor(SK_ColorMAGENTA);

    SkPath fillpath;
    p.getFillPath(path, &fillpath);

    canvas->drawPath(fillpath, fillp);
}

void draw_stroked_cubic(SkCanvas *canvas) {
    canvas->translate(400, 0);
    draw_large_cubic(canvas);
    draw_cubic_fillpath(canvas);
}

////////// ovals

void draw_small_oval(SkCanvas *canvas) {
    // scaled so it's visible
    canvas->scale(8, 8);

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(3);

    SkPath path = oval_path();

    canvas->drawPath(path, p);
}

void draw_large_oval(SkCanvas *canvas) {
    SkPaint p = make_overstroke_paint();
    SkPath path = oval_path();

    canvas->drawPath(path, p);
}

void draw_oval_fillpath(SkCanvas *canvas) {
    SkPath path = oval_path();
    SkPaint p = make_overstroke_paint();

    SkPaint fillp;
    fillp.setAntiAlias(true);
    fillp.setStyle(SkPaint::kStroke_Style);
    fillp.setColor(SK_ColorMAGENTA);

    SkPath fillpath;
    p.getFillPath(path, &fillpath);

    canvas->drawPath(fillpath, fillp);
}

void draw_stroked_oval(SkCanvas *canvas) {
    canvas->translate(400, 0);
    draw_large_oval(canvas);
    draw_oval_fillpath(canvas);
}

////////// gm

void (*examples[])(SkCanvas *canvas) = {
    draw_small_quad,    draw_stroked_quad, draw_small_cubic,
    draw_stroked_cubic, draw_small_oval,   draw_stroked_oval,
};

DEF_SIMPLE_GM(OverStroke, canvas, 500, 500) {
    const size_t length = sizeof(examples) / sizeof(examples[0]);
    const size_t width = 2;

    for (size_t i = 0; i < length; i++) {
        int x = (int)(i % width);
        int y = (int)(i / width);

        canvas->save();
        canvas->translate(200.0f * x, 150.0f * y);
        canvas->scale(0.25f, 0.25f);
        canvas->translate(100.0f, 400.0f);

        examples[i](canvas);

        canvas->restore();
    }
}
