/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRect_DEFINED
#define GrRect_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"

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

    static GrIRect16 SK_WARN_UNUSED_RESULT Make(const SkIRect& ir) {
        GrIRect16 r;
        r.set(ir);
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

    void offset(int16_t dx, int16_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
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
    SkMatrix::RectToRect(inRect, outRect).mapPoints(outPts, inPts, ptCount);
}

/**
 * Clips the srcRect and the dstPoint to the bounds of the srcSize and dstSize respectively. Returns
 * true if the srcRect and dstRect intersect the srcRect and dst rect (dstPoint with srcRect
 * width/height). Returns false otherwise. The clipped values are returned in clippedSrcRect and
 * clippedDstPoint.
 */
static inline bool GrClipSrcRectAndDstPoint(const SkISize& dstSize,
                                            const SkISize& srcSize,
                                            const SkIRect& srcRect,
                                            const SkIPoint& dstPoint,
                                            SkIRect* clippedSrcRect,
                                            SkIPoint* clippedDstPoint) {
    *clippedSrcRect = srcRect;
    *clippedDstPoint = dstPoint;

    // clip the left edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fLeft < 0) {
        clippedDstPoint->fX -= clippedSrcRect->fLeft;
        clippedSrcRect->fLeft = 0;
    }
    if (clippedDstPoint->fX < 0) {
        clippedSrcRect->fLeft -= clippedDstPoint->fX;
        clippedDstPoint->fX = 0;
    }

    // clip the top edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fTop < 0) {
        clippedDstPoint->fY -= clippedSrcRect->fTop;
        clippedSrcRect->fTop = 0;
    }
    if (clippedDstPoint->fY < 0) {
        clippedSrcRect->fTop -= clippedDstPoint->fY;
        clippedDstPoint->fY = 0;
    }

    // clip the right edge to the src and dst bounds.
    if (clippedSrcRect->fRight > srcSize.width()) {
        clippedSrcRect->fRight = srcSize.width();
    }
    if (clippedDstPoint->fX + clippedSrcRect->width() > dstSize.width()) {
        clippedSrcRect->fRight = clippedSrcRect->fLeft + dstSize.width() - clippedDstPoint->fX;
    }

    // clip the bottom edge to the src and dst bounds.
    if (clippedSrcRect->fBottom > srcSize.height()) {
        clippedSrcRect->fBottom = srcSize.height();
    }
    if (clippedDstPoint->fY + clippedSrcRect->height() > dstSize.height()) {
        clippedSrcRect->fBottom = clippedSrcRect->fTop + dstSize.height() - clippedDstPoint->fY;
    }

    // The above clipping steps may have inverted the rect if it didn't intersect either the src or
    // dst bounds.
    return !clippedSrcRect->isEmpty();
}
#endif
