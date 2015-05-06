/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFunction.h"
#include "Test.h"

static void test_add_five(skiatest::Reporter* r, SkFunction<int(int)>& f) {
    REPORTER_ASSERT(r, f(3) == 8);
    REPORTER_ASSERT(r, f(4) == 9);
}

static void test_add_five(skiatest::Reporter* r, SkFunction<int(int)>&& f) { test_add_five(r, f); }

static int add_five(int x) { return x + 5; }

struct AddFive {
    int operator()(int x) const { return x + 5; };
};

class MoveOnlyThree : SkNoncopyable {
public:
    MoveOnlyThree() {}
    MoveOnlyThree(MoveOnlyThree&&) {}
    MoveOnlyThree& operator=(MoveOnlyThree&&) { return *this; }

    int val() { return 3; }
};

DEF_TEST(Function, r) {
    // We should be able to turn a function pointer, an explicit functor, or a
    // lambda into an SkFunction all equally well.
    test_add_five(r, &add_five);
    test_add_five(r, AddFive());
    test_add_five(r, [](int x) { return x + 5; });

    // AddFive and the lambda above are both small enough to test small-object optimization.
    // Now test a lambda that's much too large for the small-object optimization.
    int a = 1, b = 1, c = 1, d = 1, e = 1;
    test_add_five(r, [&](int x) { return x + a + b + c + d + e; });

    // Makes sure we forward arguments when calling SkFunction.
    SkFunction<int(int, MoveOnlyThree&&, int)> f([](int x, MoveOnlyThree&& three, int y) {
            return x * three.val() + y;
    });
    REPORTER_ASSERT(r, f(2, MoveOnlyThree(), 4) == 10);

    // SkFunctions can go in containers.
    SkTArray<SkFunction<int(int)>> add_fivers;
    add_fivers.push_back(&add_five);
    add_fivers.push_back(AddFive());
    add_fivers.push_back([](int x) { return x + 5; });
    add_fivers.push_back([&](int x) { return x + a + b + c + d + e; });
    for (auto& f : add_fivers) {
        test_add_five(r, f);
    }

    // SkFunctions are assignable.
    SkFunction<int(int)> empty;
    empty = [](int x) { return x + 5; };
    test_add_five(r, empty);

    // This all is silly acrobatics, but it should at least work correctly.
    SkFunction<int(int)> emptyA, emptyB(emptyA);
    emptyA = emptyB;
    emptyA = emptyA;
}
