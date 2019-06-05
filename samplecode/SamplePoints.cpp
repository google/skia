/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"

static void draw_points(SkCanvas* canvas, SkSize) {
    canvas->translate(1, 1);

    SkRandom rand;
    SkPaint p0(SkColors::kRed),
            p1(SkColors::kGreen),
            p2(SkColors::kBlue),
            p3(SkColors::kWhite);
    constexpr size_t n = 99;

    p0.setStrokeWidth(SkIntToScalar(4));
    p2.setStrokeCap(SkPaint::kRound_Cap);
    p2.setStrokeWidth(SkIntToScalar(6));

    std::unique_ptr<SkPoint[]> pts(new SkPoint[n]);
    for (size_t i = 0; i < n; i++) {
        pts[i].fX = rand.nextUScalar1() * 640;
        pts[i].fY = rand.nextUScalar1() * 480;
    }

    canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts.get(), p0);
    canvas->drawPoints(SkCanvas::kLines_PointMode,   n, pts.get(), p1);
    canvas->drawPoints(SkCanvas::kPoints_PointMode,  n, pts.get(), p2);
    canvas->drawPoints(SkCanvas::kPoints_PointMode,  n, pts.get(), p3);
}
DEF_SIMPLE_SAMPLE("Points", draw_points);
