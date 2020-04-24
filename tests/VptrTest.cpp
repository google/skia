/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMakeUnique.h"
#include "src/core/SkVptr.h"
#include "tests/Test.h"

namespace {

    struct Base {
        virtual ~Base() = default;
        virtual size_t val() const = 0;
    };

    struct SubclassA : public Base {
        SubclassA(size_t val) : fVal(val) {}

        size_t val() const override { return fVal; }

        size_t fVal;
    };

    struct SubclassB : public Base {
        SubclassB() {}

        size_t val() const override { return 42; }
    };

}

DEF_TEST(Vptr, r) {
    std::unique_ptr<Base> a = skstd::make_unique<SubclassA>(21),
                          b = skstd::make_unique<SubclassB>(),
                          c = skstd::make_unique<SubclassA>(22),
                          d = skstd::make_unique<SubclassB>();

    // These 4 objects all have unique identities.
    REPORTER_ASSERT(r, a != b);
    REPORTER_ASSERT(r, a != c);
    REPORTER_ASSERT(r, a != d);
    REPORTER_ASSERT(r, b != c);
    REPORTER_ASSERT(r, b != d);
    REPORTER_ASSERT(r, c != d);

    // Only b and d have the same val().
    REPORTER_ASSERT(r, a->val() != b->val());
    REPORTER_ASSERT(r, a->val() != c->val());
    REPORTER_ASSERT(r, a->val() != d->val());
    REPORTER_ASSERT(r, b->val() != c->val());
    REPORTER_ASSERT(r, b->val() == d->val());
    REPORTER_ASSERT(r, c->val() != d->val());

    // SkVptr() returns the same value for objects of the same concrete type.
    REPORTER_ASSERT(r, SkVptr(*a) == SkVptr(*c));
    REPORTER_ASSERT(r, SkVptr(*b) == SkVptr(*d));
    REPORTER_ASSERT(r, SkVptr(*a) != SkVptr(*b));
}
