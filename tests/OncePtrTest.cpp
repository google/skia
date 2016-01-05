/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkOncePtr.h"
#include "SkTaskGroup.h"

DEF_TEST(OncePtr, r) {
    SkOncePtr<int> once;

    static SkAtomic<int> calls(0);
    auto create = [&] {
        calls.fetch_add(1);
        return new int(5);
    };

    SkTaskGroup().batch(sk_num_cores()*4, [&](size_t) {
        int* n = once.get(create);
        REPORTER_ASSERT(r, *n == 5);
    });
    REPORTER_ASSERT(r, calls.load() == 1);
}

/* TODO(mtklein): next CL

SK_DECLARE_STATIC_ONCE(once_noptr);
DEF_TEST(OnceNoPtr, r) {
    static SkAtomic<int> calls(0);

    SkAtomic<int> force_a_race(sk_num_cores());
    SkTaskGroup().batch(sk_num_cores()*4, [&](size_t) {
        force_a_race.fetch_add(-1);
        while (force_a_race.load() > 0);

        SkOnce(&once_noptr, [&] { calls.fetch_add(1); });
    });
    REPORTER_ASSERT(r, calls.load() == 1);
}
*/
