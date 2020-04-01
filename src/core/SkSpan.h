/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpan_DEFINED
#define SkSpan_DEFINED

#include <cstddef>
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
    constexpr auto rbegin() const { return std::make_reverse_iterator(this->end()); }
    constexpr auto rend() const { return std::make_reverse_iterator(this->begin()); }
    constexpr auto crbegin() const { return std::make_reverse_iterator(this->cend()); }
    constexpr auto crend() const { return std::make_reverse_iterator(this->cbegin()); }
    constexpr T* data() const { return fPtr; }
    constexpr int count() const { return SkTo<int>(fSize); }
    constexpr size_t size() const { return fSize; }
    constexpr bool empty() const { return fSize == 0; }
    constexpr size_t size_bytes() const { return fSize * sizeof(T); }
    constexpr SkSpan<T> first(size_t prefixLen) const {
        SkASSERT(prefixLen <= this->size());
        if (prefixLen == 0) { return SkSpan{}; }
        return SkSpan{fPtr, prefixLen};
    }
    constexpr SkSpan<T> last(size_t postfixLen) const {
        SkASSERT(postfixLen <= this->size());
        if (postfixLen == 0) { return SkSpan{}; }
        return SkSpan{fPtr + (this->size() - postfixLen), postfixLen};
    }

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
#endif  // SkSpan_DEFINED
