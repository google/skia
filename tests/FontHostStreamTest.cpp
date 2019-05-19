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
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "tests/Test.h"

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

        SkFont font(SkTypeface::MakeFromName("Georgia", SkFontStyle()), 30);
        font.setEdging(SkFont::Edging::kAlias);

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
        origCanvas.drawString("A", point.fX, point.fY, font, paint);

        sk_sp<SkTypeface> typeface = font.refTypefaceOrDefault();
        int ttcIndex;
        std::unique_ptr<SkStreamAsset> fontData = typeface->openStream(&ttcIndex);
        if (!fontData) {
            // We're using a SkTypeface that can't give us a stream.
            // This happens with portable or system fonts.  End the test now.
            return;
        }

        sk_sp<SkTypeface> streamTypeface(SkTypeface::MakeFromStream(std::move(fontData)));

        SkFontDescriptor desc;
        bool isLocalStream = false;
        streamTypeface->getFontDescriptor(&desc, &isLocalStream);
        REPORTER_ASSERT(reporter, isLocalStream);

        font.setTypeface(streamTypeface);
        drawBG(&streamCanvas);
        streamCanvas.drawString("A", point.fX, point.fY, font, paint);

        REPORTER_ASSERT(reporter,
                        compare(origBitmap, origRect, streamBitmap, streamRect));
    }
    //Make sure the typeface is deleted and removed.
    SkGraphics::PurgeFontCache();
}
