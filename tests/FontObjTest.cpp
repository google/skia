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

static bool is_use_nonlinear_metrics(const SkPaint& paint) {
    return !paint.isSubpixelText() && !paint.isLinearText();
}

static bool is_enable_auto_hints(const SkPaint& paint) {
    return paint.isAutohinted();
}

static bool is_enable_bytecode_hints(const SkPaint& paint) {
    return paint.getHinting() >= SkPaint::kFull_Hinting;
}

static void test_cachedfont(skiatest::Reporter* reporter, const SkPaint& paint) {
    sk_sp<SkFont> font(SkFont::Testing_CreateFromPaint(paint));

    // Currently SkFont resolves null into the default, so only test if paint's is not null
    if (paint.getTypeface()) {
        REPORTER_ASSERT(reporter, font->getTypeface() == paint.getTypeface());
    }
    REPORTER_ASSERT(reporter, font->getSize() == paint.getTextSize());
    REPORTER_ASSERT(reporter, font->getScaleX() == paint.getTextScaleX());
    REPORTER_ASSERT(reporter, font->getSkewX() == paint.getTextSkewX());

    REPORTER_ASSERT(reporter, font->isVertical() == paint.isVerticalText());
    REPORTER_ASSERT(reporter, font->isEmbolden() == paint.isFakeBoldText());

    REPORTER_ASSERT(reporter, font->isUseNonLinearMetrics() == is_use_nonlinear_metrics(paint));
    REPORTER_ASSERT(reporter, font->isEnableAutoHints() == is_enable_auto_hints(paint));
    REPORTER_ASSERT(reporter, font->isEnableByteCodeHints() == is_enable_bytecode_hints(paint));
}

static void test_cachedfont(skiatest::Reporter* reporter) {
    static const char* const faces[] = {
        nullptr,   // default font
        "Arial", "Times", "Times New Roman", "Helvetica", "Courier",
        "Courier New", "Verdana", "monospace",
    };

    static const struct {
        SkPaint::Hinting    hinting;
        unsigned            flags;
    } settings[] = {
        { SkPaint::kNo_Hinting,     0                               },
        { SkPaint::kNo_Hinting,     SkPaint::kLinearText_Flag       },
        { SkPaint::kNo_Hinting,     SkPaint::kSubpixelText_Flag     },
        { SkPaint::kSlight_Hinting, 0                               },
        { SkPaint::kSlight_Hinting, SkPaint::kLinearText_Flag       },
        { SkPaint::kSlight_Hinting, SkPaint::kSubpixelText_Flag     },
        { SkPaint::kNormal_Hinting, 0                               },
        { SkPaint::kNormal_Hinting, SkPaint::kLinearText_Flag       },
        { SkPaint::kNormal_Hinting, SkPaint::kSubpixelText_Flag     },
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

                test_cachedfont(reporter, paint);

                SkRect bounds;

                // For no hinting and light hinting this should take the
                // optimized generateAdvance path.
                SkScalar width1 = paint.measureText(txt, strlen(txt));

                // Requesting the bounds forces a generateMetrics call.
                SkScalar width2 = paint.measureText(txt, strlen(txt), &bounds);

                REPORTER_ASSERT(reporter, width1 == width2);

                sk_sp<SkFont> font(SkFont::Testing_CreateFromPaint(paint));
                SkScalar font_width1 = font->measureText(txt, strlen(txt), kUTF8_SkTextEncoding);
                // measureText not yet implemented...
                REPORTER_ASSERT(reporter, font_width1 == -1);
//                REPORTER_ASSERT(reporter, width1 == font_width1);
            }
        }
    }
}

DEF_TEST(FontObj, reporter) {
    test_cachedfont(reporter);
}

// need tests for SkStrSearch
