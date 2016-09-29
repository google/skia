/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExchange_DEFINED
#define SkExchange_DEFINED

#include <utility>

namespace skstd {

// std::exchange is in C++14
template<typename T, typename U = T>
inline static T exchange(T& obj, U&& new_val) {
    T old_val = std::move(obj);
    obj = std::forward<U>(new_val);
    return old_val;
}

}

#endif  // SkExchange_DEFINED
