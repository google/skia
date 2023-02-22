/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>

DEF_TEST(SkFree_SafeToPassNull, reporter) {
    // This test passes by not crashing
    sk_free(nullptr);
}

DEF_TEST(SkCalloc_DataIsZeroInitializedAndWriteable, reporter) {
    constexpr size_t alloc_count = 100;

    uint8_t* bytes = (uint8_t*) sk_calloc_throw(alloc_count); // provide num bytes directly
    SkASSERT_RELEASE(bytes != nullptr);
    for (size_t i = 0; i < alloc_count; i++) {
        REPORTER_ASSERT(reporter, bytes[i] == 0);
        bytes[i] = (uint8_t)i;
    }
    sk_free(bytes);

    int32_t* ints = (int32_t*) sk_calloc_throw(alloc_count, sizeof(int32_t)); // count + elem size
    SkASSERT_RELEASE(ints != nullptr);
    for (size_t i = 0; i < alloc_count; i++) {
        REPORTER_ASSERT(reporter, ints[i] == 0);
        ints[i] = (int32_t)i;
    }
    sk_free(ints);
}

DEF_TEST(SkMalloc_DataIsWriteable, reporter) {
    constexpr size_t alloc_count = 100;

    uint8_t* bytes = (uint8_t*) sk_malloc_throw(alloc_count); // provide num bytes directly
    SkASSERT_RELEASE(bytes != nullptr);
    for (size_t i = 0; i < alloc_count; i++) {
        bytes[i] = (uint8_t)i;
    }
    sk_free(bytes);

    int32_t* ints = (int32_t*) sk_malloc_throw(alloc_count, sizeof(int32_t)); // count + elem size
    SkASSERT_RELEASE(ints != nullptr);
    for (size_t i = 0; i < alloc_count; i++) {
        ints[i] = (int32_t)i;
    }
    sk_free(ints);
}

DEF_TEST(SkRealloc_DataIsWriteableAndEventuallyFreed, reporter) {
    // Calling sk_realloc_throw with null should be treated as if it was sk_malloc_throw
    uint8_t* bytes = (uint8_t*) sk_realloc_throw(nullptr, 10);
    SkASSERT_RELEASE(bytes != nullptr);
    // Make sure those 10 bytes are writeable
    for (size_t i = 0; i < 10; i++) {
        bytes[i] = (uint8_t)i;
    }

    // Make it smaller
    bytes = (uint8_t*) sk_realloc_throw(bytes, 5);
    SkASSERT_RELEASE(bytes != nullptr);
    // Make sure those 5 bytes are still writeable and contain the previous values
    for (int i = 0; i < 5; i++) {
        REPORTER_ASSERT(reporter, bytes[i] == i, "bytes[%d] != %d", i, i);
        bytes[i] = (uint8_t)i + 17;
    }

    // Make it bigger
    bytes = (uint8_t*) sk_realloc_throw(bytes, 20, sizeof(uint8_t)); // count + elem size
    SkASSERT_RELEASE(bytes != nullptr);
    // Make sure the first 5 bytes are still writeable and contain the previous values
    for (int i = 0; i < 5; i++) {
        REPORTER_ASSERT(reporter, bytes[i] == (i + 17), "bytes[%d] != %d", i, i+17);
        bytes[i] = (uint8_t)i + 43;
    }
    // The next 15 bytes are uninitialized, so just make sure we can write to them.
    for (int i = 5; i < 20; i++) {
        bytes[i] = (uint8_t)i + 43;
    }

    // This should free the memory and return nullptr.
    bytes = (uint8_t*) sk_realloc_throw(bytes, 0);
    REPORTER_ASSERT(reporter, bytes == nullptr);
    // We run our tests with LeakSanitizer, so if bytes is *not* freed, we should see a failure.
}
