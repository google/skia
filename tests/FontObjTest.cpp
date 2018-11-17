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
        nullptr,
        "Arial", "Times", "Times New Roman", "Helvetica", "Courier",
        "Courier New", "Verdana", "monospace",
    };

    static const struct {
        SkScalar    fScaleX;
        SkScalar    fSkewX;
    } gScaleRec[] = {
        { SK_Scalar1, 0 },
        { SK_Scalar1/2, -SK_Scalar1/4 },
    };

    SkPaint paint;
    char txt[] = "long .text .with .lots .of.dots.";

    unsigned mask = SkPaint::kAntiAlias_Flag            |
                    SkPaint::kFakeBoldText_Flag         |
                    SkPaint::kLinearText_Flag           |
                    SkPaint::kSubpixelText_Flag         |
                    SkPaint::kLCDRenderText_Flag        |
                    SkPaint::kEmbeddedBitmapText_Flag   |
                    SkPaint::kAutoHinting_Flag;

    for (size_t i = 0; i < SK_ARRAY_COUNT(faces); i++) {
        paint.setTypeface(SkTypeface::MakeFromName(faces[i], SkFontStyle()));
        for (unsigned flags = 0; flags <= 0xFFF; ++flags) {
            if (flags & ~mask) {
                continue;
            }
            paint.setFlags(flags);
            for (int hint = 0; hint <= 3; ++hint) {
                paint.setHinting((SkFontHinting)hint);
                for (size_t k = 0; k < SK_ARRAY_COUNT(gScaleRec); ++k) {
                    paint.setTextScaleX(gScaleRec[k].fScaleX);
                    paint.setTextSkewX(gScaleRec[k].fSkewX);

                    const SkFont font(SkFont::LEGACY_ExtractFromPaint(paint));

                    test_cachedfont(reporter, paint, font);
                    test_fontmetrics(reporter, paint, font);

                    SkRect pbounds, fbounds;

                    // Requesting the bounds forces a generateMetrics call.
                    SkScalar pwidth = paint.measureText(txt, strlen(txt), &pbounds);
                    SkScalar fwidth = font.measureText(txt, strlen(txt), kUTF8_SkTextEncoding,
                                                      &fbounds);
                    REPORTER_ASSERT(reporter, pwidth == fwidth);
                    REPORTER_ASSERT(reporter, pbounds == fbounds);
                }
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
