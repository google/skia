/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"
#include "SkTArray.h"
#include "Test.h"

// Tests the SkTArray<T> class template.

template <bool MEM_COPY>
static void TestTSet_basic(skiatest::Reporter* reporter) {
    SkTArray<int, MEM_COPY> a;

    // Starts empty.
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.count() == 0);

    // { }, add a default constructed element
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);

    // { 0 }, removeShuffle the only element.
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.count() == 0);

    // { }, add a default, add a 1, remove first
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, a.push_back() = 1);
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);

    // { 1 }, replace with new array
    int b[5] = { 0, 1, 2, 3, 4 };
    a.reset(b, SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a[2] == 2);
    REPORTER_ASSERT(reporter, a[4] == 4);

    // { 0, 1, 2, 3, 4 }, removeShuffle the last
    a.removeShuffle(4);
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b) - 1);
    REPORTER_ASSERT(reporter, a[3] == 3);

    // { 0, 1, 2, 3 }, remove a middle, note shuffle
    a.removeShuffle(1);
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b) - 2);
    REPORTER_ASSERT(reporter, a[0] == 0);
    REPORTER_ASSERT(reporter, a[1] == 3);
    REPORTER_ASSERT(reporter, a[2] == 2);

    // {0, 3, 2 }
}

template <typename T> static void test_swap(skiatest::Reporter* reporter,
                                            SkTArray<T>* (&arrays)[4],
                                            int (&sizes)[7])
{
    for (auto a : arrays) {
    for (auto b : arrays) {
        if (a == b) {
            continue;
        }

        for (auto sizeA : sizes) {
        for (auto sizeB : sizes) {
            a->reset();
            b->reset();

            int curr = 0;
            for (int i = 0; i < sizeA; i++) { a->push_back(curr++); }
            for (int i = 0; i < sizeB; i++) { b->push_back(curr++); }

            a->swap(b);
            REPORTER_ASSERT(reporter, b->count() == sizeA);
            REPORTER_ASSERT(reporter, a->count() == sizeB);

            curr = 0;
            for (auto&& x : *b) { REPORTER_ASSERT(reporter, x == curr++); }
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }

            a->swap(a);
            curr = sizeA;
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }
        }}
    }}
}

static void test_swap(skiatest::Reporter* reporter) {
    int sizes[] = {0, 1, 5, 10, 15, 20, 25};

    SkTArray<int> arr;
    SkSTArray< 5, int> arr5;
    SkSTArray<10, int> arr10;
    SkSTArray<20, int> arr20;
    SkTArray<int>* arrays[] = { &arr, &arr5, &arr10, &arr20 };
    test_swap(reporter, arrays, sizes);

    struct MoveOnlyInt {
        MoveOnlyInt(int i) : fInt(i) {}
        MoveOnlyInt(MoveOnlyInt&& that) : fInt(that.fInt) {}
        bool operator==(int i) { return fInt == i; }
        int fInt;
    };

    SkTArray<MoveOnlyInt> moi;
    SkSTArray< 5, MoveOnlyInt> moi5;
    SkSTArray<10, MoveOnlyInt> moi10;
    SkSTArray<20, MoveOnlyInt> moi20;
    SkTArray<MoveOnlyInt>* arraysMoi[] = { &moi, &moi5, &moi10, &moi20 };
    test_swap(reporter, arraysMoi, sizes);
}

template <typename T, bool MEM_MOVE>
void test_copy_ctor(skiatest::Reporter* reporter, SkTArray<T, MEM_MOVE>&& array) {
    SkASSERT(array.empty());
    for (int i = 0; i < 5; ++i) {
        array.emplace_back(new SkRefCnt);
        REPORTER_ASSERT(reporter, array.back()->unique());
    }

    {
        SkTArray<T, MEM_MOVE> copy(array);
        for (const auto& ref : array)
            REPORTER_ASSERT(reporter, !ref->unique());
        for (const auto& ref : copy)
            REPORTER_ASSERT(reporter, !ref->unique());
    }

    for (const auto& ref : array)
        REPORTER_ASSERT(reporter, ref->unique());
}

static void test_move(skiatest::Reporter* reporter) {
#define TEST_MOVE do {                                 \
    SRC_T src;                                         \
    src.emplace_back(sk_make_sp<SkRefCnt>());          \
    {                                                  \
        /* copy ctor */                                \
        DST_T copy(src);                               \
        REPORTER_ASSERT(reporter, !copy[0]->unique()); \
    }                                                  \
    {                                                  \
        /* move ctor */                                \
        DST_T move(std::move(src));                    \
        REPORTER_ASSERT(reporter, move[0]->unique());  \
    }                                                  \
    REPORTER_ASSERT(reporter, src.empty());            \
    src.emplace_back(sk_make_sp<SkRefCnt>());          \
    {                                                  \
        /* copy assignment */                          \
        DST_T copy;                                    \
        copy = src;                                    \
        REPORTER_ASSERT(reporter, !copy[0]->unique()); \
    }                                                  \
    {                                                  \
        /* move assignment */                          \
        DST_T move;                                    \
        move = std::move(src);                         \
        REPORTER_ASSERT(reporter, move[0]->unique());  \
    }                                                  \
    REPORTER_ASSERT(reporter, src.empty());            \
} while (false)

    {
        using SRC_T = SkTArray<sk_sp<SkRefCnt>, false>;
        using DST_T = SkTArray<sk_sp<SkRefCnt>, false>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkTArray<sk_sp<SkRefCnt>, true>;
        using DST_T = SkTArray<sk_sp<SkRefCnt>, true>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkSTArray<1, sk_sp<SkRefCnt>, false>;
        using DST_T = SkSTArray<1, sk_sp<SkRefCnt>, false>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkSTArray<1, sk_sp<SkRefCnt>, true>;
        using DST_T = SkSTArray<1, sk_sp<SkRefCnt>, true>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkTArray<sk_sp<SkRefCnt>, false>;
        using DST_T = SkSTArray<1, sk_sp<SkRefCnt>, false>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkTArray<sk_sp<SkRefCnt>, true>;
        using DST_T = SkSTArray<1, sk_sp<SkRefCnt>, true>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkSTArray<1, sk_sp<SkRefCnt>, false>;
        using DST_T = SkTArray<sk_sp<SkRefCnt>, false>;
        TEST_MOVE;
    }

    {
        using SRC_T = SkSTArray<1, sk_sp<SkRefCnt>, true>;
        using DST_T = SkTArray<sk_sp<SkRefCnt>, true>;
        TEST_MOVE;
    }
#undef TEST_MOVE
}

DEF_TEST(TArray, reporter) {
    TestTSet_basic<true>(reporter);
    TestTSet_basic<false>(reporter);
    test_swap(reporter);

    test_copy_ctor(reporter, SkTArray<sk_sp<SkRefCnt>, false>());
    test_copy_ctor(reporter, SkTArray<sk_sp<SkRefCnt>,  true>());
    test_copy_ctor(reporter, SkSTArray< 1, sk_sp<SkRefCnt>, false>());
    test_copy_ctor(reporter, SkSTArray< 1, sk_sp<SkRefCnt>,  true>());
    test_copy_ctor(reporter, SkSTArray<10, sk_sp<SkRefCnt>, false>());
    test_copy_ctor(reporter, SkSTArray<10, sk_sp<SkRefCnt>,  true>());

    test_move(reporter);
}
