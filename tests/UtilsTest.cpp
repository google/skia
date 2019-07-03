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
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

template<typename... Ts>
class SkZip {
    class Iterator {
    public:
        Iterator(const SkZip* zip, size_t index) : fZip{zip}, fIndex{index} { }
        Iterator(const Iterator& that) : Iterator{ that.fZip, that.fIndex } { }
        Iterator& operator++() { ++fIndex; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        std::tuple<Ts&...> operator*() { return (*fZip)[fIndex]; }

    private:
        const SkZip* const fZip = nullptr;
        size_t fIndex = 0;
    };

public:
    SkZip(size_t) = delete;
    explicit SkZip(size_t size, Ts*... pointers) : fPointers{pointers...}, fSize{size} {}

    template<typename... Us>
    explicit SkZip(const SkZip<Us...>& that) : fPointers{that.fPointers}, fSize{that.fSize} {}

    std::tuple<Ts&...> operator[](size_t i) const {
        return this->index(i);
    }

    size_t size() const { return fSize; }
    bool empty() const { return this->size() == 0; }
    size_t size_bytes() const {
        return kRowSize * this->size();
    }
    std::tuple<Ts&...> front() const { return this->index(0); }
    std::tuple<Ts&...> back() const { return this->index(this->size() - 1); }
    Iterator begin() const { return Iterator{this, 0}; }
    Iterator end() const { return Iterator{this, this->size() - 1}; }

private:
    static constexpr size_t sum(std::initializer_list<size_t> l) {
        size_t sum = 0;
        for (size_t v : l) { sum += v; }
        return sum;
    }
    static constexpr size_t kRowSize = sum( {sizeof(Ts)...} );
    std::tuple<Ts&...> index(size_t i) const {
        SkASSERT( this->size() > 0);
        SkASSERT( i < this->size());
        return indexDetail(i, skstd::make_index_sequence<sizeof...(Ts)>{});
    }
    template<std::size_t... Is>
    std::tuple<Ts&...> indexDetail(size_t i, skstd::index_sequence<Is...>) const {
        return std::tuple<Ts&...>(std::get<Is>(fPointers)[i]...);
    }

    std::tuple<Ts*...> fPointers;
    size_t fSize;
};

template<typename... Ts>
class SkZip2 {
    using ReturnTuple = std::tuple<decltype(std::declval<Ts>()[0])...>;

    class Iterator {
    public:
        Iterator(const SkZip2* zip, size_t index) : fZip{zip}, fIndex{index} { }
        Iterator(const Iterator& that) : Iterator{ that.fZip, that.fIndex } { }
        Iterator& operator++() { ++fIndex; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        ReturnTuple operator*() { return (*fZip)[fIndex]; }

    private:
        const SkZip2* const fZip = nullptr;
        size_t fIndex = 0;
    };

public:
    SkZip2(size_t) = delete;
    explicit SkZip2(size_t size, Ts&&... ts)
        : fPointers{std::forward<Ts>(ts)...}
        , fSize{size} {}

    template<typename... Us>
    explicit SkZip2(const SkZip<Us...>& that) : fPointers{that.fPointers}, fSize{that.fSize} {}

    ReturnTuple operator[](size_t i) const {
        return this->index(i);
    }

    size_t size() const { return fSize; }
    bool empty() const { return this->size() == 0; }
    size_t size_bytes() const {
        return kRowSize * this->size();
    }
    std::tuple<Ts&...> front() const { return this->index(0); }
    std::tuple<Ts&...> back() const { return this->index(this->size() - 1); }
    Iterator begin() const { return Iterator{this, 0}; }
    Iterator end() const { return Iterator{this, this->size() - 1}; }

private:
    static constexpr size_t sum(std::initializer_list<size_t> l) {
        size_t sum = 0;
        for (size_t v : l) { sum += v; }
        return sum;
    }
    static constexpr size_t kRowSize = sum( {sizeof(Ts)...} );
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

    SkIota(const SkIota& that) : fBegin{that.fBegin}, fEnd{that.fEnd} {}
    SkIota operator=(const SkIota& that) {
        return SkIota{that.fBegin, that.fEnd};
    }

    size_t size() const { return fEnd - fBegin; }
    bool empty() const { return this->size() == 0; }

    Iterator begin() const { return Iterator{fBegin}; }
    Iterator end() const { return Iterator{fEnd}; }
    size_t operator[](size_t i) { SkASSERT(i <= fEnd); return i; }

private:
    size_t fBegin;
    size_t fEnd;
};

template<typename T> struct Extract{ };

template<typename T> struct Extract<T*> {
    explicit Extract(T* t) : v{t} {}
    using type = T;
    using pointer = type*;
    size_t size() const { return 0; }
    pointer data() const { return v; }
    T* v;
};

template<typename T, size_t N> struct Extract<T(&)[N]> {
    explicit Extract(T(&a)[N]) : v{a} {}
    using type = T;
    using pointer = type*;
    size_t size() const { return N; }
    pointer data() const { return v; }
    T* v;
};

template<> struct Extract<SkIota> {
    explicit Extract(const SkIota i) : iota{i} {}
    using type = size_t;
    using pointer = type*;
    size_t size() const { return iota.size(); }
    pointer data() const { return nullptr; }
    SkIota iota;
};

template<> struct Extract<SkIota&> {
    explicit Extract(const SkIota i) : iota{i} {}
    using type = size_t;
    using pointer = type*;
    size_t size() const { return iota.size(); }
    pointer data() const { return nullptr; }
    SkIota iota;
};

template<typename T> struct Extract<SkSpan<T>> {
    explicit Extract(const SkSpan<T>& t) : v{t} {}
    using type = T;
    using pointer = type*;
    size_t size() const { return v.size(); }
    pointer data() const { return v.data(); }
    const SkSpan<T>& v;
};
template<typename T, size_t N> struct Extract<std::array<T, N>&> {
    explicit Extract(std::array<T, N>& t) : v{t} {}
    using type = T;
    using pointer = type*;
    size_t size() const { return v.size(); }
    pointer data() const { return v.data(); }
    std::array<T, N>& v;
};
// Things with a type parameter and an allocator.
template<template<class, class> class T, typename U, typename V> struct Extract<T<U, V>&> {
    explicit Extract(T<U, V>& t) : v{t} {}
    using type = typename T<U, V>::value_type;
    using pointer = type*;
    size_t size() const { return v.size(); }
    pointer data() const { return v.data(); }
    T<U, V>& v;
};


template<typename C>
struct Size {
    C& c;
    explicit Size(C& container) : c{container} {}
    template<typename T>
    static auto s(T& t) -> decltype(std::declval<T>().size()) {
        return t.size();
    }
    template <typename T, size_t N>
    static size_t s(T(&)[N]) {
        return N;
    }

    template <typename T>
    static size_t s(...) {
        return 0;
    }

    size_t size() const {return s(c); }
};

template<typename... Ts>
SkZip<typename Extract<Ts>::type...> SkMakeZip(Ts&&... ts) {
    size_t minSize = SIZE_MAX;
    size_t maxSize = 0;
    for (size_t s : { Extract<Ts>{ts}.size()... }) {
        if (s != 0) {
            minSize = std::min(minSize, s);
            maxSize = std::max(maxSize, s);
        }
    }
    SkASSERT(maxSize > 0);
    SkASSERT(minSize == maxSize);

    return SkZip<typename Extract<Ts>::type...>{maxSize, Extract<Ts>{ts}.data()...};
}

template<typename... Ts>
auto SkMakeZip2(Ts&&... ts) -> SkZip2<decltype(std::forward<Ts>(ts))...> {
    size_t minSize = SIZE_MAX;
    size_t maxSize = 0;
    for (size_t s : { Size<Ts>{ts}.size()... }) {
        if (s != 0) {
            minSize = std::min(minSize, s);
            maxSize = std::max(maxSize, s);
        }
    }
    SkASSERT(maxSize > 0);
    SkASSERT(minSize == maxSize);

    return SkZip2<decltype(std::forward<Ts>(ts))...>{4, std::forward<Ts>(ts)...};
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
}

DEF_TEST(SkZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    REPORTER_ASSERT(reporter, Size<decltype(A)>{A}.size() == 4);

    float    B[] = {10.f, 20.f, 30.f, 40.f};

    SkZip<uint16_t, float> z{4, A, B};

    {
        auto t = z[0];
        REPORTER_ASSERT(reporter, std::get<0>(t) == 1);
        REPORTER_ASSERT(reporter, std::get<1>(t) == 10.f);
        REPORTER_ASSERT(reporter, &std::get<0>(t) == &A[0]);
        REPORTER_ASSERT(reporter, &std::get<1>(t) == &B[0]);
    }

    {
        auto t = z.back();
        REPORTER_ASSERT(reporter, std::get<0>(t) == 4);
        REPORTER_ASSERT(reporter, std::get<1>(t) == 40.f);
    }

    REPORTER_ASSERT(reporter, z.size_bytes() == 24);

    {
        int i = 0;
        for (auto t : z) {
            REPORTER_ASSERT(reporter, std::get<0>(t) == A[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == B[i]);
            i++;
        }
    }

    SkZip<const uint16_t, const float> q{4, A, B};

    std::tuple<uint16_t*, float*> vv{A, B};
    std::tuple<const uint16_t*, const float*> ww{vv};

    static_assert(std::is_same<int, typename Extract<int*>::type>::value, "bad - pointer");
    static_assert(std::is_same<int, typename Extract<int(&)[5]>::type>::value, "bad - array");
    static_assert(std::is_same<int, typename Extract<SkSpan<int>>::type>::value, "bad - Span");
    static_assert(std::is_same<int,
            typename Extract<std::vector<int>&>::type>::value, "bad - vector");
    static_assert(std::is_same<int,
            typename Extract<std::array<int, 5>&>::type>::value, "bad - std::array");

    static_assert(std::is_same<size_t, typename Extract<SkIota>::type>::value, "bad - SkIota");

    {
        auto zz = SkMakeZip(SkMakeSpan(A), B);

        int i = 0;
        for (auto t : zz) {
            REPORTER_ASSERT(reporter, std::get<0>(t) == A[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == B[i]);
            i++;
        }
    }

    std::vector<int> C = {20, 30, 40, 50};
    {
        auto zz = SkMakeZip(C, B);

        int i = 0;
        for (auto t : zz) {
            REPORTER_ASSERT(reporter, std::get<0>(t) == C[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == B[i]);
            i++;
        }
    }

    std::array<int, 4> S = {{ 100, 200, 300, 400 }};
    {
        auto zz = SkMakeZip(C, S);

        int i = 0;
        for (auto t : zz) {
            REPORTER_ASSERT(reporter, std::get<0>(t) == C[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == S[i]);
            i++;
        }
    }

    {
        auto zz = SkMakeZip(C, S, A, &B[0]);

        int i = 0;
        for (auto t : zz) {
            REPORTER_ASSERT(reporter, std::get<0>(t) == C[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == S[i]);
            REPORTER_ASSERT(reporter, std::get<2>(t) == A[i]);
            REPORTER_ASSERT(reporter, std::get<3>(t) == B[i]);

            i++;
        }

        // Check copy
        auto zzz{zz};
        i = 0;
        for (auto t : zzz) {
            REPORTER_ASSERT(reporter, std::get<0>(t) == C[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == S[i]);
            REPORTER_ASSERT(reporter, std::get<2>(t) == A[i]);
            REPORTER_ASSERT(reporter, std::get<3>(t) == B[i]);

            i++;
        }
    }

#if 1
    {
        SkIota iota{4};
        auto zz = SkMakeZip2(SkMakeSpan(C), S, A, &B[0], iota, SkIota{4});

        for (auto t : zz) {
            size_t i = std::get<4>(t);
            REPORTER_ASSERT(reporter, std::get<0>(t) == C[i]);
            REPORTER_ASSERT(reporter, std::get<1>(t) == S[i]);
            REPORTER_ASSERT(reporter, std::get<2>(t) == A[i]);
            REPORTER_ASSERT(reporter, std::get<3>(t) == B[i]);
        }
    }
#endif
}
