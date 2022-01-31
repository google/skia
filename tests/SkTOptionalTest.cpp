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

DEF_TEST(SkTOptionalNulloptCtor, r) {
    skstd::optional<int> o(skstd::nullopt);
    REPORTER_ASSERT(r, !o);
    REPORTER_ASSERT(r, !o.has_value());
}

DEF_TEST(SkTOptionalValueOr, r) {
    {
        skstd::optional<const char*> o;
        REPORTER_ASSERT(r, !strcmp(o.value_or("Hello"), "Hello"));
    }
    {
        skstd::optional<const char*> o("Bye");
        REPORTER_ASSERT(r, !strcmp(o.value_or("Hello"), "Bye"));
    }
    {
        skstd::optional<std::unique_ptr<int>> o;
        std::unique_ptr<int> a = std::move(o).value_or(std::make_unique<int>(5));
        REPORTER_ASSERT(r, *a == 5);
    }
    {
        skstd::optional<std::unique_ptr<int>> o(std::make_unique<int>(3));
        std::unique_ptr<int> a = std::move(o).value_or(std::make_unique<int>(5));
        REPORTER_ASSERT(r, *a == 3);
    }
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

DEF_TEST(SkTOptionalNulloptAssignment, r) {
    skstd::optional<const char*> o("test");
    REPORTER_ASSERT(r, o);
    REPORTER_ASSERT(r, o.has_value());
    o = skstd::nullopt;
    REPORTER_ASSERT(r, !o);
    REPORTER_ASSERT(r, !o.has_value());
}

DEF_TEST(SkTOptionalNulloptReturn, r) {
    auto fn = []() -> skstd::optional<float> { return skstd::nullopt; };

    skstd::optional<float> o = fn();
    REPORTER_ASSERT(r, !o);
    REPORTER_ASSERT(r, !o.has_value());
}

DEF_TEST(SkTOptionalComparisons, r) {
    int v[] = { 1, 2, 3, 4, 5 };
    skstd::optional<int> o[] = {1, 2, skstd::nullopt, 4, 5};
    skstd::optional<int> five = 5;
    skstd::optional<int> six = 6;

    for (int index = 0; index < (int)SK_ARRAY_COUNT(v); ++index) {
        REPORTER_ASSERT(r, v[index] < six);
        REPORTER_ASSERT(r, o[index] < six);
        REPORTER_ASSERT(r, six > v[index]);
        REPORTER_ASSERT(r, six > o[index]);

        REPORTER_ASSERT(r, v[index] < 6);
        REPORTER_ASSERT(r, o[index] < 6);
        REPORTER_ASSERT(r, 6 > v[index]);
        REPORTER_ASSERT(r, 6 > o[index]);

        REPORTER_ASSERT(r, !(six < v[index]));
        REPORTER_ASSERT(r, !(six < o[index]));
        REPORTER_ASSERT(r, !(v[index] > six));
        REPORTER_ASSERT(r, !(o[index] > six));

        REPORTER_ASSERT(r, !(6 < v[index]));
        REPORTER_ASSERT(r, !(6 < o[index]));
        REPORTER_ASSERT(r, !(v[index] > 6));
        REPORTER_ASSERT(r, !(o[index] > 6));

        REPORTER_ASSERT(r, v[index] <= five);
        REPORTER_ASSERT(r, o[index] <= five);
        REPORTER_ASSERT(r, five >= v[index]);
        REPORTER_ASSERT(r, five >= o[index]);

        REPORTER_ASSERT(r, v[index] <= 5);
        REPORTER_ASSERT(r, o[index] <= 5);
        REPORTER_ASSERT(r, 5 >= v[index]);
        REPORTER_ASSERT(r, 5 >= o[index]);

        REPORTER_ASSERT(r, skstd::nullopt <= o[index]);
        REPORTER_ASSERT(r, !(skstd::nullopt > o[index]));
        REPORTER_ASSERT(r, o[index] >= skstd::nullopt);
        REPORTER_ASSERT(r, !(o[index] < skstd::nullopt));

        if (o[index].has_value()) {
            REPORTER_ASSERT(r, o[index] != skstd::nullopt);
            REPORTER_ASSERT(r, skstd::nullopt != o[index]);

            REPORTER_ASSERT(r, o[index] == o[index]);
            REPORTER_ASSERT(r, o[index] != six);
            REPORTER_ASSERT(r, o[index] == v[index]);
            REPORTER_ASSERT(r, v[index] == o[index]);
            REPORTER_ASSERT(r, o[index] > 0);
            REPORTER_ASSERT(r, o[index] >= 1);
            REPORTER_ASSERT(r, o[index] <= 5);
            REPORTER_ASSERT(r, o[index] < 6);
            REPORTER_ASSERT(r, 0 < o[index]);
            REPORTER_ASSERT(r, 1 <= o[index]);
            REPORTER_ASSERT(r, 5 >= o[index]);
            REPORTER_ASSERT(r, 6 > o[index]);
        } else {
            REPORTER_ASSERT(r, o[index] == skstd::nullopt);
            REPORTER_ASSERT(r, skstd::nullopt == o[index]);

            REPORTER_ASSERT(r, o[index] == o[index]);
            REPORTER_ASSERT(r, o[index] != five);
            REPORTER_ASSERT(r, o[index] != v[index]);
            REPORTER_ASSERT(r, v[index] != o[index]);
            REPORTER_ASSERT(r, o[index] < 0);
            REPORTER_ASSERT(r, o[index] <= 0);
            REPORTER_ASSERT(r, 0 > o[index]);
            REPORTER_ASSERT(r, 0 >= o[index]);
            REPORTER_ASSERT(r, !(o[index] > 0));
            REPORTER_ASSERT(r, !(o[index] >= 0));
            REPORTER_ASSERT(r, !(0 < o[index]));
            REPORTER_ASSERT(r, !(0 <= o[index]));
        }
    }
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

DEF_TEST(SkTOptionalEmplace, r) {
    skstd::optional<std::vector<int>> o;
    REPORTER_ASSERT(r, !o);

    // Emplace with the no-argument constructor
    o.emplace();
    REPORTER_ASSERT(r, o.has_value());
    REPORTER_ASSERT(r, o->empty());

    // Emplace with the initializer-list constructor
    o.emplace({1, 2, 3});
    REPORTER_ASSERT(r, o.has_value());
    REPORTER_ASSERT(r, (*o == std::vector<int>{1, 2, 3}));

    // Emplace with a normal constructor
    std::vector<int> otherVec = {4, 5, 6};
    o.emplace(otherVec.begin(), otherVec.end());
    REPORTER_ASSERT(r, o.has_value());
    REPORTER_ASSERT(r, (*o == std::vector<int>{4, 5, 6}));
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

DEF_TEST(SkTOptionalDestroyed, r) {
    bool destroyed = false;
    struct NotifyWhenDestroyed {
        NotifyWhenDestroyed(bool* e) : fE(e) {}
        ~NotifyWhenDestroyed() { *fE = true; }
        bool* fE;
    };
    {
        skstd::optional<NotifyWhenDestroyed> notify(&destroyed);
    }
    REPORTER_ASSERT(r, destroyed);
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
