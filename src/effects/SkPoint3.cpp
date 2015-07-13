/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPoint3.h"

// Returns the square of the Euclidian distance to (x,y,z).
static inline float get_length_squared(float x, float y, float z) {
    return x * x + y * y + z * z;
}

// Calculates the square of the Euclidian distance to (x,y,z) and stores it in
// *lengthSquared.  Returns true if the distance is judged to be "nearly zero".
//
// This logic is encapsulated in a helper method to make it explicit that we
// always perform this check in the same manner, to avoid inconsistencies
// (see http://code.google.com/p/skia/issues/detail?id=560 ).
static inline bool is_length_nearly_zero(float x, float y, float z, float *lengthSquared) {
    *lengthSquared = get_length_squared(x, y, z);
    return *lengthSquared <= (SK_ScalarNearlyZero * SK_ScalarNearlyZero);
}

SkScalar SkPoint3::Length(SkScalar x, SkScalar y, SkScalar z) {
    float magSq = get_length_squared(x, y, z);
    if (SkScalarIsFinite(magSq)) {
        return sk_float_sqrt(magSq);
    } else {
        double xx = x;
        double yy = y;
        double zz = z;
        return (float)sqrt(xx * xx + yy * yy + zz * zz);
    }
}

/*
 *  We have to worry about 2 tricky conditions:
 *  1. underflow of magSq (compared against nearlyzero^2)
 *  2. overflow of magSq (compared w/ isfinite)
 *
 *  If we underflow, we return false. If we overflow, we compute again using
 *  doubles, which is much slower (3x in a desktop test) but will not overflow.
 */
bool SkPoint3::normalize() {
    float magSq;
    if (is_length_nearly_zero(fX, fY, fZ, &magSq)) {
        this->set(0, 0, 0);
        return false;
    }

    float scale;
    if (SkScalarIsFinite(magSq)) {
        scale = 1.0f / sk_float_sqrt(magSq);
    } else {
        // our magSq step overflowed to infinity, so use doubles instead.
        // much slower, but needed when x, y or z is very large, otherwise we
        // divide by inf. and return (0,0,0) vector.
        double xx = fX;
        double yy = fY;
        double zz = fZ;
#ifdef SK_CPU_FLUSH_TO_ZERO
        // The iOS ARM processor discards small denormalized numbers to go faster.
        // Casting this to a float would cause the scale to go to zero. Keeping it
        // as a double for the multiply keeps the scale non-zero.
        double dscale = 1.0f / sqrt(xx * xx + yy * yy + zz * zz);
        fX = x * dscale;
        fY = y * dscale;
        fZ = z * dscale;
        return true;
#else
        scale = (float)(1.0f / sqrt(xx * xx + yy * yy + zz * zz));
#endif
    }
    fX *= scale;
    fY *= scale;
    fZ *= scale;
    return true;
}
