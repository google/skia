/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkEdge.h"

#include "include/core/SkRect.h"
#include "include/private/SkDebug.h"
#include "include/private/SkSafe32.h"
#include "include/private/SkTo.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkMathPriv.h"

#include <algorithm>
#include <utility>

/*
    Note on Coordinate Formats and Antialiasing:
    In setLine, setQuadratic, setCubic, the first thing we do is to convert the points
    into FDot6. In some variants (such as SkAnalyticEdge), this conversion is modulated
    by an external antialiasing/supersampling shift parameter (usually 0, or 2 for
    antialiasing) which scales up the input coordinates.

    In the float case, we turn the float into FDot6 by multiplying by 64 (or 256 for
    antialiasing). This is implemented as 1 << (shift + 6).

    In the fixed case, we turn the fixed (FDot16) into FDot6 by shifting right by 10
    (or 8 for antialiasing). This is implemented as pt >> (10 - shift).

    Do not confuse this external coordinate-scaling shift with the internal
    variables used during curve subdivision and forward-differencing in quadratics
    and cubics:
      - 'stepExponent' (defines subdivision resolution N = 2^stepExponent)
      - 'precisionUpScale' (scales coordinates up for precision during setup)
      - 'toFixedShift' (realigns internal derivatives back to standard SkFixed)
*/

// Local type aliases to clarify different fixed-point formats.
//
// 1. SkFDot6Scaled: FDot6 coordinates that are shifted left (scaled up) by the
//    precision upscale factor 'precisionUpScale' before evaluating the polynomial.
//    This format has (6 + precisionUpScale) fractional bits.
//
// 2. SkFixedScaled: Derivatives stored at a scaled precision so they can be efficiently
//    updated and then scaled down back to standard SkFixed (FDot16) inside the loop
//    using 'toFixedShift'.
using SkFDot6Scaled = int32_t;
using SkFixedScaled = int32_t;

static inline SkFixed SkFDot6ToFixedDiv2(SkFDot6 value) {
    // we want to return SkFDot6ToFixed(value >> 1), but we don't want to throw
    // away data in value, so just perform a modify up-shift
    return SkLeftShift(value, 16 - 6 - 1);
}

/////////////////////////////////////////////////////////////////////////

#if defined(SK_DEBUG)
void SkEdge::dump() const {
    SkASSERT(fSegmentCount == 0);
    SkDebugf("line edge: firstY:%d lastY:%d x:%g dx/dy:%g\n"
             "\twinding:%d curveShift:%u\n",
             fFirstY,
             fLastY,
             SkFixedToFloat(fX),
             SkFixedToFloat(fDxDy),
             static_cast<int8_t>(fWinding),
             fCurveShift);
}

void SkQuadraticEdge::dump() const {
    SkDebugf("quad edge; %u segment(s) left: firstY:%d lastY:%d x:%g dx/dy:%g\n"
             "\tqx:%g qy:%g dqx:%g dqy:%g ddqx:%g ddqy:%g qLastX:%g qLastY:%g\n"
             "\twinding:%d curveShift:%u\n",
             fSegmentCount,
             fFirstY,
             fLastY,
             SkFixedToFloat(fX),
             SkFixedToFloat(fDxDy),
             SkFixedToFloat(fQx),
             SkFixedToFloat(fQy),
             SkFixedToFloat(fQDxDt),
             SkFixedToFloat(fQDyDt),
             SkFixedToFloat(fQD2xDt2),
             SkFixedToFloat(fQD2yDt2),
             SkFixedToFloat(fQLastX),
             SkFixedToFloat(fQLastY),
             static_cast<int8_t>(fWinding),
             fCurveShift);
}

void SkCubicEdge::dump() const {
    SkDebugf("cube edge; %u segment(s) left: firstY:%d lastY:%d x:%g dx/dy:%g\n"
             "qx:%g qy:%g dcx:%g dcy:%g ddcx:%g ddcy:%g dddcx:%g dddcy:%g cLastX:%g cLastY:%g\n"
             "\twinding:%d curveShift:%u toFixedShift:%u\n",
             fSegmentCount,
             fFirstY,
             fLastY,
             SkFixedToFloat(fX),
             SkFixedToFloat(fDxDy),
             SkFixedToFloat(fCx),
             SkFixedToFloat(fCy),
             SkFixedToFloat(fCDxDt),
             SkFixedToFloat(fCDyDt),
             SkFixedToFloat(fCD2xDt2),
             SkFixedToFloat(fCD2yDt2),
             SkFixedToFloat(fCD3xDt3),
             SkFixedToFloat(fCD3yDt3),
             SkFixedToFloat(fCLastX),
             SkFixedToFloat(fCLastY),
             static_cast<int8_t>(fWinding),
             fCurveShift,
             fToFixedShift);
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
        std::swap(x0, x1);
        std::swap(y0, y1);
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
    fSegmentCount = 0;
    fWinding    = winding;
    fCurveShift = 0;

    if (clip) {
        this->chopLineWithClip(*clip);
    }
    return true;
}

bool SkEdge::nextSegment() {
    SkDEBUGFAILF("Shouldn't be asking a linear edge to go to the next curve.");
    return false;
}

// Draws a line between the provided points and then calculates the slope and starting
// x value to line up with the closest pixel center. Updates the fields in the SkEdge
// base class appropriately. Returns false if this edge would start and stop in the
// same row.
bool SkEdge::updateLine(SkFixed xStart, SkFixed yStart, SkFixed xEnd, SkFixed yEnd) {
    SkASSERT(fWinding == Winding::kCW || fWinding == Winding::kCCW);
    SkASSERT(fSegmentCount != 0);

    const SkFDot6 y0 = SkFixedToFDot6(yStart);
    const SkFDot6 y1 = SkFixedToFDot6(yEnd);

    SkASSERT(y0 <= y1);

    const int top = SkFDot6Round(y0);
    const int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot) {
        return false;
    }

    const SkFDot6 x0 = SkFixedToFDot6(xStart);
    const SkFDot6 x1 = SkFixedToFDot6(xEnd);

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy = SkEdge_Compute_DY(top, y0);

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
    TODO(kjlubick): now that this is in an unsigned byte, we could go up to 7
*/
#define MAX_COEFF_SHIFT     6

// Approximate the distance from (0,0) to (dx, dy).
// When dx and dy are about the same
//   sqrt(dx^2 + dy^2) => sqrt(2dx^2) => dx sqrt(2) = 1.41 * dx
// When dx >> dy
//   sqrt(dx^2 + dy^2) => sqrt(dx^2) => dx
// So this is a reasonable approximation
static inline SkFDot6 cheap_distance(SkFDot6 dx, SkFDot6 dy) {
    dx = SkAbs32(dx);
    dy = SkAbs32(dy);
    // return max + min/2
    if (dx > dy) {
        return dx + (dy / 2);
    }
    return dy + (dx / 2);
}

static inline int diff_to_steps(SkFDot6 dx, SkFDot6 dy, int accuracy) {
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
        std::swap(x0, x2);
        std::swap(y0, y2);
        winding = Winding::kCCW;
    }
    SkASSERTF(y0 <= y1 && y1 <= y2, "curve must be monotonic");

    const int top = SkFDot6Round(y0);
    const int bot = SkFDot6Round(y2);

    // are we a zero-height quad (line)?
    if (top == bot) {
        return false;
    }

    // compute number of steps needed (2^shift) based on the distance between
    // this curve at the half-way point (t=0.5) and the midpoint of a straight
    // line between p0 and p2.
    // B(1/2) = p0 (1-t)^2 + 2 p1 t(1-t) + p2 t^2; t = 1/2
    //        = p0 (1/2)^2 + 2 p1 (1/2)(1/2) + p2 (1/2)^2
    //        = 1/4 (p0 + 2 p1 + p2)
    // Midpoint of p0 and p2 is M(p0, p2) = (p2 + p0) / 2
    // Subtracting the two terms to get the vector representing the difference
    // distance = B(1/2) - M(p0, p2)
    //          = 1/4 (p0 + 2 p1 + p2) - (p2 + p0) / 2
    //          = 1/4 (p0 + 2 p1 + p2) - (2 p2 + 2 p0) / 4
    //          = 1/4 (-p0 + 2 p1 - p2)
    SkFDot6 deltaX = (2*x1 - x0 - x2) >> 2;
    SkFDot6 deltaY = (2*y1 - y0 - y2) >> 2;
    // We pass those points into this function which will find the total distance
    // and use a heuristic to reduce the error to some threshold.
    int shift = diff_to_steps(deltaX, deltaY, 0);
    SkASSERT(shift >= 0);

    // We need at least 2 line segments for us to be able to save the derivatives as
    // half their values to avoid overflow.
    if (shift == 0) {
        shift = 1;
    } else if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    fWinding = winding;
    fEdgeType = Type::kQuad;
    fSegmentCount = SkToU8(1 << shift);

    /*
     *  By re-arranging the Bezier curve in polynomial form, it is easier to
     *  find the derivatives and forward-differentiate from one segment to the next.
     *
     *  p0 (1-t)^2 + 2 p1 t(1-t) + p2 t^2 ==> At^2 + Bt + C
     *
     *  A = p0 - 2p1 + p2
     *  B = 2(p1 - p0)
     *  C = p0
     *
     *  Our caller must have constrained our inputs (p0..p2) to all fit into
     *  16.16. However, as seen above, we sometimes compute values that can be
     *  larger (e.g. B = 2*(p1 - p0)). To guard against overflow, we will store
     *  A and B at 1/2 of their actual value, and just apply a 2x scale during
     *  application in nextSegment(). Hence we store (shift - 1) in
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

    return this->nextSegment();
}

bool SkQuadraticEdge::nextSegment() {
    bool    success;
    int     count = fSegmentCount;
    SkFixed oldx = fQx;
    SkFixed oldy = fQy;
    SkFixed dx = fQDxDt;
    SkFixed dy = fQDyDt;
    SkFixed newx, newy;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        if (--count > 0) {
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

    fQx = newx;
    fQy = newy;
    fQDxDt = dx;
    fQDyDt = dy;
    fSegmentCount = SkToU8(count);
    return success;
}

/////////////////////////////////////////////////////////////////////////

static inline SkFDot6Scaled SkFDot6UpShift(SkFDot6 x, int upShift) {
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
    if (y0 > y3) {
        std::swap(x0, x3);
        std::swap(x1, x2);
        std::swap(y0, y3);
        std::swap(y1, y2);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y3);

    // are we a zero-height cubic (line)?
    if (top == bot) {
        return false;
    }

    // compute number of steps needed (1 << stepExponent)
    // Can't use (center of curve - center of baseline), since center-of-curve
    // need not be the max delta from the baseline (it could even be coincident)
    // so we try just looking at the two off-curve points
    SkFDot6 dx = cubic_delta_from_line(x0, x1, x2, x3);
    SkFDot6 dy = cubic_delta_from_line(y0, y1, y2, y3);
    // add 1 (by observation)
    int stepExponent = diff_to_steps(dx, dy, 2) + 1;
    // need at least 1 subdivision for our bias trick
    SkASSERT(stepExponent > 0);
    if (stepExponent > MAX_COEFF_SHIFT) {
        stepExponent = MAX_COEFF_SHIFT;
    }

    // To maintain maximum precision and avoid intermediate divisions, we manage
    // three different "shifts" for the cubic forward-differencing math:
    //
    // 1. "stepExponent" (Subdivision Exponent):
    //    - The exponent for the number of segments: N = 2^stepExponent.
    //    - Parametric step size: h = 1/N = 2^-stepExponent.
    //    - Stored in fCurveShift. Used to update the first derivative step-to-step.
    //
    // 2. "precisionUpScale" (Precision Upscale Shift):
    //    - We scale up the incoming FDot6 coordinates by 2^precisionUpScale (via SkFDot6UpShift)
    //      before constructing our polynomial coefficients (A, B, C).
    //    - This prevents fractional bits from being shifted off and lost when dividing
    //      coefficients by powers of 2^stepExponent (i.e. >> stepExponent or >> 2*stepExponent)
    //      during setup.
    //    - Capped at 6 to prevent signed 32-bit integer overflow during intermediate
    //      computations (which involve multiplications by 3 and 6).
    //
    // 3. "toFixedShift" (Coordinate Realignment Downshift):
    //    - Stored in fToFixedShift. Used in nextSegment() to scale the step delta
    //      back down to standard SkFixed (FDot16) format.
    //    - Since the coefficients are scaled up by 2^precisionUpScale, the step size h is
    //      2^-stepExponent, and standard SkFixed has 10 more fractional bits than
    //      FDot6 (16 - 6 = 10), the alignment factor for the position update (x + fCDxDt * h) is:
    //          2^10 / (2^precisionUpScale * 2^stepExponent) =
    //             1 / 2^(stepExponent + precisionUpScale - 10)
    //      which is implemented as a right-shift by:
    //          toFixedShift = stepExponent + precisionUpScale - 10.
    //    - If toFixedShift is negative (which would require an unsupported left-shift),
    //      we clamp toFixedShift to 0 and reduce precisionUpScale accordingly to
    //      10 - stepExponent.
    int precisionUpScale = 6;    // largest safe value
    int toFixedShift = stepExponent + precisionUpScale - 10;
    if (toFixedShift < 0) {
        toFixedShift = 0;
        precisionUpScale = 10 - stepExponent;
    }

    fWinding = winding;
    fEdgeType = Type::kCubic;
    fSegmentCount = SkToU8(SkLeftShift(1, stepExponent));
    fCurveShift = SkToU8(stepExponent);
    fToFixedShift = SkToU8(toFixedShift);

    // By re-arranging the Bezier curve in polynomial form, it is easier to
    // find the derivatives and forward-differentiate from one segment to the next.

    // p0 (1-t)^3 + 3 p1 t(1-t)^2 + 3 p2 t^2 (1-t) + p3 t^3 ==> At^3 + Bt^2 + Ct + D
    // Where A = -p0 + 3p1 + -3p2 + p3
    //       B = 3p0 - 6p1 + 3p2
    //       C = -3p0 + 3p1
    //       D = p0
    // TODO(kjlubick): Can we use SkVx and calculate both X and Y at once?
    SkFDot6Scaled A_scaled = SkFDot6UpShift(x3 + 3 * (x1 - x2) - x0, precisionUpScale);
    SkFDot6Scaled B_scaled = SkFDot6UpShift(3 * (x0 - 2*x1 + x2), precisionUpScale);
    SkFDot6Scaled C_scaled = SkFDot6UpShift(3 * (x1 - x0), precisionUpScale);


    // The cubic curve in polynomial form is: x(t) = A*t^3 + B*t^2 + C*t + D
    // With a step size of h = 1/N = 1/(2^stepExponent), the forward differences at t=0 are:
    //   1) First Difference:  Δx(0) = x(h) - x(0)      = A*h^3 + B*h^2 + C*h
    //   2) Second Difference: Δ²x(0) = Δx(h) - Δx(0)    = 6A*h^3 + 2B*h^2
    //   3) Third Difference:  Δ³x(0) = Δ²x(h) - Δ²x(0)  = 6A*h^3

    // To keep the math as precise as possible, we scale up each difference term.
    // Because the step size h is a power of 2 (1 / 2^stepExponent), scaling them up
    // allows us to perform all loop updates using bit-shifts instead of slow division:

    // - fCDxDt   = Δx(0)  / h  = A*h^2 + B*h + C
    //                          = A*(1/2^stepExponent)*(1/2^stepExponent) + B*(1/2^stepExponent) + C
    //                          = A/(1^(2*stepExponent) + B/(2^stepExponent) + C
    // - fCD2xDt2 = Δ²x(0) / h² = 6A*h + 2B
    //                          = 6A*(1/2^stepExponent) + 2B # cancel 2 on top and bottom of A
    //                          = 3A/2^(stepExponent-1) + 2B
    // - fCD3xDt3 = Δ³x(0) / h² = 6A*h
    //                      = 6A*(1/2^stepExponent)
    //                      = 3A/2^(stepExponent-1)
    // These are stored in the edge struct as SkFixedScaled because they must be scaled down
    // by toFixedShift or stepExponent (ddshift) before they can be added to standard SkFixed
    // coordinates or used to update other derivative terms.
    fCDxDt   = (A_scaled >> 2*stepExponent) + (B_scaled >> stepExponent) + C_scaled;
    fCD2xDt2 = (3*A_scaled >> (stepExponent - 1)) + 2*B_scaled;
    fCD3xDt3 = 3*A_scaled >> (stepExponent - 1);

    A_scaled = SkFDot6UpShift(y3 + 3 * (y1 - y2) - y0, precisionUpScale);
    B_scaled = SkFDot6UpShift(3 * (y0 - 2*y1 + y2), precisionUpScale);
    C_scaled = SkFDot6UpShift(3 * (y1 - y0), precisionUpScale);

    fCDyDt   = (A_scaled >> 2*stepExponent) + (B_scaled >> stepExponent) + C_scaled;
    fCD2yDt2 = (3*A_scaled >> (stepExponent - 1)) + 2*B_scaled;
    fCD3yDt3 = 3*A_scaled >> (stepExponent - 1);

    fCx = SkFDot6ToFixed(x0);
    fCy = SkFDot6ToFixed(y0);
    fCLastX = SkFDot6ToFixed(x3);
    fCLastY = SkFDot6ToFixed(y3);

    return this->nextSegment();
}

bool SkCubicEdge::nextSegment() {
    bool    success;
    int     count = fSegmentCount;
    SkFixed oldx = fCx;
    SkFixed oldy = fCy;
    SkFixed newx, newy;
    const int stepExponent = fCurveShift; // Subdivision exponent (stepExponent)
    const int toFixedShift = fToFixedShift;

    SkASSERT(count > 0);

    do {
        if (--count > 0)
        {
            // 1. Position Update: x_next = x + (fCDxDt * h)
            //    Since fCDxDt has units of FDot6 scaled up by precisionUpScale, and is stored
            //    as (Δx / h) (which scales it up by 1/h = 2^stepExponent), we must:
            //      a) Divide by 2^stepExponent to multiply by step-size h
            //      b) Divide by 2^precisionUpScale to remove the precision upscaling
            //      c) Multiply by 2^10 to convert from FDot6 to SkFixed (since 16 - 6 = 10)
            //    Combining these: Δx = fCDxDt * (2^10) / (2^stepExponent * 2^precisionUpScale)
            //                        = fCDxDt >> (stepExponent + precisionUpScale - 10)
            //    This is implemented as shifting right by toFixedShift.
            newx = oldx + (fCDxDt >> toFixedShift);

            // 2. First Difference Update: fCDxDt_next = fCDxDt + h * fCD2xDt2
            //    Since fCD2xDt2 has units of Δ²x / h², multiplying by h yields Δ²x / h.
            //    This is accomplished by right-shifting fCD2xDt2 by stepExponent
            //    (h = 1/2^stepExponent).
            fCDxDt += fCD2xDt2 >> stepExponent;

            // 3. Second Difference Update: fCD2xDt2_next = fCD2xDt2 + fCD3xDt3
            //    Since both fCD2xDt2 and fCD3xDt3 are scaled by 1/h², we add them directly.
            fCD2xDt2 += fCD3xDt3;

            newy = oldy + (fCDyDt >> toFixedShift);
            fCDyDt += fCD2yDt2 >> stepExponent;
            fCD2yDt2 += fCD3yDt3;
        }
        else // last segment
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
    } while (count > 0 && !success);

    fCx = newx;
    fCy = newy;
    fSegmentCount = SkToU8(count);
    return success;
}
