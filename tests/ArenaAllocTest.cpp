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

    struct Start {
        ~Start() {
            if (start) {
                start->~Node();
            }
        }
        Node* start;
    };

    struct FooRefCnt : public SkRefCnt {
        FooRefCnt() : x(-2), y(-3.0f) { created++; }
        FooRefCnt(int X, float Y) : x(X), y(Y) { created++; }
        ~FooRefCnt() { destroyed++; }

        int x;
        float y;
    };

}

struct WithDtor {
    ~WithDtor() { }
};

DEF_TEST(ArenaAlloc, r) {

    {
        created = 0;
        destroyed = 0;

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
        arena.make<typename std::aligned_storage<10,8>::type>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
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
        arena.make<typename std::aligned_storage<10,8>::type>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
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
        arena.make<typename std::aligned_storage<10,8>::type>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        SkSTArenaAlloc<64> arena;
        arena.makeArrayDefault<char>(256);
        arena.reset();
        arena.reset();
    }

    {
        created = 0;
        destroyed = 0;
        SkSTArenaAlloc<64> arena;

        Start start;
        Node* current = nullptr;
        for (int i = 0; i < 128; i++) {
            uint64_t* temp = arena.makeArrayDefault<uint64_t>(sizeof(Node) / sizeof(Node*));
            current = new (temp)Node(current);
        }
        start.start = current;
    }

    REPORTER_ASSERT(r, created == 128);
    REPORTER_ASSERT(r, destroyed == 128);
}

// Tests for the lower-level arena operations: makeBytes, makeAtleast, resize,  and release:
DEF_TEST(LowLevelArenaAlloc, r) {
    // We include the block overhead in the initial allocation size so that it's easy to reason
    // about what's remaining in the 64 bytes available for the requested allocations.
    static constexpr int kBlockOverhead = 8;
    {
        // Fixed allocation, then dynamic allocation that can fit max size in current block.
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        // 48 bytes remain
        size_t allocated;
        void* dynamic = arena.makeAtLeastBytesAlignedTo(16, 32, alignof(int32_t), &allocated);
        REPORTER_ASSERT(r, allocated == 32);
        REPORTER_ASSERT(r,
                reinterpret_cast<uintptr_t>(dynamic) - reinterpret_cast<uintptr_t>(fixed) == 16);
    }

    {
        // Fixed allocation, then dynamic allocation that can uses the remainder of the block.
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        // 48 bytes remain
        size_t allocated;
        void* dynamic = arena.makeAtLeastBytesAlignedTo(16, 64, alignof(int32_t), &allocated);
        REPORTER_ASSERT(r, allocated == 48);
        REPORTER_ASSERT(r,
                reinterpret_cast<uintptr_t>(dynamic) - reinterpret_cast<uintptr_t>(fixed) == 16);
    }

    {
        // Fixed allocation, then dynamic allocation that cannot fit min size in current block.
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        // 48 bytes remain
        size_t allocated;
        void* dynamic = arena.makeAtLeastBytesAlignedTo(64, 128, alignof(int32_t), &allocated);
        REPORTER_ASSERT(r, allocated == 128);
        // Should be a new block for 'dynamic', so it definitely can't point to the end of 'fixed'
        REPORTER_ASSERT(r,
                reinterpret_cast<uintptr_t>(dynamic) - reinterpret_cast<uintptr_t>(fixed) != 16);
    }

    {
        // Grow an allocation up to the requested size
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(16, alignof(int32_t));
        // 48 bytes remain
        size_t newSize = 32;
        bool resizeResult = arena.resize(dynamic, 16, &newSize);
        // 16 bytes remain
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        REPORTER_ASSERT(r, resizeResult && newSize == 32);
        REPORTER_ASSERT(r,
                reinterpret_cast<uintptr_t>(fixed) - reinterpret_cast<uintptr_t>(dynamic) == 32);
    }

    {
        // Grow an allocation up to the remaining size
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(16, alignof(int32_t));
        // 48 bytes remain
        size_t newSize = 128;
        bool resizeResult = arena.resize(dynamic, 16, &newSize);
        // 0 bytes remain, so this should be on a new block
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        REPORTER_ASSERT(r, resizeResult && newSize == 64);
        REPORTER_ASSERT(r,
                reinterpret_cast<uintptr_t>(fixed) - reinterpret_cast<uintptr_t>(dynamic) != 64);
    }

    {
        // Grow unsuccessful due to subsequent allocation
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(16, alignof(int32_t));
        // 48 bytes remain
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        (void) fixed;
        // 32 bytes remain, but resize will fail since 'fixed' came after
        size_t newSize = 32;
        REPORTER_ASSERT(r, !arena.resize(dynamic, 16, &newSize));
    }

    {
        // Grow unsuccessful due to erroneous size
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(16, alignof(int32_t));
        size_t newSize = 32;
        REPORTER_ASSERT(r, !arena.resize(dynamic, 24, &newSize));
    }

    {
        // Shrink to a partial allocation
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(32, alignof(int32_t));
        // 32 bytes remain
        size_t newSize = 16;
        bool resizeResult = arena.resize(dynamic, 32, &newSize);
        // 48 bytes remain
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        REPORTER_ASSERT(r, resizeResult && newSize == 16);
        REPORTER_ASSERT(r,
                reinterpret_cast<uintptr_t>(fixed) - reinterpret_cast<uintptr_t>(dynamic) == 16);
    }

    {
        // Successful release of POD
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(32, alignof(int32_t));
        REPORTER_ASSERT(r, arena.release(dynamic, 32));
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        REPORTER_ASSERT(r, fixed == dynamic); // Reuse the same space
        size_t allocated;
        dynamic = arena.makeAtLeastBytesAlignedTo(16, 64, alignof(int32_t), &allocated);
        REPORTER_ASSERT(r, allocated > 16 && allocated < 64);
        REPORTER_ASSERT(r, arena.release(dynamic, allocated));
    }

    {
        // Successful release of a T with a trivial destructor
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        Big* big = arena.make<Big>();
        REPORTER_ASSERT(r, arena.release(big));
    }

    {
        // Unsuccessful release due to subsequent allocation
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(32, alignof(int32_t));
        void* fixed = arena.makeBytesAlignedTo(16, alignof(int32_t));
        (void) fixed;
        REPORTER_ASSERT(r, !arena.release(dynamic, 32)); // Fails due to later 16 bytes
        REPORTER_ASSERT(r, arena.release(dynamic, 48)); // Succeeds, reclaiming consecutive allocs
    }

    {
        // Unsuccessful release due to erroneous size
        SkSTArenaAlloc<64 + kBlockOverhead> arena;
        void* dynamic = arena.makeBytesAlignedTo(32, alignof(int32_t));
        REPORTER_ASSERT(r, !arena.release(dynamic, 64));
        REPORTER_ASSERT(r, !arena.release(dynamic, 16));
    }
}
