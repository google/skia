/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkRandom.h"

// Clang seems to think only 32-bit alignment is guaranteed on 32-bit x86 Android.
// See https://reviews.llvm.org/D8357
// This is why we have disabled -Wover-aligned there (we allocate 8-byte aligned structs in Ganesh).
DEF_TEST(OverAligned, r) {
    SkRandom rand;
    // Let's test that assertion.  We think it really should be providing 8-byte alignment.
    for (int i = 0; i < 1000; i++) {
        void* p = sk_malloc_throw(rand.nextRangeU(0,100));
        REPORTER_ASSERT(r, SkIsAlign8((uintptr_t)p));
        sk_free(p);
    }
}
