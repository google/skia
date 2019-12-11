/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/SkFloatBits.h"
#include "include/private/SkTArray.h"

#include <functional>

constexpr SkScalar kStarts[] = {0.f, 10.f, 30.f, 45.f, 90.f, 165.f, 180.f, 270.f};
constexpr SkScalar kSweeps[] = {1.f, 45.f, 90.f, 130.f, 180.f, 184.f, 300.f, 355.f};
constexpr SkScalar kDiameter = 40.f;
constexpr SkRect kRect = {0.f, 0.f, kDiameter, kDiameter};
constexpr int kW = 1000;
constexpr int kH = 1000;
constexpr SkScalar kPad = 20.f;

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
    constexpr SkScalar kGridW = kW / 2.f;
    constexpr SkScalar kGridH = kH / 2.f;
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

DEF_ARC_GM(stroke_and_fill_butt) {
    auto setStroke = [] (SkPaint* p) {
        p->setStyle(SkPaint::kStrokeAndFill_Style);
        p->setStrokeCap(SkPaint::kButt_Cap);
    };
    draw_arcs(canvas, setStroke);
}

DEF_ARC_GM(stroke_and_fill_square) {
    auto setStroke = [] (SkPaint* p) {
        p->setStyle(SkPaint::kStrokeAndFill_Style);
        p->setStrokeCap(SkPaint::kSquare_Cap);
    };
    draw_arcs(canvas, setStroke);
}

DEF_ARC_GM(stroke_and_fill_round) {
    auto setStroke = [] (SkPaint* p) {
        p->setStyle(SkPaint::kStrokeAndFill_Style);
        p->setStrokeCap(SkPaint::kRound_Cap);
    };
    draw_arcs(canvas, setStroke);
}

DEF_SIMPLE_GM(circular_arcs_weird, canvas, 1000, 400) {
    constexpr SkScalar kS = 50;
    struct Arc {
        SkRect   fOval;
        SkScalar fStart;
        SkScalar fSweep;
    };
    const Arc noDrawArcs[] = {
        // no sweep
        {SkRect::MakeWH(kS, kS),  0,  0},
        // empty rect in x
        {SkRect::MakeWH(-kS, kS), 0, 90},
        // empty rect in y
        {SkRect::MakeWH(kS, -kS), 0, 90},
        // empty rect in x and y
        {SkRect::MakeWH( 0,   0), 0, 90},
    };
    const Arc arcs[] = {
        // large start
        {SkRect::MakeWH(kS, kS),   810.f,   90.f},
        // large negative start
        {SkRect::MakeWH(kS, kS),  -810.f,   90.f},
        // exactly 360 sweep
        {SkRect::MakeWH(kS, kS),     0.f,  360.f},
        // exactly -360 sweep
        {SkRect::MakeWH(kS, kS),     0.f, -360.f},
        // exactly 540 sweep
        {SkRect::MakeWH(kS, kS),     0.f,  540.f},
        // exactly -540 sweep
        {SkRect::MakeWH(kS, kS),     0.f, -540.f},
        // generic large sweep and large start
        {SkRect::MakeWH(kS, kS),  1125.f,  990.f},
    };
    SkTArray<SkPaint> paints;
    // fill
    paints.push_back();
    // stroke
    paints.push_back().setStyle(SkPaint::kStroke_Style);
    paints.back().setStrokeWidth(kS / 6.f);
    // hairline
    paints.push_back().setStyle(SkPaint::kStroke_Style);
    paints.back().setStrokeWidth(0.f);
    // stroke and fill
    paints.push_back().setStyle(SkPaint::kStrokeAndFill_Style);
    paints.back().setStrokeWidth(kS / 6.f);
    // dash effect
    paints.push_back().setStyle(SkPaint::kStroke_Style);
    paints.back().setStrokeWidth(kS / 6.f);
    constexpr SkScalar kDashIntervals[] = {kS / 15, 2 * kS / 15};
    paints.back().setPathEffect(SkDashPathEffect::Make(kDashIntervals, 2, 0.f));

    canvas->translate(kPad, kPad);
    // This loop should draw nothing.
    for (auto arc : noDrawArcs) {
        for (auto paint : paints) {
            paint.setAntiAlias(true);
            canvas->drawArc(arc.fOval, arc.fStart, arc.fSweep, false, paint);
            canvas->drawArc(arc.fOval, arc.fStart, arc.fSweep, true, paint);
        }
    }

    SkPaint linePaint;
    linePaint.setAntiAlias(true);
    linePaint.setColor(SK_ColorRED);
    SkScalar midX   = SK_ARRAY_COUNT(arcs) * (kS + kPad) - kPad/2.f;
    SkScalar height = paints.count() * (kS + kPad);
    canvas->drawLine(midX, -kPad, midX, height, linePaint);

    for (auto paint : paints) {
        paint.setAntiAlias(true);
        canvas->save();
        for (auto arc : arcs) {
            canvas->drawArc(arc.fOval, arc.fStart, arc.fSweep, false, paint);
            canvas->translate(kS + kPad, 0.f);
        }
        for (auto arc : arcs) {
            canvas->drawArc(arc.fOval, arc.fStart, arc.fSweep, true, paint);
            canvas->translate(kS + kPad, 0.f);
        }
        canvas->restore();
        canvas->translate(0, kS + kPad);
    }
}

DEF_SIMPLE_GM(onebadarc, canvas, 100, 100) {
    SkPath path;
    path.moveTo(SkBits2Float(0x41a00000), SkBits2Float(0x41a00000));  // 20, 20
    path.lineTo(SkBits2Float(0x4208918c), SkBits2Float(0x4208918c));  // 34.1421f, 34.1421f
    path.conicTo(SkBits2Float(0x41a00000), SkBits2Float(0x42412318),  // 20, 48.2843f
            SkBits2Float(0x40bb73a0), SkBits2Float(0x4208918c),       // 5.85786f, 34.1421f
            SkBits2Float(0x3f3504f3));                                // 0.707107f
    path.quadTo(SkBits2Float(0x40bb73a0), SkBits2Float(0x4208918c),   // 5.85786f, 34.1421f
            SkBits2Float(0x40bb73a2), SkBits2Float(0x4208918c));      // 5.85787f, 34.1421f
    path.lineTo(SkBits2Float(0x41a00000), SkBits2Float(0x41a00000));  // 20, 20
    path.close();
    SkPaint p0;
    p0.setColor(SK_ColorRED);
    p0.setStrokeWidth(15.f);
    p0.setStyle(SkPaint::kStroke_Style);
    p0.setAlpha(100);
    canvas->translate(20, 0);
    canvas->drawPath(path, p0);

    SkRect kRect = { 60, 0, 100, 40};
    canvas->drawArc(kRect, 45, 90, true, p0);
}

DEF_SIMPLE_GM(crbug_888453, canvas, 480, 150) {
    // Two GPU path renderers were using a too-large tolerance when chopping connics to quads.
    // This manifested as not-very-round circular arcs at certain radii. All the arcs being drawn
    // here should look like circles.
    SkPaint fill;
    fill.setAntiAlias(true);
    SkPaint hairline = fill;
    hairline.setStyle(SkPaint::kStroke_Style);
    SkPaint stroke = hairline;
    stroke.setStrokeWidth(2.0f);
    int x = 4;
    int y0 = 25, y1 = 75, y2 = 125;
    for (int r = 2; r <= 20; ++r) {
        canvas->drawArc(SkRect::MakeXYWH(x - r, y0 - r, 2 * r, 2 * r), 0, 360, false, fill);
        canvas->drawArc(SkRect::MakeXYWH(x - r, y1 - r, 2 * r, 2 * r), 0, 360, false, hairline);
        canvas->drawArc(SkRect::MakeXYWH(x - r, y2 - r, 2 * r, 2 * r), 0, 360, false, stroke);
        x += 2 * r + 4;
    }
}

DEF_SIMPLE_GM(circular_arc_stroke_matrix, canvas, 820, 1090) {
    static constexpr SkScalar kRadius = 40.f;
    static constexpr SkScalar kStrokeWidth = 5.f;
    static constexpr SkScalar kStart = 89.f;
    static constexpr SkScalar kSweep = 180.f/SK_ScalarPI; // one radian

    SkTArray<SkMatrix> matrices;
    matrices.push_back().setRotate(kRadius, kRadius, 45.f);
    matrices.push_back(SkMatrix::I());
    matrices.push_back().setAll(-1,  0,  2*kRadius,
                                 0,  1,  0,
                                 0,  0,  1);
    matrices.push_back().setAll( 1,  0,  0,
                                 0, -1,  2*kRadius,
                                 0,  0,  1);
    matrices.push_back().setAll( 1,  0,  0,
                                 0, -1,  2*kRadius,
                                 0,  0,  1);
    matrices.push_back().setAll( 0, -1,  2*kRadius,
                                -1,  0,  2*kRadius,
                                 0,  0,  1);
    matrices.push_back().setAll( 0, -1,  2*kRadius,
                                 1,  0,  0,
                                 0,  0,  1);
    matrices.push_back().setAll( 0,  1,  0,
                                 1,  0,  0,
                                 0,  0,  1);
    matrices.push_back().setAll( 0,  1,  0,
                                -1,  0,  2*kRadius,
                                 0,  0,  1);
    int baseMatrixCnt = matrices.count();


    SkMatrix tinyCW;
    tinyCW.setRotate(0.001f, kRadius, kRadius);
    for (int i = 0; i < baseMatrixCnt; ++i) {
        matrices.push_back().setConcat(matrices[i], tinyCW);
    }
    SkMatrix tinyCCW;
    tinyCCW.setRotate(-0.001f, kRadius, kRadius);
    for (int i = 0; i < baseMatrixCnt; ++i) {
        matrices.push_back().setConcat(matrices[i], tinyCCW);
    }
    SkMatrix cw45;
    cw45.setRotate(45.f, kRadius, kRadius);
    for (int i = 0; i < baseMatrixCnt; ++i) {
        matrices.push_back().setConcat(matrices[i], cw45);
    }

    int x = 0;
    int y = 0;
    static constexpr SkScalar kPad = 2*kStrokeWidth;
    canvas->translate(kPad, kPad);
    auto bounds = SkRect::MakeWH(2*kRadius, 2*kRadius);
    for (auto cap : {SkPaint::kRound_Cap, SkPaint::kButt_Cap, SkPaint::kSquare_Cap}) {
        for (const auto& m : matrices) {
            SkPaint paint;
            paint.setStrokeCap(cap);
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(kStrokeWidth);
            canvas->save();
                canvas->translate(x * (2*kRadius + kPad), y * (2*kRadius + kPad));
                canvas->concat(m);
                paint.setColor(SK_ColorRED);
                paint.setAlpha(0x80);
                canvas->drawArc(bounds, kStart, kSweep, false, paint);
                paint.setColor(SK_ColorBLUE);
                paint.setAlpha(0x80);
                canvas->drawArc(bounds, kStart, kSweep - 360.f, false, paint);
            canvas->restore();
            ++x;
            if (x == baseMatrixCnt) {
                x = 0;
                ++y;
            }
        }
    }
}
