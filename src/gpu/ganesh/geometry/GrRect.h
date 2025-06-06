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

/** Returns true if the rectangles have a nonzero area of overlap. It assumed that rects can be
    infinitely small but not "inverted". */
static inline bool GrRectsOverlap(const SkRect& a, const SkRect& b) {
    // See skbug.com/40037824 about the isFinite() checks.
    SkASSERT(!a.isFinite() || (a.fLeft <= a.fRight && a.fTop <= a.fBottom));
    SkASSERT(!b.isFinite() || (b.fLeft <= b.fRight && b.fTop <= b.fBottom));
    return a.fRight > b.fLeft && a.fBottom > b.fTop && b.fRight > a.fLeft && b.fBottom > a.fTop;
}

/** Returns true if the rectangles overlap or share an edge or corner. It assumed that rects can be
    infinitely small but not "inverted". */
static inline bool GrRectsTouchOrOverlap(const SkRect& a, const SkRect& b) {
    // See skbug.com/40037824 about the isFinite() checks.
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
    SkMatrix::RectToRect(inRect, outRect).mapPoints({outPts, ptCount}, {inPts, ptCount});
}

/**
 * Clips the srcRect and the dstPoint to the bounds of the srcSize and dstSize respectively. Returns
 * true if the srcRect and dstRect intersect the srcRect and dst rect (dstPoint with srcRect
 * width/height). Returns false otherwise. The clipped values are stored back into 'dstPoint'
 * and 'srcRect'
 */
static inline bool GrClipSrcRectAndDstPoint(const SkISize& dstSize,
                                            SkIPoint* dstPoint,
                                            const SkISize& srcSize,
                                            SkIRect* srcRect) {
    // clip the left edge to src and dst bounds, adjusting dstPoint if necessary
    if (srcRect->fLeft < 0) {
        dstPoint->fX -= srcRect->fLeft;
        srcRect->fLeft = 0;
    }
    if (dstPoint->fX < 0) {
        srcRect->fLeft -= dstPoint->fX;
        dstPoint->fX = 0;
    }

    // clip the top edge to src and dst bounds, adjusting dstPoint if necessary
    if (srcRect->fTop < 0) {
        dstPoint->fY -= srcRect->fTop;
        srcRect->fTop = 0;
    }
    if (dstPoint->fY < 0) {
        srcRect->fTop -= dstPoint->fY;
        dstPoint->fY = 0;
    }

    // clip the right edge to the src and dst bounds.
    if (srcRect->fRight > srcSize.width()) {
        srcRect->fRight = srcSize.width();
    }
    if (dstPoint->fX + srcRect->width() > dstSize.width()) {
        srcRect->fRight = srcRect->fLeft + dstSize.width() - dstPoint->fX;
    }

    // clip the bottom edge to the src and dst bounds.
    if (srcRect->fBottom > srcSize.height()) {
        srcRect->fBottom = srcSize.height();
    }
    if (dstPoint->fY + srcRect->height() > dstSize.height()) {
        srcRect->fBottom = srcRect->fTop + dstSize.height() - dstPoint->fY;
    }

    // The above clipping steps may have inverted the rect if it didn't intersect either the src or
    // dst bounds.
    return !srcRect->isEmpty();
}

#endif
