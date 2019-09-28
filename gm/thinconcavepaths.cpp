/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkScalar.h"

namespace {
// Test thin stroked rect (stroked "by hand", not by stroking).
void draw_thin_stroked_rect(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(10 + width, 10 + width);
    path.lineTo(40,         10 + width);
    path.lineTo(40,         20);
    path.lineTo(10 + width, 20);
    path.moveTo(10,         10);
    path.lineTo(10,         20 + width);
    path.lineTo(40 + width, 20 + width);
    path.lineTo(40 + width, 10);
    canvas->drawPath(path, paint);
}

void draw_thin_right_angle(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(10 + width, 10 + width);
    path.lineTo(40,         10 + width);
    path.lineTo(40,         20);
    path.lineTo(40 + width, 20 + width);
    path.lineTo(40 + width, 10);
    path.lineTo(10,         10);
    canvas->drawPath(path, paint);
}

// Test thin horizontal line (<1 pixel) which should give lower alpha.
void draw_golf_club(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(20, 10);
    path.lineTo(80, 10);
    path.lineTo(80, 10 + width);
    path.lineTo(30, 10 + width);
    path.lineTo(30, 20);
    path.lineTo(20, 20);
    canvas->drawPath(path, paint);
}

// Test thin lines between two filled regions. The outer edges overlap, but
// there are no inverted edges to fix.
void draw_barbell(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkScalar offset = width * 0.5f;
    SkPath path;
    path.moveTo(30,  5);
    path.lineTo(40 - offset, 15 - offset);
    path.lineTo(60 + offset, 15 - offset);
    path.lineTo(70,  5);
    path.lineTo(70, 25);
    path.lineTo(60 + offset, 15 + offset);
    path.lineTo(40 - offset, 15 + offset);
    path.lineTo(30, 25);
    canvas->drawPath(path, paint);
}

// Test a thin rectangle and triangle. The top and bottom inner edges of the
// rectangle and all inner edges of the triangle invert on stroking.
void draw_thin_rect_and_triangle(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(30,  5);
    path.lineTo(30 + width,  5);
    path.lineTo(30 + width,  25);
    path.lineTo(30,  25);
    path.moveTo(40,  5);
    path.lineTo(40 + width,  5);
    path.lineTo(40,  25);
    canvas->drawPath(path, paint);
}

// Two triangles joined by a very thin bridge. The tiny triangle formed
// by the inner edges at the bridge is inverted.
// (These are actually now more phat pants than hipster pants.)
void draw_hipster_pants(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(10, 10);
    path.lineTo(10, 20);
    path.lineTo(50, 10 + width);
    path.lineTo(90, 20);
    path.lineTo(90, 10);
    canvas->drawPath(path, paint);
}

// A thin z-shape whose interior inverts on stroking. The top and bottom inner edges invert, and
// the connector edges at the "elbows" intersect the inner edges.
void draw_skinny_snake(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(20 + width, 10);
    path.lineTo(20 + width, 20);
    path.lineTo(10 + width, 30);
    path.lineTo(10 + width, 40);
    path.lineTo(10 - width, 40);
    path.lineTo(10 - width, 30);
    path.lineTo(20 - width, 20);
    path.lineTo(20 - width, 10);
    canvas->drawPath(path, paint);
}

// Test pointy features whose outer edges extend far to the right on stroking.
void draw_pointy_golf_club(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(20, 10);
    path.lineTo(80, 10 + width * 0.5);
    path.lineTo(30, 10 + width);
    path.lineTo(30, 20);
    path.lineTo(20, 20);
    canvas->drawPath(path, paint);
}

void draw_small_i(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(1.25 - width, 18.75 + width);
    path.lineTo(1.25 - width, 12.25 - width);
    path.lineTo(2.50 + width, 12.25 - width);
    path.lineTo(2.50 + width, 18.75 + width);
    path.moveTo(1.25 - width, 11.75 + width);
    path.lineTo(1.25 - width, 10.25 - width);
    path.lineTo(2.50 + width, 10.25 - width);
    path.lineTo(2.50 + width, 11.75 + width);
    canvas->drawPath(path, paint);
}



};

DEF_SIMPLE_GM(thinconcavepaths, canvas, 550, 400) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    canvas->save();
    for (SkScalar width = 0.5f; width < 2.05f; width += 0.25f) {
        draw_thin_stroked_rect(canvas, paint, width);
        canvas->translate(0, 25);
    }
    canvas->restore();
    canvas->translate(50, 0);
    canvas->save();
    for (SkScalar width = 0.5f; width < 2.05f; width += 0.25f) {
        draw_thin_right_angle(canvas, paint, width);
        canvas->translate(0, 25);
    }
    canvas->restore();
    canvas->translate(40, 0);
    canvas->save();
    for (SkScalar width = 0.2f; width < 2.1f; width += 0.2f) {
        draw_golf_club(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(70, 0);
    canvas->save();
    for (SkScalar width = 0.2f; width < 2.1f; width += 0.2f) {
        draw_thin_rect_and_triangle(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(30, 0);
    canvas->save();

    for (SkScalar width = 0.2f; width < 2.1f; width += 0.2f) {
        draw_barbell(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(80, 0);
    canvas->save();
    for (SkScalar width = 0.2f; width < 2.1f; width += 0.2f) {
        draw_hipster_pants(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(100, 0);
    canvas->save();
    for (SkScalar width = 0.2f; width < 2.1f; width += 0.2f) {
        draw_skinny_snake(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(30, 0);
    canvas->save();
    for (SkScalar width = 0.2f; width < 2.1f; width += 0.2f) {
        draw_pointy_golf_club(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(100, 0);
    canvas->save();
    for (SkScalar width = 0.0f; width < 0.5f; width += 0.05f) {
        draw_small_i(canvas, paint, width);
        canvas->translate(0, 30);
    }
    canvas->restore();
    canvas->translate(100, 0);
}
