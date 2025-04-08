/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkEdge.h"

#include "include/core/SkRect.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkFDot6.h"

#include <algorithm>
#include <utility>

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
    return SkLeftShift(value, 16 - 6 - 1);
}

/////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkEdge::dump() const {
    int realLastY = SkScalarToFixed(fLastY);
    if (fCurveCount > 0) {
        realLastY = static_cast<const SkQuadraticEdge*>(this)->fQLastY;
    } else if (fCurveCount < 0) {
        realLastY = static_cast<const SkCubicEdge*>(this)->fCLastY;
    }
    SkDebugf("edge (%c): firstY:%d lastY:%d (%g) x:%g dx:%g w:%d\n",
             fCurveCount > 0 ? 'Q' : (fCurveCount < 0 ? 'C' : 'L'),
             fFirstY,
             fLastY,
             SkFixedToFloat(realLastY),
             SkFixedToFloat(fX),
             SkFixedToFloat(fDxDy),
             static_cast<int8_t>(fWinding));
}
#endif

bool SkEdge::setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip) {
    SkFDot6 x0, y0, x1, y1;

#ifdef SK_RASTERIZE_EVEN_ROUNDING
    x0 = SkScalarRoundToFDot6(p0.fX, 0);
    y0 = SkScalarRoundToFDot6(p0.fY, 0);
    x1 = SkScalarRoundToFDot6(p1.fX, 0);
    y1 = SkScalarRoundToFDot6(p1.fY, 0);
#else
    x0 = SkFloatToFDot6(p0.fX);
    y0 = SkFloatToFDot6(p0.fY);
    x1 = SkFloatToFDot6(p1.fX);
    y1 = SkFloatToFDot6(p1.fY);
#endif

    Winding winding = Winding::kCW;
    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot) {
        return false;
    }
    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop)) {
        return false;
    }

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy  = SkEdge_Compute_DY(top, y0);

    // Note that SkFixedMul(SkFixed, SkFDot6) produces results in SkFDot6
    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));
    fDxDy       = slope;
    fFirstY     = top;
    fLastY      = bot - 1;
    fEdgeType   = Type::kLine;
    fCurveCount = 0;
    fWinding    = winding;
    fCurveShift = 0;

    if (clip) {
        this->chopLineWithClip(*clip);
    }
    return true;
}

// Draws a line between the provided points and then calculates the slope and starting
// x value to line up with the closest pixel center. Updates the fields in the SkEdge
// base class appropriately. Returns false if this edge would start and stop in the
// same row.
bool SkEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1) {
    SkASSERT(fWinding == Winding::kCW || fWinding == Winding::kCCW);
    SkASSERT(fCurveCount != 0);
//    SkASSERT(fCurveShift != 0);

    y0 >>= 10;
    y1 >>= 10;

    SkASSERT(y0 <= y1);

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

//  SkASSERT(top >= fFirstY);

    // are we a zero-height line?
    if (top == bot) {
        return false;
    }

    x0 >>= 10;
    x1 >>= 10;

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy  = SkEdge_Compute_DY(top, y0);

    // We could do this math in fixed point, but it would potentially require some
    // rebaselining https://codereview.chromium.org/960353005/#msg6
    // Note that SkFixedMul(SkFixed, SkFDot6) produces results in SkFDot6
    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));
    fDxDy       = slope;
    fFirstY     = top;
    fLastY      = bot - 1;

    return true;
}

void SkEdge::chopLineWithClip(const SkIRect& clip)
{
    int top = fFirstY;

    SkASSERT(top < clip.fBottom);

    // clip the line to the top
    if (top < clip.fTop)
    {
        SkASSERT(fLastY >= clip.fTop);
        fX += fDxDy * (clip.fTop - top);
        fFirstY = clip.fTop;
    }
}

///////////////////////////////////////////////////////////////////////////////

/*  This limits the number of lines we use to approximate a curve.
    If we need to increase this, we need to store fSegmentCount in a larger data type.
    TODO(kjlubick): Could this be an unsigned byte and go up to 7?
*/
#define MAX_COEFF_SHIFT     6

static inline SkFDot6 cheap_distance(SkFDot6 dx, SkFDot6 dy) {
    dx = SkAbs32(dx);
    dy = SkAbs32(dy);
    // return max + min/2
    if (dx > dy) {
        dx += dy >> 1;
    } else {
        dx = dy + (dx >> 1);
    }
    return dx;
}

static inline int diff_to_shift(SkFDot6 dx, SkFDot6 dy, int accuracy) {
    // cheap calc of distance from center of p0-p2 to the center of the curve
    SkFDot6 dist = cheap_distance(dx, dy);

    // shift down dist (it is currently in dot6)
    // down by 3 should give us 1/8 pixel accuracy (assuming our dist is accurate...)
    // this is chosen by heuristic: make it as big as possible (to minimize segments)
    // ... but small enough so that our curves still look smooth
    // When shift > 0, we're using AA and everything is scaled up so we can
    // lower the accuracy.
    // For cubics still, we have shift > 0. TODO(kjlubick) can we align cubics and quads?
    dist = (dist + (1 << (2 + accuracy))) >> (3 + accuracy);

    // each subdivision (shift value) cuts this dist (error) by 1/4
    return (32 - SkCLZ(dist)) >> 1;
}

bool SkQuadraticEdge::setQuadratic(const SkPoint pts[3]) {
    SkFDot6 x0, y0, x1, y1, x2, y2;

#if defined(SK_RASTERIZE_EVEN_ROUNDING)
    x0 = SkScalarRoundToFDot6(pts[0].fX, 0);
    y0 = SkScalarRoundToFDot6(pts[0].fY, 0);
    x1 = SkScalarRoundToFDot6(pts[1].fX, 0);
    y1 = SkScalarRoundToFDot6(pts[1].fY, 0);
    x2 = SkScalarRoundToFDot6(pts[2].fX, 0);
    y2 = SkScalarRoundToFDot6(pts[2].fY, 0);
#else
    x0 = SkFloatToFDot6(pts[0].fX);
    y0 = SkFloatToFDot6(pts[0].fY);
    x1 = SkFloatToFDot6(pts[1].fX);
    y1 = SkFloatToFDot6(pts[1].fY);
    x2 = SkFloatToFDot6(pts[2].fX);
    y2 = SkFloatToFDot6(pts[2].fY);
#endif

    Winding winding = Winding::kCW;
    if (y0 > y2) {
        using std::swap;
        swap(x0, x2);
        swap(y0, y2);
        winding = Winding::kCCW;
    }
    SkASSERTF(y0 <= y1 && y1 <= y2, "curve must be monotonic");

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y2);

    // are we a zero-height quad (line)?
    if (top == bot) {
        return false;
    }

    // compute number of steps needed (1 << shift)
    SkFDot6 dx = (SkLeftShift(x1, 1) - x0 - x2) >> 2;
    SkFDot6 dy = (SkLeftShift(y1, 1) - y0 - y2) >> 2;

    int shift = diff_to_shift(dx, dy, 0);
    SkASSERT(shift >= 0);

    // We need at least 2 line segments for us to be able to save the derivatives as
    // half their values to avoid overflow.
    if (shift == 0) {
        shift = 1;
    } else if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    fWinding = winding;
    //fCubicDShift only set for cubics
    fEdgeType = Type::kQuad;
    fCurveCount = SkToS8(1 << shift);

    /*
     *  By re-arranging the Bezier curve in polynomial form, it is easier to
     *  find the derivatives and forward-differentiate from one segment to the next.
     *
     *  p0 (1-t)^2 + p1 t(1-t) + p2 t^2 ==> At^2 + Bt + C
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
    // TODO(kjlubick): Can we use SkVx and calculate both X and Y at once?

    // The extra 1/2 factor avoids overflow
    SkFixed A_half = SkFDot6ToFixedDiv2(x0 - x1 - x1 + x2);
    SkFixed B_half = SkFDot6ToFixed(x1 - x0);

    // We want to calculate the slope at the midpoint of our first segment. This means evaluating
    //   dx/dt = 2A*t + B
    //   dx^2/dt^2 = 2A
    // at t = 1/N * 1/2
    // There's an extra 1/2 on the whole expression to avoid overflows (as above).
    //  1/2 ( 2A*t + B) => 1/2 (2A*1/2N + B) => A/2*1/N + B/2 => A/2 * 1/2^shift + B/2
    fQDxDt = B_half + (A_half >> shift);
    // The second derivatives are constant, so we can pre-multiply them by 1/N to save having
    // to do it in nextSegment(). Since A_half was already calculated we can use a smaller shift.
    // 1/2 (2A * 1/N) => A * 1/N => A * 1/2^shift => A/2 * 1/2^(shift-1)
    fQD2xDt2 = A_half >> (shift - 1);

    A_half = SkFDot6ToFixedDiv2(y0 - y1 - y1 + y2);
    B_half = SkFDot6ToFixed(y1 - y0);

    fQDyDt = B_half + (A_half >> shift);
    fQD2yDt2 = A_half >> (shift - 1);

    fQx     = SkFDot6ToFixed(x0);
    fQy     = SkFDot6ToFixed(y0);
    fQLastX = SkFDot6ToFixed(x2);
    fQLastY = SkFDot6ToFixed(y2);

    return this->updateQuadratic();
}

bool SkQuadraticEdge::updateQuadratic()
{
    bool    success;
    int     count = fCurveCount;
    SkFixed oldx = fQx;
    SkFixed oldy = fQy;
    SkFixed dx = fQDxDt;
    SkFixed dy = fQDyDt;
    SkFixed newx, newy;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        if (--count > 0)
        {
            newx = oldx + (dx >> shift);
            dx += fQD2xDt2;
            newy = oldy + (dy >> shift);
            dy += fQD2yDt2;
        }
        else    // last segment
        {
            newx = fQLastX;
            newy = fQLastY;
        }
        success = this->updateLine(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;
    } while (count > 0 && !success);

    fQx         = newx;
    fQy         = newy;
    fQDxDt        = dx;
    fQDyDt        = dy;
    fCurveCount = SkToS8(count);
    return success;
}

/////////////////////////////////////////////////////////////////////////

static inline int SkFDot6UpShift(SkFDot6 x, int upShift) {
    SkASSERT((SkLeftShift(x, upShift) >> upShift) == x);
    return SkLeftShift(x, upShift);
}

/*  f(1/3) = (8a + 12b + 6c + d) / 27
    f(2/3) = (a + 6b + 12c + 8d) / 27

    f(1/3)-b = (8a - 15b + 6c + d) / 27
    f(2/3)-c = (a + 6b - 15c + 8d) / 27

    use 16/512 to approximate 1/27
*/
static SkFDot6 cubic_delta_from_line(SkFDot6 a, SkFDot6 b, SkFDot6 c, SkFDot6 d)
{
    // since our parameters may be negative, we don't use << to avoid ASAN warnings
    SkFDot6 oneThird = (a*8 - b*15 + 6*c + d) * 19 >> 9;
    SkFDot6 twoThird = (a + 6*b - c*15 + d*8) * 19 >> 9;

    return std::max(SkAbs32(oneThird), SkAbs32(twoThird));
}

bool SkCubicEdge::setCubic(const SkPoint pts[4]) {
    SkFDot6 x0, y0, x1, y1, x2, y2, x3, y3;

#if defined(SK_RASTERIZE_EVEN_ROUNDING)
    x0 = SkScalarRoundToFDot6(pts[0].fX, 0);
    y0 = SkScalarRoundToFDot6(pts[0].fY, 0);
    x1 = SkScalarRoundToFDot6(pts[1].fX, 0);
    y1 = SkScalarRoundToFDot6(pts[1].fY, 0);
    x2 = SkScalarRoundToFDot6(pts[2].fX, 0);
    y2 = SkScalarRoundToFDot6(pts[2].fY, 0);
    x3 = SkScalarRoundToFDot6(pts[3].fX, 0);
    y3 = SkScalarRoundToFDot6(pts[3].fY, 0);
#else
    x0 = SkFloatToFDot6(pts[0].fX);
    y0 = SkFloatToFDot6(pts[0].fY);
    x1 = SkFloatToFDot6(pts[1].fX);
    y1 = SkFloatToFDot6(pts[1].fY);
    x2 = SkFloatToFDot6(pts[2].fX);
    y2 = SkFloatToFDot6(pts[2].fY);
    x3 = SkFloatToFDot6(pts[3].fX);
    y3 = SkFloatToFDot6(pts[3].fY);
#endif

    Winding winding = Winding::kCW;
    if (y0 > y3)
    {
        using std::swap;
        swap(x0, x3);
        swap(x1, x2);
        swap(y0, y3);
        swap(y1, y2);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y3);

    // are we a zero-height cubic (line)?
    if (top == bot) {
        return false;
    }

    // compute number of steps needed (1 << shift)
    // Can't use (center of curve - center of baseline), since center-of-curve
    // need not be the max delta from the baseline (it could even be coincident)
    // so we try just looking at the two off-curve points
    SkFDot6 dx = cubic_delta_from_line(x0, x1, x2, x3);
    SkFDot6 dy = cubic_delta_from_line(y0, y1, y2, y3);
    // add 1 (by observation)
    int shift = diff_to_shift(dx, dy, 2) + 1;
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

    fWinding = winding;
    fEdgeType = Type::kCubic;
    fCurveCount = SkToS8(SkLeftShift(-1, shift));
    fCurveShift = SkToU8(shift);
    fCubicDShift = SkToU8(downShift);

    SkFixed B = SkFDot6UpShift(3 * (x1 - x0), upShift);
    SkFixed C = SkFDot6UpShift(3 * (x0 - x1 - x1 + x2), upShift);
    SkFixed D = SkFDot6UpShift(x3 + 3 * (x1 - x2) - x0, upShift);

    fCx     = SkFDot6ToFixed(x0);
    fCDxDt    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCD2xDt2   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCD3xDt3  = 3*D >> (shift - 1);                   // biased by 2*shift

    B = SkFDot6UpShift(3 * (y1 - y0), upShift);
    C = SkFDot6UpShift(3 * (y0 - y1 - y1 + y2), upShift);
    D = SkFDot6UpShift(y3 + 3 * (y1 - y2) - y0, upShift);

    fCy     = SkFDot6ToFixed(y0);
    fCDyDt    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCD2yDt2   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCD3yDt3  = 3*D >> (shift - 1);                   // biased by 2*shift

    fCLastX = SkFDot6ToFixed(x3);
    fCLastY = SkFDot6ToFixed(y3);

    return this->updateCubic();
}

bool SkCubicEdge::updateCubic()
{
    bool    success;
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
            newx = oldx + (fCDxDt >> dshift);
            fCDxDt += fCD2xDt2 >> ddshift;
            fCD2xDt2 += fCD3xDt3;

            newy = oldy + (fCDyDt >> dshift);
            fCDyDt += fCD2yDt2 >> ddshift;
            fCD2yDt2 += fCD3yDt3;
        }
        else    // last segment
        {
            newx = fCLastX;
            newy = fCLastY;
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
