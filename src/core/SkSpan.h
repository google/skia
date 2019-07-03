/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpan_DEFINED
#define SkSpan_DEFINED

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "include/private/SkTLogic.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"

template <typename T>
class SkSpan {
public:
    constexpr SkSpan() : fPtr{nullptr}, fSize{0} {}
    constexpr SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} {}
    template <typename U, typename = typename std::enable_if<std::is_same<const U, T>::value>::type>
    constexpr SkSpan(const SkSpan<U>& that) : fPtr(that.data()), fSize{that.size()} {}
    constexpr SkSpan(const SkSpan& o) = default;
    constexpr SkSpan& operator=(const SkSpan& that) {
        fPtr = that.fPtr;
        fSize = that.fSize;
        return *this;
    }
    constexpr T& operator [] (size_t i) const { return fPtr[i]; }
    constexpr T& front() const { return fPtr[0]; }
    constexpr T& back()  const { return fPtr[fSize - 1]; }
    constexpr T* begin() const { return fPtr; }
    constexpr T* end() const { return fPtr + fSize; }
    constexpr const T* cbegin() const { return fPtr; }
    constexpr const T* cend() const { return fPtr + fSize; }
    constexpr T* data() const { return fPtr; }
    constexpr size_t size() const { return fSize; }
    constexpr bool empty() const { return fSize == 0; }
    constexpr size_t size_bytes() const { return fSize * sizeof(T); }
    constexpr SkSpan<T> first(size_t prefixLen) { return SkSpan<T>{fPtr, prefixLen}; }

private:
    T* fPtr;
    size_t fSize;
};

template <typename T, typename S>
inline constexpr SkSpan<T> SkMakeSpan(T* p, S s) { return SkSpan<T>{p, SkTo<size_t>(s)}; }

template <size_t N, typename T>
inline constexpr SkSpan<T> SkMakeSpan(T(&a)[N]) { return SkSpan<T>{a, N}; }

template <typename Container>
inline auto SkMakeSpan(Container& c)
        -> SkSpan<typename std::remove_reference<decltype(*(c.data()))>::type> {
    return {c.data(), c.size()};
}

// Take a list of things that can be pointers, and use them all in parallel. The iterators and
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
    SkZip(const SkZip<Us...>& that)
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

// SkIota returns a tuple with an index and the value returned by the iterator. The index always
// starts at 0.
template <typename Iter, typename C = skstd::monostate>
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
    SkIota(Iter begin, Iter end) : fBegin{begin}, fEnd{end} { }
    SkIota(C&& c)
            : fCollection{std::move(c)}
            , fBegin{std::begin(fCollection)}
            , fEnd{std::end(fCollection)} { }
    SkIota(const SkIota& that) = default;
    SkIota& operator=(const SkIota& that) { fBegin = that.fBegin;  fEnd = that.fEnd; return *this; }
    Iterator begin() const { return Iterator{0, fBegin}; }
    Iterator end() const { return Iterator{0, fEnd}; }

private:
    C fCollection;
    Iter fBegin;
    Iter fEnd;
};

template <typename C, typename Iter = decltype(std::begin(std::declval<C>()))>
inline SkIota<Iter> SkMakeIota(C& c) { return SkIota<Iter>{std::begin(c), std::end(c)}; }

template <typename C, typename Iter = decltype(std::begin(std::declval<C>()))>
inline SkIota<Iter, C> SkMakeIota(C&& c) { return SkIota<Iter, C>{std::forward<C>(c)}; }

template <class T, std::size_t N, typename Iter = decltype(std::begin(std::declval<T(&)[N]>()))>
inline SkIota<Iter> SkMakeIota(T (&a)[N]) { return SkIota<Iter>{std::begin(a), std::end(a)}; }
#endif  // SkSpan_DEFINED
