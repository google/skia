/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTArray.h"
#include "include/private/SkTOptional.h"

#include "tests/Test.h"

DEF_TEST(SkTOptionalEmpty, r) {
    skstd::optional<int> o;
    REPORTER_ASSERT(r, !o);
}

DEF_TEST(SkTOptionalValue, r) {
    skstd::optional<const char*> o("test");
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, !strcmp(*o, "test"));
}

DEF_TEST(SkTOptionalAssignment, r) {
    skstd::optional<SkTArray<int>> o;
    REPORTER_ASSERT(r, !o);

    o = skstd::optional<SkTArray<int>>(50);
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->capacity() == 50);

    o = skstd::optional<SkTArray<int>>(SkTArray<int>(100));
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->capacity() == 100);

    o = skstd::optional<SkTArray<int>>();
    REPORTER_ASSERT(r, !o);
}

DEF_TEST(SkTOptionalNoDefaultConstructor, r) {
    class NoDefaultConstructor {
    public:
        NoDefaultConstructor(int value)
            : fValue(value) {}

        int fValue;
    };

    skstd::optional<NoDefaultConstructor> o1;
    REPORTER_ASSERT(r, !o1);
    skstd::optional<NoDefaultConstructor> o2(5);
    REPORTER_ASSERT(r, o2);
    REPORTER_ASSERT(r, o2->fValue == 5);
    o1 = std::move(o2);
    REPORTER_ASSERT(r, o1);
    REPORTER_ASSERT(r, o1->fValue == 5);
}

DEF_TEST(SkTOptionalSelfAssignment, r) {
    skstd::optional<SkString> empty;
    skstd::optional<SkString>& emptyRef = empty;
    empty = std::move(emptyRef);
    REPORTER_ASSERT(r, !empty);

    skstd::optional<SkString> full("full");
    skstd::optional<SkString>& fullRef = full;
    full = std::move(fullRef);
    REPORTER_ASSERT(r, full);
    REPORTER_ASSERT(r, *full == SkString("full"));
}
