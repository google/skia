/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRect_DEFINED
#define SkRect_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTFitsIn.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>

struct SkRect;
class SkString;

/** \struct SkIRect
    SkIRect holds four 32-bit integer coordinates describing the upper and
    lower bounds of a rectangle. SkIRect may be created from outer bounds or
    from position, width, and height. SkIRect describes an area; if its right
    is less than or equal to its left, or if its bottom is less than or equal to
    its top, it is considered empty.
*/
struct SK_API SkIRect {
    int32_t fLeft   = 0; //!< smaller x-axis bounds
    int32_t fTop    = 0; //!< smaller y-axis bounds
    int32_t fRight  = 0; //!< larger x-axis bounds
    int32_t fBottom = 0; //!< larger y-axis bounds

    /** Returns constructed SkIRect set to (0, 0, 0, 0).
        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.

        @return  bounds (0, 0, 0, 0)
    */
    [[nodiscard]] static constexpr SkIRect MakeEmpty() {
        return SkIRect{0, 0, 0, 0};
    }

    /** Returns constructed SkIRect set to (0, 0, w, h). Does not validate input; w or h
        may be negative.

        @param w  width of constructed SkIRect
        @param h  height of constructed SkIRect
        @return   bounds (0, 0, w, h)
    */
    [[nodiscard]] static constexpr SkIRect MakeWH(int32_t w, int32_t h) {
        return SkIRect{0, 0, w, h};
    }

    /** Returns constructed SkIRect set to (0, 0, size.width(), size.height()).
        Does not validate input; size.width() or size.height() may be negative.

        @param size  values for SkIRect width and height
        @return      bounds (0, 0, size.width(), size.height())
    */
    [[nodiscard]] static constexpr SkIRect MakeSize(const SkISize& size) {
        return SkIRect{0, 0, size.fWidth, size.fHeight};
    }

    /** Returns constructed SkIRect set to (pt.x(), pt.y(), pt.x() + size.width(),
        pt.y() + size.height()). Does not validate input; size.width() or size.height() may be
        negative.

        @param pt    values for SkIRect fLeft and fTop
        @param size  values for SkIRect width and height
        @return      bounds at pt with width and height of size
    */
    [[nodiscard]] static constexpr SkIRect MakePtSize(SkIPoint pt, SkISize size) {
        return MakeXYWH(pt.x(), pt.y(), size.width(), size.height());
    }

    /** Returns constructed SkIRect set to (l, t, r, b). Does not sort input; SkIRect may
        result in fLeft greater than fRight, or fTop greater than fBottom.

        @param l  integer stored in fLeft
        @param t  integer stored in fTop
        @param r  integer stored in fRight
        @param b  integer stored in fBottom
        @return   bounds (l, t, r, b)
    */
    [[nodiscard]] static constexpr SkIRect MakeLTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        return SkIRect{l, t, r, b};
    }

    /** Returns constructed SkIRect set to: (x, y, x + w, y + h).
        Does not validate input; w or h may be negative.

        @param x  stored in fLeft
        @param y  stored in fTop
        @param w  added to x and stored in fRight
        @param h  added to y and stored in fBottom
        @return   bounds at (x, y) with width w and height h
    */
    [[nodiscard]] static constexpr SkIRect MakeXYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        return { x, y, Sk32_sat_add(x, w), Sk32_sat_add(y, h) };
    }

    /** Returns left edge of SkIRect, if sorted.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    constexpr int32_t left() const { return fLeft; }

    /** Returns top edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    constexpr int32_t top() const { return fTop; }

    /** Returns right edge of SkIRect, if sorted.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fRight
    */
    constexpr int32_t right() const { return fRight; }

    /** Returns bottom edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fBottom
    */
    constexpr int32_t bottom() const { return fBottom; }

    /** Returns left edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    constexpr int32_t x() const { return fLeft; }

    /** Returns top edge of SkIRect, if sorted. Call isEmpty() to see if SkIRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    constexpr int32_t y() const { return fTop; }

    // Experimental
    constexpr SkIPoint topLeft() const { return {fLeft, fTop}; }

    /** Returns span on the x-axis. This does not check if SkIRect is sorted, or if
        result fits in 32-bit signed integer; result may be negative.

        @return  fRight minus fLeft
    */
    constexpr int32_t width() const { return Sk32_can_overflow_sub(fRight, fLeft); }

    /** Returns span on the y-axis. This does not check if SkIRect is sorted, or if
        result fits in 32-bit signed integer; result may be negative.

        @return  fBottom minus fTop
    */
    constexpr int32_t height() const { return Sk32_can_overflow_sub(fBottom, fTop); }

    /** Returns spans on the x-axis and y-axis. This does not check if SkIRect is sorted,
        or if result fits in 32-bit signed integer; result may be negative.

        @return  SkISize (width, height)
    */
    constexpr SkISize size() const { return SkISize::Make(this->width(), this->height()); }

    /** Returns span on the x-axis. This does not check if SkIRect is sorted, so the
        result may be negative. This is safer than calling width() since width() might
        overflow in its calculation.

        @return  fRight minus fLeft cast to int64_t
    */
    constexpr int64_t width64() const { return (int64_t)fRight - (int64_t)fLeft; }

    /** Returns span on the y-axis. This does not check if SkIRect is sorted, so the
        result may be negative. This is safer than calling height() since height() might
        overflow in its calculation.

        @return  fBottom minus fTop cast to int64_t
    */
    constexpr int64_t height64() const { return (int64_t)fBottom - (int64_t)fTop; }

    /** Returns true if fLeft is equal to or greater than fRight, or if fTop is equal
        to or greater than fBottom. Call sort() to reverse rectangles with negative
        width64() or height64().

        @return  true if width64() or height64() are zero or negative
    */
    bool isEmpty64() const { return fRight <= fLeft || fBottom <= fTop; }

    /** Returns true if width() or height() are zero or negative.

        @return  true if width() or height() are zero or negative
    */
    bool isEmpty() const {
        int64_t w = this->width64();
        int64_t h = this->height64();
        if (w <= 0 || h <= 0) {
            return true;
        }
        // Return true if either exceeds int32_t
        return !SkTFitsIn<int32_t>(w | h);
    }

    /** Returns true if all members in a: fLeft, fTop, fRight, and fBottom; are
        identical to corresponding members in b.

        @param a  SkIRect to compare
        @param b  SkIRect to compare
        @return   true if members are equal
    */
    friend bool operator==(const SkIRect& a, const SkIRect& b) {
        return a.fLeft == b.fLeft && a.fTop == b.fTop &&
               a.fRight == b.fRight && a.fBottom == b.fBottom;
    }

    /** Returns true if any member in a: fLeft, fTop, fRight, and fBottom; is not
        identical to the corresponding member in b.

        @param a  SkIRect to compare
        @param b  SkIRect to compare
        @return   true if members are not equal
    */
    friend bool operator!=(const SkIRect& a, const SkIRect& b) {
        return a.fLeft != b.fLeft || a.fTop != b.fTop ||
               a.fRight != b.fRight || a.fBottom != b.fBottom;
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

        @param left    stored in fLeft
        @param top     stored in fTop
        @param right   stored in fRight
        @param bottom  stored in fBottom
    */
    void setLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }

    /** Sets SkIRect to: (x, y, x + width, y + height).
        Does not validate input; width or height may be negative.

        @param x       stored in fLeft
        @param y       stored in fTop
        @param width   added to x and stored in fRight
        @param height  added to y and stored in fBottom
    */
    void setXYWH(int32_t x, int32_t y, int32_t width, int32_t height) {
        fLeft   = x;
        fTop    = y;
        fRight  = Sk32_sat_add(x, width);
        fBottom = Sk32_sat_add(y, height);
    }

    void setWH(int32_t width, int32_t height) {
        fLeft   = 0;
        fTop    = 0;
        fRight  = width;
        fBottom = height;
    }

    void setSize(SkISize size) {
        fLeft = 0;
        fTop = 0;
        fRight = size.width();
        fBottom = size.height();
    }

    /** Returns SkIRect offset by (dx, dy).

        If dx is negative, SkIRect returned is moved to the left.
        If dx is positive, SkIRect returned is moved to the right.
        If dy is negative, SkIRect returned is moved upward.
        If dy is positive, SkIRect returned is moved downward.

        @param dx  offset added to fLeft and fRight
        @param dy  offset added to fTop and fBottom
        @return    SkIRect offset by dx and dy, with original width and height
    */
    constexpr SkIRect makeOffset(int32_t dx, int32_t dy) const {
        return {
            Sk32_sat_add(fLeft,  dx), Sk32_sat_add(fTop,    dy),
            Sk32_sat_add(fRight, dx), Sk32_sat_add(fBottom, dy),
        };
    }

    /** Returns SkIRect offset by (offset.x(), offset.y()).

        If offset.x() is negative, SkIRect returned is moved to the left.
        If offset.x() is positive, SkIRect returned is moved to the right.
        If offset.y() is negative, SkIRect returned is moved upward.
        If offset.y() is positive, SkIRect returned is moved downward.

        @param offset  translation vector
        @return    SkIRect translated by offset, with original width and height
    */
    constexpr SkIRect makeOffset(SkIVector offset) const {
        return this->makeOffset(offset.x(), offset.y());
    }

    /** Returns SkIRect, inset by (dx, dy).

        If dx is negative, SkIRect returned is wider.
        If dx is positive, SkIRect returned is narrower.
        If dy is negative, SkIRect returned is taller.
        If dy is positive, SkIRect returned is shorter.

        @param dx  offset added to fLeft and subtracted from fRight
        @param dy  offset added to fTop and subtracted from fBottom
        @return    SkIRect inset symmetrically left and right, top and bottom
    */
    SkIRect makeInset(int32_t dx, int32_t dy) const {
        return {
            Sk32_sat_add(fLeft,  dx), Sk32_sat_add(fTop,    dy),
            Sk32_sat_sub(fRight, dx), Sk32_sat_sub(fBottom, dy),
        };
    }

    /** Returns SkIRect, outset by (dx, dy).

        If dx is negative, SkIRect returned is narrower.
        If dx is positive, SkIRect returned is wider.
        If dy is negative, SkIRect returned is shorter.
        If dy is positive, SkIRect returned is taller.

        @param dx  offset subtracted to fLeft and added from fRight
        @param dy  offset subtracted to fTop and added from fBottom
        @return    SkIRect outset symmetrically left and right, top and bottom
    */
    SkIRect makeOutset(int32_t dx, int32_t dy) const {
        return {
            Sk32_sat_sub(fLeft,  dx), Sk32_sat_sub(fTop,    dy),
            Sk32_sat_add(fRight, dx), Sk32_sat_add(fBottom, dy),
        };
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
        fLeft   = Sk32_sat_add(fLeft,   dx);
        fTop    = Sk32_sat_add(fTop,    dy);
        fRight  = Sk32_sat_add(fRight,  dx);
        fBottom = Sk32_sat_add(fBottom, dy);
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
        fRight  = Sk64_pin_to_s32((int64_t)fRight + newX - fLeft);
        fBottom = Sk64_pin_to_s32((int64_t)fBottom + newY - fTop);
        fLeft   = newX;
        fTop    = newY;
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
        fLeft   = Sk32_sat_add(fLeft,   dx);
        fTop    = Sk32_sat_add(fTop,    dy);
        fRight  = Sk32_sat_sub(fRight,  dx);
        fBottom = Sk32_sat_sub(fBottom, dy);
    }

    /** Outsets SkIRect by (dx, dy).

        If dx is positive, makes SkIRect wider.
        If dx is negative, makes SkIRect narrower.
        If dy is positive, makes SkIRect taller.
        If dy is negative, makes SkIRect shorter.

        @param dx  subtracted to fLeft and added from fRight
        @param dy  subtracted to fTop and added from fBottom
    */
    void outset(int32_t dx, int32_t dy)  { this->inset(-dx, -dy); }

    /** Adjusts SkIRect by adding dL to fLeft, dT to fTop, dR to fRight, and dB to fBottom.

        If dL is positive, narrows SkIRect on the left. If negative, widens it on the left.
        If dT is positive, shrinks SkIRect on the top. If negative, lengthens it on the top.
        If dR is positive, narrows SkIRect on the right. If negative, widens it on the right.
        If dB is positive, shrinks SkIRect on the bottom. If negative, lengthens it on the bottom.

        The resulting SkIRect is not checked for validity. Thus, if the resulting SkIRect left is
        greater than right, the SkIRect will be considered empty. Call sort() after this call
        if that is not the desired behavior.

        @param dL  offset added to fLeft
        @param dT  offset added to fTop
        @param dR  offset added to fRight
        @param dB  offset added to fBottom
    */
    void adjust(int32_t dL, int32_t dT, int32_t dR, int32_t dB) {
        fLeft   = Sk32_sat_add(fLeft,   dL);
        fTop    = Sk32_sat_add(fTop,    dT);
        fRight  = Sk32_sat_add(fRight,  dR);
        fBottom = Sk32_sat_add(fBottom, dB);
    }

    /** Returns true if: fLeft <= x < fRight && fTop <= y < fBottom.
        Returns false if SkIRect is empty.

        Considers input to describe constructed SkIRect: (x, y, x + 1, y + 1) and
        returns true if constructed area is completely enclosed by SkIRect area.

        @param x  test SkIPoint x-coordinate
        @param y  test SkIPoint y-coordinate
        @return   true if (x, y) is inside SkIRect
    */
    bool contains(int32_t x, int32_t y) const {
        return x >= fLeft && x < fRight && y >= fTop && y < fBottom;
    }

    /** Returns true if SkIRect contains r.
     Returns false if SkIRect is empty or r is empty.

     SkIRect contains r when SkIRect area completely includes r area.

     @param r  SkIRect contained
     @return   true if all sides of SkIRect are outside r
     */
    bool contains(const SkIRect& r) const {
        return  !r.isEmpty() && !this->isEmpty() &&     // check for empties
                fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /** Returns true if SkIRect contains r.
        Returns false if SkIRect is empty or r is empty.

        SkIRect contains r when SkIRect area completely includes r area.

        @param r  SkRect contained
        @return   true if all sides of SkIRect are outside r
    */
    inline bool contains(const SkRect& r) const;

    /** Returns true if SkIRect contains construction.
        Asserts if SkIRect is empty or construction is empty, and if SK_DEBUG is defined.

        Return is undefined if SkIRect is empty or construction is empty.

        @param r  SkIRect contained
        @return   true if all sides of SkIRect are outside r
    */
    bool containsNoEmptyCheck(const SkIRect& r) const {
        SkASSERT(fLeft < fRight && fTop < fBottom);
        SkASSERT(r.fLeft < r.fRight && r.fTop < r.fBottom);
        return fLeft <= r.fLeft && fTop <= r.fTop && fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /** Returns true if SkIRect intersects r, and sets SkIRect to intersection.
        Returns false if SkIRect does not intersect r, and leaves SkIRect unchanged.

        Returns false if either r or SkIRect is empty, leaving SkIRect unchanged.

        @param r  limit of result
        @return   true if r and SkIRect have area in common
    */
    bool intersect(const SkIRect& r) {
        return this->intersect(*this, r);
    }

    /** Returns true if a intersects b, and sets SkIRect to intersection.
        Returns false if a does not intersect b, and leaves SkIRect unchanged.

        Returns false if either a or b is empty, leaving SkIRect unchanged.

        @param a  SkIRect to intersect
        @param b  SkIRect to intersect
        @return   true if a and b have area in common
    */
    [[nodiscard]] bool intersect(const SkIRect& a, const SkIRect& b);

    /** Returns true if a intersects b.
        Returns false if either a or b is empty, or do not intersect.

        @param a  SkIRect to intersect
        @param b  SkIRect to intersect
        @return   true if a and b have area in common
    */
    static bool Intersects(const SkIRect& a, const SkIRect& b) {
        return SkIRect{}.intersect(a, b);
    }

    /** Sets SkIRect to the union of itself and r.

     Has no effect if r is empty. Otherwise, if SkIRect is empty, sets SkIRect to r.

     @param r  expansion SkIRect

        example: https://fiddle.skia.org/c/@IRect_join_2
     */
    void join(const SkIRect& r);

    /** Swaps fLeft and fRight if fLeft is greater than fRight; and swaps
        fTop and fBottom if fTop is greater than fBottom. Result may be empty,
        and width() and height() will be zero or positive.
    */
    void sort() {
        using std::swap;
        if (fLeft > fRight) {
            swap(fLeft, fRight);
        }
        if (fTop > fBottom) {
            swap(fTop, fBottom);
        }
    }

    /** Returns SkIRect with fLeft and fRight swapped if fLeft is greater than fRight; and
        with fTop and fBottom swapped if fTop is greater than fBottom. Result may be empty;
        and width() and height() will be zero or positive.

        @return  sorted SkIRect
    */
    SkIRect makeSorted() const {
        return MakeLTRB(std::min(fLeft, fRight), std::min(fTop, fBottom),
                        std::max(fLeft, fRight), std::max(fTop, fBottom));
    }

    /** Returns pointer to first int32 in SkIRect, to treat it as an array with four
        entries.

        @return  pointer to fLeft
    */
    const int32_t* asInt32s() const { return &fLeft; }
};

/** \struct SkRect
    SkRect holds four float coordinates describing the upper and
    lower bounds of a rectangle. SkRect may be created from outer bounds or
    from position, width, and height. SkRect describes an area; if its right
    is less than or equal to its left, or if its bottom is less than or equal to
    its top, it is considered empty.
*/
struct SK_API SkRect {
    float fLeft   = 0; //!< smaller x-axis bounds
    float fTop    = 0; //!< smaller y-axis bounds
    float fRight  = 0; //!< larger x-axis bounds
    float fBottom = 0; //!< larger y-axis bounds

    /** Returns constructed SkRect set to (0, 0, 0, 0).
        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.

        @return  bounds (0, 0, 0, 0)
    */
    [[nodiscard]] static constexpr SkRect MakeEmpty() {
        return SkRect{0, 0, 0, 0};
    }

    /** Returns constructed SkRect set to float values (0, 0, w, h). Does not
        validate input; w or h may be negative.

        Passing integer values may generate a compiler warning since SkRect cannot
        represent 32-bit integers exactly. Use SkIRect for an exact integer rectangle.

        @param w  float width of constructed SkRect
        @param h  float height of constructed SkRect
        @return   bounds (0, 0, w, h)
    */
    [[nodiscard]] static constexpr SkRect MakeWH(float w, float h) {
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
    [[nodiscard]] static SkRect MakeIWH(int w, int h) {
        return {0, 0, static_cast<float>(w), static_cast<float>(h)};
    }

    /** Returns constructed SkRect set to (0, 0, size.width(), size.height()). Does not
        validate input; size.width() or size.height() may be negative.

        @param size  float values for SkRect width and height
        @return      bounds (0, 0, size.width(), size.height())
    */
    [[nodiscard]] static constexpr SkRect MakeSize(const SkSize& size) {
        return SkRect{0, 0, size.fWidth, size.fHeight};
    }

    /** Returns constructed SkRect set to (l, t, r, b). Does not sort input; SkRect may
        result in fLeft greater than fRight, or fTop greater than fBottom.

        @param l  float stored in fLeft
        @param t  float stored in fTop
        @param r  float stored in fRight
        @param b  float stored in fBottom
        @return   bounds (l, t, r, b)
    */
    [[nodiscard]] static constexpr SkRect MakeLTRB(float l, float t, float r, float b) {
        return SkRect {l, t, r, b};
    }

    /** Returns constructed SkRect set to (x, y, x + w, y + h).
        Does not validate input; w or h may be negative.

        @param x  stored in fLeft
        @param y  stored in fTop
        @param w  added to x and stored in fRight
        @param h  added to y and stored in fBottom
        @return   bounds at (x, y) with width w and height h
    */
    [[nodiscard]] static constexpr SkRect MakeXYWH(float x, float y, float w, float h) {
        return SkRect {x, y, x + w, y + h};
    }

    /** Returns constructed SkIRect set to (0, 0, size.width(), size.height()).
        Does not validate input; size.width() or size.height() may be negative.

        @param size  integer values for SkRect width and height
        @return      bounds (0, 0, size.width(), size.height())
    */
    static SkRect Make(const SkISize& size) {
        return MakeIWH(size.width(), size.height());
    }

    /** Returns constructed SkIRect set to irect, promoting integers to float.
        Does not validate input; fLeft may be greater than fRight, fTop may be greater
        than fBottom.

        @param irect  integer unsorted bounds
        @return       irect members converted to float
    */
    [[nodiscard]] static SkRect Make(const SkIRect& irect) {
        return {
            static_cast<float>(irect.fLeft), static_cast<float>(irect.fTop),
            static_cast<float>(irect.fRight), static_cast<float>(irect.fBottom)
        };
    }

    /** Returns true if fLeft is equal to or greater than fRight, or if fTop is equal
        to or greater than fBottom. Call sort() to reverse rectangles with negative
        width() or height().

        @return  true if width() or height() are zero or negative
    */
    bool isEmpty() const {
        // We write it as the NOT of a non-empty rect, so we will return true if any values
        // are NaN.
        return !(fLeft < fRight && fTop < fBottom);
    }

    /** Returns true if fLeft is equal to or less than fRight, or if fTop is equal
        to or less than fBottom. Call sort() to reverse rectangles with negative
        width() or height().

        @return  true if width() or height() are zero or positive
    */
    bool isSorted() const { return fLeft <= fRight && fTop <= fBottom; }

    /** Returns true if all values in the rectangle are finite.

        @return  true if no member is infinite or NaN
    */
    bool isFinite() const {
        return SkIsFinite(fLeft, fTop, fRight, fBottom);
    }

    /** Returns left edge of SkRect, if sorted. Call isSorted() to see if SkRect is valid.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    constexpr float x() const { return fLeft; }

    /** Returns top edge of SkRect, if sorted. Call isEmpty() to see if SkRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    constexpr float y() const { return fTop; }

    /** Returns left edge of SkRect, if sorted. Call isSorted() to see if SkRect is valid.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fLeft
    */
    constexpr float left() const { return fLeft; }

    /** Returns top edge of SkRect, if sorted. Call isEmpty() to see if SkRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fTop
    */
    constexpr float top() const { return fTop; }

    /** Returns right edge of SkRect, if sorted. Call isSorted() to see if SkRect is valid.
        Call sort() to reverse fLeft and fRight if needed.

        @return  fRight
    */
    constexpr float right() const { return fRight; }

    /** Returns bottom edge of SkRect, if sorted. Call isEmpty() to see if SkRect may be invalid,
        and sort() to reverse fTop and fBottom if needed.

        @return  fBottom
    */
    constexpr float bottom() const { return fBottom; }

    /** Returns span on the x-axis. This does not check if SkRect is sorted, or if
        result fits in 32-bit float; result may be negative or infinity.

        @return  fRight minus fLeft
    */
    constexpr float width() const { return fRight - fLeft; }

    /** Returns span on the y-axis. This does not check if SkRect is sorted, or if
        result fits in 32-bit float; result may be negative or infinity.

        @return  fBottom minus fTop
    */
    constexpr float height() const { return fBottom - fTop; }

    /** Returns average of left edge and right edge. Result does not change if SkRect
        is sorted. Result may overflow to infinity if SkRect is far from the origin.

        @return  midpoint on x-axis
    */
    constexpr float centerX() const {
        return sk_float_midpoint(fLeft, fRight);
    }

    /** Returns average of top edge and bottom edge. Result does not change if SkRect
        is sorted.

        @return  midpoint on y-axis
    */
    constexpr float centerY() const {
        return sk_float_midpoint(fTop, fBottom);
    }

    /** Returns the point this->centerX(), this->centerY().
        @return  rectangle center
     */
    constexpr SkPoint center() const { return {this->centerX(), this->centerY()}; }

    /** Returns true if all members in a: fLeft, fTop, fRight, and fBottom; are
        equal to the corresponding members in b.

        a and b are not equal if either contain NaN. a and b are equal if members
        contain zeroes with different signs.

        @param a  SkRect to compare
        @param b  SkRect to compare
        @return   true if members are equal
    */
    friend bool operator==(const SkRect& a, const SkRect& b) {
        return a.fLeft == b.fLeft &&
               a.fTop == b.fTop &&
               a.fRight == b.fRight &&
               a.fBottom == b.fBottom;
    }

    /** Returns true if any in a: fLeft, fTop, fRight, and fBottom; does not
        equal the corresponding members in b.

        a and b are not equal if either contain NaN. a and b are equal if members
        contain zeroes with different signs.

        @param a  SkRect to compare
        @param b  SkRect to compare
        @return   true if members are not equal
    */
    friend bool operator!=(const SkRect& a, const SkRect& b) {
        return !(a == b);
    }

    SkPoint TL() const { return {fLeft,  fTop}; }
    SkPoint TR() const { return {fRight, fTop}; }
    SkPoint BL() const { return {fLeft,  fBottom}; }
    SkPoint BR() const { return {fRight, fBottom}; }

    /** Returns four points in quad that enclose SkRect ordered as: TL() ,TR(), BR(), BL()
     */
    std::array<SkPoint, 4> toQuad() const {
        return {
            this->TL(),
            this->TR(),
            this->BR(),
            this->BL(),
        };
    }
    // deprected: use array version, and/or we should add a Span version
    void toQuad(SkPoint quad[4]) const;

    /** Sets SkRect to (0, 0, 0, 0).

        Many other rectangles are empty; if left is equal to or greater than right,
        or if top is equal to or greater than bottom. Setting all members to zero
        is a convenience, but does not designate a special empty rectangle.
    */
    void setEmpty() { *this = MakeEmpty(); }

    /** Sets SkRect to src, promoting src members from integer to float.
        Very large values in src may lose precision.

        @param src  integer SkRect
    */
    void set(const SkIRect& src) {
        fLeft   = src.fLeft;
        fTop    = src.fTop;
        fRight  = src.fRight;
        fBottom = src.fBottom;
    }

    /** Sets SkRect to (left, top, right, bottom).
        left and right are not sorted; left is not necessarily less than right.
        top and bottom are not sorted; top is not necessarily less than bottom.

        @param left    stored in fLeft
        @param top     stored in fTop
        @param right   stored in fRight
        @param bottom  stored in fBottom
    */
    void setLTRB(float left, float top, float right, float bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }

    /**
     * Compute the bounds of the span of points.
     * If the span is empty, returns the empty-rect {0, 0, 0, 0.
     * If the span contains non-finite values (inf or nan), returns {}
     */
    static std::optional<SkRect> Bounds(SkSpan<const SkPoint> pts);

    static SkRect BoundsOrEmpty(SkSpan<const SkPoint> pts) {
        if (auto bounds = Bounds(pts)) {
            return bounds.value();
        } else {
            return MakeEmpty();
        }
    }

    /** Sets to bounds of SkPoint array with count entries. If count is zero or smaller,
        or if SkPoint array contains an infinity or NaN, sets to (0, 0, 0, 0).

        Result is either empty or sorted: fLeft is less than or equal to fRight, and
        fTop is less than or equal to fBottom.

        @param pts    SkPoint span
    */
    void setBounds(SkSpan<const SkPoint> pts) {
        (void)this->setBoundsCheck(pts);
    }

    /** Sets to bounds of the span of points, and return true (if all point values were finite).
     *
     * If the span is empty, set the rect to empty() and return true.
     * If any point contains an infinity or NaN, set the rect to empty and return false.
     *
     * @param pts    SkPoint span
     * example: https://fiddle.skia.org/c/@Rect_setBoundsCheck
     */
    bool setBoundsCheck(SkSpan<const SkPoint> pts);

    /** Sets to bounds of the span of points.
     *
     * If the span is empty, set the rect to empty().
     * If any point contains an infinity or NaN, set the rect to NaN.
     *
     * @param pts    SkPoint span
     * example: https://fiddle.skia.org/c/@Rect_setBoundsNoCheck
     */
    void setBoundsNoCheck(SkSpan<const SkPoint> pts);

#ifdef SK_SUPPORT_UNSPANNED_APIS
    void setBounds(const SkPoint pts[], int count) {
        this->setBounds({pts, count});
    }
    void setBoundsNoCheck(const SkPoint pts[], int count) {
        this->setBoundsNoCheck({pts, count});
    }
    bool setBoundsCheck(const SkPoint pts[], int count) {
        return this->setBoundsCheck({pts, count});
    }
#endif

    /** Sets bounds to the smallest SkRect enclosing SkPoint p0 and p1. The result is
        sorted and may be empty. Does not check to see if values are finite.

        @param p0  corner to include
        @param p1  corner to include
    */
    void set(const SkPoint& p0, const SkPoint& p1) {
        fLeft =   std::min(p0.fX, p1.fX);
        fRight =  std::max(p0.fX, p1.fX);
        fTop =    std::min(p0.fY, p1.fY);
        fBottom = std::max(p0.fY, p1.fY);
    }

    /** Sets SkRect to (x, y, x + width, y + height).
        Does not validate input; width or height may be negative.

        @param x       stored in fLeft
        @param y       stored in fTop
        @param width   added to x and stored in fRight
        @param height  added to y and stored in fBottom
    */
    void setXYWH(float x, float y, float width, float height) {
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
    void setWH(float width, float height) {
        fLeft = 0;
        fTop = 0;
        fRight = width;
        fBottom = height;
    }
    void setIWH(int32_t width, int32_t height) {
        this->setWH(width, height);
    }

    /** Returns SkRect offset by (dx, dy).

        If dx is negative, SkRect returned is moved to the left.
        If dx is positive, SkRect returned is moved to the right.
        If dy is negative, SkRect returned is moved upward.
        If dy is positive, SkRect returned is moved downward.

        @param dx  added to fLeft and fRight
        @param dy  added to fTop and fBottom
        @return    SkRect offset on axes, with original width and height
    */
    constexpr SkRect makeOffset(float dx, float dy) const {
        return MakeLTRB(fLeft + dx, fTop + dy, fRight + dx, fBottom + dy);
    }

    /** Returns SkRect offset by v.

        @param v  added to rect
        @return    SkRect offset on axes, with original width and height
    */
    constexpr SkRect makeOffset(SkVector v) const { return this->makeOffset(v.x(), v.y()); }

    /** Returns SkRect, inset by (dx, dy).

        If dx is negative, SkRect returned is wider.
        If dx is positive, SkRect returned is narrower.
        If dy is negative, SkRect returned is taller.
        If dy is positive, SkRect returned is shorter.

        @param dx  added to fLeft and subtracted from fRight
        @param dy  added to fTop and subtracted from fBottom
        @return    SkRect inset symmetrically left and right, top and bottom
    */
    SkRect makeInset(float dx, float dy) const {
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
    SkRect makeOutset(float dx, float dy) const {
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
    void offset(float dx, float dy) {
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
    void offsetTo(float newX, float newY) {
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
    void inset(float dx, float dy)  {
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
    void outset(float dx, float dy)  { this->inset(-dx, -dy); }

    /** Returns true if SkRect intersects r, and sets SkRect to intersection.
        Returns false if SkRect does not intersect r, and leaves SkRect unchanged.

        Returns false if either r or SkRect is empty, leaving SkRect unchanged.

        @param r  limit of result
        @return   true if r and SkRect have area in common

        example: https://fiddle.skia.org/c/@Rect_intersect
    */
    bool intersect(const SkRect& r);

    /** Returns true if a intersects b, and sets SkRect to intersection.
        Returns false if a does not intersect b, and leaves SkRect unchanged.

        Returns false if either a or b is empty, leaving SkRect unchanged.

        @param a  SkRect to intersect
        @param b  SkRect to intersect
        @return   true if a and b have area in common
    */
    [[nodiscard]] bool intersect(const SkRect& a, const SkRect& b);


private:
    static bool Intersects(float al, float at, float ar, float ab,
                           float bl, float bt, float br, float bb) {
        float L = std::max(al, bl);
        float R = std::min(ar, br);
        float T = std::max(at, bt);
        float B = std::min(ab, bb);
        return L < R && T < B;
    }

public:

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

    /** Sets SkRect to the union of itself and r.

        Has no effect if r is empty. Otherwise, if SkRect is empty, sets
        SkRect to r.

        @param r  expansion SkRect

        example: https://fiddle.skia.org/c/@Rect_join_2
    */
    void join(const SkRect& r);

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
        fLeft   = std::min(fLeft, r.left());
        fTop    = std::min(fTop, r.top());
        fRight  = std::max(fRight, r.right());
        fBottom = std::max(fBottom, r.bottom());
    }

    /** Returns true if: fLeft <= x < fRight && fTop <= y < fBottom.
        Returns false if SkRect is empty.

        @param x  test SkPoint x-coordinate
        @param y  test SkPoint y-coordinate
        @return   true if (x, y) is inside SkRect
    */
    bool contains(float x, float y) const {
        return x >= fLeft && x < fRight && y >= fTop && y < fBottom;
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
                fLeft <= r.fLeft && fTop <= r.fTop &&
                fRight >= r.fRight && fBottom >= r.fBottom;
    }

    /** Sets SkIRect by adding 0.5 and discarding the fractional portion of SkRect
        members, using (sk_float_round2int(fLeft), sk_float_round2int(fTop),
                        sk_float_round2int(fRight), sk_float_round2int(fBottom)).

        @param dst  storage for SkIRect
    */
    void round(SkIRect* dst) const {
        SkASSERT(dst);
        dst->setLTRB(sk_float_round2int(fLeft),  sk_float_round2int(fTop),
                     sk_float_round2int(fRight), sk_float_round2int(fBottom));
    }

    /** Sets SkIRect by discarding the fractional portion of fLeft and fTop; and rounding
        up fRight and fBottom, using
        (sk_float_floor2int(fLeft), sk_float_floor2int(fTop),
         sk_float_ceil2int(fRight), sk_float_ceil2int(fBottom)).

        @param dst  storage for SkIRect
    */
    void roundOut(SkIRect* dst) const {
        SkASSERT(dst);
        dst->setLTRB(sk_float_floor2int(fLeft), sk_float_floor2int(fTop),
                     sk_float_ceil2int(fRight), sk_float_ceil2int(fBottom));
    }

    /** Sets SkRect by discarding the fractional portion of fLeft and fTop; and rounding
        up fRight and fBottom, using
        (std::floor(fLeft), std::floor(fTop),
         std::ceil(fRight), std::ceil(fBottom)).

        @param dst  storage for SkRect
    */
    void roundOut(SkRect* dst) const {
        dst->setLTRB(std::floor(fLeft), std::floor(fTop),
                     std::ceil(fRight), std::ceil(fBottom));
    }

    /** Sets SkRect by rounding up fLeft and fTop; and discarding the fractional portion
        of fRight and fBottom, using
        (sk_float_ceil2int(fLeft), sk_float_ceil2int(fTop),
         sk_float_floor2int(fRight), sk_float_floor2int(fBottom)).

        @param dst  storage for SkIRect
    */
    void roundIn(SkIRect* dst) const {
        SkASSERT(dst);
        dst->setLTRB(sk_float_ceil2int(fLeft),   sk_float_ceil2int(fTop),
                     sk_float_floor2int(fRight), sk_float_floor2int(fBottom));
    }

    /** Returns SkIRect by adding 0.5 and discarding the fractional portion of SkRect
        members, using (sk_float_round2int(fLeft), sk_float_round2int(fTop),
                        sk_float_round2int(fRight), sk_float_round2int(fBottom)).

        @return  rounded SkIRect
    */
    SkIRect round() const {
        SkIRect ir;
        this->round(&ir);
        return ir;
    }

    /** Sets SkIRect by discarding the fractional portion of fLeft and fTop; and rounding
        up fRight and fBottom, using
        (sk_float_floor2int(fLeft), sk_float_floor2int(fTop),
         sk_float_ceil2int(fRight), sk_float_ceil2int(fBottom)).

        @return  rounded SkIRect
    */
    SkIRect roundOut() const {
        SkIRect ir;
        this->roundOut(&ir);
        return ir;
    }
    /** Sets SkIRect by rounding up fLeft and fTop; and discarding the fractional portion
        of fRight and fBottom, using
        (sk_float_ceil2int(fLeft), sk_float_ceil2int(fTop),
         sk_float_floor2int(fRight), sk_float_floor2int(fBottom)).

        @return  rounded SkIRect
    */
    SkIRect roundIn() const {
        SkIRect ir;
        this->roundIn(&ir);
        return ir;
    }

    /** Swaps fLeft and fRight if fLeft is greater than fRight; and swaps
        fTop and fBottom if fTop is greater than fBottom. Result may be empty;
        and width() and height() will be zero or positive.
    */
    void sort() {
        using std::swap;
        if (fLeft > fRight) {
            swap(fLeft, fRight);
        }

        if (fTop > fBottom) {
            swap(fTop, fBottom);
        }
    }

    /** Returns SkRect with fLeft and fRight swapped if fLeft is greater than fRight; and
        with fTop and fBottom swapped if fTop is greater than fBottom. Result may be empty;
        and width() and height() will be zero or positive.

        @return  sorted SkRect
    */
    SkRect makeSorted() const {
        return MakeLTRB(std::min(fLeft, fRight), std::min(fTop, fBottom),
                        std::max(fLeft, fRight), std::max(fTop, fBottom));
    }

    /** Returns pointer to first float in SkRect, to treat it as an array with four
        entries.

        @return  pointer to fLeft
    */
    const float* asScalars() const { return &fLeft; }

    /** Writes text representation of SkRect to standard output. Set asHex to true to
        generate exact binary representations of floating point numbers.

        @param asHex  true if SkScalar values are written as hexadecimal

        example: https://fiddle.skia.org/c/@Rect_dump
    */
    void dump(bool asHex) const;
    SkString dumpToString(bool asHex) const;

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
            fLeft <= r.fLeft && fTop <= r.fTop &&
            fRight >= r.fRight && fBottom >= r.fBottom;
}

#endif
