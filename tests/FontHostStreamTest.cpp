/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkFontDescriptor.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "Test.h"

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

DEF_TEST(FontHostStream, reporter) {
    {
        SkPaint paint;
        paint.setColor(SK_ColorGRAY);
        paint.setTextSize(SkIntToScalar(30));

        paint.setTypeface(SkTypeface::MakeFromName("Georgia", SkFontStyle()));

        SkIRect origRect = SkIRect::MakeWH(64, 64);
        SkBitmap origBitmap;
        create(&origBitmap, origRect);
        SkCanvas origCanvas(origBitmap);

        SkIRect streamRect = SkIRect::MakeWH(64, 64);
        SkBitmap streamBitmap;
        create(&streamBitmap, streamRect);
        SkCanvas streamCanvas(streamBitmap);

        SkPoint point = SkPoint::Make(24, 32);

        // Test: origTypeface and streamTypeface from orig data draw the same
        drawBG(&origCanvas);
        origCanvas.drawString("A", point.fX, point.fY, paint);

        sk_sp<SkTypeface> typeface(paint.getTypeface() ? paint.refTypeface()
                                                       : SkTypeface::MakeDefault());
        int ttcIndex;
        std::unique_ptr<SkStreamAsset> fontData(typeface->openStream(&ttcIndex));
        sk_sp<SkTypeface> streamTypeface(SkTypeface::MakeFromStream(fontData.release()));

        SkFontDescriptor desc;
        bool isLocalStream = false;
        streamTypeface->getFontDescriptor(&desc, &isLocalStream);
        REPORTER_ASSERT(reporter, isLocalStream);

        paint.setTypeface(streamTypeface);
        drawBG(&streamCanvas);
        streamCanvas.drawPosText("A", 1, &point, paint);

        REPORTER_ASSERT(reporter,
                        compare(origBitmap, origRect, streamBitmap, streamRect));
    }
    //Make sure the typeface is deleted and removed.
    SkGraphics::PurgeFontCache();
}
