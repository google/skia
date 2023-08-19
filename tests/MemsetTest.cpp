/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMemset.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>

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

static void compare16(skiatest::Reporter* r, const uint16_t base[],
                      uint16_t value, int count) {
    for (int i = 0; i < count; ++i) {
        if (base[i] != value) {
            ERRORF(r, "[%d] expected %x found %x\n", i, value, base[i]);
            return;
        }
    }
}

static void compare32(skiatest::Reporter* r, const uint32_t base[],
                      uint32_t value, int count) {
    for (int i = 0; i < count; ++i) {
        if (base[i] != value) {
            ERRORF(r, "[%d] expected %x found %x\n", i, value, base[i]);
            return;
        }
    }
}

static void test_16(skiatest::Reporter* reporter) {
    uint16_t buffer[TOTAL];

    for (int count = 0; count < MAX_COUNT; ++count) {
        for (int alignment = 0; alignment < MAX_ALIGNMENT; ++alignment) {
            set_zero(buffer, sizeof(buffer));

            uint16_t* base = &buffer[PAD + alignment];
            SkOpts::memset16(base, VALUE16, count);

            compare16(reporter, buffer,       0,       PAD + alignment);
            compare16(reporter, base,         VALUE16, count);
            compare16(reporter, base + count, 0,       TOTAL - count - PAD - alignment);
        }
    }
}

static void test_32(skiatest::Reporter* reporter) {
    uint32_t buffer[TOTAL];

    for (int count = 0; count < MAX_COUNT; ++count) {
        for (int alignment = 0; alignment < MAX_ALIGNMENT; ++alignment) {
            set_zero(buffer, sizeof(buffer));

            uint32_t* base = &buffer[PAD + alignment];
            SkOpts::memset32(base, VALUE32, count);

            compare32(reporter, buffer,       0,       PAD + alignment);
            compare32(reporter, base,         VALUE32, count);
            compare32(reporter, base + count, 0,       TOTAL - count - PAD - alignment);
        }
    }
}

/**
 *  Test SkOpts::memset16 and SkOpts::memset32.
 *  For performance considerations, implementations may take different paths
 *  depending on the alignment of the dst, and/or the size of the count.
 */
DEF_TEST(Memset, reporter) {
    test_16(reporter);
    test_32(reporter);
}
