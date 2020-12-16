/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkArenaAlloc.h"
#include "tests/Test.h"

#include <memory>
#include <new>
#include <type_traits>

DEF_TEST(ArenaAlloc, r) {
    static int created = 0,
               destroyed = 0;

    struct Foo {
        Foo() : x(-2), y(-3.0f) { created++; }
        Foo(int X, float Y) : x(X), y(Y) { created++; }
        ~Foo() { destroyed++; }
        int x;
        float y;
    };

    struct alignas(8) OddAlignment {
        char buf[10];
    };

    created = 0;
    destroyed = 0;
    {
        SkArenaAlloc arena{0};
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
        arena.make<OddAlignment>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    created = 0;
    destroyed = 0;
    {
        SkSTArenaAlloc<64> arena;
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
        arena.make<OddAlignment>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    created = 0;
    destroyed = 0;
    {
        std::unique_ptr<char[]> block{new char[1024]};
        SkArenaAlloc arena{block.get(), 1024, 0};
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
        arena.make<OddAlignment>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        SkSTArenaAllocWithReset<64> arena;
        arena.makeArrayDefault<char>(256);
        arena.reset();
        arena.reset();
    }

    // Make sure that multiple blocks are handled correctly.
    created = 0;
    destroyed = 0;
    {
        struct Node {
            Node(Node* n) : next(n) { created++; }
            ~Node() { destroyed++; }
            Node *next;
            char filler[64];
        };

        SkSTArenaAlloc<64> arena;
        Node* current = nullptr;
        for (int i = 0; i < 128; i++) {
            current = arena.make<Node>(current);
        }
    }
    REPORTER_ASSERT(r, created == 128);
    REPORTER_ASSERT(r, destroyed == 128);

    // Make sure that objects and blocks are destroyed in the correct order. If they are not,
    // then there will be a use after free error in asan.
    created = 0;
    destroyed = 0;
    {
        struct Node {
            Node(Node* n) : next(n) { created++; }
            ~Node() {
                destroyed++;
                if (next) {
                    next->~Node();
                }
            }
            Node *next;
        };

        SkSTArenaAlloc<64> arena;
        Node* current = nullptr;
        for (int i = 0; i < 128; i++) {
            uint64_t* temp = arena.makeArrayDefault<uint64_t>(sizeof(Node) / sizeof(Node*));
            current = new (temp)Node(current);
        }
        current->~Node();
    }
    REPORTER_ASSERT(r, created == 128);
    REPORTER_ASSERT(r, destroyed == 128);

    {
        SkSTArenaAlloc<64> arena;
        auto a = arena.makeInitializedArray<int>(8, [](size_t i ) { return i; });
        for (size_t i = 0; i < 8; i++) {
            REPORTER_ASSERT(r, a[i] == (int)i);
        }
    }

    {
        SkArenaAlloc arena(4096);
        // Move to a 1 character boundary.
        arena.make<char>();
        // Allocate something with interesting alignment.
        void* ptr = arena.makeBytesAlignedTo(4081, 8);
        REPORTER_ASSERT(r, ((intptr_t)ptr & 7) == 0);
    }
}

DEF_TEST(SkFibBlockSizes, r) {
    {
        SkFibBlockSizes<std::numeric_limits<uint32_t>::max()> fibs{1, 1};
        uint32_t lastSize = 1;
        for (int i = 0; i < 64; i++) {
            uint32_t size = fibs.nextBlockSize();
            REPORTER_ASSERT(r, lastSize <= size);
            lastSize = size;
        }
        REPORTER_ASSERT(r, lastSize == 2971215073u);
    }
    {
        SkFibBlockSizes<std::numeric_limits<uint32_t>::max()> fibs{0, 1024};
        uint32_t lastSize = 1;
        for (int i = 0; i < 64; i++) {
            uint32_t size = fibs.nextBlockSize();
            REPORTER_ASSERT(r, lastSize <= size);
            lastSize = size;
            REPORTER_ASSERT(r, lastSize <= std::numeric_limits<uint32_t>::max());
        }
        REPORTER_ASSERT(r, lastSize == 3524578u * 1024);
    }

    {
        SkFibBlockSizes<std::numeric_limits<uint32_t>::max() / 2> fibs{1024, 0};
        uint32_t lastSize = 1;
        for (int i = 0; i < 64; i++) {
            uint32_t size = fibs.nextBlockSize();
            REPORTER_ASSERT(r, lastSize <= size);
            lastSize = size;
            REPORTER_ASSERT(r, lastSize <= std::numeric_limits<uint32_t>::max() / 2);
        }
        REPORTER_ASSERT(r, lastSize == 1346269u * 1024);
    }
}
