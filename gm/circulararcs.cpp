/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include "SkCanvas.h"
#include "gm.h"

static constexpr SkScalar kStarts[] = {0.f, 10.f, 30.f, 45.f, 90.f, 165.f, 180.f, 270.f};
static constexpr SkScalar kSweeps[] = {1.f, 45.f, 90.f, 130.f, 180.f, 270.f, 300.f, 355.f};
static constexpr SkScalar kDiameter = 40.f;
static constexpr SkRect kRect = {0.f, 0.f, kDiameter, kDiameter};
static constexpr int kW = 1000;
static constexpr int kH = 1000;
static constexpr SkScalar kPad = 20.f;

void draw_arcs(SkCanvas* canvas, std::function<void(SkPaint*)> configureStyle) {
    // Draws grid of arcs with different start/sweep angles in red and their complement arcs in
    // blue.
    auto drawGrid = [canvas, &configureStyle] (SkScalar x, SkScalar y, bool useCenter, bool aa) {
        SkPaint p0;
        p0.setColor(SK_ColorRED);
        p0.setAntiAlias(aa);
        // Set a reasonable stroke width that configureStyle can override.
        p0.setStrokeWidth(15.f);
        SkPaint p1 = p0;
        p1.setColor(SK_ColorBLUE);
        // Use alpha so we see magenta on overlap between arc and its complement.
        p0.setAlpha(100);
        p1.setAlpha(100);
        configureStyle(&p0);
        configureStyle(&p1);

        canvas->save();
        canvas->translate(kPad + x, kPad + y);
        for (auto start : kStarts) {
            canvas->save();
            for (auto sweep : kSweeps) {
                canvas->drawArc(kRect, start, sweep, useCenter, p0);
                canvas->drawArc(kRect, start, -(360.f - sweep), useCenter, p1);
                canvas->translate(kRect.width() + kPad, 0.f);
            }
            canvas->restore();
            canvas->translate(0, kRect.height() + kPad);
        }
        canvas->restore();
    };
    // Draw a grids for combo of enabling/disabling aa and using center.
    static constexpr SkScalar kGridW = kW / 2.f;
    static constexpr SkScalar kGridH = kH / 2.f;
    drawGrid(0.f   , 0.f   , false, false);
    drawGrid(kGridW, 0.f   , true , false);
    drawGrid(0.f   , kGridH, false, true );
    drawGrid(kGridW, kGridH, true , true );
    // Draw separators between the grids.
    SkPaint linePaint;
    linePaint.setAntiAlias(true);
    linePaint.setColor(SK_ColorBLACK);
    canvas->drawLine(kGridW, 0.f   , kGridW,            SkIntToScalar(kH), linePaint);
    canvas->drawLine(0.f   , kGridH, SkIntToScalar(kW), kGridH,            linePaint);
}

#define DEF_ARC_GM(name) DEF_SIMPLE_GM(circular_arcs_##name, canvas, kW, kH)

DEF_ARC_GM(fill) {
    auto setFill = [] (SkPaint*p) { p->setStyle(SkPaint::kFill_Style); };
    draw_arcs(canvas, setFill);
}

DEF_ARC_GM(hairline) {
    auto setHairline = [] (SkPaint* p) {
        p->setStyle(SkPaint::kStroke_Style);
        p->setStrokeWidth(0.f);
    };
    draw_arcs(canvas, setHairline);
}

DEF_ARC_GM(stroke_butt) {
    auto setStroke = [](SkPaint* p) {
        p->setStyle(SkPaint::kStroke_Style);
        p->setStrokeCap(SkPaint::kButt_Cap);
    };
    draw_arcs(canvas, setStroke);
}

DEF_ARC_GM(stroke_square) {
    auto setStroke = [] (SkPaint* p) {
        p->setStyle(SkPaint::kStroke_Style);
        p->setStrokeCap(SkPaint::kSquare_Cap);
    };
    draw_arcs(canvas, setStroke);
}

DEF_ARC_GM(stroke_round) {
    auto setStroke = [] (SkPaint* p) {
        p->setStyle(SkPaint::kStroke_Style);
        p->setStrokeCap(SkPaint::kRound_Cap);
    };
    draw_arcs(canvas, setStroke);
}
