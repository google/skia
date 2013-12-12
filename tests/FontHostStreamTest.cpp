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
#include "SkFontHost.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkTypeface.h"
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

DEF_TEST(FontHostStream, reporter) {
    {
        SkPaint paint;
        paint.setColor(SK_ColorGRAY);
        paint.setTextSize(SkIntToScalar(30));

        SkTypeface* fTypeface = SkTypeface::CreateFromName("Georgia",
                                                           SkTypeface::kNormal);
        SkSafeUnref(paint.setTypeface(fTypeface));

        SkIRect origRect = SkIRect::MakeWH(64, 64);
        SkBitmap origBitmap;
        create(&origBitmap, origRect, SkBitmap::kARGB_8888_Config);
        SkCanvas origCanvas(origBitmap);

        SkIRect streamRect = SkIRect::MakeWH(64, 64);
        SkBitmap streamBitmap;
        create(&streamBitmap, streamRect, SkBitmap::kARGB_8888_Config);
        SkCanvas streamCanvas(streamBitmap);

        SkPoint point = SkPoint::Make(24, 32);

        // Test: origTypeface and streamTypeface from orig data draw the same
        drawBG(&origCanvas);
        origCanvas.drawText("A", 1, point.fX, point.fY, paint);

        SkTypeface* origTypeface = paint.getTypeface();
        SkAutoTUnref<SkTypeface> aur;
        if (NULL == origTypeface) {
            origTypeface = aur.reset(SkTypeface::RefDefault());
        }

        int ttcIndex;
        SkAutoTUnref<SkStream> fontData(origTypeface->openStream(&ttcIndex));
        SkTypeface* streamTypeface = SkTypeface::CreateFromStream(fontData);
        SkSafeUnref(paint.setTypeface(streamTypeface));
        drawBG(&streamCanvas);
        streamCanvas.drawPosText("A", 1, &point, paint);

        REPORTER_ASSERT(reporter,
                        compare(origBitmap, origRect, streamBitmap, streamRect));
    }
    //Make sure the typeface is deleted and removed.
    SkGraphics::PurgeFontCache();
}
