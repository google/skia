/* libs/graphics/sgl/SkEdge.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkEdge.h"
#include "SkFDot6.h"

/*
    In setLine, setQuadratic, setCubic, the first thing we do is to convert
    the points into FDot6. This is modulated by the shift parameter, which
    will either be 0, or something like 2 for antialiasing.

    In the float case, we want to turn the float into .6 by saying pt * 64,
    or pt * 256 for antialiasing. This is implemented as 1 << (shift + 6).

    In the fixed case, we want to turn the fixed into .6 by saying pt >> 10,
    or pt >> 8 for antialiasing. This is implemented as pt >> (10 - shift).
*/

/////////////////////////////////////////////////////////////////////////

int SkEdge::setLine(const SkPoint pts[2], const SkRect16* clip, int shift)
{
    SkFDot6 x0, y0, x1, y1;

    {
#ifdef SK_SCALAR_IS_FLOAT
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
#else
        shift = 10 - shift;
        x0 = pts[0].fX >> shift;
        y0 = pts[0].fY >> shift;
        x1 = pts[1].fX >> shift;
        y1 = pts[1].fY >> shift;
#endif
    }

    int     winding = 1;

    if (y0 > y1)
    {
        SkTSwap(x0, x1);
        SkTSwap(y0, y1);
        winding = -1;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot)
        return 0;

    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop))
        return 0;

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, (32 - y0) & 63));   // + SK_Fixed1/2
    fDX         = slope;
    fFirstY     = SkToS16(top);
    fLastY      = SkToS16(bot - 1);
    fCurveCount = 0;
    fWinding    = SkToS8(winding);
    fCurveShift = 0;

    if (clip)
        this->chopLineWithClip(*clip);
    return 1;
}

// called from a curve subclass
int SkEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1)
{
    SkASSERT(fWinding == 1 || fWinding == -1);
    SkASSERT(fCurveCount != 0);
    SkASSERT(fCurveShift != 0);

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

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, (32 - y0) & 63));   // + SK_Fixed1/2
    fDX         = slope;
    fFirstY     = SkToS16(top);
    fLastY      = SkToS16(bot - 1);

    return 1;
}

void SkEdge::chopLineWithClip(const SkRect16& clip)
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

/////////////////////////////////////////////////////////////////////////

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
    dist >>= 5;

    // each subdivision (shift value) cuts this dist (error) by 1/4
    return (32 - SkCLZ(dist)) >> 1;
}

int SkQuadraticEdge::setQuadratic(const SkPoint pts[3], const SkRect16* clip, int shift)
{
    SkFDot6 x0, y0, x1, y1, x2, y2;

    {
#ifdef SK_SCALAR_IS_FLOAT
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
        x2 = int(pts[2].fX * scale);
        y2 = int(pts[2].fY * scale);
#else
        shift = 10 - shift;
        x0 = pts[0].fX >> shift;
        y0 = pts[0].fY >> shift;
        x1 = pts[1].fX >> shift;
        y1 = pts[1].fY >> shift;
        x2 = pts[2].fX >> shift;
        y2 = pts[2].fY >> shift;
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
    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop))
        return 0;

    // compute number of steps needed (1 << shift)
    {
        SkFDot6 dx = ((x1 << 1) - x0 - x2) >> 2;
        SkFDot6 dy = ((y1 << 1) - y0 - y2) >> 2;
        shift = diff_to_shift(dx, dy);
    }
    // need at least 1 subdivision for our bias trick
    if (shift == 0)
        shift = 1;

    fWinding    = SkToS8(winding);
    fCurveShift = SkToU8(shift);
    fCurveCount = SkToS16(1 << shift);

    SkFixed A = SkFDot6ToFixed(x0 - x1 - x1 + x2);
    SkFixed B = SkFDot6ToFixed(x1 - x0 + x1 - x0);

    fQx     = SkFDot6ToFixed(x0);
    fQDx    = B + (A >> shift);     // biased by shift
    fQDDx   = A >> (shift - 1);     // biased by shift

    A = SkFDot6ToFixed(y0 - y1 - y1 + y2);
    B = SkFDot6ToFixed(y1 - y0 + y1 - y0);

    fQy     = SkFDot6ToFixed(y0);
    fQDy    = B + (A >> shift);     // biased by shift
    fQDDy   = A >> (shift - 1);     // biased by shift

    fQLastX = SkFDot6ToFixed(x2);
    fQLastY = SkFDot6ToFixed(y2);

    if (clip)
    {
        do {
            for (;!this->updateQuadratic();)
                ;
        } while (!this->intersectsClip(*clip));
        this->chopLineWithClip(*clip);
        return 1;
    }
    return this->updateQuadratic();
}

int SkQuadraticEdge::updateQuadratic()
{
    int     success;
    int     count = fCurveCount;
    SkFixed oldx = fQx;
    SkFixed oldy = fQy;
    SkFixed newx, newy;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        if (--count > 0)
        {
            newx    = oldx + (fQDx >> shift);
            fQDx    += fQDDx;
            newy    = oldy + (fQDy >> shift);
            fQDy    += fQDDy;
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
    fCurveCount = SkToS16(count);
    return success;
}

/////////////////////////////////////////////////////////////////////////

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

int SkCubicEdge::setCubic(const SkPoint pts[4], const SkRect16* clip, int shift)
{
    SkFDot6 x0, y0, x1, y1, x2, y2, x3, y3;

    {
#ifdef SK_SCALAR_IS_FLOAT
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
        x2 = int(pts[2].fX * scale);
        y2 = int(pts[2].fY * scale);
        x3 = int(pts[3].fX * scale);
        y3 = int(pts[3].fY * scale);
#else
        shift = 10 - shift;
        x0 = pts[0].fX >> shift;
        y0 = pts[0].fY >> shift;
        x1 = pts[1].fX >> shift;
        y1 = pts[1].fY >> shift;
        x2 = pts[2].fX >> shift;
        y2 = pts[2].fY >> shift;
        x3 = pts[3].fX >> shift;
        y3 = pts[3].fY >> shift;
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

    fWinding    = SkToS8(winding);
    fCurveShift = SkToU8(shift);
    fCurveCount = SkToS16(-1 << shift);

    SkFixed B = SkFDot6ToFixed(3 * (x1 - x0));
    SkFixed C = SkFDot6ToFixed(3 * (x0 - x1 - x1 + x2));
    SkFixed D = SkFDot6ToFixed(x3 + 3 * (x1 - x2) - x0);

    fCx     = SkFDot6ToFixed(x0);
    fCDx    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDx   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDx  = 3*D >> (shift - 1);                   // biased by 2*shift

    B = SkFDot6ToFixed(3 * (y1 - y0));
    C = SkFDot6ToFixed(3 * (y0 - y1 - y1 + y2));
    D = SkFDot6ToFixed(y3 + 3 * (y1 - y2) - y0);

    fCy     = SkFDot6ToFixed(y0);
    fCDy    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDy   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDy  = 3*D >> (shift - 1);                   // biased by 2*shift

    fCLastX = SkFDot6ToFixed(x3);
    fCLastY = SkFDot6ToFixed(y3);

    if (clip)
    {
        do {
            for (;!this->updateCubic();)
                ;
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
    int     shift = fCurveShift;

    SkASSERT(count < 0);

    do {
        if (++count < 0)
        {
            newx    = oldx + (fCDx >> shift);
            fCDx    += fCDDx >> shift;
            fCDDx   += fCDDDx;

            newy    = oldy + (fCDy >> shift);
            fCDy    += fCDDy >> shift;
            fCDDy   += fCDDDy;
        }
        else    // last segment
        {
        //  SkDebugf("LastX err=%d, LastY err=%d\n", (oldx + (fCDx >> shift) - fLastX), (oldy + (fCDy >> shift) - fLastY));
            newx    = fCLastX;
            newy    = fCLastY;
        }
        success = this->updateLine(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;
    } while (count < 0 && !success);

    fCx         = newx;
    fCy         = newy;
    fCurveCount = SkToS16(count);
    return success;
}



