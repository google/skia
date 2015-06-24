
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMathPriv.h"
#include "SkPoint.h"

void SkIPoint::rotateCW(SkIPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    int32_t tmp = fX;
    dst->fX = -fY;
    dst->fY = tmp;
}

void SkIPoint::rotateCCW(SkIPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    int32_t tmp = fX;
    dst->fX = fY;
    dst->fY = -tmp;
}

///////////////////////////////////////////////////////////////////////////////

void SkPoint::setIRectFan(int l, int t, int r, int b, size_t stride) {
    SkASSERT(stride >= sizeof(SkPoint));

    ((SkPoint*)((intptr_t)this + 0 * stride))->set(SkIntToScalar(l),
                                                   SkIntToScalar(t));
    ((SkPoint*)((intptr_t)this + 1 * stride))->set(SkIntToScalar(l),
                                                   SkIntToScalar(b));
    ((SkPoint*)((intptr_t)this + 2 * stride))->set(SkIntToScalar(r),
                                                   SkIntToScalar(b));
    ((SkPoint*)((intptr_t)this + 3 * stride))->set(SkIntToScalar(r),
                                                   SkIntToScalar(t));
}

void SkPoint::rotateCW(SkPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    SkScalar tmp = fX;
    dst->fX = -fY;
    dst->fY = tmp;
}

void SkPoint::rotateCCW(SkPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    SkScalar tmp = fX;
    dst->fX = fY;
    dst->fY = -tmp;
}

void SkPoint::scale(SkScalar scale, SkPoint* dst) const {
    SkASSERT(dst);
    dst->set(SkScalarMul(fX, scale), SkScalarMul(fY, scale));
}

bool SkPoint::normalize() {
    return this->setLength(fX, fY, SK_Scalar1);
}

bool SkPoint::setNormalize(SkScalar x, SkScalar y) {
    return this->setLength(x, y, SK_Scalar1);
}

bool SkPoint::setLength(SkScalar length) {
    return this->setLength(fX, fY, length);
}

// Returns the square of the Euclidian distance to (dx,dy).
static inline float getLengthSquared(float dx, float dy) {
    return dx * dx + dy * dy;
}

// Calculates the square of the Euclidian distance to (dx,dy) and stores it in
// *lengthSquared.  Returns true if the distance is judged to be "nearly zero".
//
// This logic is encapsulated in a helper method to make it explicit that we
// always perform this check in the same manner, to avoid inconsistencies
// (see http://code.google.com/p/skia/issues/detail?id=560 ).
static inline bool isLengthNearlyZero(float dx, float dy,
                                      float *lengthSquared) {
    *lengthSquared = getLengthSquared(dx, dy);
    return *lengthSquared <= (SK_ScalarNearlyZero * SK_ScalarNearlyZero);
}

SkScalar SkPoint::Normalize(SkPoint* pt) {
    float x = pt->fX;
    float y = pt->fY;
    float mag2;
    if (isLengthNearlyZero(x, y, &mag2)) {
        pt->set(0, 0);
        return 0;
    }

    float mag, scale;
    if (SkScalarIsFinite(mag2)) {
        mag = sk_float_sqrt(mag2);
        scale = 1 / mag;
    } else {
        // our mag2 step overflowed to infinity, so use doubles instead.
        // much slower, but needed when x or y are very large, other wise we
        // divide by inf. and return (0,0) vector.
        double xx = x;
        double yy = y;
        double magmag = sqrt(xx * xx + yy * yy);
        mag = (float)magmag;
        // we perform the divide with the double magmag, to stay exactly the
        // same as setLength. It would be faster to perform the divide with
        // mag, but it is possible that mag has overflowed to inf. but still
        // have a non-zero value for scale (thanks to denormalized numbers).
        scale = (float)(1 / magmag);
    }
    pt->set(x * scale, y * scale);
    return mag;
}

SkScalar SkPoint::Length(SkScalar dx, SkScalar dy) {
    float mag2 = dx * dx + dy * dy;
    if (SkScalarIsFinite(mag2)) {
        return sk_float_sqrt(mag2);
    } else {
        double xx = dx;
        double yy = dy;
        return (float)sqrt(xx * xx + yy * yy);
    }
}

/*
 *  We have to worry about 2 tricky conditions:
 *  1. underflow of mag2 (compared against nearlyzero^2)
 *  2. overflow of mag2 (compared w/ isfinite)
 *
 *  If we underflow, we return false. If we overflow, we compute again using
 *  doubles, which is much slower (3x in a desktop test) but will not overflow.
 */
bool SkPoint::setLength(float x, float y, float length) {
    float mag2;
    if (isLengthNearlyZero(x, y, &mag2)) {
        this->set(0, 0);
        return false;
    }

    float scale;
    if (SkScalarIsFinite(mag2)) {
        scale = length / sk_float_sqrt(mag2);
    } else {
        // our mag2 step overflowed to infinity, so use doubles instead.
        // much slower, but needed when x or y are very large, other wise we
        // divide by inf. and return (0,0) vector.
        double xx = x;
        double yy = y;
    #ifdef SK_CPU_FLUSH_TO_ZERO
        // The iOS ARM processor discards small denormalized numbers to go faster.
        // Casting this to a float would cause the scale to go to zero. Keeping it
        // as a double for the multiply keeps the scale non-zero.
        double dscale = length / sqrt(xx * xx + yy * yy);
        fX = x * dscale;
        fY = y * dscale;
        return true;
    #else
        scale = (float)(length / sqrt(xx * xx + yy * yy));
    #endif
    }
    fX = x * scale;
    fY = y * scale;
    return true;
}

bool SkPoint::setLengthFast(float length) {
    return this->setLengthFast(fX, fY, length);
}

bool SkPoint::setLengthFast(float x, float y, float length) {
    float mag2;
    if (isLengthNearlyZero(x, y, &mag2)) {
        this->set(0, 0);
        return false;
    }

    float scale;
    if (SkScalarIsFinite(mag2)) {
        scale = length * sk_float_rsqrt(mag2);  // <--- this is the difference
    } else {
        // our mag2 step overflowed to infinity, so use doubles instead.
        // much slower, but needed when x or y are very large, other wise we
        // divide by inf. and return (0,0) vector.
        double xx = x;
        double yy = y;
        scale = (float)(length / sqrt(xx * xx + yy * yy));
    }
    fX = x * scale;
    fY = y * scale;
    return true;
}


///////////////////////////////////////////////////////////////////////////////

SkScalar SkPoint::distanceToLineBetweenSqd(const SkPoint& a,
                                           const SkPoint& b,
                                           Side* side) const {

    SkVector u = b - a;
    SkVector v = *this - a;

    SkScalar uLengthSqd = u.lengthSqd();
    SkScalar det = u.cross(v);
    if (side) {
        SkASSERT(-1 == SkPoint::kLeft_Side &&
                  0 == SkPoint::kOn_Side &&
                  1 == kRight_Side);
        *side = (Side) SkScalarSignAsInt(det);
    }
    SkScalar temp = det / uLengthSqd;
    temp *= det;
    return temp;
}

SkScalar SkPoint::distanceToLineSegmentBetweenSqd(const SkPoint& a,
                                                  const SkPoint& b) const {
    // See comments to distanceToLineBetweenSqd. If the projection of c onto
    // u is between a and b then this returns the same result as that
    // function. Otherwise, it returns the distance to the closer of a and
    // b. Let the projection of v onto u be v'.  There are three cases:
    //    1. v' points opposite to u. c is not between a and b and is closer
    //       to a than b.
    //    2. v' points along u and has magnitude less than y. c is between
    //       a and b and the distance to the segment is the same as distance
    //       to the line ab.
    //    3. v' points along u and has greater magnitude than u. c is not
    //       not between a and b and is closer to b than a.
    // v' = (u dot v) * u / |u|. So if (u dot v)/|u| is less than zero we're
    // in case 1. If (u dot v)/|u| is > |u| we are in case 3. Otherwise
    // we're in case 2. We actually compare (u dot v) to 0 and |u|^2 to
    // avoid a sqrt to compute |u|.

    SkVector u = b - a;
    SkVector v = *this - a;

    SkScalar uLengthSqd = u.lengthSqd();
    SkScalar uDotV = SkPoint::DotProduct(u, v);

    if (uDotV <= 0) {
        return v.lengthSqd();
    } else if (uDotV > uLengthSqd) {
        return b.distanceToSqd(*this);
    } else {
        SkScalar det = u.cross(v);
        SkScalar temp = det / uLengthSqd;
        temp *= det;
        return temp;
    }
}
