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

static void test_cachedfont(skiatest::Reporter* reporter,
                            const SkPaint& paint, const SkFont& font) {
    // Currently SkFont resolves null into the default, so only test if paint's is not null
    if (paint.getTypeface()) {
        REPORTER_ASSERT(reporter, font.getTypeface() == paint.getTypeface());
    }
    REPORTER_ASSERT(reporter, font.getSize() == paint.getTextSize());
    REPORTER_ASSERT(reporter, font.getScaleX() == paint.getTextScaleX());
    REPORTER_ASSERT(reporter, font.getSkewX() == paint.getTextSkewX());

    uint32_t mask = SkPaint::kLinearText_Flag |
                    SkPaint::kSubpixelText_Flag |
                    SkPaint::kFakeBoldText_Flag |
                    SkPaint::kEmbeddedBitmapText_Flag |
                    SkPaint::kAutoHinting_Flag;

    SkPaint p;
    font.LEGACY_applyToPaint(&p);
    uint32_t oldFlags = paint.getFlags() & mask;
    uint32_t newFlags = p.getFlags() & mask;
    REPORTER_ASSERT(reporter, oldFlags == newFlags);
    REPORTER_ASSERT(reporter, paint.getHinting() == p.getHinting());
}

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

static void test_cachedfont(skiatest::Reporter* reporter) {
    static const char* const faces[] = {
        nullptr,   // default font
        "Arial", "Times", "Times New Roman", "Helvetica", "Courier",
        "Courier New", "Verdana", "monospace",
    };

    static const struct {
        SkFontHinting   hinting;
        unsigned        flags;
    } settings[] = {
        { kNo_SkFontHinting,     0                               },
        { kNo_SkFontHinting,     SkPaint::kLinearText_Flag       },
        { kNo_SkFontHinting,     SkPaint::kSubpixelText_Flag     },
        { kSlight_SkFontHinting, 0                               },
        { kSlight_SkFontHinting, SkPaint::kLinearText_Flag       },
        { kSlight_SkFontHinting, SkPaint::kSubpixelText_Flag     },
        { kNormal_SkFontHinting, 0                               },
        { kNormal_SkFontHinting, SkPaint::kLinearText_Flag       },
        { kNormal_SkFontHinting, SkPaint::kSubpixelText_Flag     },
    };

    static const struct {
        SkScalar    fScaleX;
        SkScalar    fSkewX;
    } gScaleRec[] = {
        { SK_Scalar1, 0 },
        { SK_Scalar1/2, 0 },
        // these two exercise obliquing (skew)
        { SK_Scalar1, -SK_Scalar1/4 },
        { SK_Scalar1/2, -SK_Scalar1/4 },
    };

    SkPaint paint;
    char txt[] = "long.text.with.lots.of.dots.";

    for (size_t i = 0; i < SK_ARRAY_COUNT(faces); i++) {
        paint.setTypeface(SkTypeface::MakeFromName(faces[i], SkFontStyle()));

        for (size_t j = 0; j  < SK_ARRAY_COUNT(settings); j++) {
            paint.setHinting(settings[j].hinting);
            paint.setLinearText((settings[j].flags & SkPaint::kLinearText_Flag) != 0);
            paint.setSubpixelText((settings[j].flags & SkPaint::kSubpixelText_Flag) != 0);

            for (size_t k = 0; k < SK_ARRAY_COUNT(gScaleRec); ++k) {
                paint.setTextScaleX(gScaleRec[k].fScaleX);
                paint.setTextSkewX(gScaleRec[k].fSkewX);

                const SkFont font(SkFont::LEGACY_ExtractFromPaint(paint));

                test_cachedfont(reporter, paint, font);
                test_fontmetrics(reporter, paint, font);

                SkRect bounds;

                // For no hinting and light hinting this should take the
                // optimized generateAdvance path.
                SkScalar width1 = paint.measureText(txt, strlen(txt));

                // Requesting the bounds forces a generateMetrics call.
                SkScalar width2 = paint.measureText(txt, strlen(txt), &bounds);

                REPORTER_ASSERT(reporter, width1 == width2);

                SkScalar font_width1 = font.measureText(txt, strlen(txt), kUTF8_SkTextEncoding);
                // measureText not yet implemented...
                REPORTER_ASSERT(reporter, font_width1 == -1);
//                REPORTER_ASSERT(reporter, width1 == font_width1);
            }
        }
    }
}

static void test_aa_hinting(skiatest::Reporter* reporter) {
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

DEF_TEST(FontObj, reporter) {
    test_cachedfont(reporter);
    test_aa_hinting(reporter);
}

// need tests for SkStrSearch
