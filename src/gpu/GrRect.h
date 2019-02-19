/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRect_DEFINED
#define GrRect_DEFINED

#include "SkTo.h"
#include "SkTypes.h"
#include "SkRect.h"

struct GrIRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeEmpty() {
        GrIRect16 r;
        r.setEmpty();
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeWH(int16_t w, int16_t h) {
        GrIRect16 r;
        r.set(0, 0, w, h);
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeXYWH(int16_t x, int16_t y, int16_t w, int16_t h) {
        GrIRect16 r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(int16_t left, int16_t top, int16_t right, int16_t bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    void set(const SkIRect& r) {
        fLeft   = SkToS16(r.fLeft);
        fTop    = SkToS16(r.fTop);
        fRight  = SkToS16(r.fRight);
        fBottom = SkToS16(r.fBottom);
    }
};

/** Returns true if the rectangles have a nonzero area of overlap. It assumed that rects can be
    infinitely small but not "inverted". */
static inline bool GrRectsOverlap(const SkRect& a, const SkRect& b) {
    // See skbug.com/6607 about the isFinite() checks.
    SkASSERT(!a.isFinite() || (a.fLeft <= a.fRight && a.fTop <= a.fBottom));
    SkASSERT(!b.isFinite() || (b.fLeft <= b.fRight && b.fTop <= b.fBottom));
    return a.fRight > b.fLeft && a.fBottom > b.fTop && b.fRight > a.fLeft && b.fBottom > a.fTop;
}

/** Returns true if the rectangles overlap or share an edge or corner. It assumed that rects can be
    infinitely small but not "inverted". */
static inline bool GrRectsTouchOrOverlap(const SkRect& a, const SkRect& b) {
    // See skbug.com/6607 about the isFinite() checks.
    SkASSERT(!a.isFinite() || (a.fLeft <= a.fRight && a.fTop <= a.fBottom));
    SkASSERT(!b.isFinite() || (b.fLeft <= b.fRight && b.fTop <= b.fBottom));
    return a.fRight >= b.fLeft && a.fBottom >= b.fTop && b.fRight >= a.fLeft && b.fBottom >= a.fTop;
}

/**
 * Apply the transform from 'inRect' to 'outRect' to each point in 'inPts', storing the mapped point
 * into the parallel index of 'outPts'.
 */
static inline void GrMapRectPoints(const SkRect& inRect, const SkRect& outRect,
                                   const SkPoint inPts[], SkPoint outPts[], int ptCount) {
    SkMatrix rectTransform = SkMatrix::MakeRectToRect(inRect, outRect, SkMatrix::kFill_ScaleToFit);
    rectTransform.mapPoints(outPts, inPts, ptCount);
}
#endif
