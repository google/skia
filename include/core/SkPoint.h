/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPoint_DEFINED
#define SkPoint_DEFINED

#include "SkMath.h"
#include "SkScalar.h"

/** \struct SkIPoint16

    SkIPoint holds two 16 bit integer coordinates
*/
struct SkIPoint16 {
    int16_t fX, fY;

    static SkIPoint16 Make(int x, int y) {
        SkIPoint16 pt;
        pt.set(x, y);
        return pt;
    }

    int16_t x() const { return fX; }
    int16_t y() const { return fY; }

    void set(int x, int y) {
        fX = SkToS16(x);
        fY = SkToS16(y);
    }
};

/** \struct SkIPoint

    SkIPoint holds two 32 bit integer coordinates
*/
struct SkIPoint {
    int32_t fX, fY;

    static SkIPoint Make(int32_t x, int32_t y) {
        SkIPoint pt;
        pt.set(x, y);
        return pt;
    }

    int32_t x() const { return fX; }
    int32_t y() const { return fY; }
    void setX(int32_t x) { fX = x; }
    void setY(int32_t y) { fY = y; }

    /**
     *  Returns true iff fX and fY are both zero.
     */
    bool isZero() const { return (fX | fY) == 0; }

    /**
     *  Set both fX and fY to zero. Same as set(0, 0)
     */
    void setZero() { fX = fY = 0; }

    /** Set the x and y values of the point. */
    void set(int32_t x, int32_t y) { fX = x; fY = y; }

    /** Rotate the point clockwise, writing the new point into dst
        It is legal for dst == this
    */
    void rotateCW(SkIPoint* dst) const;

    /** Rotate the point clockwise, writing the new point back into the point
    */

    void rotateCW() { this->rotateCW(this); }

    /** Rotate the point counter-clockwise, writing the new point into dst.
        It is legal for dst == this
    */
    void rotateCCW(SkIPoint* dst) const;

    /** Rotate the point counter-clockwise, writing the new point back into
        the point
    */
    void rotateCCW() { this->rotateCCW(this); }

    /** Negate the X and Y coordinates of the point.
    */
    void negate() { fX = -fX; fY = -fY; }

    /** Return a new point whose X and Y coordinates are the negative of the
        original point's
    */
    SkIPoint operator-() const {
        SkIPoint neg;
        neg.fX = -fX;
        neg.fY = -fY;
        return neg;
    }

    /** Add v's coordinates to this point's */
    void operator+=(const SkIPoint& v) {
        fX += v.fX;
        fY += v.fY;
    }

    /** Subtract v's coordinates from this point's */
    void operator-=(const SkIPoint& v) {
        fX -= v.fX;
        fY -= v.fY;
    }

    /** Returns true if the point's coordinates equal (x,y) */
    bool equals(int32_t x, int32_t y) const {
        return fX == x && fY == y;
    }

    friend bool operator==(const SkIPoint& a, const SkIPoint& b) {
        return a.fX == b.fX && a.fY == b.fY;
    }

    friend bool operator!=(const SkIPoint& a, const SkIPoint& b) {
        return a.fX != b.fX || a.fY != b.fY;
    }

    /** Returns a new point whose coordinates are the difference between
        a and b (i.e. a - b)
    */
    friend SkIPoint operator-(const SkIPoint& a, const SkIPoint& b) {
        SkIPoint v;
        v.set(a.fX - b.fX, a.fY - b.fY);
        return v;
    }

    /** Returns a new point whose coordinates are the sum of a and b (a + b)
    */
    friend SkIPoint operator+(const SkIPoint& a, const SkIPoint& b) {
        SkIPoint v;
        v.set(a.fX + b.fX, a.fY + b.fY);
        return v;
    }

    /** Returns the dot product of a and b, treating them as 2D vectors
    */
    static int32_t DotProduct(const SkIPoint& a, const SkIPoint& b) {
        return a.fX * b.fX + a.fY * b.fY;
    }

    /** Returns the cross product of a and b, treating them as 2D vectors
    */
    static int32_t CrossProduct(const SkIPoint& a, const SkIPoint& b) {
        return a.fX * b.fY - a.fY * b.fX;
    }
};

struct SK_API SkPoint {
    SkScalar    fX, fY;

    static SkPoint Make(SkScalar x, SkScalar y) {
        SkPoint pt;
        pt.set(x, y);
        return pt;
    }

    SkScalar x() const { return fX; }
    SkScalar y() const { return fY; }

    /**
     *  Returns true iff fX and fY are both zero.
     */
    bool isZero() const { return (0 == fX) & (0 == fY); }

    /** Set the point's X and Y coordinates */
    void set(SkScalar x, SkScalar y) { fX = x; fY = y; }

    /** Set the point's X and Y coordinates by automatically promoting (x,y) to
        SkScalar values.
    */
    void iset(int32_t x, int32_t y) {
        fX = SkIntToScalar(x);
        fY = SkIntToScalar(y);
    }

    /** Set the point's X and Y coordinates by automatically promoting p's
        coordinates to SkScalar values.
    */
    void iset(const SkIPoint& p) {
        fX = SkIntToScalar(p.fX);
        fY = SkIntToScalar(p.fY);
    }

    void setAbs(const SkPoint& pt) {
        fX = SkScalarAbs(pt.fX);
        fY = SkScalarAbs(pt.fY);
    }

    // counter-clockwise fan
    void setIRectFan(int l, int t, int r, int b) {
        SkPoint* v = this;
        v[0].set(SkIntToScalar(l), SkIntToScalar(t));
        v[1].set(SkIntToScalar(l), SkIntToScalar(b));
        v[2].set(SkIntToScalar(r), SkIntToScalar(b));
        v[3].set(SkIntToScalar(r), SkIntToScalar(t));
    }
    void setIRectFan(int l, int t, int r, int b, size_t stride);

    // counter-clockwise fan
    void setRectFan(SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
        SkPoint* v = this;
        v[0].set(l, t);
        v[1].set(l, b);
        v[2].set(r, b);
        v[3].set(r, t);
    }
    void setRectFan(SkScalar l, SkScalar t, SkScalar r, SkScalar b, size_t stride);

    static void Offset(SkPoint points[], int count, const SkPoint& offset) {
        Offset(points, count, offset.fX, offset.fY);
    }

    static void Offset(SkPoint points[], int count, SkScalar dx, SkScalar dy) {
        for (int i = 0; i < count; ++i) {
            points[i].offset(dx, dy);
        }
    }

    void offset(SkScalar dx, SkScalar dy) {
        fX += dx;
        fY += dy;
    }

    /** Return the euclidian distance from (0,0) to the point
    */
    SkScalar length() const { return SkPoint::Length(fX, fY); }
    SkScalar distanceToOrigin() const { return this->length(); }

    /**
     *  Return true if the computed length of the vector is >= the internal
     *  tolerance (used to avoid dividing by tiny values).
     */
    static bool CanNormalize(SkScalar dx, SkScalar dy) {
        // Simple enough (and performance critical sometimes) so we inline it.
        return (dx*dx + dy*dy) > (SK_ScalarNearlyZero * SK_ScalarNearlyZero);
    }

    bool canNormalize() const {
        return CanNormalize(fX, fY);
    }

    /** Set the point (vector) to be unit-length in the same direction as it
        already points.  If the point has a degenerate length (i.e. nearly 0)
        then return false and do nothing; otherwise return true.
    */
    bool normalize();

    /** Set the point (vector) to be unit-length in the same direction as the
        x,y params. If the vector (x,y) has a degenerate length (i.e. nearly 0)
        then return false and do nothing, otherwise return true.
    */
    bool setNormalize(SkScalar x, SkScalar y);

    /** Scale the point (vector) to have the specified length, and return that
        length. If the original length is degenerately small (nearly zero),
        do nothing and return false, otherwise return true.
    */
    bool setLength(SkScalar length);

    /** Set the point (vector) to have the specified length in the same
     direction as (x,y). If the vector (x,y) has a degenerate length
     (i.e. nearly 0) then return false and do nothing, otherwise return true.
    */
    bool setLength(SkScalar x, SkScalar y, SkScalar length);

    /** Same as setLength, but favoring speed over accuracy.
    */
    bool setLengthFast(SkScalar length);

    /** Same as setLength, but favoring speed over accuracy.
    */
    bool setLengthFast(SkScalar x, SkScalar y, SkScalar length);

    /** Scale the point's coordinates by scale, writing the answer into dst.
        It is legal for dst == this.
    */
    void scale(SkScalar scale, SkPoint* dst) const;

    /** Scale the point's coordinates by scale, writing the answer back into
        the point.
    */
    void scale(SkScalar value) { this->scale(value, this); }

    /** Rotate the point clockwise by 90 degrees, writing the answer into dst.
        It is legal for dst == this.
    */
    void rotateCW(SkPoint* dst) const;

    /** Rotate the point clockwise by 90 degrees, writing the answer back into
        the point.
    */
    void rotateCW() { this->rotateCW(this); }

    /** Rotate the point counter-clockwise by 90 degrees, writing the answer
        into dst. It is legal for dst == this.
    */
    void rotateCCW(SkPoint* dst) const;

    /** Rotate the point counter-clockwise by 90 degrees, writing the answer
        back into the point.
    */
    void rotateCCW() { this->rotateCCW(this); }

    /** Negate the point's coordinates
    */
    void negate() {
        fX = -fX;
        fY = -fY;
    }

    /** Returns a new point whose coordinates are the negative of the point's
    */
    SkPoint operator-() const {
        SkPoint neg;
        neg.fX = -fX;
        neg.fY = -fY;
        return neg;
    }

    /** Add v's coordinates to the point's
    */
    void operator+=(const SkPoint& v) {
        fX += v.fX;
        fY += v.fY;
    }

    /** Subtract v's coordinates from the point's
    */
    void operator-=(const SkPoint& v) {
        fX -= v.fX;
        fY -= v.fY;
    }

    /**
     *  Returns true if both X and Y are finite (not infinity or NaN)
     */
    bool isFinite() const {
        SkScalar accum = 0;
        accum *= fX;
        accum *= fY;

        // accum is either NaN or it is finite (zero).
        SkASSERT(0 == accum || !(accum == accum));

        // value==value will be true iff value is not NaN
        // TODO: is it faster to say !accum or accum==accum?
        return accum == accum;
    }

    /**
     *  Returns true if the point's coordinates equal (x,y)
     */
    bool equals(SkScalar x, SkScalar y) const {
        return fX == x && fY == y;
    }

    friend bool operator==(const SkPoint& a, const SkPoint& b) {
        return a.fX == b.fX && a.fY == b.fY;
    }

    friend bool operator!=(const SkPoint& a, const SkPoint& b) {
        return a.fX != b.fX || a.fY != b.fY;
    }

    /** Return true if this point and the given point are far enough apart
        such that a vector between them would be non-degenerate.

        WARNING: Unlike the explicit tolerance version,
        this method does not use componentwise comparison.  Instead, it
        uses a comparison designed to match judgments elsewhere regarding
        degeneracy ("points A and B are so close that the vector between them
        is essentially zero").
    */
    bool equalsWithinTolerance(const SkPoint& p) const {
        return !CanNormalize(fX - p.fX, fY - p.fY);
    }

    /** WARNING: There is no guarantee that the result will reflect judgments
        elsewhere regarding degeneracy ("points A and B are so close that the
        vector between them is essentially zero").
    */
    bool equalsWithinTolerance(const SkPoint& p, SkScalar tol) const {
        return SkScalarNearlyZero(fX - p.fX, tol)
               && SkScalarNearlyZero(fY - p.fY, tol);
    }

    /** Returns a new point whose coordinates are the difference between
        a's and b's (a - b)
    */
    friend SkPoint operator-(const SkPoint& a, const SkPoint& b) {
        SkPoint v;
        v.set(a.fX - b.fX, a.fY - b.fY);
        return v;
    }

    /** Returns a new point whose coordinates are the sum of a's and b's (a + b)
    */
    friend SkPoint operator+(const SkPoint& a, const SkPoint& b) {
        SkPoint v;
        v.set(a.fX + b.fX, a.fY + b.fY);
        return v;
    }

    /** Returns the euclidian distance from (0,0) to (x,y)
    */
    static SkScalar Length(SkScalar x, SkScalar y);

    /** Normalize pt, returning its previous length. If the prev length is too
        small (degenerate), return 0 and leave pt unchanged. This uses the same
        tolerance as CanNormalize.

        Note that this method may be significantly more expensive than
        the non-static normalize(), because it has to return the previous length
        of the point.  If you don't need the previous length, call the
        non-static normalize() method instead.
     */
    static SkScalar Normalize(SkPoint* pt);

    /** Returns the euclidian distance between a and b
    */
    static SkScalar Distance(const SkPoint& a, const SkPoint& b) {
        return Length(a.fX - b.fX, a.fY - b.fY);
    }

    /** Returns the dot product of a and b, treating them as 2D vectors
    */
    static SkScalar DotProduct(const SkPoint& a, const SkPoint& b) {
        return a.fX * b.fX + a.fY * b.fY;
    }

    /** Returns the cross product of a and b, treating them as 2D vectors
    */
    static SkScalar CrossProduct(const SkPoint& a, const SkPoint& b) {
        return a.fX * b.fY - a.fY * b.fX;
    }

    SkScalar cross(const SkPoint& vec) const {
        return CrossProduct(*this, vec);
    }

    SkScalar dot(const SkPoint& vec) const {
        return DotProduct(*this, vec);
    }

    SkScalar lengthSqd() const {
        return DotProduct(*this, *this);
    }

    SkScalar distanceToSqd(const SkPoint& pt) const {
        SkScalar dx = fX - pt.fX;
        SkScalar dy = fY - pt.fY;
        return dx * dx + dy * dy;
    }

    /**
     * The side of a point relative to a line. If the line is from a to b then
     * the values are consistent with the sign of (b-a) cross (pt-a)
     */
    enum Side {
        kLeft_Side  = -1,
        kOn_Side    =  0,
        kRight_Side =  1
    };

    /**
     * Returns the squared distance to the infinite line between two pts. Also
     * optionally returns the side of the line that the pt falls on (looking
     * along line from a to b)
     */
    SkScalar distanceToLineBetweenSqd(const SkPoint& a,
                                      const SkPoint& b,
                                      Side* side = NULL) const;

    /**
     * Returns the distance to the infinite line between two pts. Also
     * optionally returns the side of the line that the pt falls on (looking
     * along the line from a to b)
     */
    SkScalar distanceToLineBetween(const SkPoint& a,
                                   const SkPoint& b,
                                   Side* side = NULL) const {
        return SkScalarSqrt(this->distanceToLineBetweenSqd(a, b, side));
    }

    /**
     * Returns the squared distance to the line segment between pts a and b
     */
    SkScalar distanceToLineSegmentBetweenSqd(const SkPoint& a,
                                             const SkPoint& b) const;

    /**
     * Returns the distance to the line segment between pts a and b.
     */
    SkScalar distanceToLineSegmentBetween(const SkPoint& a,
                                          const SkPoint& b) const {
        return SkScalarSqrt(this->distanceToLineSegmentBetweenSqd(a, b));
    }

    /**
     * Make this vector be orthogonal to vec. Looking down vec the
     * new vector will point in direction indicated by side (which
     * must be kLeft_Side or kRight_Side).
     */
    void setOrthog(const SkPoint& vec, Side side = kLeft_Side) {
        // vec could be this
        SkScalar tmp = vec.fX;
        if (kRight_Side == side) {
            fX = -vec.fY;
            fY = tmp;
        } else {
            SkASSERT(kLeft_Side == side);
            fX = vec.fY;
            fY = -tmp;
        }
    }

    /**
     *  cast-safe way to treat the point as an array of (2) SkScalars.
     */
    const SkScalar* asScalars() const { return &fX; }
};

typedef SkPoint SkVector;

#endif
