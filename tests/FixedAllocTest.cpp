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
        Foo() : x(-2), y(-3.0f) { created++; }
        Foo(int X, float Y) : x(X), y(Y) { created++; }
        ~Foo() { destroyed++; }

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
        // Test alignment gurantees.
        uint8_t buf[64];
        SkFixedAlloc fa(buf+3, sizeof(buf)-3);

        Foo* foo = fa.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, SkIsAlign4((uintptr_t)foo));
        REPORTER_ASSERT(r, created == 3);
        REPORTER_ASSERT(r, destroyed == 2);

        // Might as well test reset() while we're at it.
        fa.reset();
        REPORTER_ASSERT(r, created == 3);
        REPORTER_ASSERT(r, destroyed == 3);
    }
    REPORTER_ASSERT(r, created == 3);
    REPORTER_ASSERT(r, destroyed == 3);
}

DEF_TEST(FallbackAlloc, r) {
    // SkFixedAlloc will eventually fail when it runs out of space in its buffer.
    int buf[32];
    SkFixedAlloc fixed(buf, sizeof(buf));
    bool fixed_failed = false;
    for (int i = 0; i < 32; i++) {
        // (Remember, there is some overhead to each make() call.)
        fixed_failed = fixed_failed || (fixed.make<int>(i) == nullptr);
    }
    REPORTER_ASSERT(r, fixed_failed);


    // SkFallbackAlloc will always succeed, using the heap as required.
    fixed.reset();
    SkFallbackAlloc fallback(&fixed);

    bool fallback_failed = false;
    for (int i = 0; i < 32; i++) {
        fallback_failed = fallback_failed || (fallback.make<int>(i) == nullptr);
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

struct WithDtor {
    ~WithDtor() { }
};

DEF_TEST(ArenaAlloc, r) {

    {
        created = 0;
        destroyed = 0;

        SkArenaAlloc arena{nullptr, 0};
        REPORTER_ASSERT(r, *arena.make<int>(3) == 3);
        Foo* foo = arena.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo->x == 3);
        REPORTER_ASSERT(r, foo->y == 4.0f);
        REPORTER_ASSERT(r, created == 1);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.makeArrayDefault<int>(10);
        int* zeroed = arena.makeArray<int>(10);
        for (int i = 0; i < 10; i++) {
            REPORTER_ASSERT(r, zeroed[i] == 0);
        }
        Foo* fooArray = arena.makeArrayDefault<Foo>(10);
        REPORTER_ASSERT(r, fooArray[3].x == -2);
        REPORTER_ASSERT(r, fooArray[4].y == -3.0f);
        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.make<typename std::aligned_storage<10,8>::type>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
        char block[1024];
        SkArenaAlloc arena{block};

        REPORTER_ASSERT(r, *arena.make<int>(3) == 3);
        Foo* foo = arena.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo->x == 3);
        REPORTER_ASSERT(r, foo->y == 4.0f);
        REPORTER_ASSERT(r, created == 1);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.makeArrayDefault<int>(10);
        int* zeroed = arena.makeArray<int>(10);
        for (int i = 0; i < 10; i++) {
            REPORTER_ASSERT(r, zeroed[i] == 0);
        }
        Foo* fooArray = arena.makeArrayDefault<Foo>(10);
        REPORTER_ASSERT(r, fooArray[3].x == -2);
        REPORTER_ASSERT(r, fooArray[4].y == -3.0f);
        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.make<typename std::aligned_storage<10,8>::type>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
        std::unique_ptr<char[]> block{new char[1024]};
        SkArenaAlloc arena{block.get(), 1024};

        REPORTER_ASSERT(r, *arena.make<int>(3) == 3);
        Foo* foo = arena.make<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo->x == 3);
        REPORTER_ASSERT(r, foo->y == 4.0f);
        REPORTER_ASSERT(r, created == 1);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.makeArrayDefault<int>(10);
        int* zeroed = arena.makeArray<int>(10);
        for (int i = 0; i < 10; i++) {
            REPORTER_ASSERT(r, zeroed[i] == 0);
        }
        Foo* fooArray = arena.makeArrayDefault<Foo>(10);
        REPORTER_ASSERT(r, fooArray[3].x == -2);
        REPORTER_ASSERT(r, fooArray[4].y == -3.0f);
        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.make<typename std::aligned_storage<10,8>::type>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

}
