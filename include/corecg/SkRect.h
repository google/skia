/* include/corecg/SkRect.h
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

#ifndef SkRect_DEFINED
#define SkRect_DEFINED

#include "SkPoint.h"

/** \struct SkRect16

    SkRect16 holds four 16 bit integer coordinates for a rectangle
*/
struct SkRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    /** Returns true if the rectangle is empty (e.g. left >= right or top >= bottom)
    */
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }
    /** Returns the rectangle's width. This does not check for a valid rectangle (i.e. left <= right)
        so the result may be negative.
    */
    int width() const { return fRight - fLeft; }
    /** Returns the rectangle's height. This does not check for a valid rectangle (i.e. top <= bottom)
        so the result may be negative.
    */
    int height() const { return fBottom - fTop; }

    friend int operator==(const SkRect16& a, const SkRect16& b)
    {
        return !memcmp(&a, &b, sizeof(a));
    }
    friend int operator!=(const SkRect16& a, const SkRect16& b)
    {
        return memcmp(&a, &b, sizeof(a));
    }

    /** Set the rectangle to (0,0,0,0)
    */
    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom)
    {
        fLeft   = SkToS16(left);
        fTop    = SkToS16(top);
        fRight  = SkToS16(right);
        fBottom = SkToS16(bottom);
    }
    /** Offset set the rectangle by adding dx to its left and right,
        and adding dy to its top and bottom.
    */
    void offset(S16CPU dx, S16CPU dy)
    {
        fLeft   = SkToS16(fLeft + dx);
        fTop    = SkToS16(fTop + dy);
        fRight  = SkToS16(fRight + dx);
        fBottom = SkToS16(fBottom + dy);
    }
    /** Inset the rectangle by (dx,dy). If dx is positive, then the sides are moved inwards,
        making the rectangle narrower. If dx is negative, then the sides are moved outwards,
        making the rectangle wider. The same hods true for dy and the top and bottom.
    */
    void inset(S16CPU dx, S16CPU dy)
    {
        fLeft   = SkToS16(fLeft + dx);
        fTop    = SkToS16(fTop + dy);
        fRight  = SkToS16(fRight - dx);
        fBottom = SkToS16(fBottom - dy);
    }
    /** Returns true if (x,y) is inside the rectangle. The left and top are considered to be
        inside, while the right and bottom are not. Thus for the rectangle (0, 0, 5, 10), the
        points (0,0) and (0,9) are inside, while (-1,0) and (5,9) are not.
    */
    bool contains(S16CPU x, S16CPU y) const
    {
        return  (unsigned)(x - fLeft) < (unsigned)(fRight - fLeft) &&
                (unsigned)(y - fTop) < (unsigned)(fBottom - fTop);
    }
    /** Returns true if the 4 specified sides of a rectangle are inside or equal to this rectangle.
    */
    bool contains(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom) const
    {
        return  fLeft <= left && fTop <= top &&
                fRight >= right && fBottom >= bottom;
    }
    /** Returns true if the specified rectangle r is inside or equal to this rectangle.
    */
    bool contains(const SkRect16& r) const
    {
        return  fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }
    /** If r intersects this rectangle, return true and set this rectangle to that
        intersection, otherwise return false and do not change this rectangle.
    */
    bool intersect(const SkRect16& r);
    /** If rectangles a and b intersect, return true and set this rectangle to that
        intersection, otherwise return false and do not change this rectangle.
    */
    bool intersect(const SkRect16& a, const SkRect16& b);
    /** If the rectangle specified by left,top,right,bottom intersects this rectangle,
        return true and set this rectangle to that intersection,
        otherwise return false and do not change this rectangle.
    */
    bool intersect(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom);
    /** Returns true if a and b intersect
    */
    static bool Intersects(const SkRect16& a, const SkRect16& b)
    {
        return  a.fLeft < b.fRight && b.fLeft < a.fRight &&
                a.fTop < b.fBottom && b.fTop < a.fBottom;
    }
    void join(const SkRect16& r)
    {
        fLeft = SkToS16(SkMin32(fLeft, r.fLeft));
        fTop = SkToS16(SkMin32(fTop, r.fTop));
        fRight = SkToS16(SkMax32(fRight, r.fRight));
        fBottom = SkToS16(SkMax32(fBottom, r.fBottom));
    }

    /** Swap top/bottom or left/right if there are flipped.
        This can be called if the edges are computed separately,
        and may have crossed over each other.
        When this returns, left <= right && top <= bottom
    */
    void sort();
};

/** \struct SkRect
*/
struct SkRect {
    SkScalar    fLeft, fTop, fRight, fBottom;

    bool        isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }
    SkScalar    width() const { return fRight - fLeft; }
    SkScalar    height() const { return fBottom - fTop; }
    SkScalar    centerX() const { return SkScalarHalf(fLeft + fRight); }
    SkScalar    centerY() const { return SkScalarHalf(fTop + fBottom); }

    friend int operator==(const SkRect& a, const SkRect& b)
    {
        return !memcmp(&a, &b, sizeof(a));
    }
    friend int operator!=(const SkRect& a, const SkRect& b)
    {
        return memcmp(&a, &b, sizeof(a));
    }

    SkPoint* asPoints() { return (SkPoint*)(void*)this; }
    const SkPoint* asPoints() const { return (const SkPoint*)(const void*)this; }

    /** return the 4 points that enclose the rectangle
    */
    void toQuad(SkPoint quad[4]) const;

    /** Set this rectangle to the empty rectangle (0,0,0,0)
    */
    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(const SkRect16& src)
    {
        fLeft   = SkIntToScalar(src.fLeft);
        fTop    = SkIntToScalar(src.fTop);
        fRight  = SkIntToScalar(src.fRight);
        fBottom = SkIntToScalar(src.fBottom);
    }

    void set(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
    {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }
    /** Set this rectangle to be the bounds of the array of points.
        If the array is empty (count == 0), then set this rectangle
        to the empty rectangle (0,0,0,0)
    */
    void set(const SkPoint pts[], int count);

    /** Offset set the rectangle by adding dx to its left and right,
        and adding dy to its top and bottom.
    */
    void offset(SkScalar dx, SkScalar dy)
    {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }   
    /** Inset the rectangle by (dx,dy). If dx is positive, then the sides are moved inwards,
        making the rectangle narrower. If dx is negative, then the sides are moved outwards,
        making the rectangle wider. The same hods true for dy and the top and bottom.
    */
    void inset(SkScalar dx, SkScalar dy)
    {
        fLeft   += dx;
        fTop    += dy;
        fRight  -= dx;
        fBottom -= dy;
    }

    /** If this rectangle intersects r, return true and set this rectangle to that
        intersection, otherwise return false and do not change this rectangle.
    */
    bool intersect(const SkRect& r);
    /** If this rectangle intersects the rectangle specified by left, top, right, bottom,
        return true and set this rectangle to that
        intersection, otherwise return false and do not change this rectangle.
    */
    bool intersect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);
    /** Return true if rectangles a and b intersect.
    */
    static bool Intersects(const SkRect& a, const SkRect& b)
    {
        return  a.fLeft < b.fRight && b.fLeft < a.fRight &&
                a.fTop < b.fBottom && b.fTop < a.fBottom;
    }

    /** Returns true if (p.fX,p.fY) is inside the rectangle. The left and top coordinates of
        the rectangle are considered to be inside, while the right and bottom coordinates
        are not. Thus for the rectangle (0, 0, 5, 10), the points (0,0) and (0,9) are inside,
        while (-1,0) and (5,9) are not.
    */
    bool contains(const SkPoint& p) const
    {
        return  fLeft <= p.fX && p.fX < fRight &&
                fTop <= p.fY && p.fY < fBottom;
    }
    /** Returns true if (x,y) is inside the rectangle. The left and top coordinates of
        the rectangle are considered to be inside, while the right and bottom coordinates
        are not. Thus for the rectangle (0, 0, 5, 10), the points (0,0) and (0,9) are inside,
        while (-1,0) and (5,9) are not.
    */
    bool contains(SkScalar x, SkScalar y) const
    {
        return  fLeft <= x && x < fRight &&
                fTop <= y && y < fBottom;
    }
    /** Return true if this rectangle contains r
    */
    bool contains(const SkRect& r) const
    {
        return  fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }
    /** Set the dst integer rectangle by rounding this rectangle's coordinates
        to their nearest integer values.
    */
    void round(SkRect16* dst) const
    {
        SkASSERT(dst);
        dst->set(SkScalarRound(fLeft), SkScalarRound(fTop), SkScalarRound(fRight), SkScalarRound(fBottom));
    }
    /** Set the dst integer rectangle by rounding "out" this rectangle, choosing the floor of top and left,
        and the ceiling of right and bototm.
    */
    void roundOut(SkRect16* dst) const
    {
        SkASSERT(dst);
        dst->set(SkScalarFloor(fLeft), SkScalarFloor(fTop), SkScalarCeil(fRight), SkScalarCeil(fBottom));
    }

    /** Swap top/bottom or left/right if there are flipped.
        This can be called if the edges are computed separately,
        and may have crossed over each other.
        When this returns, left <= right && top <= bottom
    */
    void sort();
};

#endif

