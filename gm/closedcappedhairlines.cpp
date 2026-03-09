/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"


static constexpr size_t kScale = 4;
static constexpr int kGridWH = 70;

static void draw_grid(SkCanvas* canvas) {
    SkPaint gridPaint;
    gridPaint.setColor(SK_ColorDKGRAY);
    gridPaint.setStyle(SkPaint::Style::kStroke_Style);
    gridPaint.setStrokeWidth(0);

    for (int y = 0; y <= (int)kGridWH; ++y) {
        canvas->drawLine(0, y, kGridWH, y, gridPaint);
    }
    for (int x = 0; x <= (int)kGridWH; ++x) {
        canvas->drawLine(x, 0, x, kGridWH, gridPaint);
    }
}

static SkRect highlight_box(SkPoint p) {
    constexpr float kOffset = 2;
    return SkRect::MakeXYWH(p.x() - kOffset, p.y() - kOffset, kOffset * 2, kOffset * 2);
}

// This function is for drawing highlight boxes around the parts of the contour we expect
// there to be a capped edge (i.e. the beginning and end of an open contour)
static void draw_highlights(SkCanvas* canvas, std::initializer_list<SkPath> paths) {
    SkPaint highlightPaint;
    highlightPaint.setStyle(SkPaint::kStroke_Style);
    highlightPaint.setStrokeWidth(0);
    highlightPaint.setColor(SK_ColorRED);
    highlightPaint.setAntiAlias(true);

    for (auto const& path : paths) {
        canvas->drawRect(highlight_box(path.points().front()), highlightPaint);
        canvas->drawRect(highlight_box(path.points().back()), highlightPaint);
    }
}

// We would like to test the ability to draw closed and open contours with
// capped hairlines. Capped hairlines have interesting behavior because they extend out
// by up to half a pixel, which has led to unintended effects when closing
// contours with antialiasing on.
// The contours here represent all simple path types,
// We include contours on pixel lines and offset by 0.5 pixels, as these differ in behavior.
// * First row: on pixel line, open contours (which should not show caps)
// * Second row: off pixel line, open contours
// * Third row: on pixel line, closed contours
// * Fourth row: off pixel line, closed contours
static void draw_hairline_contours_with_caps(SkCanvas* canvas, SkPaint::Cap cap) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    paint.setStrokeCap(cap);

    auto pathSurface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kGridWH, kGridWH));
    SkASSERT_RELEASE(pathSurface);

    auto pathCanvas = pathSurface->getCanvas();
    SkASSERT_RELEASE(pathCanvas);

    // first row
    auto lineOnOpen = SkPathBuilder().lineTo(0, 5).lineTo(5, 5).lineTo(5, 0).detach()
                        .makeOffset(5, 5);
    pathCanvas->drawPath(lineOnOpen, paint);
    auto quadOnOpen = SkPathBuilder().quadTo(15, 5, 0, 10).detach()
                        .makeOffset(20, 5);
    pathCanvas->drawPath(quadOnOpen, paint);
    auto cubicOnOpen = SkPathBuilder().cubicTo(-5, 0, -5, 5, 0, 10).detach()
                        .makeOffset(40, 5);
    pathCanvas->drawPath(cubicOnOpen, paint);

    // second row
    auto lineOffOpen = lineOnOpen
                        .makeOffset(0.5, 15);
    pathCanvas->drawPath(lineOffOpen, paint);
    auto quadOffOpen = quadOnOpen
                        .makeOffset(0.5, 15);
    pathCanvas->drawPath(quadOffOpen, paint);
    auto cubicOffOpen = cubicOnOpen
                        .makeOffset(0.5, 15);
    pathCanvas->drawPath(cubicOffOpen, paint);

    // third row
    auto lineOnClosed = SkPathBuilder(lineOnOpen).close().detach()
                            .makeOffset(0, 30);
    pathCanvas->drawPath(lineOnClosed, paint);
    auto quadOnClosed = SkPathBuilder(quadOnOpen).close().detach()
                            .makeOffset(0, 30);
    pathCanvas->drawPath(quadOnClosed, paint);
    auto cubicOnClosed = SkPathBuilder(cubicOnOpen).close().detach()
                            .makeOffset(0, 30);
    pathCanvas->drawPath(cubicOnClosed, paint);

    // forth row
    auto lineOffClosed = SkPathBuilder(lineOnOpen).close().detach()
                            .makeOffset(0.5, 45);
    pathCanvas->drawPath(lineOffClosed, paint);
    auto quadOffClosed = SkPathBuilder(quadOnOpen).close().detach()
                            .makeOffset(0.5, 45);
    pathCanvas->drawPath(quadOffClosed, paint);
    auto cubicOffClosed = SkPathBuilder(cubicOnOpen).close().detach()
                            .makeOffset(0.5, 45);
    pathCanvas->drawPath(cubicOffClosed, paint);

    auto pathImg = pathSurface->makeImageSnapshot();
    canvas->drawImage(pathImg, 0, 0);

    canvas->scale(kScale, kScale);
    canvas->drawImage(pathImg, 15, 0);
    canvas->translate(15, 0);

    draw_grid(canvas);

    draw_highlights(canvas, {lineOnOpen, quadOnOpen, cubicOnOpen, lineOffOpen, quadOffOpen, cubicOffOpen});
}

#define WIDTH 250
#define HEIGHT 250

DEF_SIMPLE_GM(hairlines_buttcap, canvas, WIDTH, HEIGHT) {
    draw_hairline_contours_with_caps(canvas, SkPaint::kButt_Cap);
}

DEF_SIMPLE_GM(hairlines_roundcap, canvas, WIDTH, HEIGHT) {
    draw_hairline_contours_with_caps(canvas, SkPaint::kRound_Cap);
}

DEF_SIMPLE_GM(hairlines_squarecap, canvas, WIDTH, HEIGHT) {
    draw_hairline_contours_with_caps(canvas, SkPaint::kSquare_Cap);
}
