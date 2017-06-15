/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkSurface.h"
#include "SkTypes.h"
#include "Test.h"
#include <math.h>

static const SkColor bgColor = SK_ColorWHITE;

static void create(SkBitmap* bm, SkIRect bound) {
    bm->allocN32Pixels(bound.width(), bound.height());
}

static void drawBG(SkCanvas* canvas) {
    canvas->drawColor(bgColor);
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

DEF_TEST(DrawText, reporter) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setTextSize(SkIntToScalar(20));

    SkIRect drawTextRect = SkIRect::MakeWH(64, 64);
    SkBitmap drawTextBitmap;
    create(&drawTextBitmap, drawTextRect);
    SkCanvas drawTextCanvas(drawTextBitmap);

    SkIRect drawPosTextRect = SkIRect::MakeWH(64, 64);
    SkBitmap drawPosTextBitmap;
    create(&drawPosTextBitmap, drawPosTextRect);
    SkCanvas drawPosTextCanvas(drawPosTextBitmap);

    // Two test cases "A" for the normal path through the code, and " " to check the
    // early return path.
    const char* cases[] = {"A", " "};
    for (auto c : cases) {
        for (float offsetY = 0.0f; offsetY < 1.0f; offsetY += (1.0f / 16.0f)) {
            for (float offsetX = 0.0f; offsetX < 1.0f; offsetX += (1.0f / 16.0f)) {
                SkPoint point = SkPoint::Make(25.0f + offsetX,
                                              25.0f + offsetY);

                for (int align = 0; align < SkPaint::kAlignCount; ++align) {
                    paint.setTextAlign(static_cast<SkPaint::Align>(align));

                    for (unsigned int flags = 0; flags < (1 << 3); ++flags) {
                        static const unsigned int antiAliasFlag = 1;
                        static const unsigned int subpixelFlag = 1 << 1;
                        static const unsigned int lcdFlag = 1 << 2;

                        paint.setAntiAlias(SkToBool(flags & antiAliasFlag));
                        paint.setSubpixelText(SkToBool(flags & subpixelFlag));
                        paint.setLCDRenderText(SkToBool(flags & lcdFlag));

                        // Test: drawText and drawPosText draw the same.
                        drawBG(&drawTextCanvas);
                        drawTextCanvas.drawText(c, 1, point.fX, point.fY, paint);

                        drawBG(&drawPosTextCanvas);
                        drawPosTextCanvas.drawPosText(c, 1, &point, paint);

                        REPORTER_ASSERT(reporter,
                                        compare(drawTextBitmap, drawTextRect,
                                                drawPosTextBitmap, drawPosTextRect));
                    }
                }
            }
        }
    }
}

// Test drawing text at some unusual coordinates.
// We measure success by not crashing or asserting.
DEF_TEST(DrawText_weirdCoordinates, r) {
    auto surface = SkSurface::MakeRasterN32Premul(10,10);
    auto canvas = surface->getCanvas();

    SkScalar oddballs[] = { 0.0f, (float)INFINITY, (float)NAN, 34359738368.0f };

    for (auto x : oddballs) {
        canvas->drawString("a", +x, 0.0f, SkPaint());
        canvas->drawString("a", -x, 0.0f, SkPaint());
    }
    for (auto y : oddballs) {
        canvas->drawString("a", 0.0f, +y, SkPaint());
        canvas->drawString("a", 0.0f, -y, SkPaint());
    }
}

// Test drawing text with some unusual matricies.
// We measure success by not crashing or asserting.
DEF_TEST(DrawText_weirdMatricies, r) {
    auto surface = SkSurface::MakeRasterN32Premul(100,100);
    auto canvas = surface->getCanvas();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setLCDRenderText(true);

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
        paint.setTextSize(testCase.textSize);
        const SkScalar(&m)[9] = testCase.matrix;
        SkMatrix mat;
        mat.setAll(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        canvas->setMatrix(mat);
        canvas->drawString("Hamburgefons", 10, 10, paint);
    }
}
