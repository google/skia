/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkOncePtr.h"
#include "SkTaskGroup.h"

SK_DECLARE_STATIC_ONCE_PTR(int, once);

DEF_TEST(OncePtr, r) {
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
