/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFunction.h"
#include "Test.h"

static void test_add_five(skiatest::Reporter* r, SkFunction<int(int)>&& f) {
    REPORTER_ASSERT(r, f(4) == 9);
}

static int add_five(int x) { return x + 5; }

struct AddFive {
    int operator()(int x) { return x + 5; };
};

DEF_TEST(Function, r) {
    // We should be able to turn a static function, an explicit functor, or a lambda
    // all into an SkFunction equally well.
    test_add_five(r, SkFunction<int(int)>(&add_five));
    test_add_five(r, SkFunction<int(int)>(AddFive()));
    test_add_five(r, SkFunction<int(int)>([](int x) { return x + 5; }));
}
