/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSafeMath_DEFINED
#define SkSafeMath_DEFINED

#include "SkTFitsIn.h"

namespace SkSafeMath {

/**
 *  Assuming T, U, and V are {uint,int}(8,16,32}_t types, add u + v and set
 *  result.  Iff the sum would overflow, return false.
 */
template <typename T, typename U, typename V>
inline bool Add(T* result, U u, V v) {
    static_assert(std::is_integral<U>::value, "");
    static_assert(std::is_integral<V>::value, "");
    static_assert(std::is_integral<T>::value, "");
    static_assert(sizeof(U) <= 4, "");
    static_assert(sizeof(V) <= 4, "");
    static_assert(sizeof(T) <= 4, "");
    int64_t r = static_cast<int64_t>(u) + static_cast<int64_t>(v);
    if (SkTFitsIn<T>(r)) {
        *result = static_cast<T>(r);
        return true;
    }
    return false;
}

/**
 *  Assuming T, U, and V are {uint,int}(8,16,32}_t types, subtract u - v and set
 *  result.  Iff the difference would overflow, return false.
 */
template <typename T, typename U, typename V>
inline bool Subtract(T* result, U u, V v) {
    static_assert(std::is_integral<U>::value, "");
    static_assert(std::is_integral<V>::value, "");
    static_assert(std::is_integral<T>::value, "");
    static_assert(sizeof(U) <= 4, "");
    static_assert(sizeof(V) <= 4, "");
    static_assert(sizeof(T) <= 4, "");
    int64_t r = static_cast<int64_t>(u) - static_cast<int64_t>(v);
    if (SkTFitsIn<T>(r)) {
        *result = static_cast<T>(r);
        return true;
    }
    return false;
}
}
#endif  // SkSafeMath_DEFINED
