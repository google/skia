/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkTypes.h"

static const SkColor bgColor = SK_ColorWHITE;

static void create(SkBitmap* bm, SkIRect bound, SkBitmap::Config config) {
    bm->setConfig(config, bound.width(), bound.height());
    bm->allocPixels();
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

    SkAutoLockPixels alpRef(ref);
    SkAutoLockPixels alpTest(test);

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
    create(&drawTextBitmap, drawTextRect, SkBitmap::kARGB_8888_Config);
    SkCanvas drawTextCanvas(drawTextBitmap);

    SkIRect drawPosTextRect = SkIRect::MakeWH(64, 64);
    SkBitmap drawPosTextBitmap;
    create(&drawPosTextBitmap, drawPosTextRect, SkBitmap::kARGB_8888_Config);
    SkCanvas drawPosTextCanvas(drawPosTextBitmap);

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
                    drawTextCanvas.drawText("A", 1, point.fX, point.fY, paint);

                    drawBG(&drawPosTextCanvas);
                    drawPosTextCanvas.drawPosText("A", 1, &point, paint);

                    REPORTER_ASSERT(reporter,
                        compare(drawTextBitmap, drawTextRect,
                                drawPosTextBitmap, drawPosTextRect));
                }
            }
        }
    }
}
