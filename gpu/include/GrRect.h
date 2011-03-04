/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrRect_DEFINED
#define GrRect_DEFINED

#include "GrPoint.h"

struct GrIRect {
    int32_t fLeft, fTop, fRight, fBottom;

    GrIRect() {}
    GrIRect(int32_t left, int32_t top, int32_t right, int32_t bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    int32_t x() const { return fLeft; }
    int32_t y() const { return fTop; }
    int32_t width() const { return fRight - fLeft; }
    int32_t height() const { return fBottom - fTop; }

    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }
    bool isInverted() const { return fLeft > fRight || fTop > fBottom; }

    void setEmpty() { fLeft = fTop = fRight = fBottom = 0; }

    void setXYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        fLeft = x;
        fTop = y;
        fRight = x + w;
        fBottom = y + h;
    }

    void setLTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        fLeft = l;
        fTop = t;
        fRight = r;
        fBottom = b;
    }

    /**
     *  Make the largest representable rectangle
     */
    void setLargest() {
        fLeft = fTop = GR_Int32Min;
        fRight = fBottom = GR_Int32Max;
    }

    bool quickReject(int l, int t, int r, int b) const {
        return l >= fRight || fLeft >= r || t >= fBottom || fTop >= b;
    }

    void unionWith(const GrIRect& r) {
        if (fLeft > r.fLeft) fLeft = r.fLeft;
        if (fTop > r.fTop) fTop = r.fTop;
        if (fRight < r.fRight) fRight = r.fRight;
        if (fBottom < r.fBottom) fBottom = r.fBottom;
    }

    /**
     * Sets this rect to the intersection with a clip rect. If there is no
     * intersection then this rect will be made empty.
     */
    void intersectWith(const GrIRect& clipRect) {
        if (fRight < clipRect.fLeft ||
            fLeft > clipRect.fRight ||
            fBottom < clipRect.fTop ||
            fTop > clipRect.fBottom) {
            this->setEmpty();
        } else {
            fLeft = GrMax(fLeft, clipRect.fLeft);
            fRight = GrMin(fRight, clipRect.fRight);
            fTop = GrMax(fTop, clipRect.fTop);
            fBottom = GrMin(fBottom, clipRect.fBottom);
        }
    }

    friend bool operator==(const GrIRect& a, const GrIRect& b) {
        return 0 == memcmp(&a, &b, sizeof(a));
    }

    friend bool operator!=(const GrIRect& a, const GrIRect& b) {
        return 0 != memcmp(&a, &b, sizeof(a));
    }

    bool equalsLTRB(int l, int t, int r, int b) const {
        return fLeft == l && fTop == t &&
               fRight == r && fBottom == b;
    }
    bool equalsXYWH(int x, int y, int w, int h) const {
        return fLeft == x && fTop == y &&
               this->width() == w && this->height() == h;
    }

    bool contains(const GrIRect& r) const {
        return fLeft   <= r.fLeft &&
               fRight  >= r.fRight &&
               fTop    <= r.fTop &&
               fBottom >= r.fBottom;
    }
};

struct GrIRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void set(const GrIRect& r) {
        fLeft   = GrToS16(r.fLeft);
        fTop    = GrToS16(r.fTop);
        fRight  = GrToS16(r.fRight);
        fBottom = GrToS16(r.fBottom);
    }
};

/**
 *  2D Rect struct
 */
struct GrRect {
    GrScalar fLeft, fTop, fRight, fBottom;

    /**
     *  Uninitialized rectangle.
     */
    GrRect() {}

    /**
     *  Initialize a rectangle to a point.
     *  @param pt the point used to initialize the rectanglee.
     */
    explicit GrRect(const GrPoint& pt) {
        setToPoint(pt);
    }

    GrRect(GrScalar left, GrScalar top, GrScalar right, GrScalar bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    explicit GrRect(const GrIRect& src) {
        fLeft = GrIntToScalar(src.fLeft);
        fTop = GrIntToScalar(src.fTop);
        fRight = GrIntToScalar(src.fRight);
        fBottom = GrIntToScalar(src.fBottom);
    }

    GrScalar x() const { return fLeft; }
    GrScalar y() const { return fTop; }
    GrScalar width() const { return fRight - fLeft; }
    GrScalar height() const { return fBottom - fTop; }

    GrScalar left() const { return fLeft; }
    GrScalar top() const { return fTop; }
    GrScalar right() const { return fRight; }
    GrScalar bottom() const { return fBottom; }

    GrScalar diagonalLengthSqd() const {
        GrScalar w = width();
        GrScalar h = height();
        return GrMul(w, w) + GrMul(h, h);
    }

    GrScalar diagonalLength() const {
        // TODO: fixed point sqrt
        return GrFloatToScalar(sqrtf(GrScalarToFloat(diagonalLengthSqd())));
    }

    /**
     *  Returns true if the width or height is <= 0
     */
    bool isEmpty() const {
        return fLeft >= fRight || fTop >= fBottom;
    }

    void setEmpty() {
        fLeft = fTop = fRight = fBottom = 0;
    }

    /**
     *  returns true if the rectangle is inverted either in x or y
     */
    bool isInverted() const {
        return (fLeft > fRight) || (fTop > fBottom);
    }

    /**
     * Does this rect contain a point.
     */
    bool contains(const GrPoint& point) const {
        return point.fX >= fLeft && point.fX < fRight &&
               point.fY >= fTop && point.fY < fBottom;
    }

    /**
     * Does this rect fully contain another rect.
     */
    bool contains(const GrRect& r) const {
        return fLeft   <= r.fLeft &&
               fRight  >= r.fRight &&
               fTop    <= r.fTop &&
               fBottom >= r.fBottom;
    }

    /**
     *  Offset the rectangle by (tx, ty), adding tx to the horizontal position
     *  and adds ty to the vertical position.
     */
    void offset(GrScalar tx, GrScalar ty) {
        fLeft  += tx;   fTop    += ty;
        fRight += tx;   fBottom += ty;
    }

    /**
     *  Initialize a rectangle to a point.
     *  @param pt the point used to initialize the rectangle.
     */
    void setToPoint(const GrPoint& pt) {
        fLeft = pt.fX;
        fTop = pt.fY;
        fRight = pt.fX;
        fBottom = pt.fY;
    }

    void set(const GrIRect& r) {
        fLeft = GrIntToScalar(r.fLeft);
        fTop = GrIntToScalar(r.fTop);
        fRight = GrIntToScalar(r.fRight);
        fBottom = GrIntToScalar(r.fBottom);
    }

    void roundOut(GrIRect* r) const {
        r->setLTRB(GrScalarFloorToInt(fLeft),
                   GrScalarFloorToInt(fTop),
                   GrScalarCeilToInt(fRight),
                   GrScalarCeilToInt(fBottom));
    }

    /**
     *  Set the rect to the union of the array of points. If the array is empty
     *  the rect will be empty [0,0,0,0]
     */
    void setBounds(const GrPoint pts[], int count);

    /**
     *  Make the largest representable rectangle
     *  Set the rect to fLeft = fTop = GR_ScalarMin and
     *  fRight = fBottom = GR_ScalarMax.
     */
    void setLargest() {
        fLeft = fTop = GR_ScalarMin;
        fRight = fBottom = GR_ScalarMax;
    }

    /**
     Set the rect to fLeft = fTop = GR_ScalarMax and
     fRight = fBottom = GR_ScalarMin.
     Useful for initializing a bounding rectangle.
     */
    void setLargestInverted() {
        fLeft = fTop = GR_ScalarMax;
        fRight = fBottom = GR_ScalarMin;
    }

    void setLTRB(GrScalar left,
                 GrScalar top,
                 GrScalar right,
                 GrScalar bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    void setXYWH(GrScalar x, GrScalar y, GrScalar width, GrScalar height) {
        fLeft = x;
        fTop = y;
        fRight = x + width;
        fBottom = y + height;
    }

    /**
     Expand the edges of the rectangle to include a point.
     Useful for constructing a bounding rectangle.
     @param pt  the point used to grow the rectangle.
     */
    void growToInclude(const GrPoint& pt) {
        fLeft  = GrMin(pt.fX, fLeft);
        fRight = GrMax(pt.fX, fRight);

        fTop    = GrMin(pt.fY, fTop);
        fBottom = GrMax(pt.fY, fBottom);
    }

    /**
     * Grows a rect to include another rect.
     * @param rect the rect to include
     */
    void growToInclude(const GrRect& rect) {
        GrAssert(!rect.isEmpty());
        fLeft  = GrMin(rect.fLeft, fLeft);
        fRight = GrMax(rect.fRight, fRight);

        fTop    = GrMin(rect.fTop, fTop);
        fBottom = GrMax(rect.fBottom, fBottom);
    }

    /**
     * Sets this rect to the intersection with a clip rect. If there is no
     * intersection then this rect will be made empty.
     */
    void intersectWith(const GrRect& clipRect) {
        if (fRight < clipRect.fLeft ||
            fLeft > clipRect.fRight ||
            fBottom < clipRect.fTop ||
            fTop > clipRect.fBottom) {
            this->setEmpty();
        } else {
            fLeft = GrMax(fLeft, clipRect.fLeft);
            fRight = GrMin(fRight, clipRect.fRight);
            fTop = GrMax(fTop, clipRect.fTop);
            fBottom = GrMin(fBottom, clipRect.fBottom);
        }
    }

    /**
     *  Assigns 4 sequential points in order to construct a counter-clockwise
     *  triangle fan, given the corners of this rect. Returns the address of
     *  the next point, treating pts as an array.
     */
    GrPoint* setRectFan(GrPoint pts[4]) const {
        pts->setRectFan(fLeft, fTop, fRight, fBottom);
        return pts + 4;
    }

    bool operator ==(const GrRect& r) const {
        return fLeft == r.fLeft     &&
               fTop == r.fTop       &&
               fRight == r.fRight   &&
               fBottom == r.fBottom;
    }
};

#endif

