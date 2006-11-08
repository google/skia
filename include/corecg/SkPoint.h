/* include/corecg/SkPoint.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkPoint_DEFINED
#define SkPoint_DEFINED

#include "SkMath.h"

/** \struct SkPoint16

    SkPoint16 holds two 16 bit integer coordinates
*/
struct SkPoint16 {
    int16_t fX, fY;

    void set(S16CPU x, S16CPU y) { fX = SkToS16(x); fY = SkToS16(y); }

    /** Rotate the point clockwise, writing the new point into dst
        It is legal for dst == this
    */
    void rotateCW(SkPoint16* dst) const;
    /** Rotate the point clockwise, writing the new point back into the point
    */
    void rotateCW() { this->rotateCW(this); }
    /** Rotate the point counter-clockwise, writing the new point into dst.
        It is legal for dst == this
    */
    void rotateCCW(SkPoint16* dst) const;
    /** Rotate the point counter-clockwise, writing the new point back into the point
    */
    void rotateCCW() { this->rotateCCW(this); }
    /** Negate the X and Y coordinates of the point.
    */
    void negate() { fX = -fX; fY = -fY; }
    /** Return a new point whose X and Y coordinates are the negative of the original point's
    */
    SkPoint16 operator-() const
    {
        SkPoint16 neg;
        neg.fX = -fX;
        neg.fY = -fY;
        return neg;
    }
    /** Add v's coordinates to this point's
    */
    void operator+=(const SkPoint16& v)
    {
        fX = SkToS16(fX + v.fX);
        fY = SkToS16(fY + v.fY);
    }
    /** Subtract v's coordinates from this point's
    */
    void operator-=(const SkPoint16& v)
    {
        fX = SkToS16(fX - v.fX);
        fY = SkToS16(fY - v.fY);
    }
    /** Returns true if the point's coordinates equal (x,y)
    */
    bool equals(S16CPU x, S16CPU y) const { return fX == x && fY == y; }
    friend bool operator==(const SkPoint16& a, const SkPoint16& b)
    {
        return a.fX == b.fX && a.fY == b.fY;
    }
    friend bool operator!=(const SkPoint16& a, const SkPoint16& b)
    {
        return a.fX != b.fX || a.fY != b.fY;
    }
    /** Returns a new point whose coordinates are the difference between a and b (a - b)
    */
    friend SkPoint16 operator-(const SkPoint16& a, const SkPoint16& b)
    {
        SkPoint16 v;
        v.set(a.fX - b.fX, a.fY - b.fY);
        return v;
    }
    /** Returns a new point whose coordinates are the sum of a and b (a + b)
    */
    friend SkPoint16 operator+(const SkPoint16& a, const SkPoint16& b)
    {
        SkPoint16 v;
        v.set(a.fX + b.fX, a.fY + b.fY);
        return v;
    }
    /** Returns the dot product of a and b, treating them as 2D vectors
    */
    static int32_t DotProduct(const SkPoint16& a, const SkPoint16& b)
    {
        return a.fX * b.fX + a.fY * b.fY;
    }
    /** Returns the cross product of a and b, treating them as 2D vectors
    */
    static int32_t CrossProduct(const SkPoint16& a, const SkPoint16& b)
    {
        return a.fX * b.fY - a.fY * b.fX;
    }
};

struct SkPoint32 {
    int32_t fX, fY;

    void set(int x, int y) { fX = x; fY = y; }

    bool equals(int x, int y) const { return fX == x && fY == y; }
};

struct SkPoint {
    SkScalar    fX, fY;

    /** Set the point's X and Y coordinates
    */
    void set(SkScalar x, SkScalar y) { fX = x; fY = y; }
    /** Set the point's X and Y coordinates by automatically promoting (x,y) to SkScalar values.
    */
    void iset(S16CPU x, S16CPU y) { fX = SkIntToScalar(x); fY = SkIntToScalar(y); }
    /** Set the point's X and Y coordinates by automatically promoting p's coordinates to SkScalar values.
    */
    void iset(const SkPoint16& p) { fX = SkIntToScalar(p.fX); fY = SkIntToScalar(p.fY); }

    /** Return the euclidian distance from (0,0) to the point
    */
    SkScalar length() const { return SkPoint::Length(fX, fY); }

    /** Set the point (vector) to be unit-length in the same direction as it
        currently is, and return its old length. If the old length is
        degenerately small (nearly zero), do nothing and return 0.
    */
    bool normalize();
    /** Set the point (vector) to be unit-length in the same direction as the
        x,y params, and return their old length. If the old length is
        degenerately small (nearly zero), do nothing and return 0.
    */
    bool setUnit(SkScalar x, SkScalar y);
    /** Scale the point to have the specified length, and return that
        length. If the original length is
        degenerately small (nearly zero), do nothing and return 0.
    */
    bool setLength(SkScalar length);
    /** Set the point to have the specified length in the same direction as (x,y),
        and return the old length of (x,y). If that old length is
        degenerately small (nearly zero), do nothing and return 0.
    */
    bool setLength(SkScalar x, SkScalar y, SkScalar length);

    /** Scale the point's coordinates by scale, writing the answer into dst.
        It is legal for dst == this.
    */
    void scale(SkScalar scale, SkPoint* dst) const;
    /** Scale the point's coordinates by scale, writing the answer back into the point.
    */
    void scale(SkScalar scale) { this->scale(scale, this); }

    /** Rotate the point clockwise by 90 degrees, writing the answer into dst.
        It is legal for dst == this.
    */
    void rotateCW(SkPoint* dst) const;
    /** Rotate the point clockwise by 90 degrees, writing the answer back into the point.
    */
    void rotateCW() { this->rotateCW(this); }
    /** Rotate the point counter-clockwise by 90 degrees, writing the answer into dst.
        It is legal for dst == this.
    */
    void rotateCCW(SkPoint* dst) const;
    /** Rotate the point counter-clockwise by 90 degrees, writing the answer back into the point.
    */
    void rotateCCW() { this->rotateCCW(this); }
    /** Negate the point's coordinates
    */
    void negate() { fX = -fX; fY = -fY; }
    /** Returns a new point whose coordinates are the negative of the point's
    */
    SkPoint operator-() const
    {
        SkPoint neg;
        neg.fX = -fX;
        neg.fY = -fY;
        return neg;
    }

    /** Add v's coordinates to the point's
    */
    void operator+=(const SkPoint& v)
    {
        fX += v.fX;
        fY += v.fY;
    }
    /** Subtract v's coordinates from the point's
    */
    void operator-=(const SkPoint& v)
    {
        fX -= v.fX;
        fY -= v.fY;
    }

    /** Returns true if the point's coordinates equal (x,y)
    */
    bool equals(SkScalar x, SkScalar y) const { return fX == x && fY == y; }
    friend bool operator==(const SkPoint& a, const SkPoint& b)
    {
        return a.fX == b.fX && a.fY == b.fY;
    }
    friend bool operator!=(const SkPoint& a, const SkPoint& b)
    {
        return a.fX != b.fX || a.fY != b.fY;
    }

    /** Returns a new point whose coordinates are the difference between a's and b's (a - b)
    */
    friend SkPoint operator-(const SkPoint& a, const SkPoint& b)
    {
        SkPoint v;
        v.set(a.fX - b.fX, a.fY - b.fY);
        return v;
    }
    /** Returns a new point whose coordinates are the sum of a's and b's (a + b)
    */
    friend SkPoint operator+(const SkPoint& a, const SkPoint& b)
    {
        SkPoint v;
        v.set(a.fX + b.fX, a.fY + b.fY);
        return v;
    }
    /** Returns the euclidian distance from (0,0) to (x,y)
    */
    static SkScalar Length(SkScalar x, SkScalar y);
    /** Returns the euclidian distance between a and b
    */
    static SkScalar Distance(const SkPoint& a, const SkPoint& b)
    {
        return Length(a.fX - b.fX, a.fY - b.fY);
    }
    /** Returns the dot product of a and b, treating them as 2D vectors
    */
    static SkScalar DotProduct(const SkPoint& a, const SkPoint& b)
    {
        return SkScalarMul(a.fX, b.fX) + SkScalarMul(a.fY, b.fY);
    }
    /** Returns the cross product of a and b, treating them as 2D vectors
    */
    static SkScalar CrossProduct(const SkPoint& a, const SkPoint& b)
    {
        return SkScalarMul(a.fX, b.fY) - SkScalarMul(a.fY, b.fX);
    }
};

typedef SkPoint SkVector;

#endif

