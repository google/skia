/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpan_DEFINED
#define SkSpan_DEFINED

#include <cstddef>
#include <vector>

template <typename T>
class SkSpan {
public:
    SkSpan() : fPtr{nullptr}, fSize{0} {}
    SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} { }
    template <typename U>
    explicit SkSpan(std::vector<U>& v) : fPtr{v.data()}, fSize{v.size()} {}
    SkSpan(const SkSpan<T>& o) = default;
    SkSpan& operator=( const SkSpan& other ) = default;
    T& operator [] (size_t i) const { return fPtr[i]; }
    T* begin() const { return fPtr; }
    T* end() const { return fPtr + fSize; }
    const T* cbegin() const { return fPtr; }
    const T* cend() const { return fPtr + fSize; }
    T* data() const { return fPtr; }
    size_t size() const { return fSize; }
    bool empty() const { return fSize == 0; }
    size_t size_bytes() const { return fSize * sizeof(T); }
    SkSpan<const T> toConst() const { return SkSpan<const T>{fPtr, fSize}; }

private:
    T* fPtr;
    size_t fSize;
};

#endif  // SkSpan_DEFINED
