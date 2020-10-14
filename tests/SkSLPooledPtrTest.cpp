/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPooledPtr.h"

#include "tests/Test.h"

using skiatest::Reporter;
using Value = float;

struct PODStruct {
    int x;
};

struct UncopyableStruct {
    UncopyableStruct() = default;
    ~UncopyableStruct() = default;
    UncopyableStruct(const UncopyableStruct&) = delete;
};

struct CountingStruct {
    CountingStruct() { ++sCount; }
    ~CountingStruct() { --sCount; }
    CountingStruct(const CountingStruct&) = delete;

    static int sCount;
};

int CountingStruct::sCount = 0;

static void construct_plain(Reporter* r) {
    SkSL::PooledPtr<Value> v;
    SkSL::PooledPtr<PODStruct> p;
    SkSL::PooledPtr<UncopyableStruct> u;
    SkSL::PooledPtr<CountingStruct> c;

    REPORTER_ASSERT(r, v == nullptr);
    REPORTER_ASSERT(r, p == nullptr);
    REPORTER_ASSERT(r, u == nullptr);
    REPORTER_ASSERT(r, c == nullptr);
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
}

static void construct_from_null(Reporter* r) {
    SkSL::PooledPtr<Value> v(nullptr);
    SkSL::PooledPtr<PODStruct> p(nullptr);
    SkSL::PooledPtr<UncopyableStruct> u(nullptr);
    SkSL::PooledPtr<CountingStruct> c(nullptr);

    REPORTER_ASSERT(r, v == nullptr);
    REPORTER_ASSERT(r, p == nullptr);
    REPORTER_ASSERT(r, u == nullptr);
    REPORTER_ASSERT(r, c == nullptr);
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
}

static void construct_from_pointer(Reporter* r) {
    SkSL::PooledPtr<Value> v(new Value);
    SkSL::PooledPtr<PODStruct> p(new PODStruct);
    SkSL::PooledPtr<UncopyableStruct> u(new UncopyableStruct);
    SkSL::PooledPtr<CountingStruct> c(new CountingStruct);

    REPORTER_ASSERT(r, v != nullptr);
    REPORTER_ASSERT(r, p != nullptr);
    REPORTER_ASSERT(r, u != nullptr);
    REPORTER_ASSERT(r, c != nullptr);
    REPORTER_ASSERT(r, CountingStruct::sCount == 1);
}

static void move_construction(Reporter* r) {
    SkSL::PooledPtr<Value> v(new Value);
    SkSL::PooledPtr<Value> v2(std::move(v));
    REPORTER_ASSERT(r, v == nullptr);
    REPORTER_ASSERT(r, v2 != nullptr);

    SkSL::PooledPtr<PODStruct> p(new PODStruct);
    SkSL::PooledPtr<PODStruct> p2(std::move(p));
    REPORTER_ASSERT(r, p == nullptr);
    REPORTER_ASSERT(r, p2 != nullptr);

    SkSL::PooledPtr<UncopyableStruct> u(new UncopyableStruct);
    SkSL::PooledPtr<UncopyableStruct> u2(std::move(u));
    REPORTER_ASSERT(r, u == nullptr);
    REPORTER_ASSERT(r, u2 != nullptr);

    SkSL::PooledPtr<CountingStruct> c(new CountingStruct);
    SkSL::PooledPtr<CountingStruct> c2(std::move(c));
    REPORTER_ASSERT(r, c == nullptr);
    REPORTER_ASSERT(r, c2 != nullptr);
}

static void move_assignment(Reporter* r) {
    SkSL::PooledPtr<Value> v(new Value);
    SkSL::PooledPtr<Value> v2;
    v2 = std::move(v);
    REPORTER_ASSERT(r, v == nullptr);
    REPORTER_ASSERT(r, v2 != nullptr);

    SkSL::PooledPtr<PODStruct> p(new PODStruct);
    SkSL::PooledPtr<PODStruct> p2;
    p2 = std::move(p);
    REPORTER_ASSERT(r, p == nullptr);
    REPORTER_ASSERT(r, p2 != nullptr);

    SkSL::PooledPtr<UncopyableStruct> u(new UncopyableStruct);
    SkSL::PooledPtr<UncopyableStruct> u2;
    u2 = std::move(u);
    REPORTER_ASSERT(r, u == nullptr);
    REPORTER_ASSERT(r, u2 != nullptr);

    SkSL::PooledPtr<CountingStruct> c(new CountingStruct);
    SkSL::PooledPtr<CountingStruct> c2;
    c2 = std::move(c);
    REPORTER_ASSERT(r, c == nullptr);
    REPORTER_ASSERT(r, c2 != nullptr);
}

static void deletes_when_leaving_scope(Reporter* r) {
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
    {
        SkSL::PooledPtr<CountingStruct> c(new CountingStruct);
        REPORTER_ASSERT(r, CountingStruct::sCount == 1);
    }
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
}

static void deletes_once_after_move_construction(Reporter* r) {
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
    {
        SkSL::PooledPtr<CountingStruct> c(new CountingStruct);
        REPORTER_ASSERT(r, CountingStruct::sCount == 1);

        SkSL::PooledPtr<CountingStruct> c2(std::move(c));
        REPORTER_ASSERT(r, CountingStruct::sCount == 1);
    }
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
}

static void deletes_once_after_move_assignment(Reporter* r) {
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
    {
        SkSL::PooledPtr<CountingStruct> c(new CountingStruct);
        REPORTER_ASSERT(r, CountingStruct::sCount == 1);

        SkSL::PooledPtr<CountingStruct> c2;
        c2 = std::move(c);
        REPORTER_ASSERT(r, CountingStruct::sCount == 1);
    }
    REPORTER_ASSERT(r, CountingStruct::sCount == 0);
}

static void comparison_operators(Reporter* r) {
    // Create two pointers in a known order. (`a` is always less than `b`.)
    Value* aPtr = new Value{};
    Value* bPtr = new Value{};
    if (aPtr > bPtr) {
        std::swap(aPtr, bPtr);
    }

    SkSL::PooledPtr<Value> a(aPtr);
    SkSL::PooledPtr<Value> b(bPtr);
    SkSL::PooledPtr<Value> c;

    REPORTER_ASSERT(r, a != b);
    REPORTER_ASSERT(r, !(a == b));
    REPORTER_ASSERT(r, a < b);
    REPORTER_ASSERT(r, a <= b);
    REPORTER_ASSERT(r, b > a);
    REPORTER_ASSERT(r, b >= a);

    REPORTER_ASSERT(r, a != nullptr);
    REPORTER_ASSERT(r, !(a == nullptr));
    REPORTER_ASSERT(r, a == a);
    REPORTER_ASSERT(r, !(a != a));

    REPORTER_ASSERT(r, c == nullptr);
    REPORTER_ASSERT(r, !(c != nullptr));
    REPORTER_ASSERT(r, c == c);
    REPORTER_ASSERT(r, !(c != c));
}

DEF_TEST(SkSLPooledPtr, r) {
    construct_plain(r);
    construct_from_null(r);
    construct_from_pointer(r);
    move_construction(r);
    move_assignment(r);
    deletes_when_leaving_scope(r);
    deletes_once_after_move_construction(r);
    deletes_once_after_move_assignment(r);
    comparison_operators(r);
}

