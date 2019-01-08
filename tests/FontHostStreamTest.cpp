/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkFont.h"
#include "SkFontDescriptor.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "Test.h"

/** Checks that all is the same.  */
static bool compare(const SkBitmap& ref, const SkBitmap& test) {
    if (ref.dimensions() != test.dimensions()) {
        return false;
    }
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            if (test.getColor(x, y) != ref.getColor(x, y)) {
                return false;
            }
        }
    }
    return true;
}

static void draw(SkBitmap* bitmap, const SkFont& font) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    SkASSERT(bitmap);
    bitmap->allocN32Pixels(64, 64);
    SkCanvas canvas(*bitmap);
    canvas.drawColor(SK_ColorWHITE);
    canvas.drawString("A", 24, 32, font, paint);
}

DEF_TEST(FontHostStream, reporter) {
    sk_sp<SkTypeface> typeface = SkTypeface::MakeFromName("Georgia", SkFontStyle());
    if (!typeface) {
        typeface = SkTypeface::MakeDefault();
    }
    SkFont font(typeface, 30);
    SkBitmap origBitmap, streamBitmap;

    // Test: origTypeface and streamTypeface from orig data draw the same
    draw(&origBitmap, font);

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

    SkFont streamFont(streamTypeface);
    draw(&streamBitmap, streamFont);

    REPORTER_ASSERT(reporter, compare(origBitmap, streamBitmap));

    //Make sure the typeface is deleted and removed.
    SkGraphics::PurgeFontCache();
}
