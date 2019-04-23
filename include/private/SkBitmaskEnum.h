/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkEnumOperators_DEFINED
#define SkEnumOperators_DEFINED

#include "include/private/SkTLogic.h"

namespace skstd {
template <typename T> struct is_bitmask_enum : std::false_type {};
}

template <typename E> SK_WHEN(skstd::is_bitmask_enum<E>::value, E) operator|(E l, E r) {
    using U = typename std::underlying_type<E>::type;
    return static_cast<E>(static_cast<U>(l) | static_cast<U>(r));
}

template <typename E> SK_WHEN(skstd::is_bitmask_enum<E>::value, E&) operator|=(E& l, E r) {
    return l = l | r;
}

template <typename E> SK_WHEN(skstd::is_bitmask_enum<E>::value, E) operator&(E l, E r) {
    using U = typename std::underlying_type<E>::type;
    return static_cast<E>(static_cast<U>(l) & static_cast<U>(r));
}

template <typename E> SK_WHEN(skstd::is_bitmask_enum<E>::value, E&) operator&=(E& l, E r) {
    return l = l & r;
}

#endif  // SkEnumOperators_DEFINED
