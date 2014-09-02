/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapHasher.h"

#include "SkBitmap.h"
#include "SkColor.h"
#include "Test.h"

// Word size that is large enough to hold results of any checksum type.
typedef uint64_t checksum_result;

// Fill in bitmap with test data.
static void CreateTestBitmap(SkBitmap* bitmap, int width, int height,
                             SkColor color, skiatest::Reporter* reporter) {
    bitmap->allocN32Pixels(width, height, kOpaque_SkAlphaType);
    bitmap->eraseColor(color);
}

DEF_TEST(BitmapHasher, reporter) {
    // Test SkBitmapHasher
    SkBitmap bitmap;
    uint64_t digest;
    // initial test case
    CreateTestBitmap(&bitmap, 333, 555, SK_ColorBLUE, reporter);
    REPORTER_ASSERT(reporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
    REPORTER_ASSERT(reporter, digest == 0xfb2903562766ef87ULL);
    // same pixel data but different dimensions should yield a different checksum
    CreateTestBitmap(&bitmap, 555, 333, SK_ColorBLUE, reporter);
    REPORTER_ASSERT(reporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
    REPORTER_ASSERT(reporter, digest == 0xfe04023fb97d0f61ULL);
    // same dimensions but different color should yield a different checksum
    CreateTestBitmap(&bitmap, 555, 333, SK_ColorGREEN, reporter);
    REPORTER_ASSERT(reporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
    REPORTER_ASSERT(reporter, digest == 0x2423c51cad6d1edcULL);
}
