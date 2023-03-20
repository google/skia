/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"

#include "include/private/base/SkAssert.h"

#include <cmath>

static inline int64_t double_to_twos_complement_bits(double x) {
    // Convert a double to its bit pattern
    int64_t bits = 0;
    static_assert(sizeof(x) == sizeof(bits));
    std::memcpy(&bits, &x, sizeof(bits));
    // Convert a sign-bit int (i.e. double interpreted as int) into a 2s complement
    // int. This also converts -0 (0x8000000000000000) to 0. Doing this to a double allows
    // it to be compared using normal C operators (<, <=, etc.)
    if (bits < 0) {
        bits &= 0x7FFFFFFFFFFFFFFF;
        bits = -bits;
    }
    return bits;
}

// Arbitrarily chosen.
constexpr static double sk_double_epsilon = 0.0000000001;

bool sk_doubles_nearly_equal_ulps(double a, double b, uint8_t max_ulps_diff) {
    // If both of these are zero (or very close), then using Units of Least Precision
    // will not be accurate and we should use sk_double_nearly_zero instead.
    SkASSERT(!(fabs(a) < sk_double_epsilon && fabs(b) < sk_double_epsilon));
    // This algorithm does not work if both inputs are NaN.
    SkASSERT(!(std::isnan(a) && std::isnan(b)));
    // If both inputs are infinity (or actually equal), this catches it.
    if (a == b) {
        return true;
    }
    int64_t aBits = double_to_twos_complement_bits(a);
    int64_t bBits = double_to_twos_complement_bits(b);

    // Find the difference in Units of Least Precision (ULPs).
    return aBits < bBits + max_ulps_diff && bBits < aBits + max_ulps_diff;
}

bool sk_double_nearly_zero(double a) {
    return a == 0 || fabs(a) < sk_double_epsilon;
}
