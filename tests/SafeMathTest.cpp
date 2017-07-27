/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkSafeMath.h"

DEF_TEST(SafeMath, r) {
    {  // int * int -> int
        SkSafeMath safe;
        REPORTER_ASSERT(r, safe.mul(2, 512*1024*1024) == 1024*1024*1024);
        REPORTER_ASSERT(r, safe);
        (void)safe.mul(4, 512*1024*1024);
        REPORTER_ASSERT(r, SkSafeMath::Unchecked() || !safe);
    }

    {  // unsigned * int -> unsigned
        SkSafeMath safe;
        REPORTER_ASSERT(r, safe.mul(2u, 512*1024*1024) ==    1024*1024*1024);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.mul(4u, 512*1024*1024) == 2u*1024*1024*1024);
        REPORTER_ASSERT(r, safe);
    }

    {  // natural type mixing when sizing an allocation
        SkSafeMath safe;

        int n = 1024;
        size_t alloc = 0;
        alloc = safe.add(alloc, safe.mul(sizeof(double), n));
        alloc = safe.add(alloc, safe.mul(sizeof(float ), n));
        alloc = safe.add(alloc, safe.mul(sizeof(int   ), n));
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, alloc == 16*1024);

        n = 1024*1024*1024;
        alloc = 0;
        alloc = safe.add(alloc, safe.mul(sizeof(double), n));
        alloc = safe.add(alloc, safe.mul(sizeof(float ), n));
        alloc = safe.add(alloc, safe.mul(sizeof(int   ), n));

        if (sizeof(size_t) == 8) {
            REPORTER_ASSERT(r, safe);
            REPORTER_ASSERT(r, alloc == 16ull * 1024 * 1024 * 1024);
        } else {
            REPORTER_ASSERT(r, SkSafeMath::Unchecked() || !safe);
        }
    }
}
