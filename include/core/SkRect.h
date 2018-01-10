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
#include "../private/SkPedanticMath.h"
#include "../private/SkTFitsIn.h"

struct SkRect;

/** \struct SkIRect
    SkIRect holds four 32 bit integer coordinates describing the upper and
    lower bounds of a rectangle. SkIRect may be created from outer bounds or
    from position, width, and height. SkIRect describes an area; if its right
    is less than or equal to its left, or if its bottom is less than or equal to
    its top, it is considered empty.
*/
struct SK_API SkIRect {

    /** May contain any value. The smaller of the horizontal values when sorted.
        When equal to or greater than fRight, SkIRect is empty.
    */
    int32_t fLeft;

    /** May contain any value. The smaller of the horizontal values when sorted.
        When equal to or greater than fBottom, SkIRect is empty.
    */
    int32_t fTop;

    /** May contain any value. The larger of the vertical values when sorted.
        When equal to or less than fLeft, SkIRect is empty.
    */
    int32_t fRight;

    /** May contain any value. The larger of the vertical values when sorted.
        When equal to or less than fTop, SkIRect is empty.
    */
    int32_t fBottom;

    /** Returns constructed SkIRect set to (0, 0, 0, 0).
        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.

        @return  bounds (0, 0, 0, 0)
    */
    static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeEmpty() {
        return SkIRect{0, 0, 0, 0};
    }

#ifdef SK_SUPPORT_LEGACY_RECTMAKELARGEST
    /** Returns constructed SkIRect setting left and top to most negative value, and
        setting right and bottom to most positive value.

        @return  bounds (SK_MinS32, SK_MinS32, SK_MaxS32, SK_MaxS32)
    */
    static SkIRect SK_WARN_UNUSED_RESULT MakeLargest() {
        return { SK_MinS32, SK_MinS32, SK_MaxS32, SK_MaxS32 };
    }
#endif

    /** Returns constructed SkIRect set to (0, 0, w, h). Does not validate input; w or h
        may be negative.

        @param w  width of constructed SkRect
        @param h  height of constructed SkRect
        @return   bounds (0, 0, w, h)
    */
    static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeWH(int32_t w, int32_t h) {
        return SkIRect{0, 0, w, h};
    }

    /** Returns constructed SkIRect set to (0, 0, size.width(), size.height()).
        Does not validate input; size.width() or size.height() may be negative.

        @param size  values for SkRect width and height
        @return      bounds (0, 0, size.width(), size.height())
    */
    static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeSize(const SkISize& size) {
        return SkIRect{0, 0, size.fWidth, size.fHeight};
    }

    /** Returns constructed SkIRect set to (l, t, r, b). Does not sort input; SkRect may
        result in fLeft greater than fRight, or fTop greater than fBottom.

        @param l  integer stored in fLeft
        @param t  integer stored in fTop
        @param r  integer stored in fRight
        @param b  integer stored in fBottom
        @return   bounds (l, t, r, b)
    */
    static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeLTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        return SkIRect{l, t, r, b};
    }

    /** Returns constructed SkIRect set to: (x, y, x + w, y + h). Does not validate input;
        w or h may be negative.

        @param x  stored in fLeft
        @param y  stored in fTop
        @param w  added to x and stored in fRight
        @param h  added to y and stored in fBottom
        @return   bounds at (x, y) with width w and height h
    */
    static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeXYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        return SkIRect{x, y, x + w, y + h};
    }

    /** Returns left edge of SkIRect, if sorted.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    int32_t left() const { return fLeft; }

    /** Returns top edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    int32_t top() const { return fTop; }

    /** Returns right edge of SkIRect, if sorted.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fRight
    */
    int32_t right() const { return fRight; }

    /** Returns bottom edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fBottom
    */
    int32_t bottom() const { return fBottom; }

    /** Returns left edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    int32_t x() const { return fLeft; }

    /** Returns top edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    int32_t y() const { return fTop; }

    /** Returns span on the x-axis. This does not check if SkIRect is sorted, or if
        result fits in 32-bit signed integer; result may be negative.

        @return  fRight minus fLeft
    */
    int32_t width() const { return SkSub32(fRight, fLeft); }

    /** Returns span on the y-axis. This does not check if SkIRect is sorted, or if
        result fits in 32-bit signed integer; result may be negative.

        @return  fBottom minus fTop
    */
    int32_t height() const { return SkSub32(fBottom, fTop); }

    /** Returns spans on the x-axis and y-axis. This does not check if SkIRect is sorted,
        or if result fits in 32-bit signed integer; result may be negative.

        @return  SkISize (width, height)
    */
    SkISize size() const { return SkISize::Make(this->width(), this->height()); }

    /** Returns average of left edge and right edge. Result does not change if SkRect
        is sorted. Result may be incorrect if SkRect is far from the origin.

        Result is rounded down.

        @return  midpoint in x
    */
    int32_t centerX() const { return (fRight + fLeft) >> 1; }

    /** Returns average of top edge and bottom edge. Result does not change if SkRect
        is sorted. Result may be incorrect if SkRect is far from the origin.

        Result is rounded down.

        @return  midpoint in y
    */
    int32_t centerY() const { return (fBottom + fTop) >> 1; }

    int64_t width64() const { return (int64_t)fRight - (int64_t)fLeft; }
    int64_t height64() const { return (int64_t)fBottom - (int64_t)fTop; }

    /** Returns true if the rectangle's dimensions are non-positive.
     *  Unlike isEmpty(), this does not worry about the size of the width or height, which could
     *  exceed int32_t but still be logically non-empty.
     */
    bool isEmpty64() const { return fRight <= fLeft || fBottom <= fTop; }

    /** Returns true if the rectangle's dimensions are non-positive or if either its width or hieght
     *  exceeds int32_t.
     */
    bool isEmpty() const {
        int64_t w = this->width64();
        int64_t h = this->height64();
        if (w <= 0 || h <= 0) {
            return true;
        }
        // Return true if either exceeds int32_t
        return !sk_64_isS32(w | h);
    }

    /** Returns true if all members in a: fLeft, fTop, fRight, and fBottom; are
        identical to corresponding members in b.

        @param a  SkIRect to compare
        @param b  SkIRect to compare
        @return   true if members are equal
    */
    friend bool operator==(const SkIRect& a, const SkIRect& b) {
        return !memcmp(&a, &b, sizeof(a));
    }

    /** Returns true if any member in a: fLeft, fTop, fRight, and fBottom; is not
        identical to the corresponding member in b.

        @param a  SkIRect to compare
        @param b  SkIRect to compare
        @return   true if members are not equal
    */
    friend bool operator!=(const SkIRect& a, const SkIRect& b) {
        return !(a == b);
    }

    /** Returns true if all members: fLeft, fTop, fRight, and fBottom; values are
        equal to or larger than -32768 and equal to or smaller than 32767.

        @return  true if members fit in 16-bit word
    */
    bool is16Bit() const {
        return  SkTFitsIn<int16_t>(fLeft) && SkTFitsIn<int16_t>(fTop) &&
                SkTFitsIn<int16_t>(fRight) && SkTFitsIn<int16_t>(fBottom);
    }

    /** Sets SkIRect to (0, 0, 0, 0).

        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.
    */
    void setEmpty() { memset(this, 0, sizeof(*this)); }

    /** Sets SkIRect to (left, top, right, bottom).
        left and right are not sorted; left is not necessarily less than right.
        top and bottom are not sorted; top is not necessarily less than bottom.

        @param left    assigned to fLeft
        @param top     assigned to fTop
        @param right   assigned to fRight
        @param bottom  assigned to fBottom
    */
    void set(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }

    /** Sets SkIRect to (left, top, right, bottom).
        left and right are not sorted; left is not necessarily less than right.
        top and bottom are not sorted; top is not necessarily less than bottom.

        @param left    stored in fLeft
        @param top     stored in fTop
        @param right   stored in fRight
        @param bottom  stored in fBottom
    */
    void setLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        this->set(left, top, right, bottom);
    }

    /** Sets SkIRect to: (x, y, x + width, y + height). Does not validate input;
        width or height may be negative.

        @param x       stored in fLeft
        @param y       stored in fTop
        @param width   added to x and stored in fRight
        @param height  added to y and stored in fBottom
    */
    void setXYWH(int32_t x, int32_t y, int32_t width, int32_t height) {
        fLeft = x;
        fTop = y;
        fRight = x + width;
        fBottom = y + height;
    }

    /** Returns SkIRect offset by (dx, dy).

        If dx is negative, SkIRect returned is moved to the left.
        If dx is positive, SkIRect returned is moved to the right.
        If dy is negative, SkIRect returned is moved upward.
        If dy is positive, SkIRect returned is moved downward.

        @param dx  offset added to fLeft and fRight
        @param dy  offset added to fTop and fBottom
        @return    SkRect offset in x or y, with original width and height
    */
    SkIRect makeOffset(int32_t dx, int32_t dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight + dx, fBottom + dy);
    }

    /** Returns SkIRect, inset by (dx, dy).

        If dx is negative, SkIRect returned is wider.
        If dx is positive, SkIRect returned is narrower.
        If dy is negative, SkIRect returned is taller.
        If dy is positive, SkIRect returned is shorter.

        @param dx  offset added to fLeft and subtracted from fRight
        @param dy  offset added to fTop and subtracted from fBottom
        @return    SkRect inset symmetrically left and right, top and bottom
    */
    SkIRect makeInset(int32_t dx, int32_t dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight - dx, fBottom - dy);
    }

    /** Returns SkIRect, outset by (dx, dy).

        If dx is negative, SkIRect returned is narrower.
        If dx is positive, SkIRect returned is wider.
        If dy is negative, SkIRect returned is shorter.
        If dy is positive, SkIRect returned is taller.

        @param dx  offset subtracted to fLeft and added from fRight
        @param dy  offset subtracted to fTop and added from fBottom
        @return    SkRect outset symmetrically left and right, top and bottom
    */
    SkIRect makeOutset(int32_t dx, int32_t dy) const {
        return MakeLTRB(fLeft - dx, fTop - dy, fRight + dx, fBottom + dy);
    }

    /** Offsets SkIRect by adding dx to fLeft, fRight; and by adding dy to fTop, fBottom.

        If dx is negative, moves SkIRect returned to the left.
        If dx is positive, moves SkIRect returned to the right.
        If dy is negative, moves SkIRect returned upward.
        If dy is positive, moves SkIRect returned downward.

        @param dx  offset added to fLeft and fRight
        @param dy  offset added to fTop and fBottom
    */
    void offset(int32_t dx, int32_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }

    /** Offsets SkIRect by adding delta.fX to fLeft, fRight; and by adding delta.fY to
        fTop, fBottom.

        If delta.fX is negative, moves SkIRect returned to the left.
        If delta.fX is positive, moves SkIRect returned to the right.
        If delta.fY is negative, moves SkIRect returned upward.
        If delta.fY is positive, moves SkIRect returned downward.

        @param delta  offset added to SkIRect
    */
    void offset(const SkIPoint& delta) {
        this->offset(delta.fX, delta.fY);
    }

    /** Offsets SkIRect so that fLeft equals newX, and fTop equals newY. width and height
        are unchanged.

        @param newX  stored in fLeft, preserving width()
        @param newY  stored in fTop, preserving height()
    */
    void offsetTo(int32_t newX, int32_t newY) {
        fRight += newX - fLeft;
        fBottom += newY - fTop;
        fLeft = newX;
        fTop = newY;
    }

    /** Insets SkIRect by (dx,dy).

        If dx is positive, makes SkIRect narrower.
        If dx is negative, makes SkIRect wider.
        If dy is positive, makes SkIRect shorter.
        If dy is negative, makes SkIRect taller.

        @param dx  offset added to fLeft and subtracted from fRight
        @param dy  offset added to fTop and subtracted from fBottom
    */
    void inset(int32_t dx, int32_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  -= dx;
        fBottom -= dy;
    }

    /** Outsets SkIRect by (dx, dy).

        If dx is positive, makes SkRect wider.
        If dx is negative, makes SkRect narrower.
        If dy is positive, makes SkRect taller.
        If dy is negative, makes SkRect shorter.

        @param dx  subtracted to fLeft and added from fRight
        @param dy  subtracted to fTop and added from fBottom
    */
    void outset(int32_t dx, int32_t dy)  { this->inset(-dx, -dy); }

    /** Constructs SkIRect (l, t, r, b) and returns true if constructed SkIRect does not
        intersect SkIRect. Does not check to see if construction or SkIRect is empty.

        Is implemented with short circuit logic so that true can be returned after
        a single compare.

        @param l  x minimum of constructed SkRect
        @param t  y minimum of constructed SkRect
        @param r  x maximum of constructed SkRect
        @param b  y maximum of constructed SkRect
        @return   true if construction and SkIRect have no area in common
    */
    bool quickReject(int l, int t, int r, int b) const {
        return l >= fRight || fLeft >= r || t >= fBottom || fTop >= b;
    }

    /** Returns true if: fLeft <= x < fRight && fTop <= y < fBottom.
        Returns false if SkRect is empty.

        Considers input to describe constructed SkIRect: (x, y, x + 1, y + 1) and
        returns true if constructed area is completely enclosed by SkIRect area.

        @param x  test SkPoint x-coordinate
        @param y  test SkPoint y-coordinate
        @return   true if (x, y) is inside SkIRect
    */
    bool contains(int32_t x, int32_t y) const {
        return  (unsigned)(x - fLeft) < (unsigned)(fRight - fLeft) &&
                (unsigned)(y - fTop) < (unsigned)(fBottom - fTop);
    }

    /** Constructs SkRect to intersect from (left, top, right, bottom). Does not sort
        construction.

        Returns true if SkRect contains construction.
        Returns false if SkRect is empty or construction is empty.

        @param left    x minimum of constructed SkRect
        @param top     y minimum of constructed SkRect
        @param right   x maximum of constructed SkRect
        @param bottom  y maximum of constructed SkRect
        @return        true if all sides of SkIRect are outside construction
    */
    bool contains(int32_t left, int32_t top, int32_t right, int32_t bottom) const {
        return  left < right && top < bottom && !this->isEmpty() && // check for empties
                fLeft <= left && fTop <= top &&
                fRight >= right && fBottom >= bottom;
    }

    /** Returns true if SkRect contains r.
        Returns false if SkRect is empty or r is empty.

        SkRect contains r when SkRect area completely includes r area.

        @param r  SkIRect contained
        @return   true if all sides of SkIRect are outside r
    */
    bool contains(const SkIRect& r) const {
        return  !r.isEmpty() && !this->isEmpty() &&     // check for empties
                fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /** Returns true if SkRect contains r.
        Returns false if SkRect is empty or r is empty.

        SkRect contains r when SkRect area completely includes r area.

        @param r  SkRect contained
        @return   true if all sides of SkIRect are outside r
    */
    bool contains(const SkRect& r) const;

    /** Constructs SkIRect from (left, top, right, bottom). Does not sort
        construction.

        Returns true if SkRect contains construction.
        Asserts if SkIRect is empty or construction is empty, and if SK_DEBUG is defined.

        Return is undefined if SkRect is empty or construction is empty.

        @param left    x minimum of constructed SkRect
        @param top     y minimum of constructed SkRect
        @param right   x maximum of constructed SkRect
        @param bottom  y maximum of constructed SkRect
        @return        true if all sides of SkIRect are outside construction
    */
    bool containsNoEmptyCheck(int32_t left, int32_t top,
                              int32_t right, int32_t bottom) const {
        SkASSERT(fLeft < fRight && fTop < fBottom);
        SkASSERT(left < right && top < bottom);

        return fLeft <= left && fTop <= top &&
               fRight >= right && fBottom >= bottom;
    }

    /** Returns true if SkRect contains construction.
        Asserts if SkIRect is empty or construction is empty, and if SK_DEBUG is defined.

        Return is undefined if SkRect is empty or construction is empty.

        @param r  SkRect contained
        @return   true if all sides of SkIRect are outside r
    */
    bool containsNoEmptyCheck(const SkIRect& r) const {
        return containsNoEmptyCheck(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** Returns true if SkIRect intersects r, and sets SkIRect to intersection.
        Returns false if SkIRect does not intersect r, and leaves SkIRect unchanged.

        Returns false if either r or SkIRect is empty, leaving SkIRect unchanged.

        @param r  limit of result
        @return   true if r and SkRect have area in common
    */
    bool intersect(const SkIRect& r) {
        return this->intersect(*this, r);
    }

    /** Returns true if a intersects b, and sets SkIRect to intersection.
        Returns false if a does not intersect b, and leaves SkIRect unchanged.

        Asserts if either a or b is empty, and if SK_DEBUG is defined.

        @param a  SkIRect to intersect
        @param b  SkIRect to intersect
        @return   true if a and b have area in common
    */
    bool SK_WARN_UNUSED_RESULT intersectNoEmptyCheck(const SkIRect& a, const SkIRect& b) {
        SkASSERT(!a.isEmpty64() && !b.isEmpty64());
        SkIRect r = {
            SkMax32(a.fLeft,   b.fLeft),
            SkMax32(a.fTop,    b.fTop),
            SkMin32(a.fRight,  b.fRight),
            SkMin32(a.fBottom, b.fBottom)
        };
        if (r.isEmpty()) {
            return false;
        }
        *this = r;
        return true;
    }

    /** Returns true if a intersects b, and sets SkIRect to intersection.
     Returns false if a does not intersect b, and leaves SkIRect unchanged.

     Returns false if either a or b is empty, leaving SkIRect unchanged.

     @param a  SkIRect to intersect
     @param b  SkIRect to intersect
     @return   true if a and b have area in common
     */
    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& a, const SkIRect& b) {
        if (a.isEmpty64() || b.isEmpty64()) {
            return false;
        }
        return this->intersectNoEmptyCheck(a, b);
    }

    /** Constructs SkIRect to intersect from (left, top, right, bottom). Does not sort
        construction.

        Returns true if SkIRect intersects construction, and sets SkIRect to intersection.
        Returns false if SkIRect does not intersect construction, and leaves SkIRect unchanged.

        Returns false if either construction or SkIRect is empty, leaving SkIRect unchanged.

        @param left    x minimum of constructed SkIRect
        @param top     y minimum of constructed SkIRect
        @param right   x maximum of constructed SkIRect
        @param bottom  y maximum of constructed SkIRect
        @return        true if construction and SkIRect have area in common
    */
    bool intersect(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        return this->intersect(*this, {left, top, right, bottom});
    }

    /** Returns true if a intersects b.
        Returns false if either a or b is empty, or do not intersect.

        @param a  SkIRect to intersect
        @param b  SkIRect to intersect
        @return   true if a and b have area in common
    */
    static bool Intersects(const SkIRect& a, const SkIRect& b) {
        SkIRect dummy;
        return dummy.intersect(a, b);
    }

    /** Returns true if a intersects b.
        Asserts if either a or b is empty, and if SK_DEBUG is defined.

        @param a  SkIRect to intersect
        @param b  SkIRect to intersect
        @return   true if a and b have area in common
    */
    static bool IntersectsNoEmptyCheck(const SkIRect& a, const SkIRect& b) {
        SkIRect dummy;
        return dummy.intersectNoEmptyCheck(a, b);
    }

    /** Constructs SkRect to intersect from (left, top, right, bottom). Does not sort
        construction.

        Sets SkRect to the union of itself and the construction.

        Has no effect if construction is empty. Otherwise, if SkRect is empty, sets
        SkRect to construction.

        @param left    x minimum of constructed SkRect
        @param top     y minimum of constructed SkRect
        @param right   x maximum of constructed SkRect
        @param bottom  y maximum of constructed SkRect
    */
    void join(int32_t left, int32_t top, int32_t right, int32_t bottom);

    /** Sets SkRect to the union of itself and r.

        Has no effect if r is empty. Otherwise, if SkRect is empty, sets SkRect to r.

        @param r  expansion SkRect
    */
    void join(const SkIRect& r) {
        this->join(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** Swaps fLeft and fRight if fLeft is greater than fRight; and swaps
        fTop and fBottom if fTop is greater than fBottom. Result may be empty,
        and width() and height() will be zero or positive.
    */
    void sort() {
        if (fLeft > fRight) {
            SkTSwap<int32_t>(fLeft, fRight);
        }
        if (fTop > fBottom) {
            SkTSwap<int32_t>(fTop, fBottom);
        }
    }

    /** Returns SkRect with fLeft and fRight swapped if fLeft is greater than fRight; and
        with fTop and fBottom swapped if fTop is greater than fBottom. Result may be empty;
        and width() and height() will be zero or positive.

        @return  sorted SkIRect
    */
    SkIRect makeSorted() const {
        return MakeLTRB(SkMin32(fLeft, fRight), SkMin32(fTop, fBottom),
                        SkMax32(fLeft, fRight), SkMax32(fTop, fBottom));
    }

    /** Returns a reference to immutable empty SkIRect, set to (0, 0, 0, 0).

        @return  global SkIRect set to all zeroes
    */
    static const SkIRect& SK_WARN_UNUSED_RESULT EmptyIRect() {
        static const SkIRect gEmpty = { 0, 0, 0, 0 };
        return gEmpty;
    }
};

/** \struct SkRect
    SkRect holds four SkScalar coordinates describing the upper and
    lower bounds of a rectangle. SkRect may be created from outer bounds or
    from position, width, and height. SkRect describes an area; if its right
    is less than or equal to its left, or if its bottom is less than or equal to
    its top, it is considered empty.
*/
struct SK_API SkRect {

    /** May contain any value, including infinities and NaN. The smaller of the
        horizontal values when sorted. When equal to or greater than fRight, SkRect is empty.
    */
    SkScalar fLeft;

    /** May contain any value, including infinities and NaN. The smaller of the
        vertical values when sorted. When equal to or greater than fBottom, SkRect is empty.
    */
    SkScalar fTop;

    /** May contain any value, including infinities and NaN. The larger of the
        horizontal values when sorted. When equal to or less than fLeft, SkRect is empty.
    */
    SkScalar fRight;

    /** May contain any value, including infinities and NaN. The larger of the
        vertical values when sorted. When equal to or less than fTop, SkRect is empty.
    */
    SkScalar fBottom;

    /** Returns constructed SkRect set to (0, 0, 0, 0).
        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.

        @return  bounds (0, 0, 0, 0)
    */
    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeEmpty() {
        return SkRect{0, 0, 0, 0};
    }

#ifdef SK_SUPPORT_LEGACY_RECTMAKELARGEST
    /** Returns constructed SkRect setting left and top to most negative finite value, and
        setting right and bottom to most positive finite value.

        @return  bounds (SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax)
    */
    static SkRect SK_WARN_UNUSED_RESULT MakeLargest() {
        return { SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax };
    }
#endif

    /** Returns constructed SkRect set to SkScalar values (0, 0, w, h). Does not
        validate input; w or h may be negative.

        Passing integer values may generate a compiler warning since SkRect cannot
        represent 32-bit integers exactly. Use SkIRect for an exact integer rectangle.

        @param w  SkScalar width of constructed SkRect
        @param h  SkScalar height of constructed SkRect
        @return   bounds (0, 0, w, h)
    */
    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeWH(SkScalar w, SkScalar h) {
        return SkRect{0, 0, w, h};
    }

    /** Returns constructed SkRect set to integer values (0, 0, w, h). Does not validate
        input; w or h may be negative.

        Use to avoid a compiler warning that input may lose precision when stored.
        Use SkIRect for an exact integer rectangle.

        @param w  integer width of constructed SkRect
        @param h  integer height of constructed SkRect
        @return   bounds (0, 0, w, h)
    */
    static SkRect SK_WARN_UNUSED_RESULT MakeIWH(int w, int h) {
        SkRect r;
        r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));
        return r;
    }

    /** Returns constructed SkRect set to (0, 0, size.width(), size.height()). Does not
        validate input; size.width() or size.height() may be negative.

        @param size  SkScalar values for SkRect width and height
        @return      bounds (0, 0, size.width(), size.height())
    */
    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeSize(const SkSize& size) {
        return SkRect{0, 0, size.fWidth, size.fHeight};
    }

    /** Returns constructed SkRect set to (l, t, r, b). Does not sort input; SkRect may
        result in fLeft greater than fRight, or fTop greater than fBottom.

        @param l  SkScalar stored in fLeft
        @param t  SkScalar stored in fTop
        @param r  SkScalar stored in fRight
        @param b  SkScalar stored in fBottom
        @return   bounds (l, t, r, b)
    */
    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeLTRB(SkScalar l, SkScalar t, SkScalar r,
                                                           SkScalar b) {
        return SkRect {l, t, r, b};
    }

    /** Returns constructed SkRect set to (x, y, x + w, y + h). Does not validate input;
        w or h may be negative.

        @param x  stored in fLeft
        @param y  stored in fTop
        @param w  added to x and stored in fRight
        @param h  added to y and stored in fBottom
        @return   bounds at (x, y) with width w and height h
    */
    static constexpr SkRect SK_WARN_UNUSED_RESULT MakeXYWH(SkScalar x, SkScalar y, SkScalar w, SkScalar h) {
        return SkRect {x, y, x + w, y + h};
    }

    /** Deprecated.

        @param irect  integer rect
        @return       irect as SkRect
    */
    SK_ATTR_DEPRECATED("use Make()")
    static SkRect SK_WARN_UNUSED_RESULT MakeFromIRect(const SkIRect& irect) {
        SkRect r;
        r.set(SkIntToScalar(irect.fLeft),
              SkIntToScalar(irect.fTop),
              SkIntToScalar(irect.fRight),
              SkIntToScalar(irect.fBottom));
        return r;
    }

    /** Returns constructed SkIRect set to (0, 0, size.width(), size.height()).
        Does not validate input; size.width() or size.height() may be negative.

        @param size  integer values for SkRect width and height
        @return      bounds (0, 0, size.width(), size.height())
    */
    static SkRect Make(const SkISize& size) {
        return MakeIWH(size.width(), size.height());
    }

    /** Returns constructed SkIRect set to irect, promoting integers to scalar.
        Does not validate input; fLeft may be greater than fRight, fTop may be greater
        than fBottom.

        @param irect  integer unsorted bounds
        @return       irect members converted to SkScalar
    */
    static SkRect SK_WARN_UNUSED_RESULT Make(const SkIRect& irect) {
        SkRect r;
        r.set(SkIntToScalar(irect.fLeft),
              SkIntToScalar(irect.fTop),
              SkIntToScalar(irect.fRight),
              SkIntToScalar(irect.fBottom));
        return r;
    }

    /** Returns true if fLeft is equal to or greater than fRight, or if fTop is equal
        to or greater than fBottom. Call sort() to reverse rectangles with negative
        width() or height().

        @return  true if width() or height() are zero or negative
    */
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    /** Returns true if fLeft is equal to or less than fRight, or if fTop is equal
        to or less than fBottom. Call sort() to reverse rectangles with negative
        width() or height().

        @return  true if width() or height() are zero or positive
    */
    bool isSorted() const { return fLeft <= fRight && fTop <= fBottom; }

    /** Returns true if all values in the rectangle are finite: SK_ScalarMin or larger,
        and SK_ScalarMax or smaller.

        @return  true if no member is infinite or NaN
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

    /** Returns left edge of SkRect, if sorted. Call isSorted() to see if SkRect is valid.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    SkScalar    x() const { return fLeft; }

    /** Returns top edge of SkRect, if sorted. Call isEmpty() to see if SkRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    SkScalar    y() const { return fTop; }

    /** Returns left edge of SkRect, if sorted. Call isSorted() to see if SkRect is valid.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    SkScalar    left() const { return fLeft; }

    /** Returns top edge of SkRect, if sorted. Call isEmpty() to see if SkRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    SkScalar    top() const { return fTop; }

    /** Returns right edge of SkRect, if sorted. Call isSorted() to see if SkRect is valid.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fRight
    */
    SkScalar    right() const { return fRight; }

    /** Returns bottom edge of SkRect, if sorted. Call isEmpty() to see if SkRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fBottom
    */
    SkScalar    bottom() const { return fBottom; }

    /** Returns span on the x-axis. This does not check if SkRect is sorted, or if
        result fits in 32-bit float; result may be negative or infinity.

        @return  fRight minus fLeft
    */
    SkScalar    width() const { return fRight - fLeft; }

    /** Returns span on the y-axis. This does not check if SkIRect is sorted, or if
        result fits in 32-bit float; result may be negative or infinity.

        @return  fBottom minus fTop
    */
    SkScalar    height() const { return fBottom - fTop; }

    /** Returns average of left edge and right edge. Result does not change if SkRect
        is sorted. Result may overflow to infinity if SkRect is far from the origin.

        @return  midpoint in x
    */
    SkScalar    centerX() const { return SkScalarHalf(fLeft + fRight); }

    /** Returns average of top edge and bottom edge. Result does not change if SkRect
        is sorted. Result may overflow to infinity if SkRect is far from the origin.

        @return  midpoint in y
    */
    SkScalar    centerY() const { return SkScalarHalf(fTop + fBottom); }

    /** Returns true if all members in a: fLeft, fTop, fRight, and fBottom; are
        equal to the corresponding members in b.

        a and b are not equal if either contain NaN. a and b are equal if members
        contain zeroes width different signs.

        @param a  SkRect to compare
        @param b  SkRect to compare
        @return   true if members are equal
    */
    friend bool operator==(const SkRect& a, const SkRect& b) {
        return SkScalarsEqual((const SkScalar*)&a, (const SkScalar*)&b, 4);
    }

    /** Returns true if any in a: fLeft, fTop, fRight, and fBottom; does not
        equal the corresponding members in b.

        a and b are not equal if either contain NaN. a and b are equal if members
        contain zeroes width different signs.

        @param a  SkRect to compare
        @param b  SkRect to compare
        @return   true if members are not equal
    */
    friend bool operator!=(const SkRect& a, const SkRect& b) {
        return !SkScalarsEqual((const SkScalar*)&a, (const SkScalar*)&b, 4);
    }

    /** Returns four points in quad that enclose SkRect ordered as: top-left, top-right,
        bottom-right, bottom-left.
        Consider adding param to control whether quad is CW or CCW.

        @param quad  storage for corners of SkRect
    */
    void toQuad(SkPoint quad[4]) const;

    /** Sets SkRect to (0, 0, 0, 0).

        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.
    */
    void setEmpty() { *this = MakeEmpty(); }

    /** Sets SkRect to src, promoting src members from integer to scalar.
        Very large values in src may lose precision.

        @param src  integer SkRect
    */
    void set(const SkIRect& src) {
        fLeft   = SkIntToScalar(src.fLeft);
        fTop    = SkIntToScalar(src.fTop);
        fRight  = SkIntToScalar(src.fRight);
        fBottom = SkIntToScalar(src.fBottom);
    }

    /** Sets SkRect to (left, top, right, bottom).
        left and right are not sorted; left is not necessarily less than right.
        top and bottom are not sorted; top is not necessarily less than bottom.

        @param left    stored in fLeft
        @param top     stored in fTop
        @param right   stored in fRight
        @param bottom  stored in fBottom
    */
    void set(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }

    /** Sets SkRect to (left, top, right, bottom).
        left and right are not sorted; left is not necessarily less than right.
        top and bottom are not sorted; top is not necessarily less than bottom.

        @param left    stored in fLeft
        @param top     stored in fTop
        @param right   stored in fRight
        @param bottom  stored in fBottom
    */
    void setLTRB(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
        this->set(left, top, right, bottom);
    }

    /** Sets SkRect to (left, top, right, bottom).
        All parameters are promoted from integer to scalar.
        left and right are not sorted; left is not necessarily less than right.
        top and bottom are not sorted; top is not necessarily less than bottom.

        @param left    promoted to SkScalar and stored in fLeft
        @param top     promoted to SkScalar and stored in fTop
        @param right   promoted to SkScalar and stored in fRight
        @param bottom  promoted to SkScalar and stored in fBottom
    */
    void iset(int left, int top, int right, int bottom) {
        fLeft   = SkIntToScalar(left);
        fTop    = SkIntToScalar(top);
        fRight  = SkIntToScalar(right);
        fBottom = SkIntToScalar(bottom);
    }

    /** Sets SkRect to (0, 0, width, height).
        width and height may be zero or negative. width and height are promoted from
        integer to SkScalar, large values may lose precision.

        @param width   promoted to SkScalar and stored in fRight
        @param height  promoted to SkScalar and stored in fBottom
    */
    void isetWH(int width, int height) {
        fLeft = fTop = 0;
        fRight = SkIntToScalar(width);
        fBottom = SkIntToScalar(height);
    }

    /** Sets to bounds of SkPoint array with count entries. If count is zero or smaller,
        or if SkPoint array contains an infinity or NaN, sets SkRect to (0, 0, 0, 0).

        Result is either empty or sorted: fLeft is less than or equal to fRight, and
        fTop is less than or equal to fBottom.

        @param pts    SkPoint array
        @param count  entries in array
    */
    void set(const SkPoint pts[], int count) {
        // set() had been checking for non-finite values, so keep that behavior
        // for now. Now that we have setBoundsCheck(), we may decide to make
        // set() be simpler/faster, and not check for those.
        (void)this->setBoundsCheck(pts, count);
    }

    /** Sets to bounds of SkPoint array with count entries. If count is zero or smaller,
        or if SkPoint array contains an infinity or NaN, sets to (0, 0, 0, 0).

        Result is either empty or sorted: fLeft is less than or equal to fRight, and
        fTop is less than or equal to fBottom.

        @param pts    SkPoint array
        @param count  entries in array
    */
    void setBounds(const SkPoint pts[], int count) {
        (void)this->setBoundsCheck(pts, count);
    }

    /** Sets to bounds of SkPoint array with count entries. Returns false if count is
        zero or smaller, or if SkPoint array contains an infinity or NaN; in these cases
        sets SkRect to (0, 0, 0, 0).

        Result is either empty or sorted: fLeft is less than or equal to fRight, and
        fTop is less than or equal to fBottom.

        @param pts    SkPoint array
        @param count  entries in array
        @return       true if all SkPoint values are finite
    */
    bool setBoundsCheck(const SkPoint pts[], int count);

    /** Sets bounds to the smallest SkRect enclosing points p0 and p1. The result is
        sorted and may be empty. Does not check to see if values are finite.

        @param p0  corner to include
        @param p1  corner to include
    */
    void set(const SkPoint& p0, const SkPoint& p1) {
        fLeft =   SkMinScalar(p0.fX, p1.fX);
        fRight =  SkMaxScalar(p0.fX, p1.fX);
        fTop =    SkMinScalar(p0.fY, p1.fY);
        fBottom = SkMaxScalar(p0.fY, p1.fY);
    }

    /** Sets SkRect to (x, y, x + width, y + height). Does not validate input;
        width or height may be negative.

        @param x       stored in fLeft
        @param y       stored in fTop
        @param width   added to x and stored in fRight
        @param height  added to y and stored in fBottom
    */
    void setXYWH(SkScalar x, SkScalar y, SkScalar width, SkScalar height) {
        fLeft = x;
        fTop = y;
        fRight = x + width;
        fBottom = y + height;
    }

    /** Sets SkRect to (0, 0, width, height). Does not validate input;
        width or height may be negative.

        @param width   stored in fRight
        @param height  stored in fBottom
    */
    void setWH(SkScalar width, SkScalar height) {
        fLeft = 0;
        fTop = 0;
        fRight = width;
        fBottom = height;
    }

    /** Returns SkRect offset by (dx, dy).

        If dx is negative, SkRect returned is moved to the left.
        If dx is positive, SkRect returned is moved to the right.
        If dy is negative, SkRect returned is moved upward.
        If dy is positive, SkRect returned is moved downward.

        @param dx  added to fLeft and fRight
        @param dy  added to fTop and fBottom
        @return    SkRect offset in x or y, with original width and height
    */
    SkRect makeOffset(SkScalar dx, SkScalar dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight + dx, fBottom + dy);
    }

    /** Returns SkRect, inset by (dx, dy).

        If dx is negative, SkRect returned is wider.
        If dx is positive, SkRect returned is narrower.
        If dy is negative, SkRect returned is taller.
        If dy is positive, SkRect returned is shorter.

        @param dx  added to fLeft and subtracted from fRight
        @param dy  added to fTop and subtracted from fBottom
        @return    SkRect inset symmetrically left and right, top and bottom
    */
    SkRect makeInset(SkScalar dx, SkScalar dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight - dx, fBottom - dy);
    }

    /** Returns SkRect, outset by (dx, dy).

        If dx is negative, SkRect returned is narrower.
        If dx is positive, SkRect returned is wider.
        If dy is negative, SkRect returned is shorter.
        If dy is positive, SkRect returned is taller.

        @param dx  subtracted to fLeft and added from fRight
        @param dy  subtracted to fTop and added from fBottom
        @return    SkRect outset symmetrically left and right, top and bottom
    */
    SkRect makeOutset(SkScalar dx, SkScalar dy) const {
        return MakeLTRB(fLeft - dx, fTop - dy, fRight + dx, fBottom + dy);
    }

    /** Offsets SkRect by adding dx to fLeft, fRight; and by adding dy to fTop, fBottom.

        If dx is negative, moves SkRect to the left.
        If dx is positive, moves SkRect to the right.
        If dy is negative, moves SkRect upward.
        If dy is positive, moves SkRect downward.

        @param dx  offset added to fLeft and fRight
        @param dy  offset added to fTop and fBottom
    */
    void offset(SkScalar dx, SkScalar dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }

    /** Offsets SkRect by adding delta.fX to fLeft, fRight; and by adding delta.fY to
        fTop, fBottom.

        If delta.fX is negative, moves SkRect to the left.
        If delta.fX is positive, moves SkRect to the right.
        If delta.fY is negative, moves SkRect upward.
        If delta.fY is positive, moves SkRect downward.

        @param delta  added to SkRect
    */
    void offset(const SkPoint& delta) {
        this->offset(delta.fX, delta.fY);
    }

    /** Offsets SkRect so that fLeft equals newX, and fTop equals newY. width and height
        are unchanged.

        @param newX  stored in fLeft, preserving width()
        @param newY  stored in fTop, preserving height()
    */
    void offsetTo(SkScalar newX, SkScalar newY) {
        fRight += newX - fLeft;
        fBottom += newY - fTop;
        fLeft = newX;
        fTop = newY;
    }

    /** Insets SkRect by (dx, dy).

        If dx is positive, makes SkRect narrower.
        If dx is negative, makes SkRect wider.
        If dy is positive, makes SkRect shorter.
        If dy is negative, makes SkRect taller.

        @param dx  added to fLeft and subtracted from fRight
        @param dy  added to fTop and subtracted from fBottom
    */
    void inset(SkScalar dx, SkScalar dy)  {
        fLeft   += dx;
        fTop    += dy;
        fRight  -= dx;
        fBottom -= dy;
    }

    /** Outsets SkRect by (dx, dy).

        If dx is positive, makes SkRect wider.
        If dx is negative, makes SkRect narrower.
        If dy is positive, makes SkRect taller.
        If dy is negative, makes SkRect shorter.

        @param dx  subtracted to fLeft and added from fRight
        @param dy  subtracted to fTop and added from fBottom
    */
    void outset(SkScalar dx, SkScalar dy)  { this->inset(-dx, -dy); }

    /** Returns true if SkRect intersects r, and sets SkRect to intersection.
        Returns false if SkRect does not intersect r, and leaves SkRect unchanged.

        Returns false if either r or SkRect is empty, leaving SkRect unchanged.

        @param r  limit of result
        @return   true if r and SkRect have area in common
    */
    bool intersect(const SkRect& r);

    /** Constructs SkRect to intersect from (left, top, right, bottom). Does not sort
        construction.

        Returns true if SkRect intersects construction, and sets SkRect to intersection.
        Returns false if SkRect does not intersect construction, and leaves SkRect unchanged.

        Returns false if either construction or SkRect is empty, leaving SkRect unchanged.

        @param left    x minimum of constructed SkRect
        @param top     y minimum of constructed SkRect
        @param right   x maximum of constructed SkRect
        @param bottom  y maximum of constructed SkRect
        @return        true if construction and SkRect have area in common
    */
    bool intersect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);

    /** Returns true if a intersects b, and sets SkRect to intersection.
        Returns false if a does not intersect b, and leaves SkRect unchanged.

        Returns false if either a or b is empty, leaving SkRect unchanged.

        @param a  SkRect to intersect
        @param b  SkRect to intersect
        @return   true if a and b have area in common
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

    /** Constructs SkRect to intersect from (left, top, right, bottom). Does not sort
        construction.

        Returns true if SkRect intersects construction.
        Returns false if either construction or SkRect is empty, or do not intersect.

        @param left    x minimum of constructed SkRect
        @param top     y minimum of constructed SkRect
        @param right   x maximum of constructed SkRect
        @param bottom  y maximum of constructed SkRect
        @return        true if construction and SkRect have area in common
    */
    bool intersects(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) const {
        return Intersects(fLeft, fTop, fRight, fBottom, left, top, right, bottom);
    }

    /** Returns true if SkRect intersects r.
        Returns false if either r or SkRect is empty, or do not intersect.

        @param r  SkRect to intersect
        @return   true if r and SkRect have area in common
    */
    bool intersects(const SkRect& r) const {
        return Intersects(fLeft, fTop, fRight, fBottom,
                          r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** Returns true if a intersects b.
        Returns false if either a or b is empty, or do not intersect.

        @param a  SkRect to intersect
        @param b  SkRect to intersect
        @return   true if a and b have area in common
    */
    static bool Intersects(const SkRect& a, const SkRect& b) {
        return Intersects(a.fLeft, a.fTop, a.fRight, a.fBottom,
                          b.fLeft, b.fTop, b.fRight, b.fBottom);
    }

    /** Constructs SkRect to intersect from (left, top, right, bottom). Does not sort
        construction.

        Sets SkRect to the union of itself and the construction.

        Has no effect if construction is empty. Otherwise, if SkRect is empty, sets
        SkRect to construction.

        @param left    x minimum of constructed SkRect
        @param top     y minimum of constructed SkRect
        @param right   x maximum of constructed SkRect
        @param bottom  y maximum of constructed SkRect
    */
    void join(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);

    /** Sets SkRect to the union of itself and r.

        Has no effect if r is empty. Otherwise, if SkRect is empty, sets
        SkRect to r.

        @param r  expansion SkRect
    */
    void join(const SkRect& r) {
        this->join(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    /** Sets SkRect to the union of itself and r.

        Asserts if r is empty and SK_DEBUG is defined.
        If SkRect is empty, sets SkRect to r.

        May produce incorrect results if r is empty.

        @param r  expansion SkRect
    */
    void joinNonEmptyArg(const SkRect& r) {
        SkASSERT(!r.isEmpty());
        // if we are empty, just assign
        if (fLeft >= fRight || fTop >= fBottom) {
            *this = r;
        } else {
            this->joinPossiblyEmptyRect(r);
        }
    }

    /** Sets SkRect to the union of itself and the construction.

        May produce incorrect results if SkRect or r is empty.

        @param r  expansion SkRect
    */
    void joinPossiblyEmptyRect(const SkRect& r) {
        fLeft   = SkMinScalar(fLeft, r.left());
        fTop    = SkMinScalar(fTop, r.top());
        fRight  = SkMaxScalar(fRight, r.right());
        fBottom = SkMaxScalar(fBottom, r.bottom());
    }

    /** Returns true if SkRect contains r.
        Returns false if SkRect is empty or r is empty.

        SkRect contains r when SkRect area completely includes r area.

        @param r  SkRect contained
        @return   true if all sides of SkRect are outside r
    */
    bool contains(const SkRect& r) const {
        // todo: can we eliminate the this->isEmpty check?
        return  !r.isEmpty() && !this->isEmpty() &&
                fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /** Returns true if SkRect contains r.
        Returns false if SkRect is empty or r is empty.

        SkRect contains r when SkRect area completely includes r area.

        @param r  SkIRect contained
        @return   true if all sides of SkRect are outside r
    */
    bool contains(const SkIRect& r) const {
        // todo: can we eliminate the this->isEmpty check?
        return  !r.isEmpty() && !this->isEmpty() &&
                fLeft <= SkIntToScalar(r.fLeft) && fTop <= SkIntToScalar(r.fTop) &&
                fRight >= SkIntToScalar(r.fRight) && fBottom >= SkIntToScalar(r.fBottom);
    }

    /** Sets SkIRect by adding 0.5 and discarding the fractional portion of SkRect
        members, using (SkScalarRoundToInt(fLeft), SkScalarRoundToInt(fTop),
                        SkScalarRoundToInt(fRight), SkScalarRoundToInt(fBottom)).

        @param dst  storage for SkIRect
    */
    void round(SkIRect* dst) const {
        SkASSERT(dst);
        dst->set(SkScalarRoundToInt(fLeft), SkScalarRoundToInt(fTop),
                 SkScalarRoundToInt(fRight), SkScalarRoundToInt(fBottom));
    }

    /** Sets SkIRect by discarding the fractional portion of fLeft and fTop; and
        rounding up fRight and fbottom, using (SkScalarFloorToInt(fLeft), SkScalarFloorToInt(fTop),
                                               SkScalarCeilToInt(fRight), SkScalarCeilToInt(fBottom)).

        @param dst  storage for SkIRect
    */
    void roundOut(SkIRect* dst) const {
        SkASSERT(dst);
        dst->set(SkScalarFloorToInt(fLeft), SkScalarFloorToInt(fTop),
                 SkScalarCeilToInt(fRight), SkScalarCeilToInt(fBottom));
    }

    /** Sets SkRect by discarding the fractional portion of fLeft and fTop; and
        rounding up fRight and fbottom, using (SkScalarFloorToInt(fLeft), SkScalarFloorToInt(fTop),
                                               SkScalarCeilToInt(fRight), SkScalarCeilToInt(fBottom)).

        @param dst  storage for SkRect
    */
    void roundOut(SkRect* dst) const {
        dst->set(SkScalarFloorToScalar(fLeft),
                 SkScalarFloorToScalar(fTop),
                 SkScalarCeilToScalar(fRight),
                 SkScalarCeilToScalar(fBottom));
    }

    /** Sets SkRect by rounding up fLeft and fTop; and
        discarding the fractional portion of fRight and fbottom, using
        (SkScalarCeilToInt(fLeft), SkScalarCeilToInt(fTop),
         SkScalarFloorToInt(fRight), SkScalarFloorToInt(fBottom)).

        @param dst  storage for SkIRect
    */
    void roundIn(SkIRect* dst) const {
        SkASSERT(dst);
        dst->set(SkScalarCeilToInt(fLeft), SkScalarCeilToInt(fTop),
                 SkScalarFloorToInt(fRight), SkScalarFloorToInt(fBottom));
    }

    /** Returns SkIRect by adding 0.5 and discarding the fractional portion of SkRect
        members, using (SkScalarRoundToInt(fLeft), SkScalarRoundToInt(fTop),
                        SkScalarRoundToInt(fRight), SkScalarRoundToInt(fBottom)).

        @return  rounded SkIRect
    */
    SkIRect round() const {
        SkIRect ir;
        this->round(&ir);
        return ir;
    }

    /** Sets SkIRect by discarding the fractional portion of fLeft and fTop; and
        rounding up fRight and fbottom, using (SkScalarFloorToInt(fLeft), SkScalarFloorToInt(fTop),
                                               SkScalarCeilToInt(fRight), SkScalarCeilToInt(fBottom)).

        @return  rounded SkIRect
    */
    SkIRect roundOut() const {
        SkIRect ir;
        this->roundOut(&ir);
        return ir;
    }

    /** Swaps fLeft and fRight if fLeft is greater than fRight; and swaps
        fTop and fBottom if fTop is greater than fBottom. Result may be empty;
        and width() and height() will be zero or positive.
    */
    void sort() {
        if (fLeft > fRight) {
            SkTSwap<SkScalar>(fLeft, fRight);
        }

        if (fTop > fBottom) {
            SkTSwap<SkScalar>(fTop, fBottom);
        }
    }

    /** Returns SkRect with fLeft and fRight swapped if fLeft is greater than fRight; and
        with fTop and fBottom swapped if fTop is greater than fBottom. Result may be empty;
        and width() and height() will be zero or positive.

        @return  sorted SkRect
    */
    SkRect makeSorted() const {
        return MakeLTRB(SkMinScalar(fLeft, fRight), SkMinScalar(fTop, fBottom),
                        SkMaxScalar(fLeft, fRight), SkMaxScalar(fTop, fBottom));
    }

    /** Returns pointer to first scalar in SkRect, to treat it as an array with four
        entries.

        @return  pointer to fLeft
    */
    const SkScalar* asScalars() const { return &fLeft; }

    /** Writes text representation of SkRect to standard output. Set asHex to true to
        generate exact binary representations of floating point numbers.

        @param asHex  true if SkScalar values are written as hexadecimal
    */
    void dump(bool asHex) const;

    /** Writes text representation of SkRect to standard output. The representation may be
        directly compiled as C++ code. Floating point values are written
        with limited precision; it may not be possible to reconstruct original SkRect
        from output.
    */
    void dump() const { this->dump(false); }

    /** Writes text representation of SkRect to standard output. The representation may be
        directly compiled as C++ code. Floating point values are written
        in hexadecimal to preserve their exact bit pattern. The output reconstructs the
        original SkRect.

        Use instead of dump() when submitting
    */
    void dumpHex() const { this->dump(true); }
};

inline bool SkIRect::contains(const SkRect& r) const {
    return  !r.isEmpty() && !this->isEmpty() &&     // check for empties
            (SkScalar)fLeft <= r.fLeft && (SkScalar)fTop <= r.fTop &&
            (SkScalar)fRight >= r.fRight && (SkScalar)fBottom >= r.fBottom;
}

#endif
