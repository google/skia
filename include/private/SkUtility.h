/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtility_DEFINED
#define SkUtility_DEFINED

#include "SkTLogic.h"

namespace skstd {

template <typename T> inline remove_reference_t<T>&& move(T&& t) {
  return static_cast<remove_reference_t<T>&&>(t);
}

template <typename T> inline T&& forward(remove_reference_t<T>& t) /*noexcept*/ {
    return static_cast<T&&>(t);
}
template <typename T> inline T&& forward(remove_reference_t<T>&& t) /*noexcept*/ {
    static_assert(!is_lvalue_reference<T>::value,
                  "Forwarding an rvalue reference as an lvalue reference is not allowed.");
    return static_cast<T&&>(t);
}

template <typename T> add_rvalue_reference_t<T> declval();

}  // namespace skstd

#endif
