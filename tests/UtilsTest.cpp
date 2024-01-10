/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTSearch.h"
#include "src/base/SkTSort.h"
#include "src/base/SkUtils.h"
#include "src/base/SkZip.h"
#include "src/core/SkEnumerate.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <new>
#include <tuple>
#include <utility>
#include <vector>

using namespace skia_private;

class RefClass : public SkRefCnt {
public:
    RefClass(int n) : fN(n) {}
    int get() const { return fN; }

private:
    int fN;

    using INHERITED = SkRefCnt;
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
        AutoSTArray<2, sk_sp<RefClass> > tmp;
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
        AutoSTArray<2, sk_sp<RefClass> > tmp(4);
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

DEF_TEST(SkEnumerate, reporter) {

    int A[] = {1, 2, 3, 4};
    auto enumeration = SkMakeEnumerate(A);

    size_t check = 0;
    for (auto [i, v] : enumeration) {
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);

        check++;
    }

    check = 0;
    for (auto [i, v] : SkMakeEnumerate(A)) {
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);

        check++;
    }

    check = 0;
    std::vector<int> vec = {1, 2, 3, 4};
    for (auto [i, v] : SkMakeEnumerate(vec)) {
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);
        check++;
    }
    REPORTER_ASSERT(reporter, check == 4);

    check = 0;
    for (auto [i, v] : SkMakeEnumerate(SkSpan(vec))) {
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);
        check++;
    }

    {
        auto e = SkMakeEnumerate(SkSpan(vec)).first(2);
        for (auto[i, v] : e) {
            REPORTER_ASSERT(reporter, v == (int) i + 1);
        }
        REPORTER_ASSERT(reporter, e.size() == 2);
    }

    {
        auto e = SkMakeEnumerate(SkSpan(vec)).last(2);
        for (auto[i, v] : e) {
            REPORTER_ASSERT(reporter, v == (int) i + 1);
        }
        REPORTER_ASSERT(reporter, e.size() == 2);
    }

    {
        auto e = SkMakeEnumerate(SkSpan(vec)).subspan(1, 2);
        for (auto[i, v] : e) {
            REPORTER_ASSERT(reporter, v == (int) i + 1);
        }
        REPORTER_ASSERT(reporter, e.size() == 2);
    }

    {
        struct I {
            I() = default;
            I(const I&) = default;
            I(int v) : i{v} { }
            ~I() {}
            int i;
        };

        I is[10];
        auto s = SkSpan(is);
        for (auto [i, v] : SkMakeEnumerate(s)) {
            new (&v) I(i);
        }

        for (size_t i = 0; i < s.size(); i++) {
            REPORTER_ASSERT(reporter, s[i].i == (int)i);
            REPORTER_ASSERT(reporter, is[i].i == (int)i);
        }
    }

    {
        std::unique_ptr<int> is[10];
        std::unique_ptr<int> os[10];
        auto s = SkSpan(is);
        for (auto [i, v] : SkMakeEnumerate(s)) {
            v = std::make_unique<int>(i);
        }

        for (auto [i, v] : SkMakeEnumerate(SkSpan(os))) {
            v = std::move(s[i]);
        }

        for (size_t i = 0; i < s.size(); i++) {
            REPORTER_ASSERT(reporter, *os[i] == (int)i);
            REPORTER_ASSERT(reporter, is[i] == nullptr);
        }
    }

    {
        std::unique_ptr<int> is[10];
        std::unique_ptr<int> os[10];
        auto s = SkSpan(is);
        for (auto [i, v] : SkMakeEnumerate(s)) {
            v = std::make_unique<int>(i);
        }

        for (auto [i, ov, iv] : SkMakeEnumerate(SkMakeZip(os, is))) {
            ov = std::move(iv);
        }

        for (size_t i = 0; i < s.size(); i++) {
            REPORTER_ASSERT(reporter, *os[i] == (int)i);
            REPORTER_ASSERT(reporter, is[i] == nullptr);
        }
    }
}

DEF_TEST(SkZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    const float B[] = {10.f, 20.f, 30.f, 40.f};
    std::vector<int> C = {{20, 30, 40, 50}};
    std::array<int, 4> D = {{100, 200, 300, 400}};
    SkSpan<int> S = SkSpan(C);

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
        for (auto [a, b, c, d, s] : z) {
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
        // Check first(n)
        int i = 0;
        for (auto [a, b, c, d, s] : z.first(2)) {
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 2);
    }

    {
        // Check last(n)
        int i = 0;
        for (auto t : z.last(2)) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(a, b, c, d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i + 2]);
            REPORTER_ASSERT(reporter, b == B[i + 2]);
            REPORTER_ASSERT(reporter, c == C[i + 2]);
            REPORTER_ASSERT(reporter, d == D[i + 2]);
            REPORTER_ASSERT(reporter, s == S[i + 2]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 2);
    }

    {
        // Check subspan(offset, count)
        int i = 0;
        for (auto t : z.subspan(1, 2)) {
            uint16_t a; float b; int c; int d; int s;
            std::tie(a, b, c, d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i + 1]);
            REPORTER_ASSERT(reporter, b == B[i + 1]);
            REPORTER_ASSERT(reporter, c == C[i + 1]);
            REPORTER_ASSERT(reporter, d == D[i + 1]);
            REPORTER_ASSERT(reporter, s == S[i + 1]);

            i++;
        }
        REPORTER_ASSERT(reporter, i = 2);
    }

    {
        // Check copy.
        auto zz{z};
        int i = 0;
        for (auto [a, b, c, d, s] : zz) {
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
        // Check const restricting copy
        SkZip<const uint16_t, const float, const int, int, int> cz = z;
        int i = 0;
        for (auto [a, b, c, d, s] : cz) {
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
        // Check data() returns all the original pointers
        auto ptrs = z.data();
        REPORTER_ASSERT(reporter,
                ptrs == std::make_tuple(&A[0], &B[0], C.data(), D.data(), S.data()));
    }

    {
        // Check index getter
        auto span = z.get<1>();
        REPORTER_ASSERT(reporter, span[1] == 20.f);
    }

    // The following mutates the data.
    {
        // Check indexing
        auto [a, b, c, d, e] = z[1];
        REPORTER_ASSERT(reporter, a == 2);
        REPORTER_ASSERT(reporter, b == 20.f);
        REPORTER_ASSERT(reporter, c == 30);
        REPORTER_ASSERT(reporter, d == 200);
        REPORTER_ASSERT(reporter, e == 30);

        // Check correct refs returned.
        REPORTER_ASSERT(reporter, &a == &A[1]);
        REPORTER_ASSERT(reporter, &b == &B[1]);
        REPORTER_ASSERT(reporter, &c == &C[1]);
        REPORTER_ASSERT(reporter, &d == &D[1]);
        REPORTER_ASSERT(reporter, &e == &S[1]);

        // Check assignment
        a = 20;
        // std::get<1>(t) = 300.f; // is const
        c = 300;
        d = 2000;
        e = 300;

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
    SkSpan<const int> S = SkSpan(C);
    uint16_t* P = &A[0];
    {
        // Check make zip
        auto zz = SkMakeZip(&A[0], B, C, D, S, P);

        int i = 0;
        for (auto [a, b, c, d, s, p] : zz) {
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
        for (auto [a, b, c, d, s] : SkMakeZip(&A[0], B, C, D, S)) {
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
        for (auto [a, b, c, d, s] : SkMakeZip(&A[0], &B[0], C, D, S)) {
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
        for (auto [s, a, b, c, d] : SkMakeZip(S, A, B, C, D)) {
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
        for (auto [c, s, a, b, d] : SkMakeZip(C, S, A, B, D)) {
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
        for (auto [i, a, b, c, d, s] : SkMakeEnumerate(zz)) {
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
        for (auto [i, a, b, c, d, s] : SkMakeEnumerate(zz)) {
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
        }
    }

    {
        // Check SkEnumerate and SkMakeZip in ranged for
        for (auto [i, a, b, c, d, s] : SkMakeEnumerate(SkMakeZip(A, B, C, D, S))) {
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

DEF_TEST(UtilsPreserveBitPatterns, r) {
    // Various kinds of floating point bit patterns. We round trip each one through float using
    // utility functions. If any of them ever do any real FP operation (including loading it into
    // the x87 FPU on x86 builds), they might change. (In practice, signaling NaN is the only one
    // that's likely to break -- it can be converted to a quiet NaN).
    const uint32_t kBitPatterns[] = {
        0x00400000,  // Denormal value
        0x80000000,  // -0.0f
        0x3f800000,  // 1.0f (arbitrary normal float)
        0x7f800000,  // Infinity
        0x7fa00000,  // Signaling NaN
        0x7fe00000,  // Quiet NaN
    };

    for (uint32_t srcBits : kBitPatterns) {
        {
            float floatVal = sk_unaligned_load<float>(&srcBits);
            uint32_t dstBits = sk_unaligned_load<uint32_t>(&floatVal);
            REPORTER_ASSERT(r, dstBits == srcBits);
        }

        {
            float floatVal;
            sk_unaligned_store(&floatVal, srcBits);
            uint32_t dstBits;
            sk_unaligned_store(&dstBits, floatVal);
            REPORTER_ASSERT(r, dstBits == srcBits);
        }

        REPORTER_ASSERT(r, sk_bit_cast<uint32_t>(sk_bit_cast<float>(srcBits)) == srcBits);
    }
}
