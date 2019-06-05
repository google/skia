/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "samplecode/Sample.h"

static SkPath lines(const SkPoint pts[], size_t n) {
    SkPath path;
    path.moveTo(pts[0]);
    for (size_t i = 1; i < n; ++i) {
        path.lineTo(pts[i]);
    }
    return path;
}

static void draw_path(SkCanvas* canvas, SkPoint translate, const SkPath& path) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    canvas->translate(translate.x(), translate.y());
    canvas->drawPath(path, paint);
}

static void draw_concave_paths(SkCanvas* canvas, SkSize) {
    // Concave test
    static constexpr SkPoint pts1[4] = {{20, 20}, {80, 20}, {30, 30}, {20, 80}};
    draw_path(canvas, {0, 0}, lines(pts1, SK_ARRAY_COUNT(pts1)));

    // Reverse concave test
    static constexpr SkPoint pts2[4] = {{20, 20}, {20, 80}, {30, 30}, {80, 20}};
    draw_path(canvas, {100, 0}, lines(pts2, SK_ARRAY_COUNT(pts2)));

    // Bowtie (intersection)
    static constexpr SkPoint pts3[4] = {{20, 20}, {80, 80}, {80, 20}, {20, 80}};
    draw_path(canvas, {200, 0}, lines(pts3, SK_ARRAY_COUNT(pts3)));

    // "fake" bowtie (concave, but no intersection)
    static constexpr SkPoint pts4[6] = {{20, 20}, {50, 40}, {80, 20}, {80, 80}, {50, 60}, {20, 80}};
    draw_path(canvas, {300, 0}, lines(pts4, SK_ARRAY_COUNT(pts4)));

    // Fish test (intersection/concave)
    static constexpr SkPoint pts5[6] = {{20, 20}, {80, 80}, {70, 50}, {80, 20}, {20, 80}, {0, 50}};
    draw_path(canvas, {0, 100}, lines(pts5, SK_ARRAY_COUNT(pts5)));

    // Collinear test
    static constexpr SkPoint pts6[4] = {{20, 20}, {50, 20}, {80, 20}, {50, 80}};
    draw_path(canvas, {100, 100}, lines(pts6, SK_ARRAY_COUNT(pts6)));

    // Hole test
    static constexpr SkPoint pts7[4] = {{20, 20}, {80, 20}, {80, 80}, {20, 80}};
    SkPath path = lines(pts7, SK_ARRAY_COUNT(pts7));
    static constexpr SkPoint pts8[4] = {{30, 30}, {30, 70}, {70, 70}, {70, 30}};
    path.addPath(lines(pts8, SK_ARRAY_COUNT(pts8)));
    draw_path(canvas, {200, 100}, path);
}
DEF_SIMPLE_SAMPLE("ConcavePaths", draw_concave_paths);
