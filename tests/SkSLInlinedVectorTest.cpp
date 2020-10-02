/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <numeric>

#include "src/sksl/containers/InlinedVector.h"
#include "tests/Test.h"

// Tests the SkSL::InlinedVector<N, T> class template.
template <int N, typename T>
static void TestInlineVector_basic(skiatest::Reporter* reporter) {
    SkSL::InlinedVector<N, T> a;

    // Starts empty.
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.size() == 0);
    REPORTER_ASSERT(reporter, a.capacity() == N);

    // { }, add a default constructed element
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a.capacity() == N);

    // { 0 }, pop the only element.
    a.pop_back();
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.size() == 0);
    REPORTER_ASSERT(reporter, a.capacity() == N);

    // { }, add a 1, add a 2, pop the back
    a.push_back() = 1;
    REPORTER_ASSERT(reporter, a.push_back() = 2);
    a.pop_back();
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);
    REPORTER_ASSERT(reporter, a.front() == 1);
    REPORTER_ASSERT(reporter, a.back() == 1);
    REPORTER_ASSERT(reporter, (N >= 2 && a.capacity() == N) || (a.capacity() >= N));

    // { 1 }, replace with new array
    T b[5] = { 0, 1, 2, 3, 4 };
    a = SkSL::InlinedVector<N, T>(b, b + SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a.size() == SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a[2] == 2);
    REPORTER_ASSERT(reporter, a[4] == 4);
    REPORTER_ASSERT(reporter, a.front() == 0);
    REPORTER_ASSERT(reporter, a.back() == 4);
    REPORTER_ASSERT(reporter, (N >= 5 && a.capacity() == N) || (a.capacity() >= N));

    // { 0, 1, 2, 3, 4 }, resize away the last
    a.resize(4);
    REPORTER_ASSERT(reporter, a.size() == 4);
    REPORTER_ASSERT(reporter, a[3] == 3);
    REPORTER_ASSERT(reporter, (N >= 5 && a.capacity() == N) || (a.capacity() >= N));

    // { 0, 1, 2, 3 }, pop the last
    a.pop_back();
    REPORTER_ASSERT(reporter, a.size() == SK_ARRAY_COUNT(b) - 2);
    REPORTER_ASSERT(reporter, a[0] == 0);
    REPORTER_ASSERT(reporter, a[1] == 1);
    REPORTER_ASSERT(reporter, a[2] == 2);
    REPORTER_ASSERT(reporter, (N >= 4 && a.capacity() == N) || (a.capacity() >= N));

    // { 0, 1, 2 }, use resize to grow to 6 elements; the previous values should now be zeroed out.
    a.resize(6);
    REPORTER_ASSERT(reporter, a.size() == 6);
    REPORTER_ASSERT(reporter, a[0] == 0);
    REPORTER_ASSERT(reporter, a[1] == 1);
    REPORTER_ASSERT(reporter, a[2] == 2);
    REPORTER_ASSERT(reporter, a[3] == 0);
    REPORTER_ASSERT(reporter, a[4] == 0);
    REPORTER_ASSERT(reporter, a[5] == 0);
    REPORTER_ASSERT(reporter, (N >= 6 && a.capacity() == N) || (a.capacity() >= N));

    // { 0, 1, 2, 0, 0, 0 }, resize the vector to 100 elements.
    a.resize(100);
    REPORTER_ASSERT(reporter, a.size() == 100);
    REPORTER_ASSERT(reporter, std::accumulate(a.begin(), a.end(), 0) == 3); // 0+1+2+0+0+0+0...+0
    REPORTER_ASSERT(reporter, (N >= 100 && a.capacity() == N) || (a.capacity() >= N));
}

template <typename T>
static void TestInlineVector_basic(skiatest::Reporter* reporter) {
    // Sizes were chosen to cause the array to switch out of inline mode at various test stages.
    TestInlineVector_basic<1, T>(reporter);
    TestInlineVector_basic<2, T>(reporter);
    TestInlineVector_basic<4, T>(reporter);
    TestInlineVector_basic<5, T>(reporter);
    TestInlineVector_basic<6, T>(reporter);
    TestInlineVector_basic<8, T>(reporter);
    TestInlineVector_basic<64, T>(reporter);
    TestInlineVector_basic<128, T>(reporter);
}

template <typename T>
static void test_swap(skiatest::Reporter* reporter) {
    static constexpr int sizes[] = {0, 1, 5, 10, 15, 20, 25};

    for (auto sizeA : sizes) {
        for (auto sizeB : sizes) {
            T a;
            T b;

            int curr = 0;
            for (int i = 0; i < sizeA; i++) { a.push_back(curr++); }
            for (int i = 0; i < sizeB; i++) { b.push_back(curr++); }

            a.swap(b);
            REPORTER_ASSERT(reporter, b.size() == sizeA);
            REPORTER_ASSERT(reporter, a.size() == sizeB);

            curr = 0;
            for (auto&& x : b) { REPORTER_ASSERT(reporter, x == curr++); }
            for (auto&& x : a) { REPORTER_ASSERT(reporter, x == curr++); }

            a.swap(a);
            curr = sizeA;
            for (auto&& x : a) { REPORTER_ASSERT(reporter, x == curr++); }
        }
    }
}

static void test_swap(skiatest::Reporter* reporter) {
    test_swap<SkSL::InlinedVector< 5, int>>(reporter);
    test_swap<SkSL::InlinedVector<10, int>>(reporter);
    test_swap<SkSL::InlinedVector<20, int>>(reporter);

    struct MoveOnlyInt {
        MoveOnlyInt(int i) : fInt(i) {}
        MoveOnlyInt(MoveOnlyInt&& that) : fInt(that.fInt) {}
        bool operator==(int i) { return fInt == i; }
        int fInt;
    };

    test_swap<SkSL::InlinedVector< 5, MoveOnlyInt>>(reporter);
    test_swap<SkSL::InlinedVector<10, MoveOnlyInt>>(reporter);
    test_swap<SkSL::InlinedVector<20, MoveOnlyInt>>(reporter);
}

static void test_self_assignment(skiatest::Reporter* reporter) {
    SkSL::InlinedVector<1, int> a;
    a.push_back(1);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);

    a = static_cast<decltype(a)&>(a);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);
}

DEF_TEST(SkSLInlinedVector, reporter) {
    TestInlineVector_basic<int>(reporter);
    TestInlineVector_basic<char>(reporter);
    TestInlineVector_basic<double>(reporter);

    test_swap(reporter);
    test_self_assignment(reporter);
}
