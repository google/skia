/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPackBits.h"
#include "Test.h"

#include "SkRandom.h"
static SkRandom gRand;
static const uint8_t gTest80[] = { 0, 0, 1, 1 };
static const uint8_t gTest81[] = { 1, 2, 3, 4, 5, 6 };
static const uint8_t gTest82[] = { 0, 0, 0, 1, 2, 3, 3, 3 };
static const uint8_t gTest83[] = { 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 0, 0, 1 };
static const uint8_t gTest84[] = { 1, 0, 3, 0, 0, 0, 2, 1, 1, 2 };

static void rand_fill(uint8_t buffer[], int count) {
    for (int i = 0; i < count; i++)
        buffer[i] = (uint8_t)((gRand.nextU() >> 8) & 0x3);
}

static void test_pack8(skiatest::Reporter* reporter) {
    static const struct {
        const uint8_t* fSrc;
        int             fCount;
    } gTests[] = {
        { gTest80, SK_ARRAY_COUNT(gTest80) },
        { gTest81, SK_ARRAY_COUNT(gTest81) },
        { gTest82, SK_ARRAY_COUNT(gTest82) },
        { gTest83, SK_ARRAY_COUNT(gTest83) },
        { gTest84, SK_ARRAY_COUNT(gTest84) }
    };

    for (size_t i = 4; i < SK_ARRAY_COUNT(gTests); i++) {
        uint8_t dst[100];
        size_t maxSize = SkPackBits::ComputeMaxSize8(gTests[i].fCount);
        size_t dstSize = SkPackBits::Pack8(gTests[i].fSrc,
                                           gTests[i].fCount, dst, maxSize - 1);
        REPORTER_ASSERT(reporter, dstSize == 0);
        dstSize = SkPackBits::Pack8(gTests[i].fSrc,
                                           gTests[i].fCount, dst, sizeof(dst));
        REPORTER_ASSERT(reporter, dstSize <= maxSize);
        uint8_t src[100];
        int srcCount = SkPackBits::Unpack8(dst, dstSize, src, gTests[i].fCount - 1);
        REPORTER_ASSERT(reporter, srcCount == 0);
        srcCount = SkPackBits::Unpack8(dst, dstSize, src, sizeof(src));
        bool match = gTests[i].fCount == srcCount &&
                    memcmp(gTests[i].fSrc, src,
                           gTests[i].fCount * sizeof(uint8_t)) == 0;
        REPORTER_ASSERT(reporter, match);
    }

    for (uint32_t size = 1; size <= 512; size += 1) {
        for (int n = 100; n; n--) {
            uint8_t src[600], src2[600];
            uint8_t dst[600];
            rand_fill(src, size);

            size_t dstSize = SkPackBits::Pack8(src, size, dst, sizeof(dst));
            size_t maxSize = SkPackBits::ComputeMaxSize8(size);
            REPORTER_ASSERT(reporter, maxSize >= dstSize);

            size_t srcCount = SkPackBits::Unpack8(dst, dstSize, src2, size);
            REPORTER_ASSERT(reporter, size == srcCount);
            bool match = memcmp(src, src2, size * sizeof(uint8_t)) == 0;
            REPORTER_ASSERT(reporter, match);
        }
    }
}

DEF_TEST(PackBits, reporter) {
    test_pack8(reporter);
}
