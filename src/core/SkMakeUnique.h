/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMakeUnique_DEFINED
#define SkMakeUnique_DEFINED

#include <memory>

namespace skstd {

// std::make_unique is in C++14
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
std::unique_ptr<T> make_unique_default(size_t n) {
    return std::unique_ptr<T>(new typename std::remove_extent<T>::type[n]);
}

}

#endif  // SkMakeUnique_DEFINED
