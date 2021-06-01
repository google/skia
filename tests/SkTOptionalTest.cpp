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
    REPORTER_ASSERT(r, !o.has_value());
}

DEF_TEST(SkTOptionalValue, r) {
    skstd::optional<const char*> o("test");
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o.has_value());
    REPORTER_ASSERT(r, !strcmp(*o, "test"));
    REPORTER_ASSERT(r, !strcmp(o.value(), "test"));
    o.reset();
    REPORTER_ASSERT(r, !o);
    REPORTER_ASSERT(r, !o.has_value());
}

class SkTOptionalTestPayload {
public:
    enum State {
        kConstructed,
        kCopyConstructed,
        kCopyAssigned,
        kMoveConstructed,
        kMoveAssigned,
        kMovedFrom
    };

    SkTOptionalTestPayload(int payload)
        : fState(kConstructed)
        , fPayload(payload) {}

    SkTOptionalTestPayload(const SkTOptionalTestPayload& other)
        : fState(kCopyConstructed)
        , fPayload(other.fPayload) {}

    SkTOptionalTestPayload(SkTOptionalTestPayload&& other)
        : fState(kMoveConstructed)
        , fPayload(other.fPayload) {
            other.fState = kMovedFrom;
        }

    SkTOptionalTestPayload& operator=(const SkTOptionalTestPayload& other) {
        fState = kCopyAssigned;
        fPayload = other.fPayload;
        return *this;
    }

    SkTOptionalTestPayload& operator=(SkTOptionalTestPayload&& other) {
        fState = kMoveAssigned;
        fPayload = other.fPayload;
        other.fState = kMovedFrom;
        return *this;
    }
    State fState;
    int fPayload;
};

DEF_TEST(SkTOptionalConstruction, r) {
    skstd::optional<SkTOptionalTestPayload> o(1);
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kConstructed);
    REPORTER_ASSERT(r, o->fPayload == 1);

    skstd::optional<SkTOptionalTestPayload> copy(o);
    REPORTER_ASSERT(r, copy);
    REPORTER_ASSERT(r, copy->fState == SkTOptionalTestPayload::kCopyConstructed);
    REPORTER_ASSERT(r, copy->fPayload == 1);
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kConstructed);

    skstd::optional<SkTOptionalTestPayload> move(std::move(o));
    REPORTER_ASSERT(r, move);
    REPORTER_ASSERT(r, move->fState == SkTOptionalTestPayload::kMoveConstructed);
    REPORTER_ASSERT(r, move->fPayload == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kMovedFrom);
}

DEF_TEST(SkTOptionalMoveAssignment, r) {
    skstd::optional<SkTOptionalTestPayload> o;
    REPORTER_ASSERT(r, !o);

    // assign to an empty optional from an empty optional
    o = skstd::optional<SkTOptionalTestPayload>();
    REPORTER_ASSERT(r, !o);

    // assign to an empty optional from a full optional
    skstd::optional<SkTOptionalTestPayload> full(1);
    o = std::move(full);
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kMoveConstructed);
    REPORTER_ASSERT(r, o->fPayload == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    REPORTER_ASSERT(r, full->fState == SkTOptionalTestPayload::kMovedFrom);

    // assign to a full optional from a full optional
    full = skstd::optional<SkTOptionalTestPayload>(2);
    o = std::move(full);
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kMoveAssigned);
    REPORTER_ASSERT(r, o->fPayload == 2);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    REPORTER_ASSERT(r, full->fState == SkTOptionalTestPayload::kMovedFrom);

    // assign to a full optional from an empty optional
    o = skstd::optional<SkTOptionalTestPayload>();
    REPORTER_ASSERT(r, !o);
}

DEF_TEST(SkTOptionalCopyAssignment, r) {
    skstd::optional<SkTOptionalTestPayload> o;
    REPORTER_ASSERT(r, !o);

    skstd::optional<SkTOptionalTestPayload> empty;
    skstd::optional<SkTOptionalTestPayload> full(1);

    // assign to an empty optional from an empty optional
    o = empty;
    REPORTER_ASSERT(r, !o);

    // assign to an empty optional from a full optional
    o = full;
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kCopyConstructed);
    REPORTER_ASSERT(r, o->fPayload == 1);

    // assign to a full optional from a full optional
    o = full;
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o->fState == SkTOptionalTestPayload::kCopyAssigned);
    REPORTER_ASSERT(r, o->fPayload == 1);

    // assign to a full optional from an empty optional
    o = empty;
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
    empty = emptyRef;
    REPORTER_ASSERT(r, !empty);
    empty = std::move(emptyRef);
    REPORTER_ASSERT(r, !empty);

    skstd::optional<SkString> full("full");
    skstd::optional<SkString>& fullRef = full;
    full = fullRef;
    REPORTER_ASSERT(r, full);
    REPORTER_ASSERT(r, *full == SkString("full"));
    full = std::move(fullRef);
    REPORTER_ASSERT(r, full);
    REPORTER_ASSERT(r, *full == SkString("full"));
}
