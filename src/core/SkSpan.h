/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpan_DEFINED
#define SkSpan_DEFINED

#include <cstddef>
#include <string>
#include <vector>

template <typename T>
class SkSpan {
public:
    template <size_t N>
    constexpr SkSpan(T(&t)[N]) : fPtr(t), fSize(N) {}
    constexpr SkSpan() : fPtr{nullptr}, fSize{0} {}
    constexpr SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} {}
    template <typename U>
    constexpr explicit SkSpan(std::vector<U>& v) : fPtr{v.data()}, fSize{v.size()} {}
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SkSpan(const SkSpan<U>& that) : fPtr{that.data()}, fSize{that.size()} {}
    constexpr explicit SkSpan(std::string& s) : fPtr{s.c_str()}, fSize{s.size()} {}
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
    constexpr SkSpan<const T> toConst() const { return SkSpan<const T>{fPtr, fSize}; }
    constexpr SkSpan<T> first(size_t prefixLen) { return SkSpan<T>{fPtr, prefixLen}; }

private:
    T* fPtr;
    size_t fSize;
};

template <typename T, typename S>
inline SkSpan<T> SkMakeSpan(T* p, S s) { return SkSpan<T>{p, SkTo<size_t>(s)}; }

template <size_t N, typename T>
inline SkSpan<T> SkMakeSpan(T(&a)[N]) { return SkSpan<T>{a, N}; }

template <typename Collection>
inline auto SkMakeSpan(const Collection& c) -> SkSpan<decltype(c.data())> {
    return SkSpan<decltype(c[0])>{&c[0], c->size()};
}

#endif  // SkSpan_DEFINED
