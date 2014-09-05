
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkEdge.h"
#include "SkFDot6.h"
#include "SkMath.h"

/*
    In setLine, setQuadratic, setCubic, the first thing we do is to convert
    the points into FDot6. This is modulated by the shift parameter, which
    will either be 0, or something like 2 for antialiasing.

    In the float case, we want to turn the float into .6 by saying pt * 64,
    or pt * 256 for antialiasing. This is implemented as 1 << (shift + 6).

    In the fixed case, we want to turn the fixed into .6 by saying pt >> 10,
    or pt >> 8 for antialiasing. This is implemented as pt >> (10 - shift).
*/

static inline SkFixed SkFDot6ToFixedDiv2(SkFDot6 value) {
    // we want to return SkFDot6ToFixed(value >> 1), but we don't want to throw
    // away data in value, so just perform a modify up-shift
    return value << (16 - 6 - 1);
}

/////////////////////////////////////////////////////////////////////////

int SkEdge::setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip,
                    int shift) {
    SkFDot6 x0, y0, x1, y1;

    {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        x0 = SkScalarRoundToFDot6(p0.fX, shift);
        y0 = SkScalarRoundToFDot6(p0.fY, shift);
        x1 = SkScalarRoundToFDot6(p1.fX, shift);
        y1 = SkScalarRoundToFDot6(p1.fY, shift);
#else
        float scale = float(1 << (shift + 6));
        x0 = int(p0.fX * scale);
        y0 = int(p0.fY * scale);
        x1 = int(p1.fX * scale);
        y1 = int(p1.fY * scale);
#endif
    }

    int winding = 1;

    if (y0 > y1) {
        SkTSwap(x0, x1);
        SkTSwap(y0, y1);
        winding = -1;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot) {
        return 0;
    }
    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop)) {
        return 0;
    }

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const int dy  = SkEdge_Compute_DY(top, y0);

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));   // + SK_Fixed1/2
    fDX         = slope;
    fFirstY     = top;
    fLastY      = bot - 1;
    fCurveCount = 0;
    fWinding    = SkToS8(winding);
    fCurveShift = 0;

    if (clip) {
        this->chopLineWithClip(*clip);
    }
    return 1;
}

// called from a curve subclass
int SkEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1)
{
    SkASSERT(fWinding == 1 || fWinding == -1);
    SkASSERT(fCurveCount != 0);
//    SkASSERT(fCurveShift != 0);

    y0 >>= 10;
    y1 >>= 10;

    SkASSERT(y0 <= y1);

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

//  SkASSERT(top >= fFirstY);

    // are we a zero-height line?
    if (top == bot)
        return 0;

    x0 >>= 10;
    x1 >>= 10;

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const int dy  = SkEdge_Compute_DY(top, y0);

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));   // + SK_Fixed1/2
    fDX         = slope;
    fFirstY     = top;
    fLastY      = bot - 1;

    return 1;
}

void SkEdge::chopLineWithClip(const SkIRect& clip)
{
    int top = fFirstY;

    SkASSERT(top < clip.fBottom);

    // clip the line to the top
    if (top < clip.fTop)
    {
        SkASSERT(fLastY >= clip.fTop);
        fX += fDX * (clip.fTop - top);
        fFirstY = clip.fTop;
    }
}

///////////////////////////////////////////////////////////////////////////////

/*  We store 1<<shift in a (signed) byte, so its maximum value is 1<<6 == 64.
    Note that this limits the number of lines we use to approximate a curve.
    If we need to increase this, we need to store fCurveCount in something
    larger than int8_t.
*/
#define MAX_COEFF_SHIFT     6

static inline SkFDot6 cheap_distance(SkFDot6 dx, SkFDot6 dy)
{
    dx = SkAbs32(dx);
    dy = SkAbs32(dy);
    // return max + min/2
    if (dx > dy)
        dx += dy >> 1;
    else
        dx = dy + (dx >> 1);
    return dx;
}

static inline int diff_to_shift(SkFDot6 dx, SkFDot6 dy)
{
    // cheap calc of distance from center of p0-p2 to the center of the curve
    SkFDot6 dist = cheap_distance(dx, dy);

    // shift down dist (it is currently in dot6)
    // down by 5 should give us 1/2 pixel accuracy (assuming our dist is accurate...)
    // this is chosen by heuristic: make it as big as possible (to minimize segments)
    // ... but small enough so that our curves still look smooth
    dist = (dist + (1 << 4)) >> 5;

    // each subdivision (shift value) cuts this dist (error) by 1/4
    return (32 - SkCLZ(dist)) >> 1;
}

int SkQuadraticEdge::setQuadratic(const SkPoint pts[3], int shift)
{
    SkFDot6 x0, y0, x1, y1, x2, y2;

    {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        x0 = SkScalarRoundToFDot6(pts[0].fX, shift);
        y0 = SkScalarRoundToFDot6(pts[0].fY, shift);
        x1 = SkScalarRoundToFDot6(pts[1].fX, shift);
        y1 = SkScalarRoundToFDot6(pts[1].fY, shift);
        x2 = SkScalarRoundToFDot6(pts[2].fX, shift);
        y2 = SkScalarRoundToFDot6(pts[2].fY, shift);
#else
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
        x2 = int(pts[2].fX * scale);
        y2 = int(pts[2].fY * scale);
#endif
    }

    int winding = 1;
    if (y0 > y2)
    {
        SkTSwap(x0, x2);
        SkTSwap(y0, y2);
        winding = -1;
    }
    SkASSERT(y0 <= y1 && y1 <= y2);

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y2);

    // are we a zero-height quad (line)?
    if (top == bot)
        return 0;

    // compute number of steps needed (1 << shift)
    {
        SkFDot6 dx = ((x1 << 1) - x0 - x2) >> 2;
        SkFDot6 dy = ((y1 << 1) - y0 - y2) >> 2;
        shift = diff_to_shift(dx, dy);
        SkASSERT(shift >= 0);
    }
    // need at least 1 subdivision for our bias trick
    if (shift == 0) {
        shift = 1;
    } else if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    fWinding    = SkToS8(winding);
    //fCubicDShift only set for cubics
    fCurveCount = SkToS8(1 << shift);

    /*
     *  We want to reformulate into polynomial form, to make it clear how we
     *  should forward-difference.
     *
     *  p0 (1 - t)^2 + p1 t(1 - t) + p2 t^2 ==> At^2 + Bt + C
     *
     *  A = p0 - 2p1 + p2
     *  B = 2(p1 - p0)
     *  C = p0
     *
     *  Our caller must have constrained our inputs (p0..p2) to all fit into
     *  16.16. However, as seen above, we sometimes compute values that can be
     *  larger (e.g. B = 2*(p1 - p0)). To guard against overflow, we will store
     *  A and B at 1/2 of their actual value, and just apply a 2x scale during
     *  application in updateQuadratic(). Hence we store (shift - 1) in
     *  fCurveShift.
     */

    fCurveShift = SkToU8(shift - 1);

    SkFixed A = SkFDot6ToFixedDiv2(x0 - x1 - x1 + x2);  // 1/2 the real value
    SkFixed B = SkFDot6ToFixed(x1 - x0);                // 1/2 the real value

    fQx     = SkFDot6ToFixed(x0);
    fQDx    = B + (A >> shift);     // biased by shift
    fQDDx   = A >> (shift - 1);     // biased by shift

    A = SkFDot6ToFixedDiv2(y0 - y1 - y1 + y2);  // 1/2 the real value
    B = SkFDot6ToFixed(y1 - y0);                // 1/2 the real value

    fQy     = SkFDot6ToFixed(y0);
    fQDy    = B + (A >> shift);     // biased by shift
    fQDDy   = A >> (shift - 1);     // biased by shift

    fQLastX = SkFDot6ToFixed(x2);
    fQLastY = SkFDot6ToFixed(y2);

    return this->updateQuadratic();
}

int SkQuadraticEdge::updateQuadratic()
{
    int     success;
    int     count = fCurveCount;
    SkFixed oldx = fQx;
    SkFixed oldy = fQy;
    SkFixed dx = fQDx;
    SkFixed dy = fQDy;
    SkFixed newx, newy;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        if (--count > 0)
        {
            newx    = oldx + (dx >> shift);
            dx    += fQDDx;
            newy    = oldy + (dy >> shift);
            dy    += fQDDy;
        }
        else    // last segment
        {
            newx    = fQLastX;
            newy    = fQLastY;
        }
        success = this->updateLine(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;
    } while (count > 0 && !success);

    fQx         = newx;
    fQy         = newy;
    fQDx        = dx;
    fQDy        = dy;
    fCurveCount = SkToS8(count);
    return success;
}

/////////////////////////////////////////////////////////////////////////

static inline int SkFDot6UpShift(SkFDot6 x, int upShift) {
    SkASSERT((x << upShift >> upShift) == x);
    return x << upShift;
}

/*  f(1/3) = (8a + 12b + 6c + d) / 27
    f(2/3) = (a + 6b + 12c + 8d) / 27

    f(1/3)-b = (8a - 15b + 6c + d) / 27
    f(2/3)-c = (a + 6b - 15c + 8d) / 27

    use 16/512 to approximate 1/27
*/
static SkFDot6 cubic_delta_from_line(SkFDot6 a, SkFDot6 b, SkFDot6 c, SkFDot6 d)
{
    SkFDot6 oneThird = ((a << 3) - ((b << 4) - b) + 6*c + d) * 19 >> 9;
    SkFDot6 twoThird = (a + 6*b - ((c << 4) - c) + (d << 3)) * 19 >> 9;

    return SkMax32(SkAbs32(oneThird), SkAbs32(twoThird));
}

int SkCubicEdge::setCubic(const SkPoint pts[4], const SkIRect* clip, int shift)
{
    SkFDot6 x0, y0, x1, y1, x2, y2, x3, y3;

    {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        x0 = SkScalarRoundToFDot6(pts[0].fX, shift);
        y0 = SkScalarRoundToFDot6(pts[0].fY, shift);
        x1 = SkScalarRoundToFDot6(pts[1].fX, shift);
        y1 = SkScalarRoundToFDot6(pts[1].fY, shift);
        x2 = SkScalarRoundToFDot6(pts[2].fX, shift);
        y2 = SkScalarRoundToFDot6(pts[2].fY, shift);
        x3 = SkScalarRoundToFDot6(pts[3].fX, shift);
        y3 = SkScalarRoundToFDot6(pts[3].fY, shift);
#else
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
        x2 = int(pts[2].fX * scale);
        y2 = int(pts[2].fY * scale);
        x3 = int(pts[3].fX * scale);
        y3 = int(pts[3].fY * scale);
#endif
    }

    int winding = 1;
    if (y0 > y3)
    {
        SkTSwap(x0, x3);
        SkTSwap(x1, x2);
        SkTSwap(y0, y3);
        SkTSwap(y1, y2);
        winding = -1;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y3);

    // are we a zero-height cubic (line)?
    if (top == bot)
        return 0;

    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop))
        return 0;

    // compute number of steps needed (1 << shift)
    {
        // Can't use (center of curve - center of baseline), since center-of-curve
        // need not be the max delta from the baseline (it could even be coincident)
        // so we try just looking at the two off-curve points
        SkFDot6 dx = cubic_delta_from_line(x0, x1, x2, x3);
        SkFDot6 dy = cubic_delta_from_line(y0, y1, y2, y3);
        // add 1 (by observation)
        shift = diff_to_shift(dx, dy) + 1;
    }
    // need at least 1 subdivision for our bias trick
    SkASSERT(shift > 0);
    if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    /*  Since our in coming data is initially shifted down by 10 (or 8 in
        antialias). That means the most we can shift up is 8. However, we
        compute coefficients with a 3*, so the safest upshift is really 6
    */
    int upShift = 6;    // largest safe value
    int downShift = shift + upShift - 10;
    if (downShift < 0) {
        downShift = 0;
        upShift = 10 - shift;
    }

    fWinding    = SkToS8(winding);
    fCurveCount = SkToS8(-1 << shift);
    fCurveShift = SkToU8(shift);
    fCubicDShift = SkToU8(downShift);

    SkFixed B = SkFDot6UpShift(3 * (x1 - x0), upShift);
    SkFixed C = SkFDot6UpShift(3 * (x0 - x1 - x1 + x2), upShift);
    SkFixed D = SkFDot6UpShift(x3 + 3 * (x1 - x2) - x0, upShift);

    fCx     = SkFDot6ToFixed(x0);
    fCDx    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDx   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDx  = 3*D >> (shift - 1);                   // biased by 2*shift

    B = SkFDot6UpShift(3 * (y1 - y0), upShift);
    C = SkFDot6UpShift(3 * (y0 - y1 - y1 + y2), upShift);
    D = SkFDot6UpShift(y3 + 3 * (y1 - y2) - y0, upShift);

    fCy     = SkFDot6ToFixed(y0);
    fCDy    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDy   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDy  = 3*D >> (shift - 1);                   // biased by 2*shift

    fCLastX = SkFDot6ToFixed(x3);
    fCLastY = SkFDot6ToFixed(y3);

    if (clip)
    {
        do {
            if (!this->updateCubic()) {
                return 0;
            }
        } while (!this->intersectsClip(*clip));
        this->chopLineWithClip(*clip);
        return 1;
    }
    return this->updateCubic();
}

int SkCubicEdge::updateCubic()
{
    int     success;
    int     count = fCurveCount;
    SkFixed oldx = fCx;
    SkFixed oldy = fCy;
    SkFixed newx, newy;
    const int ddshift = fCurveShift;
    const int dshift = fCubicDShift;

    SkASSERT(count < 0);

    do {
        if (++count < 0)
        {
            newx    = oldx + (fCDx >> dshift);
            fCDx    += fCDDx >> ddshift;
            fCDDx   += fCDDDx;

            newy    = oldy + (fCDy >> dshift);
            fCDy    += fCDDy >> ddshift;
            fCDDy   += fCDDDy;
        }
        else    // last segment
        {
        //  SkDebugf("LastX err=%d, LastY err=%d\n", (oldx + (fCDx >> shift) - fLastX), (oldy + (fCDy >> shift) - fLastY));
            newx    = fCLastX;
            newy    = fCLastY;
        }

        // we want to say SkASSERT(oldy <= newy), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (newy < oldy) {
            newy = oldy;
        }

        success = this->updateLine(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;
    } while (count < 0 && !success);

    fCx         = newx;
    fCy         = newy;
    fCurveCount = SkToS8(count);
    return success;
}
