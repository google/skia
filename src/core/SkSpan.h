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
#endif  // SkSpan_DEFINED
