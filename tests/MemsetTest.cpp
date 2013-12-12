/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkChunkAlloc.h"
#include "SkUtils.h"

static void test_chunkalloc(skiatest::Reporter* reporter) {
    size_t min = 256;
    SkChunkAlloc alloc(min);

    REPORTER_ASSERT(reporter, 0 == alloc.totalCapacity());
    REPORTER_ASSERT(reporter, 0 == alloc.totalUsed());
    REPORTER_ASSERT(reporter, 0 == alloc.blockCount());
    REPORTER_ASSERT(reporter, !alloc.contains(NULL));
    REPORTER_ASSERT(reporter, !alloc.contains(reporter));

    alloc.reset();
    REPORTER_ASSERT(reporter, 0 == alloc.totalCapacity());
    REPORTER_ASSERT(reporter, 0 == alloc.totalUsed());
    REPORTER_ASSERT(reporter, 0 == alloc.blockCount());

    size_t size = min >> 1;
    void* ptr = alloc.allocThrow(size);
    REPORTER_ASSERT(reporter, alloc.totalCapacity() >= size);
    REPORTER_ASSERT(reporter, alloc.totalUsed() == size);
    REPORTER_ASSERT(reporter, alloc.blockCount() > 0);
    REPORTER_ASSERT(reporter, alloc.contains(ptr));

    alloc.reset();
    REPORTER_ASSERT(reporter, !alloc.contains(ptr));
    REPORTER_ASSERT(reporter, 0 == alloc.totalCapacity());
    REPORTER_ASSERT(reporter, 0 == alloc.totalUsed());
}

///////////////////////////////////////////////////////////////////////////////

static void set_zero(void* dst, size_t bytes) {
    char* ptr = (char*)dst;
    for (size_t i = 0; i < bytes; ++i) {
        ptr[i] = 0;
    }
}

#define MAX_ALIGNMENT   64
#define MAX_COUNT       ((MAX_ALIGNMENT) * 32)
#define PAD             32
#define TOTAL           (PAD + MAX_ALIGNMENT + MAX_COUNT + PAD)

#define VALUE16         0x1234
#define VALUE32         0x12345678

static bool compare16(const uint16_t base[], uint16_t value, int count) {
    for (int i = 0; i < count; ++i) {
        if (base[i] != value) {
            SkDebugf("[%d] expected %x found %x\n", i, value, base[i]);
            return false;
        }
    }
    return true;
}

static bool compare32(const uint32_t base[], uint32_t value, int count) {
    for (int i = 0; i < count; ++i) {
        if (base[i] != value) {
            SkDebugf("[%d] expected %x found %x\n", i, value, base[i]);
            return false;
        }
    }
    return true;
}

static void test_16(skiatest::Reporter* reporter) {
    uint16_t buffer[TOTAL];

    for (int count = 0; count < MAX_COUNT; ++count) {
        for (int alignment = 0; alignment < MAX_ALIGNMENT; ++alignment) {
            set_zero(buffer, sizeof(buffer));

            uint16_t* base = &buffer[PAD + alignment];
            sk_memset16(base, VALUE16, count);

            REPORTER_ASSERT(reporter,
                compare16(buffer,       0,       PAD + alignment) &&
                compare16(base,         VALUE16, count) &&
                compare16(base + count, 0,       TOTAL - count - PAD - alignment));
        }
    }
}

static void test_32(skiatest::Reporter* reporter) {
    uint32_t buffer[TOTAL];

    for (int count = 0; count < MAX_COUNT; ++count) {
        for (int alignment = 0; alignment < MAX_ALIGNMENT; ++alignment) {
            set_zero(buffer, sizeof(buffer));

            uint32_t* base = &buffer[PAD + alignment];
            sk_memset32(base, VALUE32, count);

            REPORTER_ASSERT(reporter,
                compare32(buffer,       0,       PAD + alignment) &&
                compare32(base,         VALUE32, count) &&
                compare32(base + count, 0,       TOTAL - count - PAD - alignment));
        }
    }
}

/**
 *  Test sk_memset16 and sk_memset32.
 *  For performance considerations, implementations may take different paths
 *  depending on the alignment of the dst, and/or the size of the count.
 */
DEF_TEST(Memset, reporter) {
    test_16(reporter);
    test_32(reporter);

    test_chunkalloc(reporter);
}
