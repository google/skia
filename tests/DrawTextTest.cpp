/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "tests/Test.h"

#include <cmath>

static const SkColor bgColor = SK_ColorWHITE;

static void create(SkBitmap* bm, SkIRect bound) {
    bm->allocN32Pixels(bound.width(), bound.height());
}

/** Assumes that the ref draw was completely inside ref canvas --
    implies that everything outside is "bgColor".
    Checks that all overlap is the same and that all non-overlap on the
    ref is "bgColor".
 */
static bool compare(const SkBitmap& ref, const SkIRect& iref,
                    const SkBitmap& test, const SkIRect& itest)
{
    const int xOff = itest.fLeft - iref.fLeft;
    const int yOff = itest.fTop - iref.fTop;

    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            int refX = x + xOff;
            int refY = y + yOff;
            SkColor refColor;
            if (refX >= 0 && refX < ref.width() &&
                refY >= 0 && refY < ref.height())
            {
                refColor = ref.getColor(refX, refY);
            } else {
                refColor = bgColor;
            }
            if (refColor != testColor) {
                return false;
            }
        }
    }
    return true;
}

/** Test that drawing glyphs with empty paths is different from drawing glyphs without paths. */
DEF_TEST(DrawText_dashout, reporter) {
    SkIRect size = SkIRect::MakeWH(64, 64);

    SkBitmap drawTextBitmap;
    create(&drawTextBitmap, size);
    SkCanvas drawTextCanvas(drawTextBitmap);

    SkBitmap drawDashedTextBitmap;
    create(&drawDashedTextBitmap, size);
    SkCanvas drawDashedTextCanvas(drawDashedTextBitmap);

    SkBitmap emptyBitmap;
    create(&emptyBitmap, size);
    SkCanvas emptyCanvas(emptyBitmap);

    SkPoint point = SkPoint::Make(25.0f, 25.0f);
    SkFont font(nullptr, 20);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    font.setSubpixel(true);

    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setStyle(SkPaint::kStroke_Style);

    // Draw a stroked "A" without a dash which will draw something.
    drawTextCanvas.drawColor(SK_ColorWHITE);
    drawTextCanvas.drawString("A", point.fX, point.fY, font, paint);

    // Draw an "A" but with a dash which will never draw anything.
    paint.setStrokeWidth(2);
    constexpr SkScalar bigInterval = 10000;
    static constexpr SkScalar intervals[] = { 1, bigInterval };
    paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 2));

    drawDashedTextCanvas.drawColor(SK_ColorWHITE);
    drawDashedTextCanvas.drawString("A", point.fX, point.fY, font, paint);

    // Draw nothing.
    emptyCanvas.drawColor(SK_ColorWHITE);

    REPORTER_ASSERT(reporter, !compare(drawTextBitmap, size, emptyBitmap, size));
    REPORTER_ASSERT(reporter, compare(drawDashedTextBitmap, size, emptyBitmap, size));
}

// Test drawing text at some unusual coordinates.
// We measure success by not crashing or asserting.
DEF_TEST(DrawText_weirdCoordinates, r) {
    auto surface = SkSurface::MakeRasterN32Premul(10,10);
    auto canvas = surface->getCanvas();

    SkScalar oddballs[] = { 0.0f, (float)INFINITY, (float)NAN, 34359738368.0f };

    for (auto x : oddballs) {
        canvas->drawString("a", +x, 0.0f, SkFont(), SkPaint());
        canvas->drawString("a", -x, 0.0f, SkFont(), SkPaint());
    }
    for (auto y : oddballs) {
        canvas->drawString("a", 0.0f, +y, SkFont(), SkPaint());
        canvas->drawString("a", 0.0f, -y, SkFont(), SkPaint());
    }
}

// Test drawing text with some unusual matricies.
// We measure success by not crashing or asserting.
DEF_TEST(DrawText_weirdMatricies, r) {
    auto surface = SkSurface::MakeRasterN32Premul(100,100);
    auto canvas = surface->getCanvas();

    SkFont font;
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    struct {
        SkScalar textSize;
        SkScalar matrix[9];
    } testCases[] = {
        // 2x2 singular
        {10, { 0,  0,  0,  0,  0,  0,  0,  0,  1}},
        {10, { 0,  0,  0,  0,  1,  0,  0,  0,  1}},
        {10, { 0,  0,  0,  1,  0,  0,  0,  0,  1}},
        {10, { 0,  0,  0,  1,  1,  0,  0,  0,  1}},
        {10, { 0,  1,  0,  0,  1,  0,  0,  0,  1}},
        {10, { 1,  0,  0,  0,  0,  0,  0,  0,  1}},
        {10, { 1,  0,  0,  1,  0,  0,  0,  0,  1}},
        {10, { 1,  1,  0,  0,  0,  0,  0,  0,  1}},
        {10, { 1,  1,  0,  1,  1,  0,  0,  0,  1}},
        // See https://bugzilla.mozilla.org/show_bug.cgi?id=1305085 .
        { 1, {10, 20,  0, 20, 40,  0,  0,  0,  1}},
    };

    for (const auto& testCase : testCases) {
        font.setSize(testCase.textSize);
        const SkScalar(&m)[9] = testCase.matrix;
        SkMatrix mat;
        mat.setAll(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        canvas->setMatrix(mat);
        canvas->drawString("Hamburgefons", 10, 10, font, SkPaint());
    }
}
