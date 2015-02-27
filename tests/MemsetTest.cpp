/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChunkAlloc.h"
#include "SkUtils.h"
#include "Test.h"

static void check_alloc(skiatest::Reporter* reporter, const SkChunkAlloc& alloc,
                        size_t capacity, size_t used, int numBlocks) {
    REPORTER_ASSERT(reporter, alloc.totalCapacity() >= capacity);
    REPORTER_ASSERT(reporter, alloc.totalUsed() == used);
    SkDEBUGCODE(REPORTER_ASSERT(reporter, alloc.blockCount() == numBlocks);)
}

static void* simple_alloc(skiatest::Reporter* reporter, SkChunkAlloc* alloc, size_t size) {
    void* ptr = alloc->allocThrow(size);
    check_alloc(reporter, *alloc, size, size, 1);
    REPORTER_ASSERT(reporter, alloc->contains(ptr));
    return ptr;
}
                        
static void test_chunkalloc(skiatest::Reporter* reporter) {
    static const size_t kMin = 1024;
    SkChunkAlloc alloc(kMin);

    //------------------------------------------------------------------------
    // check empty
    check_alloc(reporter, alloc, 0, 0, 0);
    REPORTER_ASSERT(reporter, !alloc.contains(NULL));
    REPORTER_ASSERT(reporter, !alloc.contains(reporter));

    // reset on empty allocator
    alloc.reset();
    check_alloc(reporter, alloc, 0, 0, 0);

    // rewind on empty allocator
    alloc.rewind();
    check_alloc(reporter, alloc, 0, 0, 0);

    //------------------------------------------------------------------------
    // test reset when something is allocated
    size_t size = kMin >> 1;
    void* ptr = simple_alloc(reporter, &alloc, size);

    alloc.reset();
    check_alloc(reporter, alloc, 0, 0, 0);
    REPORTER_ASSERT(reporter, !alloc.contains(ptr));

    //------------------------------------------------------------------------
    // test rewind when something is allocated
    ptr = simple_alloc(reporter, &alloc, size);

    alloc.rewind();
    check_alloc(reporter, alloc, size, 0, 1);
    REPORTER_ASSERT(reporter, !alloc.contains(ptr));

    // use the available block
    ptr = simple_alloc(reporter, &alloc, size);
    alloc.reset();

    //------------------------------------------------------------------------
    // test out allocating a second block
    ptr = simple_alloc(reporter, &alloc, size);

    ptr = alloc.allocThrow(kMin);
    check_alloc(reporter, alloc, 2*kMin, size+kMin, 2);
    REPORTER_ASSERT(reporter, alloc.contains(ptr));

    //------------------------------------------------------------------------
    // test out unalloc
    size_t freed = alloc.unalloc(ptr);
    REPORTER_ASSERT(reporter, freed == kMin);
    check_alloc(reporter, alloc, 2*kMin, size, 2);
    REPORTER_ASSERT(reporter, !alloc.contains(ptr));
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
