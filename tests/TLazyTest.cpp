/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTLazy.h"
#include "tests/Test.h"

DEF_TEST(TLazy_copy, r) {
    SkTLazy<int> lazy;

    REPORTER_ASSERT(r, !lazy.isValid());
    REPORTER_ASSERT(r, lazy.getMaybeNull() == nullptr);

    {
        SkTLazy<int> lazy_copy(lazy);
        REPORTER_ASSERT(r, !lazy_copy.isValid());
        REPORTER_ASSERT(r, lazy_copy.getMaybeNull() == nullptr);
    }

    lazy.init(42);

    REPORTER_ASSERT(r, lazy.isValid());
    REPORTER_ASSERT(r, 42 == *lazy.get());

    {
        SkTLazy<int> lazy_copy(lazy);
        REPORTER_ASSERT(r, lazy_copy.isValid());
        REPORTER_ASSERT(r, 42 == *lazy_copy.get());
        REPORTER_ASSERT(r, lazy.get() != lazy_copy.get());
    }
}

DEF_TEST(TCopyOnFirstWrite_copy, r) {
    const int v = 42;

    SkTCopyOnFirstWrite<int> cow(v);

    REPORTER_ASSERT(r, 42 == *cow);
    REPORTER_ASSERT(r, &v ==  cow.get());

    {
        SkTCopyOnFirstWrite<int> cow_copy(cow);
        REPORTER_ASSERT(r, 42 == *cow_copy);
        REPORTER_ASSERT(r, &v ==  cow_copy.get());
        REPORTER_ASSERT(r, cow.get() ==  cow_copy.get());

        *cow_copy.writable() = 43;
        REPORTER_ASSERT(r, 42 == *cow);
        REPORTER_ASSERT(r, &v ==  cow.get());
        REPORTER_ASSERT(r, 43 == *cow_copy);
        REPORTER_ASSERT(r, &v !=  cow_copy.get());
        REPORTER_ASSERT(r, cow.get() !=  cow_copy.get());
    }

    *cow.writable() = 43;

    REPORTER_ASSERT(r, 43 == *cow);
    REPORTER_ASSERT(r, &v !=  cow.get());

    {
        SkTCopyOnFirstWrite<int> cow_copy(cow);
        REPORTER_ASSERT(r, 43 == *cow_copy);
        REPORTER_ASSERT(r, &v !=  cow_copy.get());
        REPORTER_ASSERT(r, cow.get() !=  cow_copy.get());

        *cow_copy.writable() = 44;

        REPORTER_ASSERT(r, 43 == *cow);
        REPORTER_ASSERT(r, &v !=  cow.get());
        REPORTER_ASSERT(r, 44 == *cow_copy);
        REPORTER_ASSERT(r, &v !=  cow_copy.get());
        REPORTER_ASSERT(r, cow.get() !=  cow_copy.get());
    }
}
