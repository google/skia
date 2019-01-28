/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFont.h"
#include "SkPaint.h"
#include "SkTypeface.h"
#include "Test.h"

#ifdef SK_SUPPORT_LEGACY_PAINT_TEXTMEASURE
static void test_fontmetrics(skiatest::Reporter* reporter,
                             const SkPaint& paint, const SkFont& font) {
    SkFontMetrics fm0, fm1;
    SkScalar h0 = paint.getFontMetrics(&fm0);
    SkScalar h1 = font.getMetrics(&fm1);

    REPORTER_ASSERT(reporter, h0 == h1);
#define CMP(field) REPORTER_ASSERT(reporter, fm0.field == fm1.field)
    CMP(fFlags);
    CMP(fTop);
    CMP(fAscent);
    CMP(fDescent);
    CMP(fBottom);
    CMP(fLeading);
#undef CMP
}

DEF_TEST(FontObj_test_cachedfont, reporter) {
    SkPaint paint;
    char txt[] = "long .text .with .lots .of.dots.";
    unsigned mask = SkPaint::kAntiAlias_Flag            |
                    SkPaint::kFakeBoldText_Flag         |
                    SkPaint::kLinearText_Flag           |
                    SkPaint::kSubpixelText_Flag         |
                    SkPaint::kLCDRenderText_Flag        |
                    SkPaint::kEmbeddedBitmapText_Flag   |
                    SkPaint::kAutoHinting_Flag;

    paint.setStrokeWidth(2);
    {
        for (unsigned flags = 0; flags <= 0xFFF; ++flags) {
            if (flags & ~mask) {
                continue;
            }
            paint.setFlags(flags);
            for (int hint = 0; hint <= 3; ++hint) {
                paint.setHinting((SkFontHinting)hint);
                {
                    for (auto style : { SkPaint::kFill_Style, SkPaint::kStroke_Style}) {
                        paint.setStyle(style);

                        const SkFont font(SkFont::LEGACY_ExtractFromPaint(paint));
                        test_fontmetrics(reporter, paint, font);

                        SkRect pbounds, fbounds;

                        // Requesting the bounds forces a generateMetrics call.
                        SkScalar pwidth = paint.measureText(txt, strlen(txt), &pbounds);
                        SkScalar fwidth = font.measureText(txt, strlen(txt), kUTF8_SkTextEncoding,
                                                          &fbounds, &paint);
                        REPORTER_ASSERT(reporter, pwidth == fwidth);
                        REPORTER_ASSERT(reporter, pbounds == fbounds);
                    }
                }
            }
        }
    }
}
#endif  // SK_SUPPORT_LEGACY_PAINT_TEXTMEASURE

#ifdef SK_SUPPORT_LEGACY_PAINT_FONT_FIELDS
DEF_TEST(FontObj_test_aa_hinting, reporter) {
    SkPaint paint;

    for (bool aa : {false, true}) {
        paint.setAntiAlias(aa);
        for (int hint = 0; hint <= 3; ++hint) {
            paint.setHinting((SkFontHinting)hint);
            SkFont font = SkFont::LEGACY_ExtractFromPaint(paint);

            SkPaint p2;
            font.LEGACY_applyToPaint(&p2);
            REPORTER_ASSERT(reporter, paint.isAntiAlias() == p2.isAntiAlias());
            REPORTER_ASSERT(reporter, paint.getHinting() == p2.getHinting());
        }
    }
}
#endif

// need tests for SkStrSearch
