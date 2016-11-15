/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkFixedAlloc.h"

namespace {

    static int created;

    struct Foo {
         Foo(int X, float Y) : x(X), y(Y) { created++; }

        int x;
        float y;
    };

    struct Big {
        Big() {}
        uint32_t array[128];
    };

}

DEF_TEST(FixedAlloc, r) {
    // Basic mechanics.
    {
        uint8_t buf[128];
        SkFixedAlloc fa(buf, sizeof(buf));

        Foo* foo = fa.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo);
        REPORTER_ASSERT(r, foo->x == 3);
        REPORTER_ASSERT(r, foo->y == 4.0f);
        REPORTER_ASSERT(r, created == 1);

        Foo* bar = fa.make<Foo>(8, 1.0f);
        REPORTER_ASSERT(r, bar);
        REPORTER_ASSERT(r, bar->x == 8);
        REPORTER_ASSERT(r, bar->y == 1.0f);
        REPORTER_ASSERT(r, created == 2);

        fa.undo();
    }

    {
        // Test alignment gurantees.
        uint8_t buf[64];
        SkFixedAlloc fa(buf+3, sizeof(buf)-3);

        Foo* foo = fa.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, SkIsAlign4((uintptr_t)foo));
        REPORTER_ASSERT(r, created == 3);

        // Might as well test reset() while we're at it.
        fa.reset();
    }
}

DEF_TEST(FallbackAlloc, r) {
    // SkFixedAlloc will eventually fail when it runs out of space in its buffer.
    int buf[32];
    SkFixedAlloc fixed(buf, sizeof(buf));
    bool fixed_failed = false;
    for (int i = 0; i < 32; i++) {
        // (Remember, there is some overhead to each copy() call.)
        fixed_failed = fixed_failed || (fixed.copy(i) == nullptr);
    }
    REPORTER_ASSERT(r, fixed_failed);


    // SkFallbackAlloc will always succeed, using the heap as required.
    fixed.reset();
    SkFallbackAlloc fallback(&fixed);

    bool fallback_failed = false;
    for (int i = 0; i < 32; i++) {
        fallback_failed = fallback_failed || (fallback.copy(i) == nullptr);
    }
    REPORTER_ASSERT(r, !fallback_failed);


    // Test small, big, small allocations to make sure once we go to the heap we stay there.
    fallback.reset();
    auto smallA = fallback.make<int>(2);
    auto    big = fallback.make<Big>();
    auto smallB = fallback.make<int>(3);

    auto in_buf = [&](void* ptr) {
        return (uintptr_t)(buf+0 ) <= (uintptr_t)ptr
            && (uintptr_t)(buf+32) >  (uintptr_t)ptr;
    };

    REPORTER_ASSERT(r,  in_buf(smallA));
    REPORTER_ASSERT(r, !in_buf(big));
    REPORTER_ASSERT(r, !in_buf(smallB));
}
