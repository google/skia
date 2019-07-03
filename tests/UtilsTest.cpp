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
}

// Take a list of things that have operator[], and use them all in parallel. The iterators and
// accessor operator[] for the class produce a tuple of the items.
template<typename... Ts>
class SkZip {
    using ReturnTuple = std::tuple<decltype(std::declval<Ts*>()[0])...>;

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
    explicit SkZip(size_t size, Ts*... ts)
            : fPointers{ts...}
            , fSize{size} {}

    template<typename... Us>
    explicit SkZip(const SkZip<Us...>& that)
            : fPointers{that.fPointers}
            , fSize{that.fSize} {}

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

    std::tuple<Ts*...> fPointers;
    size_t fSize;
};

class SkMakeZipDetail {
    template<typename C> struct Type {
        using type = typename std::remove_pointer<decltype(std::declval<C>().data())>::type;
        static type* ptr(C& c) { return c.data(); }
    };
    template<typename T, size_t N> struct Type<T(&)[N]> {
        using type = T;
        static type* ptr(T* t) { return t; }
    };
    template<typename T> struct Type<T*> {
        using type = T;
        static type* ptr(T* t) { return t; }
    };

    template<typename C, typename... Ts> struct OneSize {
        static size_t Size(C& c, Ts... ts) { return c.size(); }
    };

    template <typename T, typename... Ts> struct OneSize<T*, Ts...> {
        static size_t Size(T* t, Ts... ts) { return OneSize<Ts...>::Size(std::forward<Ts>(ts)...); }
    };

    template <typename T, typename... Ts, size_t N> struct OneSize<T(&)[N], Ts...> {
        static size_t Size(T* t, Ts... ts) { return N; }
    };

#ifdef SK_DEBUG
    template<typename C> struct Size {
        static decltype(std::declval<C>().size(), size_t()) size(const C& c) { return c.size(); }
    };
    template<typename T, size_t N> struct Size<T(&)[N]> {
        static size_t size(const T(&)[N]) { return N; }
    };
    template<typename T> struct Size<T*> { static size_t size(const T* s) { return 0; } };
#endif

public:
    template<typename... Ts>
    static auto MakeZip(Ts&& ... ts) -> SkZip<typename Type<Ts>::type...> {

        // Pick the first collection that has a size, and use that for the size.
        size_t size = OneSize<Ts...>::Size(std::forward<Ts>(ts)...);

#ifdef SK_DEBUG
        // Check that all sizes are the same.
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
#endif

        return SkZip<typename Type<Ts>::type...>{size, Type<Ts>::ptr(std::forward<Ts>(ts))...};
    }
};

template<typename... Ts>
inline auto SkMakeZip(Ts&& ... ts) -> decltype(SkMakeZipDetail::MakeZip(std::forward<Ts>(ts)...)) {
    return SkMakeZipDetail::MakeZip(std::forward<Ts>(ts)...);
}

// SkIota represents the sequence [begin ... end).
template <typename Iter>
class SkIota {
    using Result = std::tuple<size_t, decltype(*std::declval<Iter>())>;

    class Iterator {
    public:
        Iterator(size_t index, Iter it) : fIndex{index}, fIt{it} { }
        Iterator(const Iterator&) = default;
        Iterator operator++() { ++fIndex; ++fIt; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIt == rhs.fIt; }
        bool operator!=(const Iterator& rhs) const { return fIt != rhs.fIt; }
        Result operator*() { return std::forward_as_tuple(fIndex, *fIt); }

    private:
        size_t fIndex;
        Iter fIt;
    };

public:
    explicit SkIota(Iter begin, Iter end) : fBegin{begin}, fEnd{end} {}
    SkIota(const SkIota& that) = default;
    SkIota& operator=(const SkIota& that) { fBegin = that.fBegin;  fEnd = that.fEnd; return *this; }
    Iterator begin() const { return Iterator{0, fBegin}; }
    Iterator end() const { return Iterator{0, fEnd}; }

private:
    Iter fBegin;
    Iter fEnd;
};

template <typename C, typename Iter = decltype(std::begin(std::declval<C>()))>
SkIota<Iter> SkMakeIota(C& c) { return SkIota<Iter>{std::begin(c), std::end(c)}; }

template <typename T, typename Iter = decltype(std::declval<std::initializer_list<T>>().begin())>
SkIota<Iter>
SkMakeIota(std::initializer_list<T> il) { return SkIota<Iter>{il.begin(), il.end()}; }

template <class T, std::size_t N, typename Iter = decltype(std::begin(std::declval<T(&)[N]>()))>
SkIota<Iter> SkMakeIota(T (&a)[N]) { return SkIota<Iter>{std::begin(a), std::end(a)}; }

DEF_TEST(SkIota, reporter) {
    auto iota = SkMakeIota({1, 2, 3, 4});

    size_t check = 0;
    for (auto t : iota) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);

        check++;
    }

    check = 0;
    for (auto t : SkMakeIota({1, 2, 3, 4})) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);

        check++;
    }

    check = 0;
    std::vector<int> vec = {1, 2, 3, 4};
    for (auto t : SkMakeIota(vec)) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);
        check++;
    }
    REPORTER_ASSERT(reporter, check == 4);

    check = 0;
    int q[] = {1, 2, 3, 4};
    for (auto t : SkMakeIota(q)) {
        size_t i; int v;
        std::tie(i, v) = t;
        REPORTER_ASSERT(reporter, i == check);
        REPORTER_ASSERT(reporter, v == (int)check+1);
        check++;
    }
    REPORTER_ASSERT(reporter, check == 4);
}

DEF_TEST(SkZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    float B[] = {10.f, 20.f, 30.f, 40.f};
    std::vector<int> C = {{20, 30, 40, 50}};
    std::array<int, 4> D = {{100, 200, 300, 400}};
    SkSpan<int> S = SkMakeSpan(C);

    // Check SkZip calls
    SkZip<uint16_t, float, int, int, int>
            z{4, &A[0], &B[0], C.data(), D.data(), S.data()};

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


DEF_TEST(SkMakeZip, reporter) {
    uint16_t A[] = {1, 2, 3, 4};
    float B[] = {10.f, 20.f, 30.f, 40.f};
    std::vector<int> C = {{20, 30, 40, 50}};
    std::array<int, 4> D = {{100, 200, 300, 400}};
    SkSpan<int> S = SkMakeSpan(C);
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
        // Check SkMakeZip in ranged for
        auto zz = SkMakeZip(A, B, C, D, S);
        for (auto t : SkMakeIota(zz)) {
            int i;
            uint16_t a; float b; int c; int d; int s;
            //i = std::get<0>(t);
            //std::tie(a, b, c, d, s) = std::get<1>(t);
            std::forward_as_tuple(i, std::tie(a, b, c, d, s)) = t;
            REPORTER_ASSERT(reporter, a == A[i]);
            REPORTER_ASSERT(reporter, b == B[i]);
            REPORTER_ASSERT(reporter, c == C[i]);
            REPORTER_ASSERT(reporter, d == D[i]);
            REPORTER_ASSERT(reporter, s == S[i]);
        }
    }

    {
        // Check SkMakeZip in ranged for
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
        // Check SkMakeZip in ranged for
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
}
