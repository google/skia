/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkSafeMath.h"

DEF_TEST(SafeMath, r) {
    SkSafeMath safe;

    // Whether 32 or 64 bit, no size_t can represent a sextillion (10^21).
    size_t x = 1;
    for (int i = 0; i < 7; i++ ) {
        x = safe.mul(x,1000);
    }
    REPORTER_ASSERT(r, !safe);


    safe.reset();

    // Let's emulate a common pattern for counting up the size of an allocation.
    size_t alloc = 0;
    alloc = safe.add(alloc, safe.mul(sizeof(double), 1024*1024));
    alloc = safe.add(alloc, safe.mul(sizeof(int),    1024*1024));
    alloc = safe.add(alloc, safe.mul(sizeof(float),  1024*1024));

    // Everyone can handle 16MB.
    REPORTER_ASSERT(r, safe);
    REPORTER_ASSERT(r, alloc == 16 * 1024 * 1024);

    // Once more, a little bigger.
    alloc = 0;
    alloc = safe.add(alloc, safe.mul(sizeof(double), 1024*1024*1024));
    alloc = safe.add(alloc, safe.mul(sizeof(int),    1024*1024*1024));
    alloc = safe.add(alloc, safe.mul(sizeof(float),  1024*1024*1024));

    // Whether size_t can handle 16GB depends on the architecture.
    if (sizeof(alloc) == 4) {
        REPORTER_ASSERT(r, !safe);
    } else {
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, alloc == 16ull * 1024 * 1024 * 1024);
    }
}
