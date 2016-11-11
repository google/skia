/*
 * Copyright 2011-2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkPaint.h"
#include "Test.h"

static void test_monotonic(skiatest::Reporter* reporter,
                           const SkPaint& paint,
                           const char* msg) {
    const char* text = "sdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj";
    const size_t length = strlen(text);
    const SkScalar width = paint.measureText(text, length);

    SkScalar mm = 0;
    size_t nn = 0;
    const SkScalar step = SkMaxScalar(width / 10, SK_Scalar1);
    for (SkScalar w = 0; w <= width; w += step) {
        SkScalar m;
        const size_t n = paint.breakText(text, length, w, &m);

        REPORTER_ASSERT_MESSAGE(reporter, n <= length, msg);
        REPORTER_ASSERT_MESSAGE(reporter, m <= width, msg);

        if (n == 0) {
            REPORTER_ASSERT_MESSAGE(reporter, m == 0, msg);
        } else {
            // now assert that we're monotonic
            if (n == nn) {
                REPORTER_ASSERT_MESSAGE(reporter, m == mm, msg);
            } else {
                REPORTER_ASSERT_MESSAGE(reporter, n > nn, msg);
                REPORTER_ASSERT_MESSAGE(reporter, m > mm, msg);
            }
        }
        nn = n;
        mm = m;
    }
}

static void test_eq_measure_text(skiatest::Reporter* reporter,
                                 const SkPaint& paint,
                                 const char* msg) {
    const char* text = "The ultimate measure of a man is not where he stands in moments of comfort "
        "and convenience, but where he stands at times of challenge and controversy.";
    const size_t length = strlen(text);
    const SkScalar width = paint.measureText(text, length);

    SkScalar mm;
    const size_t length2 = paint.breakText(text, length, width, &mm);
    REPORTER_ASSERT_MESSAGE(reporter, length2 == length, msg);
    REPORTER_ASSERT_MESSAGE(reporter, mm == width, msg);
}

static void test_long_text(skiatest::Reporter* reporter,
                           const SkPaint& paint,
                           const char* msg) {
    static const int kSize = 16 * 1024;
    SkAutoMalloc block(kSize);
    memset(block.get(), 'a', kSize - 1);
    char* text = static_cast<char*>(block.get());
    text[kSize - 1] = '\0';
    const SkScalar width = paint.measureText(text, kSize);

    SkScalar mm;
    const size_t length = paint.breakText(text, kSize, width, &mm);
    REPORTER_ASSERT_MESSAGE(reporter, length == kSize, msg);
    REPORTER_ASSERT_MESSAGE(reporter, mm == width, msg);
}

DEF_TEST(PaintBreakText, reporter) {
    SkPaint paint;
    test_monotonic(reporter, paint, "default");
    test_eq_measure_text(reporter, paint, "default");
    test_long_text(reporter, paint, "default");
    paint.setTextSize(SkIntToScalar(1 << 17));
    test_monotonic(reporter, paint, "huge text size");
    test_eq_measure_text(reporter, paint, "huge text size");
    paint.setTextSize(0);
    test_monotonic(reporter, paint, "zero text size");
}
