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
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>
#include <utility>

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

        SkFont font(ToolUtils::CreateTestTypeface("Georgia", SkFontStyle()), 30);
        font.setEdging(SkFont::Edging::kAlias);

        const SkIRect origRect = SkIRect::MakeWH(64, 64);
        SkBitmap origBitmap;
        create(&origBitmap, origRect);
        SkCanvas origCanvas(origBitmap);

        SkPoint point = SkPoint::Make(24, 32);

        // Test: origTypeface and streamTypeface from orig data draw the same
        drawBG(&origCanvas);
        origCanvas.drawString("A", point.fX, point.fY, font, paint);

        sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
        sk_sp<SkTypeface> typeface = font.refTypeface();
        SkASSERT_RELEASE(typeface);

        {
            SkDynamicMemoryWStream wstream;
            typeface->serialize(&wstream, SkTypeface::SerializeBehavior::kDoIncludeData);
            std::unique_ptr<SkStreamAsset> stream = wstream.detachAsStream();
            sk_sp<SkTypeface> deserializedTypeface = SkTypeface::MakeDeserialize(&*stream, mgr);
            if (!deserializedTypeface) {
                REPORTER_ASSERT(reporter, deserializedTypeface);
                return;
            }

            SkFontDescriptor desc;
            bool mustSerializeData = false;
            deserializedTypeface->getFontDescriptor(&desc, &mustSerializeData);
            REPORTER_ASSERT(reporter, mustSerializeData);

            SkBitmap deserializedBitmap;
            create(&deserializedBitmap, origRect);
            SkCanvas deserializedCanvas(deserializedBitmap);

            font.setTypeface(deserializedTypeface);
            drawBG(&deserializedCanvas);
            deserializedCanvas.drawString("A", point.fX, point.fY, font, paint);

            REPORTER_ASSERT(reporter, compare(origBitmap, origRect, deserializedBitmap, origRect));
        }

        {
            int ttcIndex;
            std::unique_ptr<SkStreamAsset> fontData = typeface->openStream(&ttcIndex);
            if (!fontData) {
                REPORTER_ASSERT(reporter, fontData);
                return;
            }

            sk_sp<SkTypeface> streamTypeface(mgr->makeFromStream(std::move(fontData), 0));
            if (!streamTypeface) {
                // TODO: enable assert after SkTypeface::MakeFromStream uses factories
                //REPORTER_ASSERT(reporter, streamTypeface);
                return;
            }

            SkBitmap streamBitmap;
            create(&streamBitmap, origRect);
            SkCanvas streamCanvas(streamBitmap);

            SkFontDescriptor desc;
            bool mustSerializeData = false;
            streamTypeface->getFontDescriptor(&desc, &mustSerializeData);
            REPORTER_ASSERT(reporter, mustSerializeData);

            font.setTypeface(streamTypeface);
            drawBG(&streamCanvas);
            streamCanvas.drawString("A", point.fX, point.fY, font, paint);

            REPORTER_ASSERT(reporter, compare(origBitmap, origRect, streamBitmap, origRect));
        }
    }
    //Make sure the typeface is deleted and removed.
    SkGraphics::PurgeFontCache();
}
