// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Int96.h"

#include <cstdint>
#include <tuple>

namespace bentleyottmann {

Int96 Int96::Make(int32_t a) {
    return {a >= 0 ? 0 : -1, (uint32_t)a};
}

Int96 Int96::Make(int64_t a) {
    return {a >> 32, (uint32_t)(a & 0xFFFFFFFF)};
}

bool operator== (const Int96& a, const Int96& b) {
    return std::tie(a.hi, a.lo) == std::tie(b.hi, b.lo);
}

bool operator< (const Int96& a, const Int96& b) {
    return std::tie(a.hi, a.lo) < std::tie(b.hi, b.lo);
}

Int96 operator+ (const Int96& a, const Int96& b) {
    uint32_t lo = a.lo + b.lo;
    int64_t carry = lo < a.lo;
    int64_t hi = a.hi + b.hi + carry;
    return {hi, lo};
}


// Multiply a 64-bit int and a 32-bit int producing a 96-bit int.
// This proceeds in two multiplies.
//   1 - unsigned multiply of the low 32-bits of a and the 32-bits of b. Using an unsigned
//   multiply for the lower 32-bits requires a compensation if b is actually negative. The lower
//   bits of a are subtracted from the upper 64-bits of the result.
//   2 - signed multiply of the upper 32-bits of a and the 32-bits of b.
Int96 multiply(int64_t a, int32_t b) {
    // Multiply the low 32-bits generating a 64-bit lower part.
    uint64_t loA = a & 0xFFFFFFFF;
    uint64_t loB = (uint32_t)b;
    uint64_t newLo = loA * loB;

    // Multiply the upper bits 32-bits of a and b resulting in the hi 64-bits of the Int96.
    int64_t newHi = (a >> 32) * (int64_t)b;

    // Calculate the overflow into the upper 32-bits.
    // Remember that newLo is unsigned so will be zero filled by the shift.
    int64_t lowOverflow = newLo >> 32;

    // Add overflow from the low multiply into hi.
    newHi += lowOverflow;

    // Compensate for the negative b in the calculation of newLo by subtracting out 2^32 * a.
    if (b < 0) {
        newHi -= loA;
    }

    return {newHi, (uint32_t)newLo};
}

Int96 multiply(int32_t a, int64_t b) {
    return multiply(b, a);
}

}  // namespace bentleyottmann
