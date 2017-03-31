/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRect_DEFINED
#define SkRect_DEFINED

#include "SkPoint.h"
#include "SkSize.h"

struct SkRect;

/** \struct SkIRect

    SkIRect holds four 32 bit integer coordinates for a rectangle
*/
struct SK_API SkIRect {
    int32_t fLeft, fTop, fRight, fBottom;

    static SkIRect SK_WARN_UNUSED_RESULT MakeEmpty() {
        SkIRect r;
        r.setEmpty();
        return r;
    }

    static SkIRect SK_WARN_UNUSED_RESULT MakeLargest() {
        SkIRect r;
        r.setLargest();
        return r;
    }

    static SkIRect SK_WARN_UNUSED_RESULT MakeWH(int32_t w, int32_t h) {
        SkIRect r;
        r.set(0, 0, w, h);
        return r;
    }

    static SkIRect SK_WARN_UNUSED_RESULT MakeSize(const SkISize& size) {
        SkIRect r;
        r.set(0, 0, size.width(), size.height());
        return r;
    }

    static SkIRect SK_WARN_UNUSED_RESULT MakeLTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        SkIRect rect;
        rect.set(l, t, r, b);
        return rect;
    }

    static SkIRect SK_WARN_UNUSED_RESULT MakeXYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        SkIRect r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    int left() const { return fLeft; }
    int top() const { return fTop; }
    int right() const { return fRight; }
    int bottom() const { return fBottom; }

    /** return the left edge of the rect */
    int x() const { return fLeft; }
    /** return the top edge of the rect */
    int y() const { return fTop; }
    /**
     *  Returns the rectangle's width. This does not check for a valid rect
     *  (i.e. left <= right) so the result may be negative.
     */
    int width() const { return fRight - fLeft; }

    /**
     *  Returns the rectangle's height. This does not check for a valid rect
     *  (i.e. top <= bottom) so the result may be negative.
     */
    int height() const { return fBottom - fTop; }

    SkISize size() const { return SkISize::Make(this->width(), this->height()); }

    /**
     *  Since the center of an integer rect may fall on a factional value, this
     *  method is defined to return (right + left) >> 1.
     *
     *  This is a specific "truncation" of the average, which is different than
     *  (right + left) / 2 when the sum is negative.
     */
    int centerX() const { return (fRight + fLeft) >> 1; }

    /**
     *  Since the center of an integer rect may fall on a factional value, this
     *  method is defined to return (bottom + top) >> 1
     *
     *  This is a specific "truncation" of the average, which is different than
     *  (bottom + top) / 2 when the sum is negative.
     */
    int centerY() const { return (fBottom + fTop) >> 1; }

    /**
     *  Return true if the rectangle's width or height are <= 0
     */
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    bool isLargest() const { return SK_MinS32 == fLeft &&
                                    SK_MinS32 == fTop &&
                                    SK_MaxS32 == fRight &&
                                    SK_MaxS32 == fBottom; }

    friend bool operator==(const SkIRect& a, const SkIRect& b) {
        return !memcmp(&a, &b, sizeof(a));
    }

    friend bool operator!=(const SkIRect& a, const SkIRect& b) {
        return !(a == b);
    }

    bool is16Bit() const {
        return  SkIsS16(fLeft) && SkIsS16(fTop) &&
                SkIsS16(fRight) && SkIsS16(fBottom);
    }

    /** Set the rectangle to (0,0,0,0)
    */
    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }
    // alias for set(l, t, r, b)
    void setLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        this->set(left, top, right, bottom);
    }

    void setXYWH(int32_t x, int32_t y, int32_t width, int32_t height) {
        fLeft = x;
        fTop = y;
        fRight = x + width;
        fBottom = y + height;
    }

    /**
     *  Make the largest representable rectangle
     */
    void setLargest() {
        fLeft = fTop = SK_MinS32;
        fRight = fBottom = SK_MaxS32;
    }

    /**
     *  Make the largest representable rectangle, but inverted (e.g. fLeft will
     *  be max 32bit and right will be min 32bit).
     */
    void setLargestInverted() {
        fLeft = fTop = SK_MaxS32;
        fRight = fBottom = SK_MinS32;
    }

    /**
     *  Return a new IRect, built as an offset of this rect.
     */
    SkIRect makeOffset(int32_t dx, int32_t dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight + dx, fBottom + dy);
    }

    /**
     *  Return a new IRect, built as an inset of this rect.
     */
    SkIRect makeInset(int32_t dx, int32_t dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight - dx, fBottom - dy);
    }

    /**
     *  Return a new Rect, built as an outset of this rect.
     */
    SkIRect makeOutset(int32_t dx, int32_t dy) const {
        return MakeLTRB(fLeft - dx, fTop - dy, fRight + dx, fBottom + dy);
    }

    /** Offset set the rectangle by adding dx to its left and right,
        and adding dy to its top and bottom.
    */
    void offset(int32_t dx, int32_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }

    void offset(const SkIPoint& delta) {
        this->offset(delta.fX, delta.fY);
    }

    /**
     *  Offset this rect such its new x() and y() will equal newX and newY.
     */
    void offsetTo(int32_t newX, int32_t newY) {
        fRight += newX - fLeft;
        fBottom += newY - fTop;
        fLeft = newX;
        fTop = newY;
    }

    /** Inset the rectangle by (dx,dy). If dx is positive, then the sides are moved inwards,
        making the rectangle narrower. If dx is negative, then the sides are moved outwards,
        making the rectangle wider. The same holds true for dy and the top and bottom.
    */
    void inset(int32_t dx, int32_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  -= dx;
        fBottom -= dy;
    }

   /** Outset the rectangle by (dx,dy). If dx is positive, then the sides are
       moved outwards, making the rectangle wider. If dx is negative, then the
       sides are moved inwards, making the rectangle narrower. The same holds
       true for dy and the top and bottom.
    */
    void outset(int32_t dx, int32_t dy)  { this->inset(-dx, -dy); }

    bool quickReject(int l, int t, int r, int b) const {
        return l >= fRight || fLeft >= r || t >= fBottom || fTop >= b;
    }

    /** Returns true if (x,y) is inside the rectangle and the rectangle is not
        empty. The left and top are considered to be inside, while the right
        and bottom are not. Thus for the rectangle (0, 0, 5, 10), the
        points (0,0) and (0,9) are inside, while (-1,0) and (5,9) are not.
    */
    bool contains(int32_t x, int32_t y) const {
        return  (unsigned)(x - fLeft) < (unsigned)(fRight - fLeft) &&
                (unsigned)(y - fTop) < (unsigned)(fBottom - fTop);
    }

    /** Returns true if the 4 specified sides of a rectangle are inside or equal to this rectangle.
        If either rectangle is empty, contains() returns false.
    */
    bool contains(int32_t left, int32_t top, int32_t right, int32_t bottom) const {
        return  left < right && top < bottom && !this->isEmpty() && // check for empties
                fLeft <= left && fTop <= top &&
                fRight >= right && fBottom >= bottom;
    }

    /** Returns true if the specified rectangle r is inside or equal to this rectangle.
    */
    bool contains(const SkIRect& r) const {
        return  !r.isEmpty() && !this->isEmpty() &&     // check for empties
                fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /** Returns true if the specified rectangle r is inside or equal to this rectangle.
    */
    bool contains(const SkRect& r) const;

    /** Return true if this rectangle contains the specified rectangle.
        For speed, this method does not check if either this or the specified
        rectangles are empty, and if either is, its return value is undefined.
        In the debugging build however, we assert that both this and the
        specified rectangles are non-empty.
    */
    bool containsNoEmptyCheck(int32_t left, int32_t top,
                              int32_t right, int32_t bottom) const {
        SkASSERT(fLeft < fRight && fTop < fBottom);
        SkASSERT(left < right && top < bottom);

        return fLeft <= left && fTop <= top &&
               fRight >= right && fBottom >= bottom;
    }

    bool containsNoEmptyCheck(const SkIRect& r) const {
        return containsNoEmptyCheck(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** If r intersects this rectangle, return true and set this rectangle to that
        intersection, otherwise return false and do not change this rectangle.
        If either rectangle is empty, do nothing and return false.
    */
    bool intersect(const SkIRect& r) {
        return this->intersect(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** If rectangles a and b intersect, return true and set this rectangle to
        that intersection, otherwise return false and do not change this
        rectangle. If either rectangle is empty, do nothing and return false.
    */
    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& a, const SkIRect& b) {

        if (!a.isEmpty() && !b.isEmpty() &&
                a.fLeft < b.fRight && b.fLeft < a.fRight &&
                a.fTop < b.fBottom && b.fTop < a.fBottom) {
            fLeft   = SkMax32(a.fLeft,   b.fLeft);
            fTop    = SkMax32(a.fTop,    b.fTop);
            fRight  = SkMin32(a.fRight,  b.fRight);
            fBottom = SkMin32(a.fBottom, b.fBottom);
            return true;
        }
        return false;
    }

    /** If rectangles a and b intersect, return true and set this rectangle to
        that intersection, otherwise return false and do not change this
        rectangle. For speed, no check to see if a or b are empty is performed.
        If either is, then the return result is undefined. In the debug build,
        we assert that both rectangles are non-empty.
    */
    bool SK_WARN_UNUSED_RESULT intersectNoEmptyCheck(const SkIRect& a, const SkIRect& b) {
        SkASSERT(!a.isEmpty() && !b.isEmpty());

        if (a.fLeft < b.fRight && b.fLeft < a.fRight &&
                a.fTop < b.fBottom && b.fTop < a.fBottom) {
            fLeft   = SkMax32(a.fLeft,   b.fLeft);
            fTop    = SkMax32(a.fTop,    b.fTop);
            fRight  = SkMin32(a.fRight,  b.fRight);
            fBottom = SkMin32(a.fBottom, b.fBottom);
            return true;
        }
        return false;
    }

    /** If the rectangle specified by left,top,right,bottom intersects this rectangle,
        return true and set this rectangle to that intersection,
        otherwise return false and do not change this rectangle.
        If either rectangle is empty, do nothing and return false.
    */
    bool intersect(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        if (left < right && top < bottom && !this->isEmpty() &&
                fLeft < right && left < fRight && fTop < bottom && top < fBottom) {
            if (fLeft < left) fLeft = left;
            if (fTop < top) fTop = top;
            if (fRight > right) fRight = right;
            if (fBottom > bottom) fBottom = bottom;
            return true;
        }
        return false;
    }

    /** Returns true if a and b are not empty, and they intersect
     */
    static bool Intersects(const SkIRect& a, const SkIRect& b) {
        return  !a.isEmpty() && !b.isEmpty() &&              // check for empties
                a.fLeft < b.fRight && b.fLeft < a.fRight &&
                a.fTop < b.fBottom && b.fTop < a.fBottom;
    }

    /**
     *  Returns true if a and b intersect. debug-asserts that neither are empty.
     */
    static bool IntersectsNoEmptyCheck(const SkIRect& a, const SkIRect& b) {
        SkASSERT(!a.isEmpty());
        SkASSERT(!b.isEmpty());
        return  a.fLeft < b.fRight && b.fLeft < a.fRight &&
                a.fTop < b.fBottom && b.fTop < a.fBottom;
    }

    /** Update this rectangle to enclose itself and the specified rectangle.
        If this rectangle is empty, just set it to the specified rectangle. If the specified
        rectangle is empty, do nothing.
    */
    void join(int32_t left, int32_t top, int32_t right, int32_t bottom);

    /** Update this rectangle to enclose itself and the specified rectangle.
        If this rectangle is empty, just set it to the specified rectangle. If the specified
        rectangle is empty, do nothing.
    */
    void join(const SkIRect& r) {
        this->join(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** Swap top/bottom or left/right if there are flipped.
        This can be called if the edges are computed separately,
        and may have crossed over each other.
        When this returns, left <= right && top <= bottom
    */
    void sort();

    static const SkIRect& SK_WARN_UNUSED_RESULT EmptyIRect() {
        static const SkIRect gEmpty = { 0, 0, 0, 0 };
        return gEmpty;
    }
};

/** \struct SkRect
*/
struct SK_API SkRect {
    SkScalar    fLeft, fTop, fRight, fBottom;

    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeEmpty() {
        return SkRect{0, 0, 0, 0};
    }

    static SkRect SK_WARN_UNUSED_RESULT MakeLargest() {
        SkRect r;
        r.setLargest();
        return r;
    }

    static SkRect SK_WARN_UNUSED_RESULT MakeWH(SkScalar w, SkScalar h) {
        SkRect r;
        r.set(0, 0, w, h);
        return r;
    }

    static SkRect SK_WARN_UNUSED_RESULT MakeIWH(int w, int h) {
        SkRect r;
        r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));
        return r;
    }

    static SkRect SK_WARN_UNUSED_RESULT MakeSize(const SkSize& size) {
        SkRect r;
        r.set(0, 0, size.width(), size.height());
        return r;
    }

    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeLTRB(SkScalar l, SkScalar t, SkScalar r,
                                                           SkScalar b) {
        return SkRect {l, t, r, b};
    }

    static SkRect SK_WARN_UNUSED_RESULT MakeXYWH(SkScalar x, SkScalar y, SkScalar w, SkScalar h) {
        SkRect r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    SK_ATTR_DEPRECATED("use Make()")
    static SkRect SK_WARN_UNUSED_RESULT MakeFromIRect(const SkIRect& irect) {
        SkRect r;
        r.set(SkIntToScalar(irect.fLeft),
              SkIntToScalar(irect.fTop),
              SkIntToScalar(irect.fRight),
              SkIntToScalar(irect.fBottom));
        return r;
    }

    static SkRect Make(const SkISize& size) {
        return MakeIWH(size.width(), size.height());
    }
    
    static SkRect SK_WARN_UNUSED_RESULT Make(const SkIRect& irect) {
        SkRect r;
        r.set(SkIntToScalar(irect.fLeft),
              SkIntToScalar(irect.fTop),
              SkIntToScalar(irect.fRight),
              SkIntToScalar(irect.fBottom));
        return r;
    }

    /**
     *  Return true if the rectangle's width or height are <= 0
     */
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    bool isLargest() const { return SK_ScalarMin == fLeft &&
                                    SK_ScalarMin == fTop &&
                                    SK_ScalarMax == fRight &&
                                    SK_ScalarMax == fBottom; }

    /**
     *  Returns true iff all values in the rect are finite. If any are
     *  infinite or NaN then this returns false.
     */
    bool isFinite() const {
        float accum = 0;
        accum *= fLeft;
        accum *= fTop;
        accum *= fRight;
        accum *= fBottom;

        // accum is either NaN or it is finite (zero).
        SkASSERT(0 == accum || SkScalarIsNaN(accum));

        // value==value will be true iff value is not NaN
        // TODO: is it faster to say !accum or accum==accum?
        return !SkScalarIsNaN(accum);
    }

    SkScalar    x() const { return fLeft; }
    SkScalar    y() const { return fTop; }
    SkScalar    left() const { return fLeft; }
    SkScalar    top() const { return fTop; }
    SkScalar    right() const { return fRight; }
    SkScalar    bottom() const { return fBottom; }
    SkScalar    width() const { return fRight - fLeft; }
    SkScalar    height() const { return fBottom - fTop; }
    SkScalar    centerX() const { return SkScalarHalf(fLeft + fRight); }
    SkScalar    centerY() const { return SkScalarHalf(fTop + fBottom); }

    friend bool operator==(const SkRect& a, const SkRect& b) {
        return SkScalarsEqual((SkScalar*)&a, (SkScalar*)&b, 4);
    }

    friend bool operator!=(const SkRect& a, const SkRect& b) {
        return !SkScalarsEqual((SkScalar*)&a, (SkScalar*)&b, 4);
    }

    /** return the 4 points that enclose the rectangle (top-left, top-right, bottom-right,
        bottom-left). TODO: Consider adding param to control whether quad is CW or CCW.
     */
    void toQuad(SkPoint quad[4]) const;

    /** Set this rectangle to the empty rectangle (0,0,0,0)
    */
    void setEmpty() { *this = MakeEmpty(); }

    void set(const SkIRect& src) {
        fLeft   = SkIntToScalar(src.fLeft);
        fTop    = SkIntToScalar(src.fTop);
        fRight  = SkIntToScalar(src.fRight);
        fBottom = SkIntToScalar(src.fBottom);
    }

    void set(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }
    // alias for set(l, t, r, b)
    void setLTRB(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
        this->set(left, top, right, bottom);
    }

    /** Initialize the rect with the 4 specified integers. The routine handles
        converting them to scalars (by calling SkIntToScalar)
     */
    void iset(int left, int top, int right, int bottom) {
        fLeft   = SkIntToScalar(left);
        fTop    = SkIntToScalar(top);
        fRight  = SkIntToScalar(right);
        fBottom = SkIntToScalar(bottom);
    }

    /**
     *  Set this rectangle to be left/top at 0,0, and have the specified width
     *  and height (automatically converted to SkScalar).
     */
    void isetWH(int width, int height) {
        fLeft = fTop = 0;
        fRight = SkIntToScalar(width);
        fBottom = SkIntToScalar(height);
    }

    /** Set this rectangle to be the bounds of the array of points.
        If the array is empty (count == 0), then set this rectangle
        to the empty rectangle (0,0,0,0)
    */
    void set(const SkPoint pts[], int count) {
        // set() had been checking for non-finite values, so keep that behavior
        // for now. Now that we have setBoundsCheck(), we may decide to make
        // set() be simpler/faster, and not check for those.
        (void)this->setBoundsCheck(pts, count);
    }

    // alias for set(pts, count)
    void setBounds(const SkPoint pts[], int count) {
        (void)this->setBoundsCheck(pts, count);
    }

    /**
     *  Compute the bounds of the array of points, and set this rect to that
     *  bounds and return true... unless a non-finite value is encountered,
     *  in which case this rect is set to empty and false is returned.
     */
    bool setBoundsCheck(const SkPoint pts[], int count);

    void set(const SkPoint& p0, const SkPoint& p1) {
        fLeft =   SkMinScalar(p0.fX, p1.fX);
        fRight =  SkMaxScalar(p0.fX, p1.fX);
        fTop =    SkMinScalar(p0.fY, p1.fY);
        fBottom = SkMaxScalar(p0.fY, p1.fY);
    }

    void setXYWH(SkScalar x, SkScalar y, SkScalar width, SkScalar height) {
        fLeft = x;
        fTop = y;
        fRight = x + width;
        fBottom = y + height;
    }

    void setWH(SkScalar width, SkScalar height) {
        fLeft = 0;
        fTop = 0;
        fRight = width;
        fBottom = height;
    }

    /**
     *  Make the largest representable rectangle
     */
    void setLargest() {
        fLeft = fTop = SK_ScalarMin;
        fRight = fBottom = SK_ScalarMax;
    }

    /**
     *  Make the largest representable rectangle, but inverted (e.g. fLeft will
     *  be max and right will be min).
     */
    void setLargestInverted() {
        fLeft = fTop = SK_ScalarMax;
        fRight = fBottom = SK_ScalarMin;
    }

    /**
     *  Return a new Rect, built as an offset of this rect.
     */
    SkRect makeOffset(SkScalar dx, SkScalar dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight + dx, fBottom + dy);
    }
    
    /**
     *  Return a new Rect, built as an inset of this rect.
     */
    SkRect makeInset(SkScalar dx, SkScalar dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight - dx, fBottom - dy);
    }

    /**
     *  Return a new Rect, built as an outset of this rect.
     */
    SkRect makeOutset(SkScalar dx, SkScalar dy) const {
        return MakeLTRB(fLeft - dx, fTop - dy, fRight + dx, fBottom + dy);
    }

    /** Offset set the rectangle by adding dx to its left and right,
        and adding dy to its top and bottom.
    */
    void offset(SkScalar dx, SkScalar dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }

    void offset(const SkPoint& delta) {
        this->offset(delta.fX, delta.fY);
    }

    /**
     *  Offset this rect such its new x() and y() will equal newX and newY.
     */
    void offsetTo(SkScalar newX, SkScalar newY) {
        fRight += newX - fLeft;
        fBottom += newY - fTop;
        fLeft = newX;
        fTop = newY;
    }

    /** Inset the rectangle by (dx,dy). If dx is positive, then the sides are
        moved inwards, making the rectangle narrower. If dx is negative, then
        the sides are moved outwards, making the rectangle wider. The same holds
         true for dy and the top and bottom.
    */
    void inset(SkScalar dx, SkScalar dy)  {
        fLeft   += dx;
        fTop    += dy;
        fRight  -= dx;
        fBottom -= dy;
    }

   /** Outset the rectangle by (dx,dy). If dx is positive, then the sides are
       moved outwards, making the rectangle wider. If dx is negative, then the
       sides are moved inwards, making the rectangle narrower. The same holds
       true for dy and the top and bottom.
    */
    void outset(SkScalar dx, SkScalar dy)  { this->inset(-dx, -dy); }

    /** If this rectangle intersects r, return true and set this rectangle to that
        intersection, otherwise return false and do not change this rectangle.
        If either rectangle is empty, do nothing and return false.
    */
    bool intersect(const SkRect& r);

    /** If this rectangle intersects the rectangle specified by left, top, right, bottom,
        return true and set this rectangle to that intersection, otherwise return false
        and do not change this rectangle.
        If either rectangle is empty, do nothing and return false.
    */
    bool intersect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);

    /**
     *  If rectangles a and b intersect, return true and set this rectangle to
     *  that intersection, otherwise return false and do not change this
     *  rectangle. If either rectangle is empty, do nothing and return false.
     */
    bool SK_WARN_UNUSED_RESULT intersect(const SkRect& a, const SkRect& b);


private:
    static bool Intersects(SkScalar al, SkScalar at, SkScalar ar, SkScalar ab,
                           SkScalar bl, SkScalar bt, SkScalar br, SkScalar bb) {
        SkScalar L = SkMaxScalar(al, bl);
        SkScalar R = SkMinScalar(ar, br);
        SkScalar T = SkMaxScalar(at, bt);
        SkScalar B = SkMinScalar(ab, bb);
        return L < R && T < B;
    }

public:
    /**
     *  Return true if this rectangle is not empty, and the specified sides of
     *  a rectangle are not empty, and they intersect.
     */
    bool intersects(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) const {
        return Intersects(fLeft, fTop, fRight, fBottom, left, top, right, bottom);
    }

    bool intersects(const SkRect& r) const {
        return Intersects(fLeft, fTop, fRight, fBottom,
                          r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /**
     *  Return true if rectangles a and b are not empty and intersect.
     */
    static bool Intersects(const SkRect& a, const SkRect& b) {
        return Intersects(a.fLeft, a.fTop, a.fRight, a.fBottom,
                          b.fLeft, b.fTop, b.fRight, b.fBottom);
    }

    /**
     *  Update this rectangle to enclose itself and the specified rectangle.
     *  If this rectangle is empty, just set it to the specified rectangle.
     *  If the specified rectangle is empty, do nothing.
     */
    void join(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);

    /** Update this rectangle to enclose itself and the specified rectangle.
        If this rectangle is empty, just set it to the specified rectangle. If the specified
        rectangle is empty, do nothing.
    */
    void join(const SkRect& r) {
        this->join(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    void joinNonEmptyArg(const SkRect& r) {
        SkASSERT(!r.isEmpty());
        // if we are empty, just assign
        if (fLeft >= fRight || fTop >= fBottom) {
            *this = r;
        } else {
            this->joinPossiblyEmptyRect(r);
        }
    }

    /**
     * Joins the rectangle with another without checking if either are empty (may produce unexpected
     * results if either rect is inverted).
     */
    void joinPossiblyEmptyRect(const SkRect& r) {
        fLeft   = SkMinScalar(fLeft, r.left());
        fTop    = SkMinScalar(fTop, r.top());
        fRight  = SkMaxScalar(fRight, r.right());
        fBottom = SkMaxScalar(fBottom, r.bottom());
    }

    /**
     *  Grow the rect to include the specified (x,y). After this call, the
     *  following will be true: fLeft <= x <= fRight && fTop <= y <= fBottom.
     *
     *  This is close, but not quite the same contract as contains(), since
     *  contains() treats the left and top different from the right and bottom.
     *  contains(x,y) -> fLeft <= x < fRight && fTop <= y < fBottom. Also note
     *  that contains(x,y) always returns false if the rect is empty.
     */
    void growToInclude(SkScalar x, SkScalar y) {
        fLeft  = SkMinScalar(x, fLeft);
        fRight = SkMaxScalar(x, fRight);
        fTop    = SkMinScalar(y, fTop);
        fBottom = SkMaxScalar(y, fBottom);
    }

    /** Bulk version of growToInclude */
    void growToInclude(const SkPoint pts[], int count) {
        this->growToInclude(pts, sizeof(SkPoint), count);
    }

    /** Bulk version of growToInclude with stride. */
    void growToInclude(const SkPoint pts[], size_t stride, int count) {
        SkASSERT(count >= 0);
        SkASSERT(stride >= sizeof(SkPoint));
        const SkPoint* end = (const SkPoint*)((intptr_t)pts + count * stride);
        for (; pts < end; pts = (const SkPoint*)((intptr_t)pts + stride)) {
            this->growToInclude(pts->fX, pts->fY);
        }
    }

    /**
     *  Return true if this rectangle contains r, and if both rectangles are
     *  not empty.
     */
    bool contains(const SkRect& r) const {
        // todo: can we eliminate the this->isEmpty check?
        return  !r.isEmpty() && !this->isEmpty() &&
                fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /**
     * Returns true if the specified rectangle r is inside or equal to this rectangle.
     */
    bool contains(const SkIRect& r) const {
        // todo: can we eliminate the this->isEmpty check?
        return  !r.isEmpty() && !this->isEmpty() &&
                fLeft <= SkIntToScalar(r.fLeft) && fTop <= SkIntToScalar(r.fTop) &&
                fRight >= SkIntToScalar(r.fRight) && fBottom >= SkIntToScalar(r.fBottom);
    }

    /**
     *  Set the dst rectangle by rounding this rectangle's coordinates to their
     *  nearest integer values using SkScalarRoundToInt.
     */
    void round(SkIRect* dst) const {
        SkASSERT(dst);
        dst->set(SkScalarRoundToInt(fLeft), SkScalarRoundToInt(fTop),
                 SkScalarRoundToInt(fRight), SkScalarRoundToInt(fBottom));
    }

    /**
     *  Set the dst rectangle by rounding "out" this rectangle, choosing the
     *  SkScalarFloor of top and left, and the SkScalarCeil of right and bottom.
     */
    void roundOut(SkIRect* dst) const {
        SkASSERT(dst);
        dst->set(SkScalarFloorToInt(fLeft), SkScalarFloorToInt(fTop),
                 SkScalarCeilToInt(fRight), SkScalarCeilToInt(fBottom));
    }

    /**
     *  Set the dst rectangle by rounding "out" this rectangle, choosing the
     *  SkScalarFloorToScalar of top and left, and the SkScalarCeilToScalar of right and bottom.
     *
     *  It is safe for this == dst
     */
    void roundOut(SkRect* dst) const {
        dst->set(SkScalarFloorToScalar(fLeft),
                 SkScalarFloorToScalar(fTop),
                 SkScalarCeilToScalar(fRight),
                 SkScalarCeilToScalar(fBottom));
    }

    /**
     *  Set the dst rectangle by rounding "in" this rectangle, choosing the
     *  ceil of top and left, and the floor of right and bottom. This does *not*
     *  call sort(), so it is possible that the resulting rect is inverted...
     *  e.g. left >= right or top >= bottom. Call isEmpty() to detect that.
     */
    void roundIn(SkIRect* dst) const {
        SkASSERT(dst);
        dst->set(SkScalarCeilToInt(fLeft), SkScalarCeilToInt(fTop),
                 SkScalarFloorToInt(fRight), SkScalarFloorToInt(fBottom));
    }

    //! Returns the result of calling round(&dst)
    SkIRect round() const {
        SkIRect ir;
        this->round(&ir);
        return ir;
    }
    
    //! Returns the result of calling roundOut(&dst)
    SkIRect roundOut() const {
        SkIRect ir;
        this->roundOut(&ir);
        return ir;
    }
    
    /**
     *  Swap top/bottom or left/right if there are flipped (i.e. if width()
     *  or height() would have returned a negative value.) This should be called
     *  if the edges are computed separately, and may have crossed over each
     *  other. When this returns, left <= right && top <= bottom
     */
    void sort() {
        if (fLeft > fRight) {
            SkTSwap<SkScalar>(fLeft, fRight);
        }

        if (fTop > fBottom) {
            SkTSwap<SkScalar>(fTop, fBottom);
        }
    }

    /**
     *  cast-safe way to treat the rect as an array of (4) SkScalars.
     */
    const SkScalar* asScalars() const { return &fLeft; }

    void dump(bool asHex) const;
    void dump() const { this->dump(false); }
    void dumpHex() const { this->dump(true); }
};

inline bool SkIRect::contains(const SkRect& r) const {
    return  !r.isEmpty() && !this->isEmpty() &&     // check for empties
            (SkScalar)fLeft <= r.fLeft && (SkScalar)fTop <= r.fTop &&
            (SkScalar)fRight >= r.fRight && (SkScalar)fBottom >= r.fBottom;
}

#endif
