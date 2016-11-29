/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAnalyticEdge.h"
#include "SkFDot6.h"
#include "SkMathPriv.h"

// This will become a bottleneck for small ovals rendering if we call SkFixedDiv twice here.
// Therefore, we'll let the outter function compute the slope once and send in the value.
// Moreover, we'll compute fDY by quickly lookup the inverse table (if possible).
bool SkAnalyticEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1, SkFixed slope) {
    // Since we send in the slope, we can no longer snap y inside this function.
    // If we don't send in the slope, or we do some more sophisticated snapping, this function
    // could be a performance bottleneck.
    SkASSERT(fWinding == 1 || fWinding == -1);
    SkASSERT(fCurveCount != 0);

    SkASSERT(y0 <= y1);

    SkFDot6 dx = SkFixedToFDot6(x1 - x0);
    SkFDot6 dy = SkFixedToFDot6(y1 - y0);

    // are we a zero-height line?
    if (dy == 0) {
        return false;
    }

    SkASSERT(slope < SK_MaxS32);

    SkFDot6     absSlope = SkAbs32(SkFixedToFDot6(slope));
    fX          = x0;
    fDX         = slope;
    fUpperX     = x0;
    fY          = y0;
    fUpperY     = y0;
    fLowerY     = y1;
    fDY         = (absSlope | dx) == 0
                  ? SK_MaxS32
                  : absSlope < kInverseTableSize
                    ? QuickFDot6Inverse::Lookup(absSlope)
                    : SkAbs32(QuickSkFDot6Div(dy, dx));

    return true;
}

void SkAnalyticEdge::chopLineWithClip(const SkIRect& clip) {
    int top = SkFixedFloorToInt(fUpperY);

    SkASSERT(top < clip.fBottom);

    // clip the line to the clip top
    if (top < clip.fTop) {
        SkASSERT(SkFixedCeilToInt(fLowerY) > clip.fTop);
        SkFixed newY = SkIntToFixed(clip.fTop);
        this->goY(newY);
        fUpperY = newY;
    }
}

bool SkAnalyticQuadraticEdge::setQuadratic(const SkPoint pts[3]) {
    if (!fQEdge.setQuadraticWithoutUpdate(pts, 2)) {
        return false;
    }
    fQEdge.fQx >>= 2;
    fQEdge.fQy >>= 2;
    fQEdge.fQDx >>= 2;
    fQEdge.fQDy >>= 2;
    fQEdge.fQDDx >>= 2;
    fQEdge.fQDDy >>= 2;
    fQEdge.fQLastX >>= 2;
    fQEdge.fQLastY >>= 2;
    fQEdge.fQy = snapY(fQEdge.fQy);
    fQEdge.fQLastY = snapY(fQEdge.fQLastY);

    fWinding = fQEdge.fWinding;
    fCurveCount = fQEdge.fCurveCount;
    fCurveShift = fQEdge.fCurveShift;

    fSnappedX = fQEdge.fQx;
    fSnappedY = fQEdge.fQy;

    return this->updateQuadratic();
}

bool SkAnalyticQuadraticEdge::updateQuadratic() {
    int     success = 0; // initialize to fail!
    int     count = fCurveCount;
    SkFixed oldx = fQEdge.fQx;
    SkFixed oldy = fQEdge.fQy;
    SkFixed dx = fQEdge.fQDx;
    SkFixed dy = fQEdge.fQDy;
    SkFixed newx, newy, newSnappedX, newSnappedY;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        SkFixed slope;
        if (--count > 0)
        {
            newx    = oldx + (dx >> shift);
            newy    = oldy + (dy >> shift);
            SkFDot6 diffY = (newy - fSnappedY) >> 10;
            slope = diffY ? QuickSkFDot6Div((newx - fSnappedX) >> 10, diffY) : SK_MaxS32;
            #ifdef SK_ANALYTIC_AA_GUARD
            slope = dy >> 10 > 0 ? QuickSkFDot6Div(dx >> 10, dy >> 10) : SK_MaxS32;
            shift = 0;
            #endif
            if (SkAbs32(dy >> shift) >= SK_Fixed1 * 2) { // only snap when dy is large enough
                newSnappedY = SkTMin<SkFixed>(fQEdge.fQLastY, SkFixedRoundToFixed(newy));
                newSnappedX = newx - SkFixedMul(slope, newy - newSnappedY);
                #ifdef SK_ANALYTIC_AA_GUARD
                // Note that SkFixedMul use (>> 16) so we'll have different answers
                newSnappedX = newx + SkFixedMul(slope, newSnappedY - newy);
                #endif
            } else {
                newSnappedY = SkTMin(fQEdge.fQLastY, snapY(newy));
                newSnappedX = newx;
            }
            #ifdef SK_ANALYTIC_AA_GUARD
            shift = fCurveShift;
            #endif
            dx += fQEdge.fQDDx;
            dy += fQEdge.fQDDy;
        }
        else    // last segment
        {
            newx    = fQEdge.fQLastX;
            newy    = fQEdge.fQLastY;
            newSnappedY = newy;
            newSnappedX = newx;
            SkFDot6 diffY = (newy - fSnappedY) >> 10;
            slope = diffY ? QuickSkFDot6Div((newx - fSnappedX) >> 10, diffY) : SK_MaxS32;
        }
        if (slope < SK_MaxS32) {
            success = this->updateLine(fSnappedX, fSnappedY, newSnappedX, newSnappedY, slope);
        }
        oldx = newx;
        oldy = newy;
    } while (count > 0 && !success);

    SkASSERT(newSnappedY <= fQEdge.fQLastY);

    fQEdge.fQx  = newx;
    fQEdge.fQy  = newy;
    fQEdge.fQDx = dx;
    fQEdge.fQDy = dy;
    fSnappedX   = newSnappedX;
    fSnappedY   = newSnappedY;
    fCurveCount = SkToS8(count);
    return success;
}

bool SkAnalyticCubicEdge::setCubic(const SkPoint pts[4]) {
    if (!fCEdge.setCubicWithoutUpdate(pts, 2)) {
        return false;
    }

    fCEdge.fCx >>= 2;
    fCEdge.fCy >>= 2;
    fCEdge.fCDx >>= 2;
    fCEdge.fCDy >>= 2;
    fCEdge.fCDDx >>= 2;
    fCEdge.fCDDy >>= 2;
    fCEdge.fCDDDx >>= 2;
    fCEdge.fCDDDy >>= 2;
    fCEdge.fCLastX >>= 2;
    fCEdge.fCLastY >>= 2;
    fCEdge.fCy = snapY(fCEdge.fCy);
    fCEdge.fCLastY = snapY(fCEdge.fCLastY);

    fWinding = fCEdge.fWinding;
    fCurveCount = fCEdge.fCurveCount;
    fCurveShift = fCEdge.fCurveShift;
    fCubicDShift = fCEdge.fCubicDShift;

    return this->updateCubic();
}

bool SkAnalyticCubicEdge::updateCubic() {
    int     success;
    int     count = fCurveCount;
    SkFixed oldx = fCEdge.fCx;
    SkFixed oldy = fCEdge.fCy;
    SkFixed newx, newy;
    const int ddshift = fCurveShift;
    const int dshift = fCubicDShift;

    SkASSERT(count < 0);

    do {
        if (++count < 0) {
            newx    = oldx + (fCEdge.fCDx >> dshift);
            fCEdge.fCDx    += fCEdge.fCDDx >> ddshift;
            fCEdge.fCDDx   += fCEdge.fCDDDx;

            newy    = oldy + (fCEdge.fCDy >> dshift);
            fCEdge.fCDy    += fCEdge.fCDDy >> ddshift;
            fCEdge.fCDDy   += fCEdge.fCDDDy;
        }
        else {    // last segment
            newx    = fCEdge.fCLastX;
            newy    = fCEdge.fCLastY;
        }

        // we want to say SkASSERT(oldy <= newy), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (newy < oldy) {
            newy = oldy;
        }

        SkFixed snappedOldY = SkAnalyticEdge::snapY(oldy);
        SkFixed snappedNewY = SkAnalyticEdge::snapY(newy);
        SkFixed slope = SkFixedToFDot6(snappedNewY - snappedOldY) == 0
                        ? SK_MaxS32
                        : SkFDot6Div(SkFixedToFDot6(newx - oldx),
                                     SkFixedToFDot6(snappedNewY - snappedOldY));

        success = this->updateLine(oldx, snappedOldY, newx, snappedNewY, slope);

        oldx = newx;
        oldy = newy;
    } while (count < 0 && !success);

    fCEdge.fCx  = newx;
    fCEdge.fCy  = newy;
    fCurveCount = SkToS8(count);
    return success;
}
