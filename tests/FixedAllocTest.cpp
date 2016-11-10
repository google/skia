/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkFixedAlloc.h"

namespace {

    static int created, destroyed;

    struct Foo {
         Foo(int X, float Y) : x(X), y(Y) { created++; }
        ~Foo() { destroyed++; }

        int x;
        float y;
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
        REPORTER_ASSERT(r, destroyed == 0);

        Foo* bar = fa.make<Foo>(8, 1.0f);
        REPORTER_ASSERT(r, bar);
        REPORTER_ASSERT(r, bar->x == 8);
        REPORTER_ASSERT(r, bar->y == 1.0f);
        REPORTER_ASSERT(r, created == 2);
        REPORTER_ASSERT(r, destroyed == 0);

        fa.undo();
        REPORTER_ASSERT(r, created == 2);
        REPORTER_ASSERT(r, destroyed == 1);
    }
    REPORTER_ASSERT(r, created == 2);
    REPORTER_ASSERT(r, destroyed == 2);

    {
        // Test 16-byte alignment gurantees.
        uint8_t buf[64];
        SkFixedAlloc fa(buf+3, sizeof(buf)-3);

        Foo* foo = fa.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, SkIsAlign16((uintptr_t)foo));
        REPORTER_ASSERT(r, created == 3);
        REPORTER_ASSERT(r, destroyed == 2);

        // There should not be any more space in buf to allocate another 16-byte aligned Foo.
        REPORTER_ASSERT(r, nullptr == fa.make<Foo>(2, 3.0f));

        // Might as well test reset() while we're at it.
        fa.reset();
        REPORTER_ASSERT(r, created == 3);
        REPORTER_ASSERT(r, destroyed == 3);
    }
    REPORTER_ASSERT(r, created == 3);
    REPORTER_ASSERT(r, destroyed == 3);
}
