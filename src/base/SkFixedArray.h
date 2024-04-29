/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTFixedArray_DEFINED
#define SkTFixedArray_DEFINED

#include "include/private/base/SkAssert.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <type_traits>  // IWYU pragma: keep for std::is_trivial_v

namespace skia_private {

/**
 * Represents an array of `T` (must be a trivial type) that cannot grow past a fixed size `N`.
 * The fixed-size restriction allows for tighter codegen and a smaller memory footprint.
 * Missing methods from TArray (e.g. `push_back_n`) can be added on demand.
 *
 * The trivial-type restriction is only to simplify implementation; if there is a need, we can
 * adopt proper move/copy semantics in this class as well.
 */
template <int N, typename T>
class FixedArray {
public:
    using value_type = T;

    FixedArray() = default;

    FixedArray(std::initializer_list<T> values) {
        SkASSERT(values.size() <= N);
        for (T value : values) {
            fData[fSize++] = value;
        }
    }

    FixedArray(int reserveCount) {
        // This is here to satisfy the TArray interface. Setting a reserve count on a fixed array
        // isn't useful.
        SkASSERT(reserveCount >= 0);
        SkASSERT(reserveCount <= N);
    }

    FixedArray(const T* array, int count) {
        this->reset(array, count);
    }

    FixedArray(const FixedArray<N, T>& that) {
        this->reset(that.data(), that.size());
    }

    FixedArray<N, T>& operator=(const FixedArray<N, T>& that) {
        if (this != &that) {
            this->reset(that.data(), that.size());
        }
        return *this;
    }

    T& operator[](size_t index) {
        SkASSERT(index < fSize);
        return fData[index];
    }

    const T& operator[](size_t index) const {
        SkASSERT(index < fSize);
        return fData[index];
    }

    bool operator==(const FixedArray<N, T>& that) const {
        return fSize == that.fSize && (0 == memcmp(fData, that.fData, fSize * sizeof(T)));
    }

    bool operator!=(const FixedArray<N, T>& that) const {
        return !this->operator==(that);
    }

    int size() const {
        return fSize;
    }

    bool empty() const {
        return fSize == 0;
    }

    void clear() {
        fSize = 0;
    }

    void reset(const T* array, int count) {
        SkASSERT(count >= 0);
        SkASSERT(count <= N);
        fSize = count;
        std::memcpy(fData, array, count * sizeof(T));
    }

    void resize(int newSize) {
        SkASSERT(newSize >= 0);
        SkASSERT(newSize <= N);

        if (fSize > newSize) {
            fSize = newSize;
        } else {
            while (fSize < newSize) {
                fData[fSize++] = T();
            }
        }
    }

    T& push_back() {
        SkASSERT(fSize < N);
        T& ref = fData[fSize++];
        ref = T();
        return ref;
    }

    void push_back(T x) {
        SkASSERT(fSize < N);
        fData[fSize++] = x;
    }

    void pop_back() {
        SkASSERT(fSize > 0);
        --fSize;
    }

    void removeShuffle(int n) {
        SkASSERT(n < fSize);
        int last = fSize - 1;
        if (n != last) {
            fData[n] = fData[last];
        }
        fSize = last;
    }

    T* data() {
        return fData;
    }

    const T* data() const {
        return fData;
    }

    T* begin() {
        return fData;
    }

    const T* begin() const {
        return fData;
    }

    T* end() {
        return fData + fSize;
    }

    const T* end() const {
        return fData + fSize;
    }

    T& front() {
        SkASSERT(fSize > 0);
        return fData[0];
    }

    const T& front() const {
        SkASSERT(fSize > 0);
        return fData[0];
    }

    T& back() {
        SkASSERT(fSize > 0);
        return fData[fSize - 1];
    }

    const T& back() const {
        SkASSERT(fSize > 0);
        return fData[fSize - 1];
    }

    void reserve(int size) {
        // This is here to satisfy the TArray interface.
        SkASSERT(size >= 0);
        SkASSERT(size <= N);
    }

    constexpr int capacity() const {
        return N;
    }

private:
    static_assert(std::is_trivial_v<T>);
    static_assert(N > 0);
    static_assert(N < 256);  // limited by `uint8_t fSize`

    T fData[N];
    uint8_t fSize = 0;
};

}  // namespace skia_private

#endif
