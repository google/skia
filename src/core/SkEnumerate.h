/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkIota_DEFINED
#define SkIota_DEFINED

#include <cstddef>
#include <iterator>
#include <tuple>

#include "include/private/SkTLogic.h"

// SkEnumerate returns a tuple with an index and the value returned by the iterator. The index always
// starts at 0.
template <typename Iter, typename C = skstd::monostate>
class SkEnumerate {
    using Result = std::tuple<size_t, decltype(*std::declval<Iter>())>;

    class Iterator {
    public:
        using value_type = Result;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::input_iterator_tag;
        constexpr Iterator(ptrdiff_t index, Iter it) : fIndex{index}, fIt{it} { }
        constexpr Iterator(const Iterator&) = default;
        constexpr Iterator operator++() { ++fIndex; ++fIt; return *this; }
        constexpr Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        constexpr bool operator==(const Iterator& rhs) const { return fIt == rhs.fIt; }
        constexpr bool operator!=(const Iterator& rhs) const { return fIt != rhs.fIt; }
        constexpr reference operator*() { return std::forward_as_tuple(fIndex, *fIt); }

    private:
        ptrdiff_t fIndex;
        Iter fIt;
    };

public:
    constexpr SkEnumerate(Iter begin, Iter end) : fBegin{begin}, fEnd{end} { }
    explicit constexpr SkEnumerate(C&& c)
            : fCollection{std::move(c)}
            , fBegin{std::begin(fCollection)}
            , fEnd{std::end(fCollection)} { }
    constexpr SkEnumerate(const SkEnumerate& that) = default;
    constexpr SkEnumerate& operator=(const SkEnumerate& that) {
        fBegin = that.fBegin;
        fEnd = that.fEnd; return *this;
    }
    constexpr Iterator begin() const { return Iterator{0, fBegin}; }
    constexpr Iterator end() const { return Iterator{fEnd - fBegin, fEnd}; }

private:
    C fCollection;
    Iter fBegin;
    Iter fEnd;
};

template <typename C, typename Iter = decltype(std::begin(std::declval<C>()))>
inline constexpr SkEnumerate<Iter> SkMakeEnumerate(C& c) {
    return SkEnumerate<Iter>{std::begin(c), std::end(c)};
}
template <typename C, typename Iter = decltype(std::begin(std::declval<C>()))>
inline constexpr SkEnumerate<Iter, C> SkMakeEnumerate(C&& c) {
    return SkEnumerate<Iter, C>{std::forward<C>(c)};
}

template <class T, std::size_t N, typename Iter = decltype(std::begin(std::declval<T(&)[N]>()))>
inline constexpr SkEnumerate<Iter> SkMakeEnumerate(T (&a)[N]) {
    return SkEnumerate<Iter>{std::begin(a), std::end(a)};
}
#endif  // SkIota_DEFINED
