/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConstexprMath_DEFINED
#define SkConstexprMath_DEFINED

#include "SkTypes.h"
#include <limits.h>

template <uintmax_t N, uintmax_t B>
struct SK_LOG {
    //! Compile-time constant ceiling(logB(N)).
    static const uintmax_t value = 1 + SK_LOG<N/B, B>::value;
};
template <uintmax_t B>
struct SK_LOG<1, B> {
    static const uintmax_t value = 0;
};
template <uintmax_t B>
struct SK_LOG<0, B> {
    static const uintmax_t value = 0;
};

template<uintmax_t N>
struct SK_2N1 {
    //! Compile-time constant (2^N)-1.
    static const uintmax_t value = (SK_2N1<N-1>::value << 1) + 1;
};
template<>
struct SK_2N1<1> {
    static const uintmax_t value = 1;
};

/** Compile-time constant number of base n digits in type t
    if the bits of type t are considered as unsigned base two.
*/
#define SK_BASE_N_DIGITS_IN(n, t) (\
    SK_LOG<SK_2N1<(sizeof(t) * CHAR_BIT)>::value, n>::value\
)
/** Compile-time constant number of base 10 digits in type t
    if the bits of type t are considered as unsigned base two.
*/
#define SK_DIGITS_IN(t) SK_BASE_N_DIGITS_IN(10, (t))

// Compile-time constant maximum value of two unsigned values.
template <uintmax_t a, uintmax_t b> struct SkTUMax {
    static const uintmax_t value = (b < a) ? a : b;
};

#endif
