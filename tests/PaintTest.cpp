/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkLayerDrawLooper.h"
#include "SkPath.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "SkUtils.h"

static size_t uni_to_utf8(const SkUnichar src[], void* dst, int count) {
    char* u8 = (char*)dst;
    for (int i = 0; i < count; ++i) {
        int n = SkUTF8_FromUnichar(src[i], u8);
        u8 += n;
    }
    return u8 - (char*)dst;
}

static size_t uni_to_utf16(const SkUnichar src[], void* dst, int count) {
    uint16_t* u16 = (uint16_t*)dst;
    for (int i = 0; i < count; ++i) {
        int n = SkUTF16_FromUnichar(src[i], u16);
        u16 += n;
    }
    return (char*)u16 - (char*)dst;
}

static size_t uni_to_utf32(const SkUnichar src[], void* dst, int count) {
    SkUnichar* u32 = (SkUnichar*)dst;
    if (src != u32) {
        memcpy(u32, src, count * sizeof(SkUnichar));
    }
    return count * sizeof(SkUnichar);
}

static SkTypeface::Encoding paint2encoding(const SkPaint& paint) {
    SkPaint::TextEncoding enc = paint.getTextEncoding();
    SkASSERT(SkPaint::kGlyphID_TextEncoding != enc);
    return (SkTypeface::Encoding)enc;
}

static int find_first_zero(const uint16_t glyphs[], int count) {
    for (int i = 0; i < count; ++i) {
        if (0 == glyphs[i]) {
            return i;
        }
    }
    return count;
}

static void test_cmap(skiatest::Reporter* reporter) {
    static const int NGLYPHS = 64;

    SkUnichar src[NGLYPHS];
    SkUnichar dst[NGLYPHS]; // used for utf8, utf16, utf32 storage

    static const struct {
        size_t (*fSeedTextProc)(const SkUnichar[], void* dst, int count);
        SkPaint::TextEncoding   fEncoding;
    } gRec[] = {
        { uni_to_utf8,  SkPaint::kUTF8_TextEncoding },
        { uni_to_utf16, SkPaint::kUTF16_TextEncoding },
        { uni_to_utf32, SkPaint::kUTF32_TextEncoding },
    };

    SkRandom rand;
    SkPaint paint;
    paint.setTypeface(SkTypeface::RefDefault())->unref();
    SkTypeface* face = paint.getTypeface();

    for (int i = 0; i < 1000; ++i) {
        // generate some random text
        for (int j = 0; j < NGLYPHS; ++j) {
            src[j] = ' ' + j;
        }
        // inject some random chars, to sometimes abort early
        src[rand.nextU() & 63] = rand.nextU() & 0xFFF;

        for (size_t k = 0; k < SK_ARRAY_COUNT(gRec); ++k) {
            paint.setTextEncoding(gRec[k].fEncoding);

            size_t len = gRec[k].fSeedTextProc(src, dst, NGLYPHS);

            uint16_t    glyphs0[NGLYPHS], glyphs1[NGLYPHS];

            bool contains = paint.containsText(dst, len);
            int nglyphs = paint.textToGlyphs(dst, len, glyphs0);
            int first = face->charsToGlyphs(dst, paint2encoding(paint), glyphs1, NGLYPHS);
            int index = find_first_zero(glyphs1, NGLYPHS);

            REPORTER_ASSERT(reporter, NGLYPHS == nglyphs);
            REPORTER_ASSERT(reporter, index == first);
            REPORTER_ASSERT(reporter,
                        !memcmp(glyphs0, glyphs1, NGLYPHS * sizeof(uint16_t)));
            if (contains) {
                REPORTER_ASSERT(reporter, NGLYPHS == first);
            } else {
                REPORTER_ASSERT(reporter, NGLYPHS > first);
            }
        }
    }
}

// temparary api for bicubic, just be sure we can set/clear it
static void test_filterlevel(skiatest::Reporter* reporter) {
    SkPaint p0, p1;

    REPORTER_ASSERT(reporter,
                    SkPaint::kNone_FilterLevel == p0.getFilterLevel());

    static const SkPaint::FilterLevel gLevels[] = {
        SkPaint::kNone_FilterLevel,
        SkPaint::kLow_FilterLevel,
        SkPaint::kMedium_FilterLevel,
        SkPaint::kHigh_FilterLevel
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gLevels); ++i) {
        p0.setFilterLevel(gLevels[i]);
        REPORTER_ASSERT(reporter, gLevels[i] == p0.getFilterLevel());
        p1 = p0;
        REPORTER_ASSERT(reporter, gLevels[i] == p1.getFilterLevel());

        p0.reset();
        REPORTER_ASSERT(reporter,
                        SkPaint::kNone_FilterLevel == p0.getFilterLevel());
    }
}

static void test_copy(skiatest::Reporter* reporter) {
    SkPaint paint;
    // set a few member variables
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setTextAlign(SkPaint::kLeft_Align);
    paint.setStrokeWidth(SkIntToScalar(2));
    // set a few pointers
    SkLayerDrawLooper* looper = new SkLayerDrawLooper();
    paint.setLooper(looper)->unref();
    SkMaskFilter* mask = SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                                      SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(1)));
    paint.setMaskFilter(mask)->unref();

    // copy the paint using the copy constructor and check they are the same
    SkPaint copiedPaint = paint;
    REPORTER_ASSERT(reporter, paint == copiedPaint);

#ifdef SK_BUILD_FOR_ANDROID
    // the copy constructor should preserve the Generation ID
    uint32_t paintGenID = paint.getGenerationID();
    uint32_t copiedPaintGenID = copiedPaint.getGenerationID();
    REPORTER_ASSERT(reporter, paintGenID == copiedPaintGenID);
    REPORTER_ASSERT(reporter, !memcmp(&paint, &copiedPaint, sizeof(paint)));
#endif

    // copy the paint using the equal operator and check they are the same
    copiedPaint = paint;
    REPORTER_ASSERT(reporter, paint == copiedPaint);

#ifdef SK_BUILD_FOR_ANDROID
    // the equals operator should increment the Generation ID
    REPORTER_ASSERT(reporter, paint.getGenerationID() == paintGenID);
    REPORTER_ASSERT(reporter, copiedPaint.getGenerationID() != copiedPaintGenID);
    copiedPaintGenID = copiedPaint.getGenerationID(); // reset to the new value
    REPORTER_ASSERT(reporter, memcmp(&paint, &copiedPaint, sizeof(paint)));
#endif

    // clean the paint and check they are back to their initial states
    SkPaint cleanPaint;
    paint.reset();
    copiedPaint.reset();
    REPORTER_ASSERT(reporter, cleanPaint == paint);
    REPORTER_ASSERT(reporter, cleanPaint == copiedPaint);

#ifdef SK_BUILD_FOR_ANDROID
    // the reset function should increment the Generation ID
    REPORTER_ASSERT(reporter, paint.getGenerationID() != paintGenID);
    REPORTER_ASSERT(reporter, copiedPaint.getGenerationID() != copiedPaintGenID);
    REPORTER_ASSERT(reporter, memcmp(&cleanPaint, &paint, sizeof(cleanPaint)));
    REPORTER_ASSERT(reporter, memcmp(&cleanPaint, &copiedPaint, sizeof(cleanPaint)));
#endif
}

// found and fixed for webkit: mishandling when we hit recursion limit on
// mostly degenerate cubic flatness test
static void regression_cubic(skiatest::Reporter* reporter) {
    SkPath path, stroke;
    SkPaint paint;

    path.moveTo(460.2881309415525f,
                303.250847066498f);
    path.cubicTo(463.36378422175284f,
                 302.1169735073363f,
                 456.32239330810046f,
                 304.720354932878f,
                 453.15255460013304f,
                 305.788586869862f);

    SkRect fillR, strokeR;
    fillR = path.getBounds();

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    paint.getFillPath(path, &stroke);
    strokeR = stroke.getBounds();

    SkRect maxR = fillR;
    SkScalar miter = SkMaxScalar(SK_Scalar1, paint.getStrokeMiter());
    SkScalar inset = paint.getStrokeJoin() == SkPaint::kMiter_Join ?
                            SkScalarMul(paint.getStrokeWidth(), miter) :
                            paint.getStrokeWidth();
    maxR.inset(-inset, -inset);

    // test that our stroke didn't explode
    REPORTER_ASSERT(reporter, maxR.contains(strokeR));
}

// found and fixed for android: not initializing rect for string's of length 0
static void regression_measureText(skiatest::Reporter* reporter) {

    SkPaint paint;
    paint.setTextSize(12.0f);

    SkRect r;
    r.setLTRB(SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN);

    // test that the rect was reset
    paint.measureText("", 0, &r, 1.0f);
    REPORTER_ASSERT(reporter, r.isEmpty());
}

DEF_TEST(Paint, reporter) {
    // TODO add general paint tests
    test_copy(reporter);

    // regression tests
    regression_cubic(reporter);
    regression_measureText(reporter);

    test_filterlevel(reporter);

    // need to implement charsToGlyphs on other backends (e.g. linux, win)
    // before we can run this tests everywhere
    if (false) {
       test_cmap(reporter);
    }
}
