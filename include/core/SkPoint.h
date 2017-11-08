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
    int16_t fX;
    int16_t fY;

    static constexpr SkIPoint16 Make(int x, int y) {
        return {SkToS16(x), SkToS16(y)};
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
    int32_t fX;
    int32_t fY;

    static constexpr SkIPoint Make(int32_t x, int32_t y) {
        return {x, y};
    }

    int32_t x() const { return fX; }
    int32_t y() const { return fY; }

    /**
     *  Returns true iff fX and fY are both zero.
     */
    bool isZero() const { return (fX | fY) == 0; }

    /** Set the x and y values of the point. */
    void set(int32_t x, int32_t y) {
        fX = x;
        fY = y;
    }

    /** Return a new point whose X and Y coordinates are the negative of the
        original point's
    */
    SkIPoint operator-() const {
        return {-fX, -fY};
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
        return {a.fX - b.fX, a.fY - b.fY};
    }

    /** Returns a new point whose coordinates are the sum of a and b (a + b)
    */
    friend SkIPoint operator+(const SkIPoint& a, const SkIPoint& b) {
        return {a.fX + b.fX, a.fY + b.fY};
    }
};

struct SK_API SkPoint {
    SkScalar    fX;
    SkScalar    fY;

    static constexpr SkPoint Make(SkScalar x, SkScalar y) {
        return {x, y};
    }

    SkScalar x() const { return fX; }
    SkScalar y() const { return fY; }

    /**
     *  Returns true iff fX and fY are both zero.
     */
    bool isZero() const { return (0 == fX) & (0 == fY); }

    /** Set the point's X and Y coordinates */
    void set(SkScalar x, SkScalar y) {
        fX = x;
        fY = y;
    }

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

    /** Set the point (vector) to be unit-length in the same direction as it
        already points.  If the point has a degenerate length (i.e. nearly 0)
        then set it to (0,0) and return false; otherwise return true.
    */
    bool normalize();

    /** Set the point (vector) to be unit-length in the same direction as the
        x,y params. If the vector (x,y) has a degenerate length (i.e. nearly 0)
        then set it to (0,0) and return false, otherwise return true.
    */
    bool setNormalize(SkScalar x, SkScalar y);

    /** Scale the point (vector) to have the specified length, and return that
        length. If the original length is degenerately small (nearly zero),
        set it to (0,0) and return false, otherwise return true.
    */
    bool setLength(SkScalar length);

    /** Set the point (vector) to have the specified length in the same
     direction as (x,y). If the vector (x,y) has a degenerate length
     (i.e. nearly 0) then set it to (0,0) and return false, otherwise return true.
    */
    bool setLength(SkScalar x, SkScalar y, SkScalar length);

    /** Scale the point's coordinates by scale, writing the answer into dst.
        It is legal for dst == this.
    */
    void scale(SkScalar scale, SkPoint* dst) const;

    /** Scale the point's coordinates by scale, writing the answer back into
        the point.
    */
    void scale(SkScalar value) { this->scale(value, this); }

    /** Negate the point's coordinates
    */
    void negate() {
        fX = -fX;
        fY = -fY;
    }

    /** Returns a new point whose coordinates are the negative of the point's
    */
    SkPoint operator-() const {
        return {-fX, -fY};
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

    SkPoint operator*(SkScalar scale) const {
        return {fX * scale, fY * scale};
    }

    SkPoint& operator*=(SkScalar scale) {
        fX *= scale;
        fY *= scale;
        return *this;
    }

    /**
     *  Returns true if both X and Y are finite (not infinity or NaN)
     */
    bool isFinite() const {
        SkScalar accum = 0;
        accum *= fX;
        accum *= fY;

        // accum is either NaN or it is finite (zero).
        SkASSERT(0 == accum || SkScalarIsNaN(accum));

        // value==value will be true iff value is not NaN
        // TODO: is it faster to say !accum or accum==accum?
        return !SkScalarIsNaN(accum);
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

    /** Returns a new point whose coordinates are the difference between
        a's and b's (a - b)
    */
    friend SkPoint operator-(const SkPoint& a, const SkPoint& b) {
        return {a.fX - b.fX, a.fY - b.fY};
    }

    /** Returns a new point whose coordinates are the sum of a's and b's (a + b)
    */
    friend SkPoint operator+(const SkPoint& a, const SkPoint& b) {
        return {a.fX + b.fX, a.fY + b.fY};
    }

    /** Returns the euclidian distance from (0,0) to (x,y)
    */
    static SkScalar Length(SkScalar x, SkScalar y);

    /** Normalize pt, returning its previous length. If the prev length is too
        small (degenerate), set pt to (0,0) and return 0. This uses the same
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

};

typedef SkPoint SkVector;

#endif
