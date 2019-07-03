/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTSearch.h"
#include "src/core/SkTSort.h"
#include "tests/Test.h"

#include <array>

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

#include "include/private/SkTemplates.h"
#include <array>
#include <initializer_list>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

// Take a list of things that have operator[], and use them all in parallel. The iterators and
// accessor operator[] for the class produce a tuple of the items.
template<typename... Ts>
class SkZip {
    using ReturnTuple = std::tuple<decltype(std::declval<Ts>()[0])...>;

    class Iterator {
    public:
        Iterator(const SkZip* zip, size_t index) : fZip{zip}, fIndex{index} { }
        Iterator(const Iterator& that) : Iterator{ that.fZip, that.fIndex } { }
        Iterator& operator++() { ++fIndex; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        ReturnTuple operator*() { return (*fZip)[fIndex]; }

    private:
        const SkZip* const fZip = nullptr;
        size_t fIndex = 0;
    };

public:
    SkZip(size_t) = delete;
    explicit SkZip(size_t size, Ts&&... ts)
        : fPointers{std::forward<Ts>(ts)...}
        , fSize{size} {}

    template<typename... Us>
    explicit SkZip(const SkZip<Us...>& that) : fPointers{that.fPointers}, fSize{that.fSize} {}

    ReturnTuple operator[](size_t i) const {
        return this->index(i);
    }

    size_t size() const { return fSize; }
    bool empty() const { return this->size() == 0; }
    ReturnTuple front() const { return this->index(0); }
    ReturnTuple back() const { return this->index(this->size() - 1); }
    Iterator begin() const { return Iterator{this, 0}; }
    Iterator end() const { return Iterator{this, this->size()}; }

private:
    ReturnTuple index(size_t i) const {
        SkASSERT( this->size() > 0);
        SkASSERT( i < this->size());
        return indexDetail(i, skstd::make_index_sequence<sizeof...(Ts)>{});
    }

    template<std::size_t... Is>
    ReturnTuple indexDetail(size_t i, skstd::index_sequence<Is...>) const {
        return ReturnTuple((std::get<Is>(fPointers))[i]...);
    }

    std::tuple<Ts...> fPointers;
    size_t fSize;
};

// SkIota represents the sequence [begin ... end).
class SkIota {
    class Iterator {
    public:
        Iterator(size_t at) : fAt{at} { }
        Iterator(const Iterator& that) : Iterator{ that.fAt } { }
        Iterator& operator++() { ++fAt; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fAt == rhs.fAt; }
        bool operator!=(const Iterator& rhs) const { return fAt != rhs.fAt; }
        size_t operator*() { return fAt; }

    private:
        size_t fAt;
    };

public:
    SkIota() : fBegin{0}, fEnd{0} {}
    explicit SkIota(size_t end) : fBegin{0}, fEnd{end} {}
    SkIota(size_t begin, size_t end) : fBegin{begin}, fEnd{end} {
        SkASSERT(begin <= end);
    }
    SkIota(const SkIota& that) = default;
    SkIota operator=(const SkIota& that) {
        return SkIota{that.fBegin, that.fEnd};
    }

    size_t size() const { return fEnd - fBegin; }
    bool empty() const { return this->size() == 0; }

    Iterator begin() const { return Iterator{fBegin}; }
    Iterator end() const { return Iterator{fEnd}; }
    size_t operator[](size_t i) const { SkASSERT(i <= fEnd); return i; }

private:
    size_t fBegin;
    size_t fEnd;
};

class SkMakeZipDetail {
    template<typename C> struct Size {
        static decltype(std::declval<C>().size(), size_t()) size(const C& c) { return c.size(); }
    };
    template<typename T, size_t N> struct Size<T(&)[N]> {
        static size_t size(const T(&)[N]) { return N; }
    };
    template<typename T> struct Size<T*> { static size_t size(const T* s) { return 0; } };
    template<typename T> using Size_t = typename Size<T>::type;

    template<typename T> struct Decay { using type = T; };
    template<typename T> struct Decay<T&&> { using type = T; };
    template<typename T> using Decay_t = typename Decay<T>::type;

public:
    template<typename... Ts>
    static auto MakeZip(Ts&& ... ts)
        -> SkZip<Decay_t<decltype(std::forward<Ts>(ts))>...> {
        size_t minSize = SIZE_MAX;
        size_t maxSize = 0;
        for (size_t s : {Size<typename std::remove_const<Ts>::type>::size(ts)...}) {
            if (s != 0) {
                minSize = std::min(minSize, s);
                maxSize = std::max(maxSize, s);
            }
        }
        SkASSERT(maxSize > 0);
        SkASSERT(minSize == maxSize);

        return SkZip<Decay_t<decltype(std::forward<Ts>(ts))>...>{maxSize, std::forward<Ts>(ts)...};
    }

    template<typename... Ts>
    static auto MakeZipWithIota(Ts&& ... ts)
    -> SkZip<SkIota, Decay_t<decltype(std::forward<Ts>(ts))>...> {
        size_t minSize = SIZE_MAX;
        size_t maxSize = 0;
        for (size_t s : {Size<typename std::remove_const<Ts>::type>::size(ts)...}) {
            if (s != 0) {
                minSize = std::min(minSize, s);
                maxSize = std::max(maxSize, s);
            }
        }
        SkASSERT(maxSize > 0);
        SkASSERT(minSize == maxSize);

        return SkZip<SkIota, Decay_t<decltype(std::forward<Ts>(ts))>...>{
            maxSize, SkIota{maxSize}, std::forward<Ts>(ts)...};
    }
};

template<typename... Ts>
inline auto SkMakeZip(Ts&& ... ts) -> decltype(SkMakeZipDetail::MakeZip(std::forward<Ts>(ts)...)) {
    return SkMakeZipDetail::MakeZip(std::forward<Ts>(ts)...);
}

template<typename... Ts>
inline auto SkMakeZipWithIota(Ts&& ... ts)
        -> decltype(SkMakeZipDetail::MakeZipWithIota(std::forward<Ts>(ts)...)) {
    return SkMakeZipDetail::MakeZipWithIota(std::forward<Ts>(ts)...);
}

DEF_TEST(SkMakeSpan, reporter) {
    // Test constness preservation for SkMakeSpan.
    {
        std::vector<int> v = {1, 2, 3, 4, 5};
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> t = {1, 2, 3, 4, 5};
        const std::vector<int>& v = t;
        auto s = SkMakeSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> v = {1, 2, 3, 4, 5};
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> t = {1, 2, 3, 4, 5};
        const std::array<int, 5>& v = t;
        auto s = SkMakeSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }
}

DEF_TEST(SkIota, reporter) {
    SkIota iota{3};

    size_t check = 0;
    for (auto i : iota) {
        REPORTER_ASSERT(reporter, i == check);
        check++;
    }

    for (size_t i = 0; i < iota.size(); i++) {
        REPORTER_ASSERT(reporter, i == iota[i]);
    }

    check = 0;
    for (size_t i : SkIota{3}) {
        REPORTER_ASSERT(reporter, i == check);
        check++;
    }
    REPORTER_ASSERT(reporter, check == 3);
}

DEF_TEST(SkZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    float B[] = {10.f, 20.f, 30.f, 40.f};
    std::vector<int> C = {20, 30, 40, 50};
    std::array<int, 4> D = {{ 100, 200, 300, 400 }};
    SkSpan<int> S = SkMakeSpan(C);

    // Check SkZip calls
    {
        SkZip<uint16_t*, float*, std::vector<int>&, std::array<int, 4>&, SkSpan<int>&>
                z{4, A, B, C, D, S};

        REPORTER_ASSERT(reporter, z.size() == 4);
        REPORTER_ASSERT(reporter, !z.empty());

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
        }

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
    }

    {
        // Check make zip
        auto zz = SkMakeZip(A, B, C, D, S);

        int i = 0;
        for (auto t : zz) {
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
        // Check SkMakeZip in ranged for
        int i = 0;
        for (auto t : SkMakeZip(A, B, C, D, S)) {
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
        // Check value object containers instead of ref passing using SkMakeSpan and SkIota
        for (auto t : SkMakeZip(A, B, C, D, SkMakeSpan(C), SkIota{4})) {
            uint16_t a; float b; int c; int d; int s; int i;
            std::tie(a, b ,c ,d, s, i) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
        }
    }

    {
        // Check value object containers instead of ref passing using SkMakeSpan and SkIota
        int check = 0;
        for (auto t : SkMakeZipWithIota(A, B, C, D, SkMakeSpan(C))) {
            int i; uint16_t a; float b; int c; int d; int s;
            std::tie(i, a, b ,c ,d, s) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
            check++;
        }
        REPORTER_ASSERT(reporter, check == 4);
    }
}
