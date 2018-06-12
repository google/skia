/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTFitsIn_DEFINED
#define SkTFitsIn_DEFINED

/**
 * In C++ an unsigned to signed cast where the source value cannot be represented in the destination
 * type results in an implementation defined destination value. Unlike C, C++ does not allow a trap.
 * This makes "(S)(D)s == s" a possibly useful test. However, there are two cases where this is
 * incorrect:
 *
 * when testing if a value of a smaller signed type can be represented in a larger unsigned type
 * (int8_t)(uint16_t)-1 == -1 => (int8_t)0xFFFF == -1 => [implementation defined] == -1
 *
 * when testing if a value of a larger unsigned type can be represented in a smaller signed type
 * (uint16_t)(int8_t)0xFFFF == 0xFFFF => (uint16_t)-1 == 0xFFFF => 0xFFFF == 0xFFFF => true.
 *
 * Consider the cases:
 * u = unsigned, s = signed, X = more digits, x = less digits
 * ux -> uX: (ux)(uX)ux == ux, trivially true
 * uX -> ux: (uX)(ux)uX == uX, both casts well defined, test works
 * sx -> sX: (sx)(sX)sx == sx, trivially true
 * sX -> sx: (sX)(sx)sX == sX, first cast implementation value, second cast defined, test works
 * sx -> uX: (sx)(uX)sx == sx, this is bad, the second cast results in implementation defined value
 * sX -> ux: (sX)(ux)sX == sX, the second cast is required to prevent promotion of rhs to unsigned
 * ux -> sX: (ux)(sX)ux == ux, trivially true
 * uX -> sx: (uX)(sx)uX == uX, this is bad,
 *                             first cast results in implementation defined value,
 *                             second cast is defined. However, this creates false positives
 *                             uint16_t x = 0xFFFF
 *                                (uint16_t)(int8_t)x == x
 *                             => (uint16_t)-1        == x
 *                             => 0xFFFF              == x
 *                             => true
 *
 * So for the eight cases three are trivially true, three more are valid casts, and two are special.
 * The two 'full' checks which otherwise require two comparisons are valid cast checks.
 * The two remaining checks uX -> sx [uX < max(sx)] and sx -> uX [sx > 0] can be done with one op.
 */

template <typename D, typename S>
constexpr inline bool SkTFitsIn(S src) {
    bool S_is_signed = (S)-1 < (S)0,
         D_is_signed = (D)-1 < (D)0;

    // E.g. (int8_t)(uint8_t) int8_t(-1) == -1, but the uint8_t == 255, not -1.
    if (S_is_signed && !D_is_signed && sizeof(S) <= sizeof(D)) {
        return src >= (S)0;
    }

    // As above in reverse: (uint8_t)(int8_t) uint8_t(255) == 255, but the int8_t == -1.
    if (D_is_signed && !S_is_signed && sizeof(D) <= sizeof(S)) {
        return (D)src >= (D)0;
    }

    return (S)(D)src == src;
}

#endif
