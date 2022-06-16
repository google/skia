// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SmoothBezierSplineInterpolation, 1024, 1024, false, 0) {
// Smooth Bézier Spline Interpolation

SkPath MakeCubicSplineInterpolation(const SkPoint* pts, size_t N) {
    // Code borrowed from https://www.particleincell.com/2012/bezier-splines/

    SkPath path;
    if (N < 2) {
        return path;
    }
    if (N == 2) {
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        return path;
    }
    size_t n = N - 1;  // number of segments
    struct Scratch {
        SkPoint a, b, c, r, p;
    };
    // Can I do this will less allocation?
    std::unique_ptr<Scratch[]> s(new Scratch[n]);
    s[0].a = {0, 0};
    s[0].b = {2, 2};
    s[0].c = {1, 1};
    s[0].r = {pts[0].x() + 2 * pts[1].x(), pts[0].y() + 2 * pts[1].y()};
    for (size_t i = 1; i < n - 1; ++i) {
        s[i].a = {1, 1};
        s[i].b = {4, 4};
        s[i].c = {1, 1};
        s[i].r = {4 * pts[i].x() + 2 * pts[i + 1].x(), 4 * pts[i].y() + 2 * pts[i + 1].y()};
    }
    s[n - 1].a = {2, 2};
    s[n - 1].b = {7, 7};
    s[n - 1].c = {0, 0};
    s[n - 1].r = {8 * pts[n - 1].x() + pts[N - 1].x(), 8 * pts[n - 1].y() + pts[N - 1].y()};
    for (size_t i = 1; i < n; i++) {
        float mx = s[i].a.x() / s[i - 1].b.x();
        float my = s[i].a.y() / s[i - 1].b.y();
        s[i].b -= {mx * s[i - 1].c.x(), my * s[i - 1].c.y()};
        s[i].r -= {mx * s[i - 1].r.x(), my * s[i - 1].r.y()};
    }
    s[n - 1].p = {s[n - 1].r.x() / s[n - 1].b.x(), s[n - 1].r.y() / s[n - 1].b.y()};
    for (int i = (int)N - 3; i >= 0; --i) {
        s[i].p = {(s[i].r.x() - s[i].c.x() * s[i + 1].p.fX) / s[i].b.x(),
                  (s[i].r.y() - s[i].c.y() * s[i + 1].p.fY) / s[i].b.y()};
    }

    path.moveTo(pts[0]);
    for (size_t i = 0; i < n - 1; i++) {
        SkPoint q = {2 * pts[i + 1].x() - s[i + 1].p.fX, 2 * pts[i + 1].y() - s[i + 1].p.fY};
        path.cubicTo(s[i].p, q, pts[i + 1]);
    }
    SkPoint q = {0.5f * (pts[N - 1].x() + s[n - 1].p.x()),
                 0.5f * (pts[N - 1].y() + s[n - 1].p.y())};
    path.cubicTo(s[n - 1].p, q, pts[n]);
    return path;
}

void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(3);
    p.setStrokeCap(SkPaint::kRound_Cap);

    // randomly generated y values in range [12,1024].
    SkPoint pts[] = {
            {62, 511}, {162, 605}, {262, 610}, {362, 402}, {462, 959},
            {562, 58}, {662, 272}, {762, 99},  {862, 759}, {962, 945},
    };

    canvas->drawPath(MakeCubicSplineInterpolation(pts, std::size(pts)), p);

    p.setStrokeWidth(10);
    p.setColor(SK_ColorBLACK);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, std::size(pts), pts, p);
}
}  // END FIDDLE
