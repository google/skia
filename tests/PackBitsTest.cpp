/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkPackBits.h"

static const uint16_t gTest0[] = { 0, 0, 1, 1 };
static const uint16_t gTest1[] = { 1, 2, 3, 4, 5, 6 };
static const uint16_t gTest2[] = { 0, 0, 0, 1, 2, 3, 3, 3 };
static const uint16_t gTest3[] = { 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 0, 0, 1 };

#include "SkRandom.h"
static SkRandom gRand;
static void rand_fill(uint16_t buffer[], int count) {
    for (int i = 0; i < count; i++)
        buffer[i] = (uint16_t)gRand.nextU();
}

static void test_pack16(skiatest::Reporter* reporter) {
    static const struct {
        const uint16_t* fSrc;
        int             fCount;
    } gTests[] = {
        { gTest0, SK_ARRAY_COUNT(gTest0) },
        { gTest1, SK_ARRAY_COUNT(gTest1) },
        { gTest2, SK_ARRAY_COUNT(gTest2) },
        { gTest3, SK_ARRAY_COUNT(gTest3) }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); i++) {
        uint8_t dst[100];
        size_t dstSize = SkPackBits::Pack16(gTests[i].fSrc,
                                            gTests[i].fCount, dst);
        uint16_t src[100];
        int srcCount = SkPackBits::Unpack16(dst, dstSize, src);
        bool match = gTests[i].fCount == srcCount && memcmp(gTests[i].fSrc, src,
                                    gTests[i].fCount * sizeof(uint16_t)) == 0;
        REPORTER_ASSERT(reporter, match);
    }

    for (int n = 1000; n; n--) {
        size_t size = 50;
        uint16_t src[100], src2[100];
        uint8_t dst[200];
        rand_fill(src, size);

        size_t dstSize = SkPackBits::Pack16(src, size, dst);
        size_t maxSize = SkPackBits::ComputeMaxSize16(size);
        REPORTER_ASSERT(reporter, maxSize >= dstSize);

        size_t srcCount = SkPackBits::Unpack16(dst, dstSize, src2);
        REPORTER_ASSERT(reporter, size == srcCount);
        bool match = memcmp(src, src2, size * sizeof(uint16_t)) == 0;
        REPORTER_ASSERT(reporter, match);
    }
}

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
                                           gTests[i].fCount, dst);
        REPORTER_ASSERT(reporter, dstSize <= maxSize);
        uint8_t src[100];
        int srcCount = SkPackBits::Unpack8(dst, dstSize, src);
        bool match = gTests[i].fCount == srcCount &&
                    memcmp(gTests[i].fSrc, src,
                           gTests[i].fCount * sizeof(uint8_t)) == 0;
        REPORTER_ASSERT(reporter, match);
    }

    for (size_t size = 1; size <= 512; size += 1) {
        for (int n = 100; n; n--) {
            uint8_t src[600], src2[600];
            uint8_t dst[600];
            rand_fill(src, size);

            size_t dstSize = SkPackBits::Pack8(src, size, dst);
            size_t maxSize = SkPackBits::ComputeMaxSize8(size);
            REPORTER_ASSERT(reporter, maxSize >= dstSize);

            size_t srcCount = SkPackBits::Unpack8(dst, dstSize, src2);
            REPORTER_ASSERT(reporter, size == srcCount);
            bool match = memcmp(src, src2, size * sizeof(uint8_t)) == 0;
            REPORTER_ASSERT(reporter, match);

            for (int j = 0; j < 100; j++) {
                size_t skip = gRand.nextU() % size;
                size_t write = gRand.nextU() % size;
                if (skip + write > size) {
                    write = size - skip;
                }
                SkPackBits::Unpack8(src, skip, write, dst);
                bool match = memcmp(src, src2 + skip, write) == 0;
                REPORTER_ASSERT(reporter, match);
            }
        }
    }
}

DEF_TEST(PackBits, reporter) {
    test_pack8(reporter);
    test_pack16(reporter);
}
