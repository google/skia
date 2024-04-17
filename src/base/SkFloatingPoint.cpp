/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

// Return the positive magnitude of a double.
// * normalized - given 1.bbb...bbb x 2^e return 2^e.
// * subnormal - return 0.
// * nan & infinity - return infinity
static double magnitude(double a) {
    static constexpr int64_t extractMagnitude =
            0b0'11111111111'0000000000000000000000000000000000000000000000000000;
    int64_t bits;
    memcpy(&bits, &a, sizeof(bits));
    bits &= extractMagnitude;
    double out;
    memcpy(&out, &bits, sizeof(out));
    return out;
}

bool sk_doubles_nearly_equal_ulps(double a, double b, uint8_t maxUlpsDiff) {

    // The maximum magnitude to construct the ulp tolerance. The proper magnitude for
    // subnormal numbers is minMagnitude, which is 2^-1021, so if a and b are subnormal (having a
    // magnitude of 0) use minMagnitude. If a or b are infinity or nan, then maxMagnitude will be
    // +infinity. This means the tolerance will also be infinity, but the expression b - a below
    // will either be NaN or infinity, so a tolerance of infinity doesn't matter.
    static constexpr double minMagnitude = std::numeric_limits<double>::min();
    const double maxMagnitude = std::max(std::max(magnitude(a), minMagnitude), magnitude(b));

    // Given a magnitude, this is the factor that generates the ulp for that magnitude.
    // In numbers, 2 ^ (-precision + 1) = 2 ^ -52.
    static constexpr double ulpFactor = std::numeric_limits<double>::epsilon();

    // The tolerance in ULPs given the maxMagnitude. Because the return statement must use <
    // for comparison instead of <= to correctly handle infinities, bump maxUlpsDiff up to get
    // the full maxUlpsDiff range.
    const double tolerance = maxMagnitude * (ulpFactor * (maxUlpsDiff + 1));

    // The expression a == b is mainly for handling infinities, but it also catches the exact
    // equals.
    return a == b || std::abs(b - a) < tolerance;
}

bool sk_double_nearly_zero(double a) {
    return a == 0 || fabs(a) < std::numeric_limits<float>::epsilon();
}
