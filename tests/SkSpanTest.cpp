/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "src/core/SkSpanPriv.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

DEF_TEST(SkSpanBasicTemplateGuide, reporter) {
    // Test constness preservation for SkSpan.
    {
        std::vector<int> v = {{1, 2, 3, 4, 5}};
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> t = {{1, 2, 3, 4, 5}};
        const std::vector<int>& v = t;
        auto s = SkSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> v = {{1, 2, 3, 4, 5}};
        auto s = SkSpan(v); // {1, 2, 3, 4, 5}
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100; // {1, 2, 3, 100, 5}
        REPORTER_ASSERT(reporter, s[3] == 100);
        auto s1 = s.subspan(1,3); // {2, 3, 100}
        REPORTER_ASSERT(reporter, s1.size() == 3);
        REPORTER_ASSERT(reporter, s1.front() == 2);
        REPORTER_ASSERT(reporter, s1.back() == 100);
        auto s2 = s.subspan(2); // {3, 100, 5}
        REPORTER_ASSERT(reporter, s2.size() == 3);
        REPORTER_ASSERT(reporter, s2.front() == 3);
        REPORTER_ASSERT(reporter, s2.back() == 5);
    }

    {
        std::array<int, 5> t = {{1, 2, 3, 4, 5}};
        const std::array<int, 5>& v = t;
        auto s = SkSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> v;
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, s.empty());
    }

    auto routine = [&](SkSpan<const int> a) {
        REPORTER_ASSERT(reporter, a.size() == 4);
    };
    routine({{1,2,3,4}});
}

static bool test_span_parameter(SkSpan<const int> s) {
    return s[0] == 1 && s[1] == 2 && s[2] == 3;
}

DEF_TEST(SkSpanDeduceParam, reporter) {
    {
        std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
        REPORTER_ASSERT(reporter, test_span_parameter(std::move(v)));
    }

    {
        const std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
        REPORTER_ASSERT(reporter, test_span_parameter(std::move(v)));
    }

    REPORTER_ASSERT(reporter, test_span_parameter(std::vector<int>{1,2,3}));

    {
        int v[]{1, 2, 3};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
    }

    {
        test_span_parameter({{1, 2, 3}});
        REPORTER_ASSERT(reporter, test_span_parameter({{1, 2, 3}}));
    }

    {
        int v[]{1, 2, 3};
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, test_span_parameter(s));
    }
}

DEF_TEST(SkSpanDeduceSize, reporter) {
    int d[] = {1, 2, 3, 4, 5};
    {
        size_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        uint32_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        size_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
}

namespace {
template <typename T, size_t Extent = SkDynamicExtent>
struct MockBaseSpan {
    static constexpr size_t extent = Extent;
    constexpr T* data() const { return fPtr; }
    constexpr size_t size() const { return fSize; }
    T* fPtr = nullptr;
    size_t fSize = 0;
};

struct BaseElem {
    int a = 1;
};
struct DerivedElem : BaseElem {
    int b = 2;
};

// Helper to test whether copy-list-initialization `To x = {args...}` is allowed (i.e. non-explicit
// multi-argument construction).
template <typename To>
[[maybe_unused]] void accept_implicit(To);

template <typename To, typename... Args>
inline constexpr bool kIsImplicitlyConstructible =
        requires { accept_implicit<To>({std::declval<Args>()...}); };

// Concept/requires detectors for compile-time subview member functions:
// first<Count>(), last<Count>(), subspan<Offset>(), subspan<Offset, Count>().
template <typename Span, size_t Count>
inline constexpr bool kCanCallFirst =
        requires(const Span& s) { s.template first<Count>(); };

template <typename Span, size_t Count>
inline constexpr bool kCanCallLast =
        requires(const Span& s) { s.template last<Count>(); };

template <typename Span, size_t Offset>
inline constexpr bool kCanCallSubspanOffset =
        requires(const Span& s) { s.template subspan<Offset>(); };

template <typename Span, size_t Offset, size_t Count>
inline constexpr bool kCanCallSubspanOffsetCount =
        requires(const Span& s) { s.template subspan<Offset, Count>(); };

inline constexpr int kConstexprArray[4] = {10, 20, 30, 40};
}  // namespace

DEF_TEST(SkSpanFixedExtent, reporter) {
    // 1. Zero-overhead fixed-extent storage vs dynamic-extent storage,
    //    plus standard layout and trivial copyability guarantees.
    static_assert(sizeof(SkSpan<int, 5>) == sizeof(int*));
    static_assert(sizeof(SkSpan<int, 0>) == sizeof(int*));
    static_assert(sizeof(SkSpan<int>) == sizeof(int*) + sizeof(size_t));
    static_assert(std::is_standard_layout_v<SkSpan<int>> &&
                  std::is_standard_layout_v<SkSpan<int, 4>>);
    static_assert(std::is_trivially_copyable_v<SkSpan<int>> &&
                  std::is_trivially_copyable_v<SkSpan<int, 4>>);
    static_assert(SkSpan<int, 5>::extent == 5);
    static_assert(SkSpan<int, 0>::extent == 0);
    static_assert(SkSpan<int>::extent == SkDynamicExtent);

    // 2. Default constructibility: allowed for Extent == 0 and SkDynamicExtent, forbidden for N > 0.
    static_assert(std::is_default_constructible_v<SkSpan<int>>);
    static_assert(std::is_default_constructible_v<SkSpan<int, 0>>);
    static_assert(!std::is_default_constructible_v<SkSpan<int, 1>>);
    static_assert(!std::is_default_constructible_v<SkSpan<int, 5>>);

    {
        constexpr SkSpan<int, 0> emptyFixed;
        static_assert(emptyFixed.empty());
        static_assert(emptyFixed.size() == 0);
        static_assert(emptyFixed.size_bytes() == 0);
        static_assert(emptyFixed.data() == nullptr);
        static_assert(emptyFixed.begin() == emptyFixed.end());
        REPORTER_ASSERT(reporter, emptyFixed.data() == nullptr);
    }

    // 3. Pointer + count constructor: implicit for dynamic extent, explicit-only for fixed extent,
    // and forbidden for non-integral counts.
    static_assert(std::is_constructible_v<SkSpan<int>, int*, size_t>);
    static_assert(kIsImplicitlyConstructible<SkSpan<int>, int*, size_t>);
    static_assert(std::is_constructible_v<SkSpan<int, 4>, int*, size_t>);
    static_assert(!kIsImplicitlyConstructible<SkSpan<int, 4>, int*, size_t>);
    static_assert(!std::is_constructible_v<SkSpan<int>, int*, void*>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, int*, void*>);

    // 4. Construction, CTAD, iterators, and accessors from C-array, std::array, and pointers.
    {
        int arr[4] = {10, 20, 30, 40};
        auto s = SkSpan(arr);
        static_assert(std::is_same_v<decltype(s), SkSpan<int, 4>>);
        static_assert(!s.empty());
        static_assert(s.size() == 4);
        static_assert(s.size_bytes() == 4 * sizeof(int));
        REPORTER_ASSERT(reporter, s.data() == arr);
        REPORTER_ASSERT(reporter, s.front() == 10);
        REPORTER_ASSERT(reporter, s.back() == 40);
        REPORTER_ASSERT(reporter, s[2] == 30);

        // Forward and reverse iterators on fixed-extent span.
        int sum = 0;
        for (int v : s) {
            sum += v;
        }
        REPORTER_ASSERT(reporter, sum == 100);
        REPORTER_ASSERT(reporter, *s.rbegin() == 40);
        REPORTER_ASSERT(reporter, *(s.rend() - 1) == 10);

        // Explicit (ptr, size) construction for fixed-extent span.
        SkSpan<int, 4> fromPtr(arr, 4);
        REPORTER_ASSERT(reporter, fromPtr.data() == arr && fromPtr.size() == 4);

        // Implicit conversion from fixed-extent to dynamic-extent and const-qualified spans.
        SkSpan<int> dyn = s;
        SkSpan<const int, 4> constFixed = s;
        SkSpan<const int> constDyn = s;
        REPORTER_ASSERT(reporter, dyn.size() == 4 && dyn[1] == 20);
        REPORTER_ASSERT(reporter, constFixed.size() == 4 && constFixed[1] == 20);
        REPORTER_ASSERT(reporter, constDyn.size() == 4 && constDyn[1] == 20);

        // Explicit conversion from dynamic-extent to fixed-extent.
        SkSpan<int, 4> backToFixed(dyn);
        REPORTER_ASSERT(reporter, backToFixed.size() == 4 && backToFixed[3] == 40);
    }

    {
        const int cArr[3] = {1, 2, 3};
        auto cs = SkSpan(cArr);
        static_assert(std::is_same_v<decltype(cs), SkSpan<const int, 3>>);
        REPORTER_ASSERT(reporter, cs[0] == 1 && cs[2] == 3);
    }

    {
        std::array<int, 3> arr = {{7, 8, 9}};
        auto s = SkSpan(arr);
        static_assert(std::is_same_v<decltype(s), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, s[0] == 7 && s[2] == 9);

        const std::array<int, 3>& constArr = arr;
        auto cs = SkSpan(constArr);
        static_assert(std::is_same_v<decltype(cs), SkSpan<const int, 3>>);
        REPORTER_ASSERT(reporter, cs[1] == 8);
    }

    {
        int mockData[3] = {1, 2, 3};
        MockBaseSpan<int, 3> mockFixed{mockData, 3};
        auto deducedFixed = SkSpan(mockFixed);
        static_assert(std::is_same_v<decltype(deducedFixed), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, deducedFixed.size() == 3 && deducedFixed[1] == 2);

        MockBaseSpan<int> mockDyn{mockData, 3};
        auto deducedDyn = SkSpan(mockDyn);
        static_assert(std::is_same_v<decltype(deducedDyn), SkSpan<int>>);
        REPORTER_ASSERT(reporter, deducedDyn.size() == 3 && deducedDyn[2] == 3);
    }

    // 5. Compile-time conversion rules (working and forbidden cases).
    // 5a. SkSpan-to-SkSpan conversions:
    static_assert(std::is_convertible_v<SkSpan<int, 4>, SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<SkSpan<int, 4>, SkSpan<const int, 4>>);
    static_assert(std::is_convertible_v<SkSpan<int, 4>, SkSpan<int>>);
    static_assert(std::is_convertible_v<SkSpan<int, 4>, SkSpan<const int>>);
    // Dynamic -> Fixed is explicit-only (forbidden implicitly):
    static_assert(std::is_constructible_v<SkSpan<int, 4>, SkSpan<int>>);
    static_assert(std::is_constructible_v<SkSpan<const int, 4>, SkSpan<int>>);
    static_assert(!std::is_convertible_v<SkSpan<int>, SkSpan<int, 4>>);
    static_assert(!std::is_convertible_v<SkSpan<int>, SkSpan<const int, 4>>);
    // Mismatched static extents are completely forbidden (neither implicit nor explicit):
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, SkSpan<int, 3>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, SkSpan<int, 5>>);
    static_assert(!std::is_convertible_v<SkSpan<int, 3>, SkSpan<int, 4>>);
    // Dropping const qualification is completely forbidden:
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, SkSpan<const int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int>, SkSpan<const int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, SkSpan<const int>>);

    // 5b. C-array and std::array conversions:
    static_assert(std::is_convertible_v<int(&)[4], SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<int(&)[4], SkSpan<const int, 4>>);
    static_assert(std::is_convertible_v<int(&)[4], SkSpan<int>>);
    static_assert(!std::is_convertible_v<const int(&)[4], SkSpan<int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, const int(&)[4]>);
    static_assert(!std::is_convertible_v<int(&)[3], SkSpan<int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, int(&)[3]>);

    static_assert(std::is_convertible_v<std::array<int, 4>&, SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<std::array<int, 4>&, SkSpan<const int, 4>>);
    static_assert(std::is_convertible_v<const std::array<int, 4>&, SkSpan<const int, 4>>);
    static_assert(!std::is_convertible_v<const std::array<int, 4>&, SkSpan<int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, const std::array<int, 4>&>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, std::array<int, 3>&>);

    // 5c. Forbidden Derived-to-Base array element slicing (kLegalDataConversion):
    static_assert(!std::is_constructible_v<SkSpan<BaseElem, 4>, SkSpan<DerivedElem, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<BaseElem>, SkSpan<DerivedElem, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<BaseElem, 4>, DerivedElem(&)[4]>);
    static_assert(!std::is_constructible_v<SkSpan<BaseElem, 4>, std::array<DerivedElem, 4>&>);
    static_assert(!std::is_constructible_v<SkSpan<BaseElem>, std::vector<DerivedElem>&>);

    // 5d. Interop with external span types exposing `::extent` (like base::span and std::span):
    static_assert(std::is_convertible_v<MockBaseSpan<int, 4>, SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<MockBaseSpan<int, 4>, SkSpan<const int, 4>>);
    static_assert(std::is_convertible_v<MockBaseSpan<int, 4>, SkSpan<int>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, MockBaseSpan<int, 3>>);
    static_assert(!std::is_convertible_v<MockBaseSpan<int>, SkSpan<int, 4>>);
    static_assert(std::is_constructible_v<SkSpan<int, 4>, MockBaseSpan<int>>);

    // 5e. Dynamic container and std::initializer_list to fixed-extent span is explicit-only:
    static_assert(std::is_convertible_v<std::vector<int>&, SkSpan<int>>);
    static_assert(!std::is_convertible_v<std::vector<int>&, SkSpan<int, 4>>);
    static_assert(std::is_constructible_v<SkSpan<int, 4>, std::vector<int>&>);

    static_assert(std::is_convertible_v<std::initializer_list<const int>, SkSpan<const int>>);
    static_assert(!std::is_convertible_v<std::initializer_list<const int>, SkSpan<const int, 3>>);
    static_assert(std::is_constructible_v<SkSpan<const int, 3>, std::initializer_list<const int>>);
    {
        auto checkFixedIL = [](SkSpan<const int, 3> ilSpan) {
            return ilSpan.size() == 3 && ilSpan[0] == 11 && ilSpan[2] == 33;
        };
        REPORTER_ASSERT(reporter, checkFixedIL(SkSpan<const int, 3>{11, 22, 33}));
    }

    // 6. Compile-time subviews: working cases, boundary cases, constexpr evaluation, and
    // forbidden out-of-bounds compile-time checks.
    static_assert(kCanCallFirst<SkSpan<int, 5>, 0>);
    static_assert(kCanCallFirst<SkSpan<int, 5>, 5>);
    static_assert(!kCanCallFirst<SkSpan<int, 5>, 6>);
    static_assert(!kCanCallFirst<SkSpan<int, 5>, SkDynamicExtent>);
    static_assert(kCanCallFirst<SkSpan<int>, 10>);  // Dynamic extent checks at runtime
    static_assert(!kCanCallFirst<SkSpan<int>, SkDynamicExtent>);

    static_assert(kCanCallLast<SkSpan<int, 5>, 0>);
    static_assert(kCanCallLast<SkSpan<int, 5>, 5>);
    static_assert(!kCanCallLast<SkSpan<int, 5>, 6>);
    static_assert(!kCanCallLast<SkSpan<int, 5>, SkDynamicExtent>);
    static_assert(kCanCallLast<SkSpan<int>, 10>);
    static_assert(!kCanCallLast<SkSpan<int>, SkDynamicExtent>);

    static_assert(kCanCallSubspanOffset<SkSpan<int, 5>, 0>);
    static_assert(kCanCallSubspanOffset<SkSpan<int, 5>, 5>);
    static_assert(!kCanCallSubspanOffset<SkSpan<int, 5>, 6>);
    static_assert(kCanCallSubspanOffset<SkSpan<int>, 10>);

    static_assert(kCanCallSubspanOffsetCount<SkSpan<int, 5>, 0, 0>);
    static_assert(kCanCallSubspanOffsetCount<SkSpan<int, 5>, 0, 5>);
    static_assert(kCanCallSubspanOffsetCount<SkSpan<int, 5>, 5, 0>);
    static_assert(kCanCallSubspanOffsetCount<SkSpan<int, 5>, 2, 3>);
    static_assert(!kCanCallSubspanOffsetCount<SkSpan<int, 5>, 2, 4>);
    static_assert(!kCanCallSubspanOffsetCount<SkSpan<int, 5>, 6, 0>);
    static_assert(!kCanCallSubspanOffsetCount<SkSpan<int, 5>, 0, 6>);
    // Overflow safety in `Count <= E - Offset`:
    static_assert(!kCanCallSubspanOffsetCount<SkSpan<int, 5>,
                                              1,
                                              std::numeric_limits<size_t>::max() - 1>);

    {
        // Full constexpr verification of fixed-extent construction and compile-time subviews.
        constexpr SkSpan<const int, 4> cs(kConstexprArray);
        static_assert(cs.size() == 4);
        static_assert(*cs.data() == 10 && *(cs.data() + 3) == 40);
        constexpr auto cf2 = cs.first<2>();
        static_assert(std::is_same_v<decltype(cf2), const SkSpan<const int, 2>>);
        static_assert(cf2.size() == 2 && *cf2.data() == 10 && *(cf2.data() + 1) == 20);
        constexpr auto cl2 = cs.last<2>();
        static_assert(std::is_same_v<decltype(cl2), const SkSpan<const int, 2>>);
        static_assert(cl2.size() == 2 && *cl2.data() == 30 && *(cl2.data() + 1) == 40);
        constexpr auto csub = cs.subspan<1, 2>();
        static_assert(std::is_same_v<decltype(csub), const SkSpan<const int, 2>>);
        static_assert(csub.size() == 2 && *csub.data() == 20 && *(csub.data() + 1) == 30);
        constexpr auto ctail = cs.subspan<4>();
        static_assert(std::is_same_v<decltype(ctail), const SkSpan<const int, 0>>);
        static_assert(ctail.empty());
    }

    {
        int data[5] = {1, 2, 3, 4, 5};
        SkSpan<int, 5> s(data);

        auto f0 = s.first<0>();
        static_assert(std::is_same_v<decltype(f0), SkSpan<int, 0>>);
        REPORTER_ASSERT(reporter, f0.empty() && f0.data() == data);

        auto f2 = s.first<2>();
        static_assert(std::is_same_v<decltype(f2), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, f2[0] == 1 && f2[1] == 2);

        auto l0 = s.last<0>();
        static_assert(std::is_same_v<decltype(l0), SkSpan<int, 0>>);
        REPORTER_ASSERT(reporter, l0.empty() && l0.data() == data + 5);

        auto l3 = s.last<3>();
        static_assert(std::is_same_v<decltype(l3), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, l3[0] == 3 && l3[2] == 5);

        auto subFixed = s.subspan<1, 3>();
        static_assert(std::is_same_v<decltype(subFixed), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, subFixed[0] == 2 && subFixed[2] == 4);

        auto subTail = s.subspan<2>();
        static_assert(std::is_same_v<decltype(subTail), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, subTail[0] == 3 && subTail[2] == 5);

        // Runtime subviews on a fixed-extent span return dynamic SkSpan<T>.
        auto rtFirst = s.first(2);
        static_assert(std::is_same_v<decltype(rtFirst), SkSpan<int>>);
        REPORTER_ASSERT(reporter, rtFirst.size() == 2 && rtFirst[1] == 2);

        auto rtLast = s.last(2);
        static_assert(std::is_same_v<decltype(rtLast), SkSpan<int>>);
        REPORTER_ASSERT(reporter, rtLast.size() == 2 && rtLast[0] == 4);

        auto rtSub = s.subspan(1, 3);
        static_assert(std::is_same_v<decltype(rtSub), SkSpan<int>>);
        REPORTER_ASSERT(reporter, rtSub.size() == 3 && rtSub[0] == 2);

        auto rtTail = s.subspan(3);
        static_assert(std::is_same_v<decltype(rtTail), SkSpan<int>>);
        REPORTER_ASSERT(reporter, rtTail.size() == 2 && rtTail[0] == 4);

        // Compile-time subviews on a dynamic SkSpan<T>.
        SkSpan<int> dyn = s;
        auto dynFirst = dyn.first<2>();
        static_assert(std::is_same_v<decltype(dynFirst), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, dynFirst[0] == 1 && dynFirst[1] == 2);

        auto dynLast = dyn.last<2>();
        static_assert(std::is_same_v<decltype(dynLast), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, dynLast[0] == 4 && dynLast[1] == 5);

        auto dynSubFixed = dyn.subspan<1, 2>();
        static_assert(std::is_same_v<decltype(dynSubFixed), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, dynSubFixed[0] == 2 && dynSubFixed[1] == 3);

        auto dynSubTail = dyn.subspan<2>();
        static_assert(std::is_same_v<decltype(dynSubTail), SkSpan<int>>);
        REPORTER_ASSERT(reporter, dynSubTail.size() == 3 && dynSubTail[0] == 3);
    }

    // 7. SkSpanPriv::EQ and SkSpanPriv::Copy with fixed-extent and mixed-extent spans.
    {
        int a[4] = {1, 2, 3, 4};
        int b[4] = {1, 2, 3, 4};
        int c[3] = {1, 2, 3};
        int dst[4] = {0, 0, 0, 0};

        auto spanA = SkSpan(a);      // SkSpan<int, 4>
        auto spanB = SkSpan(b);      // SkSpan<int, 4>
        auto spanC = SkSpan(c);      // SkSpan<int, 3>
        SkSpan<int> dynB = spanB;    // SkSpan<int>
        auto spanDst = SkSpan(dst);  // SkSpan<int, 4>

        REPORTER_ASSERT(reporter, SkSpanPriv::EQ(spanA, spanB));
        REPORTER_ASSERT(reporter, SkSpanPriv::EQ(spanA, dynB));
        REPORTER_ASSERT(reporter, !SkSpanPriv::EQ(spanA, spanC));

        SkSpanPriv::Copy(spanDst, spanA);
        REPORTER_ASSERT(reporter, SkSpanPriv::EQ(spanDst, spanA));
    }
}
