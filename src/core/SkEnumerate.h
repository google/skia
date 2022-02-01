/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEnumerate_DEFINED
#define SkEnumerate_DEFINED

#include <cstddef>
#include <iterator>
#include <tuple>
#include <variant>

template <typename Iter, typename C = std::monostate>
class SkEnumerate {
    using Captured = decltype(*std::declval<Iter>());
    template <typename> struct is_tuple : std::false_type {};
    template <typename... T> struct is_tuple<std::tuple<T...>> : std::true_type {};

    // v must be a r-value to bind to temporary non-const references.
    static constexpr auto MakeResult(size_t i, Captured&& v) {
        if constexpr (is_tuple<Captured>::value) {
            return std::tuple_cat(std::tuple<size_t>{i}, v);
        } else {
            // Capture v by reference instead of by value by using std::tie.
            return std::tuple_cat(std::tuple<size_t>{i}, std::tie(v));
        }
    }

    using Result = decltype(MakeResult(0, std::declval<Captured>()));

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
        constexpr reference operator*() { return MakeResult(fIndex, *fIt); }

    private:
        ptrdiff_t fIndex;
        Iter fIt;
    };

public:
    constexpr SkEnumerate(Iter begin, Iter end) : SkEnumerate{0, begin, end} {}
    explicit constexpr SkEnumerate(C&& c)
            : fCollection{std::move(c)}
            , fBeginIndex{0}
            , fBegin{std::begin(fCollection)}
            , fEnd{std::end(fCollection)} { }
    constexpr SkEnumerate(const SkEnumerate& that) = default;
    constexpr SkEnumerate& operator=(const SkEnumerate& that) {
        fBegin = that.fBegin;
        fEnd = that.fEnd;
        return *this;
    }
    constexpr Iterator begin() const { return Iterator{fBeginIndex, fBegin}; }
    constexpr Iterator end() const { return Iterator{fBeginIndex + this->ssize(), fEnd}; }
    constexpr bool empty() const { return fBegin == fEnd; }
    constexpr size_t size() const { return std::distance(fBegin,  fEnd); }
    constexpr ptrdiff_t ssize() const { return std::distance(fBegin,  fEnd); }
    constexpr SkEnumerate first(size_t n) {
        SkASSERT(n <= this->size());
        ptrdiff_t deltaEnd = this->ssize() - n;
        return SkEnumerate{fBeginIndex, fBegin, std::prev(fEnd, deltaEnd)};
    }
    constexpr SkEnumerate last(size_t n) {
        SkASSERT(n <= this->size());
        ptrdiff_t deltaBegin = this->ssize() - n;
        return SkEnumerate{fBeginIndex + deltaBegin, std::next(fBegin, deltaBegin), fEnd};
    }
    constexpr SkEnumerate subspan(size_t offset, size_t count) {
        SkASSERT(offset < this->size());
        SkASSERT(count <= this->size() - offset);
        auto newBegin = std::next(fBegin, offset);
        return SkEnumerate(fBeginIndex + offset, newBegin, std::next(newBegin, count));
    }

private:
    constexpr SkEnumerate(ptrdiff_t beginIndex, Iter begin, Iter end)
        : fBeginIndex{beginIndex}
        , fBegin(begin)
        , fEnd(end) {}

    C fCollection;
    const ptrdiff_t fBeginIndex;
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
#endif  // SkEnumerate_DEFINED
