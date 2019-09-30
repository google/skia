/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTSearch.h"
#include "src/core/SkTSort.h"
#include "src/core/SkZip.h"
#include "tests/Test.h"

#include <array>
#include <initializer_list>
#include <tuple>
#include <vector>

class RefClass : public SkRefCnt {
public:
    RefClass(int n) : fN(n) {}
    int get() const { return fN; }

private:
    int fN;

    typedef SkRefCnt INHERITED;
};

static void test_autounref(skiatest::Reporter* reporter) {
    RefClass obj(0);
    REPORTER_ASSERT(reporter, obj.unique());

    sk_sp<RefClass> tmp(&obj);
    REPORTER_ASSERT(reporter, &obj == tmp.get());
    REPORTER_ASSERT(reporter, obj.unique());

    REPORTER_ASSERT(reporter, &obj == tmp.release());
    REPORTER_ASSERT(reporter, obj.unique());
    REPORTER_ASSERT(reporter, nullptr == tmp.release());
    REPORTER_ASSERT(reporter, nullptr == tmp.get());

    obj.ref();
    REPORTER_ASSERT(reporter, !obj.unique());
    {
        sk_sp<RefClass> tmp2(&obj);
    }
    REPORTER_ASSERT(reporter, obj.unique());
}

static void test_autostarray(skiatest::Reporter* reporter) {
    RefClass obj0(0);
    RefClass obj1(1);
    REPORTER_ASSERT(reporter, obj0.unique());
    REPORTER_ASSERT(reporter, obj1.unique());

    {
        SkAutoSTArray<2, sk_sp<RefClass> > tmp;
        REPORTER_ASSERT(reporter, 0 == tmp.count());

        tmp.reset(0);   // test out reset(0) when already at 0
        tmp.reset(4);   // this should force a new allocation
        REPORTER_ASSERT(reporter, 4 == tmp.count());
        tmp[0].reset(SkRef(&obj0));
        tmp[1].reset(SkRef(&obj1));
        REPORTER_ASSERT(reporter, !obj0.unique());
        REPORTER_ASSERT(reporter, !obj1.unique());

        // test out reset with data in the array (and a new allocation)
        tmp.reset(0);
        REPORTER_ASSERT(reporter, 0 == tmp.count());
        REPORTER_ASSERT(reporter, obj0.unique());
        REPORTER_ASSERT(reporter, obj1.unique());

        tmp.reset(2);   // this should use the preexisting allocation
        REPORTER_ASSERT(reporter, 2 == tmp.count());
        tmp[0].reset(SkRef(&obj0));
        tmp[1].reset(SkRef(&obj1));
    }

    // test out destructor with data in the array (and using existing allocation)
    REPORTER_ASSERT(reporter, obj0.unique());
    REPORTER_ASSERT(reporter, obj1.unique());

    {
        // test out allocating ctor (this should allocate new memory)
        SkAutoSTArray<2, sk_sp<RefClass> > tmp(4);
        REPORTER_ASSERT(reporter, 4 == tmp.count());

        tmp[0].reset(SkRef(&obj0));
        tmp[1].reset(SkRef(&obj1));
        REPORTER_ASSERT(reporter, !obj0.unique());
        REPORTER_ASSERT(reporter, !obj1.unique());

        // Test out resut with data in the array and malloced storage
        tmp.reset(0);
        REPORTER_ASSERT(reporter, obj0.unique());
        REPORTER_ASSERT(reporter, obj1.unique());

        tmp.reset(2);   // this should use the preexisting storage
        tmp[0].reset(SkRef(&obj0));
        tmp[1].reset(SkRef(&obj1));
        REPORTER_ASSERT(reporter, !obj0.unique());
        REPORTER_ASSERT(reporter, !obj1.unique());

        tmp.reset(4);   // this should force a new malloc
        REPORTER_ASSERT(reporter, obj0.unique());
        REPORTER_ASSERT(reporter, obj1.unique());

        tmp[0].reset(SkRef(&obj0));
        tmp[1].reset(SkRef(&obj1));
        REPORTER_ASSERT(reporter, !obj0.unique());
        REPORTER_ASSERT(reporter, !obj1.unique());
    }

    REPORTER_ASSERT(reporter, obj0.unique());
    REPORTER_ASSERT(reporter, obj1.unique());
}

/////////////////////////////////////////////////////////////////////////////

#define kSEARCH_COUNT   91

static void test_search(skiatest::Reporter* reporter) {
    int         i, array[kSEARCH_COUNT];
    SkRandom    rand;

    for (i = 0; i < kSEARCH_COUNT; i++) {
        array[i] = rand.nextS();
    }

    SkTHeapSort<int>(array, kSEARCH_COUNT);
    // make sure we got sorted properly
    for (i = 1; i < kSEARCH_COUNT; i++) {
        REPORTER_ASSERT(reporter, array[i-1] <= array[i]);
    }

    // make sure we can find all of our values
    for (i = 0; i < kSEARCH_COUNT; i++) {
        int index = SkTSearch<int>(array, kSEARCH_COUNT, array[i], sizeof(int));
        REPORTER_ASSERT(reporter, index == i);
    }

    // make sure that random values are either found, or the correct
    // insertion index is returned
    for (i = 0; i < 10000; i++) {
        int value = rand.nextS();
        int index = SkTSearch<int>(array, kSEARCH_COUNT, value, sizeof(int));

        if (index >= 0) {
            REPORTER_ASSERT(reporter,
                            index < kSEARCH_COUNT && array[index] == value);
        } else {
            index = ~index;
            REPORTER_ASSERT(reporter, index <= kSEARCH_COUNT);
            if (index < kSEARCH_COUNT) {
                REPORTER_ASSERT(reporter, value < array[index]);
                if (index > 0) {
                    REPORTER_ASSERT(reporter, value > array[index - 1]);
                }
            } else {
                // we should append the new value
                REPORTER_ASSERT(reporter, value > array[kSEARCH_COUNT - 1]);
            }
        }
    }
}

DEF_TEST(Utils, reporter) {
    test_search(reporter);
    test_autounref(reporter);
    test_autostarray(reporter);
}

DEF_TEST(SkMakeSpan, reporter) {
    // Test constness preservation for SkMakeSpan.
    {
        std::vector<int> v = {{1, 2, 3, 4, 5}};
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> t = {{1, 2, 3, 4, 5}};
        const std::vector<int>& v = t;
        auto s = SkMakeSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> v = {{1, 2, 3, 4, 5}};
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> t = {{1, 2, 3, 4, 5}};
        const std::array<int, 5>& v = t;
        auto s = SkMakeSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> v;
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s.empty());
    }
}

DEF_TEST(SkEnumerate, reporter) {

    int A[] = {1, 2, 3, 4};
    auto enumeration = SkMakeEnumerate(A);

    size_t check = 0;
    for (auto t : enumeration) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);

        check++;
    }

    check = 0;
    for (auto t : SkMakeEnumerate(A)) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);

        check++;
    }

    check = 0;
    std::vector<int> vec = {1, 2, 3, 4};
    for (auto t : SkMakeEnumerate(vec)) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);
        check++;
    }
    REPORTER_ASSERT(reporter, check == 4);

    check = 0;
    for (auto t : SkMakeEnumerate(SkMakeSpan(vec))) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);
        check++;
    }
}

DEF_TEST(SkZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    const float B[] = {10.f, 20.f, 30.f, 40.f};
    std::vector<int> C = {{20, 30, 40, 50}};
    std::array<int, 4> D = {{100, 200, 300, 400}};
    SkSpan<int> S = SkMakeSpan(C);

    // Check SkZip calls
    SkZip<uint16_t, const float, int, int, int>
            z{4, &A[0], &B[0], C.data(), D.data(), S.data()};

    REPORTER_ASSERT(reporter, z.size() == 4);
    REPORTER_ASSERT(reporter, !z.empty());

    {
        // Check front
        auto t = z.front();
        REPORTER_ASSERT(reporter, std::get<0>(t) == 1);
        REPORTER_ASSERT(reporter, std::get<1>(t) == 10.f);
        REPORTER_ASSERT(reporter, std::get<2>(t) == 20);
        REPORTER_ASSERT(reporter, std::get<3>(t) == 100);
        REPORTER_ASSERT(reporter, std::get<4>(t) == 20);
    }

    {
        // Check back
        auto t = z.back();
        REPORTER_ASSERT(reporter, std::get<0>(t) == 4);
        REPORTER_ASSERT(reporter, std::get<1>(t) == 40.f);
    }

    {
        // Check ranged-for
        int i = 0;
        for (auto t : z) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(a, b, c, d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    // Check copy.
    auto zz{z};
    {
        int i = 0;
        for (auto t : zz) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(a, b, c, d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    // Check index getter
    {
        auto span = z.get<1>();
        REPORTER_ASSERT(reporter, span[1] == 20.f);
    }

    // The following mutates the data.
    {
        // Check indexing
        auto t = z[1];
        REPORTER_ASSERT(reporter, std::get<0>(t) == 2);
        REPORTER_ASSERT(reporter, std::get<1>(t) == 20.f);
        REPORTER_ASSERT(reporter, std::get<2>(t) == 30);
        REPORTER_ASSERT(reporter, std::get<3>(t) == 200);
        REPORTER_ASSERT(reporter, std::get<4>(t) == 30);

        // Check correct refs returned.
        REPORTER_ASSERT(reporter, &std::get<0>(t) == &A[1]);
        REPORTER_ASSERT(reporter, &std::get<1>(t) == &B[1]);
        REPORTER_ASSERT(reporter, &std::get<2>(t) == &C[1]);
        REPORTER_ASSERT(reporter, &std::get<3>(t) == &D[1]);
        REPORTER_ASSERT(reporter, &std::get<4>(t) == &S[1]);

        // Check assignment
        std::get<0>(t) = 20;
        // std::get<1>(t) = 300.f; // is const
        std::get<2>(t) = 300;
        std::get<3>(t) = 2000;
        std::get<4>(t) = 300;

        auto t1 = z[1];
        REPORTER_ASSERT(reporter, std::get<0>(t1) == 20);
        REPORTER_ASSERT(reporter, std::get<1>(t1) == 20.f);
        REPORTER_ASSERT(reporter, std::get<2>(t1) == 300);
        REPORTER_ASSERT(reporter, std::get<3>(t1) == 2000);
        REPORTER_ASSERT(reporter, std::get<4>(t1) == 300);
    }
}

DEF_TEST(SkMakeZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    const float B[] = {10.f, 20.f, 30.f, 40.f};
    const std::vector<int> C = {{20, 30, 40, 50}};
    std::array<int, 4> D = {{100, 200, 300, 400}};
    SkSpan<const int> S = SkMakeSpan(C);
    uint16_t* P = &A[0];
    {
        // Check make zip
        auto zz = SkMakeZip(&A[0], B, C, D, S, P);

        int i = 0;
        for (auto t : zz) {
            uint16_t a; float b; int c; int d; int s; uint16_t p;
            std::tie(a, b ,c ,d, s, p) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
            REPORTER_ASSERT(reporter, p == P[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    {
        // Check SkMakeZip in ranged for check OneSize calc of B.
        int i = 0;
        for (auto t : SkMakeZip(&A[0], B, C, D, S)) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(a, b ,c ,d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    {
        // Check SkMakeZip in ranged for OneSize of C
        int i = 0;
        for (auto t : SkMakeZip(&A[0], &B[0], C, D, S)) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(a, b ,c ,d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    {
        // Check SkMakeZip in ranged for OneSize for S
        int i = 0;
        for (auto t : SkMakeZip(S, A, B, C, D)) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(s, a, b, c, d) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    {
        // Check SkMakeZip in ranged for
        int i = 0;
        for (auto t : SkMakeZip(C, S, A, B, D)) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(c, s, a, b, d) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 4);
    }

    {
        // Check SkEnumerate and SkMakeZip in ranged for
        auto zz = SkMakeZip(A, B, C, D, S);
        for (auto t : SkMakeEnumerate(zz)) {
            int i;
            uint16_t a; float b; int c; int d; int s;
            std::forward_as_tuple(i, std::tie(a, b, c, d, s)) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
        }
    }

    {
        // Check SkEnumerate and SkMakeZip in ranged for
        const auto& zz = SkMakeZip(A, B, C, D, S);
        for (auto t : SkMakeEnumerate(zz)) {
            int i;
            uint16_t a; float b; int c; int d; int s;
            std::forward_as_tuple(i, std::tie(a, b, c, d, s)) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
        }
    }

    {
        // Check SkEnumerate and SkMakeZip in ranged for
        for (auto t : SkMakeEnumerate(SkMakeZip(A, B, C, D, S))) {
            int i;
            uint16_t a; float b; int c; int d; int s;
            std::forward_as_tuple(i, std::tie(a, b, c, d, s)) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
        }
    }

    {
        std::vector<int>v;
        auto z = SkMakeZip(v);
        REPORTER_ASSERT(reporter, z.empty());
    }

    {
        constexpr static uint16_t cA[] = {1, 2, 3, 4};
        // Not constexpr in stdc++11 library.
        //constexpr static std::array<int, 4> cD = {{100, 200, 300, 400}};
        constexpr static const uint16_t* cP = &cA[0];
        constexpr auto z = SkMakeZip(cA, cP);
        REPORTER_ASSERT(reporter, !z.empty());
    }
}
