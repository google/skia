/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkFont.h"
#include "SkFontDescriptor.h"
#include "SkFontPriv.h"
#include "SkGraphics.h"
#include "Test.h"
#include "sk_tool_utils.h"

static void draw_a(SkBitmap* bm, const SkFont& font) {
    bm->allocN32Pixels(64, 64);
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    SkCanvas canvas(*bm);
    canvas.drawColor(SK_ColorWHITE);
    canvas.drawString("A", 24, 32, font, paint);
}

DEF_TEST(FontHostStream, reporter) {
    {
        SkFont font(SkTypeface::MakeFromName("Georgia", SkFontStyle()), 30);
        SkBitmap origBitmap, streamBitmap;

        // Test: origTypeface and streamTypeface from orig data draw the same
        draw_a(&origBitmap, font);

        SkTypeface* typeface = SkFontPriv::GetTypefaceOrDefault(font);
        if (!typeface) {
            ERRORF(reporter, "SkFontPriv::GetTypefaceOrDefault returned nullptr.");
            return;
        }
        int ttcIndex;
        std::unique_ptr<SkStreamAsset> fontData(typeface->openStream(&ttcIndex));
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
        draw_a(&streamBitmap, font);

        REPORTER_ASSERT(reporter, sk_tool_utils::equal_pixels(origBitmap, streamBitmap));
    }
    //Make sure the typeface is deleted and removed.
    SkGraphics::PurgeFontCache();
}
