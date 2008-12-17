/* libs/graphics/sgl/SkScan_Hairline.cpp
**
** Copyright 2006, The Android Open Source Project
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

#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRegion.h"
#include "SkFDot6.h"

static void horiline(int x, int stopx, SkFixed fy, SkFixed dy, SkBlitter* blitter)
{
    SkASSERT(x < stopx);

    do {
        blitter->blitH(x, fy >> 16, 1);
        fy += dy;
    } while (++x < stopx);
}

static void vertline(int y, int stopy, SkFixed fx, SkFixed dx, SkBlitter* blitter)
{
    SkASSERT(y < stopy);

    do {
        blitter->blitH(fx >> 16, y, 1);
        fx += dx;
    } while (++y < stopy);
}

void SkScan::HairLine(const SkPoint& pt0, const SkPoint& pt1, const SkRegion* clip, SkBlitter* blitter)
{
    SkBlitterClipper    clipper;

    SkFDot6 x0 = SkScalarToFDot6(pt0.fX);
    SkFDot6 y0 = SkScalarToFDot6(pt0.fY);
    SkFDot6 x1 = SkScalarToFDot6(pt1.fX);
    SkFDot6 y1 = SkScalarToFDot6(pt1.fY);

    if (clip)
    {
        SkRect      r;
        SkIRect     ir;
        SkPoint     pts[2];

        pts[0] = pt0;
        pts[1] = pt1;
        r.set(pts, 2);
        r.roundOut(&ir);
        
        // if we're given a horizontal or vertical line
        // this rect could be empty (in area), in which case
        // clip->quickReject() will always return true.
        // hence we bloat the rect to avoid that case
        if (ir.width() == 0)
            ir.fRight += 1;
        if (ir.height() == 0)
            ir.fBottom += 1;

        if (clip->quickReject(ir))
            return;
        if (clip->quickContains(ir))
            clip = NULL;
        else
        {
            blitter = clipper.apply(blitter, clip);
        }
    }

    SkFDot6 dx = x1 - x0;
    SkFDot6 dy = y1 - y0;

    if (SkAbs32(dx) > SkAbs32(dy))  // mostly horizontal
    {
        if (x0 > x1)    // we want to go left-to-right
        {
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }
        int ix0 = SkFDot6Round(x0);
        int ix1 = SkFDot6Round(x1);
        if (ix0 == ix1) // too short to draw
            return;

        SkFixed slope = SkFixedDiv(dy, dx);
        SkFixed startY = SkFDot6ToFixed(y0) + (slope * ((32 - x0) & 63) >> 6);

        horiline(ix0, ix1, startY, slope, blitter);
    }
    else                // mostly vertical
    {
        if (y0 > y1)    // we want to go top-to-bottom
        {
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }
        int iy0 = SkFDot6Round(y0);
        int iy1 = SkFDot6Round(y1);
        if (iy0 == iy1) // too short to draw
            return;

        SkFixed slope = SkFixedDiv(dx, dy);
        SkFixed startX = SkFDot6ToFixed(x0) + (slope * ((32 - y0) & 63) >> 6);

        vertline(iy0, iy1, startX, slope, blitter);
    }
}

// we don't just draw 4 lines, 'cause that can leave a gap in the bottom-right
// and double-hit the top-left.
void SkScan::HairRect(const SkRect& rect, const SkRegion* clip, SkBlitter* blitter)
{
    SkBlitterClipper    clipper;
    SkIRect             r;

    r.set(SkScalarToFixed(rect.fLeft) >> 16,
          SkScalarToFixed(rect.fTop) >> 16,
          (SkScalarToFixed(rect.fRight) >> 16) + 1,
          (SkScalarToFixed(rect.fBottom) >> 16) + 1);

    if (clip)
    {
        if (clip->quickReject(r))
            return;
        if (!clip->quickContains(r))
            blitter = clipper.apply(blitter, clip);
    }

    int width = r.width();
    int height = r.height();
    
    if ((width | height) == 0)
        return;
    if (width <= 2 || height <= 2)
    {
        blitter->blitRect(r.fLeft, r.fTop, width, height);
        return;
    }
    // if we get here, we know we have 4 segments to draw
    blitter->blitH(r.fLeft, r.fTop, width);                     // top
    blitter->blitRect(r.fLeft, r.fTop + 1, 1, height - 2);      // left
    blitter->blitRect(r.fRight - 1, r.fTop + 1, 1, height - 2); // right
    blitter->blitH(r.fLeft, r.fBottom - 1, width);              // bottom
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"
#include "SkGeometry.h"

static bool quad_too_curvy(const SkPoint pts[3])
{
    return true;
}

static int compute_int_quad_dist(const SkPoint pts[3]) {
    // compute the vector between the control point ([1]) and the middle of the
    // line connecting the start and end ([0] and [2])
    SkScalar dx = SkScalarHalf(pts[0].fX + pts[2].fX) - pts[1].fX;
    SkScalar dy = SkScalarHalf(pts[0].fY + pts[2].fY) - pts[1].fY;
    // we want everyone to be positive
    dx = SkScalarAbs(dx);
    dy = SkScalarAbs(dy);
    // convert to whole pixel values (use ceiling to be conservative)
    int idx = SkScalarCeil(dx);
    int idy = SkScalarCeil(dy);
    // use the cheap approx for distance
    if (idx > idy) {
        return idx + (idy >> 1);
    } else {
        return idy + (idx >> 1);
    }
}

static void hairquad(const SkPoint pts[3], const SkRegion* clip, SkBlitter* blitter, int level,
                     void (*lineproc)(const SkPoint&, const SkPoint&, const SkRegion* clip, SkBlitter*))
{
#if 1
    if (level > 0 && quad_too_curvy(pts))
    {
        SkPoint tmp[5];

        SkChopQuadAtHalf(pts, tmp);
        hairquad(tmp, clip, blitter, level - 1, lineproc);
        hairquad(&tmp[2], clip, blitter, level - 1, lineproc);
    }
    else
        lineproc(pts[0], pts[2], clip, blitter);
#else
    int count = 1 << level;
    const SkScalar dt = SkFixedToScalar(SK_Fixed1 >> level);
    SkScalar t = dt;
    SkPoint prevPt = pts[0];
    for (int i = 1; i < count; i++) {
        SkPoint nextPt;
        SkEvalQuadAt(pts, t, &nextPt);
        lineproc(prevPt, nextPt, clip, blitter);
        t += dt;
        prevPt = nextPt;
    }
    // draw the last line explicitly to 1.0, in case t didn't match that exactly
    lineproc(prevPt, pts[2], clip, blitter);
#endif
}

static bool cubic_too_curvy(const SkPoint pts[4])
{
    return true;
}

static void haircubic(const SkPoint pts[4], const SkRegion* clip, SkBlitter* blitter, int level,
                      void (*lineproc)(const SkPoint&, const SkPoint&, const SkRegion*, SkBlitter*))
{
    if (level > 0 && cubic_too_curvy(pts))
    {
        SkPoint tmp[7];

        SkChopCubicAt(pts, tmp, SK_Scalar1/2);
        haircubic(tmp, clip, blitter, level - 1, lineproc);
        haircubic(&tmp[3], clip, blitter, level - 1, lineproc);
    }
    else
        lineproc(pts[0], pts[3], clip, blitter);
}

#define kMaxCubicSubdivideLevel 6
#define kMaxQuadSubdivideLevel  5

static void hair_path(const SkPath& path, const SkRegion* clip, SkBlitter* blitter,
                      void (*lineproc)(const SkPoint&, const SkPoint&, const SkRegion*, SkBlitter*))
{
    if (path.isEmpty())
        return;

    const SkIRect* clipR = NULL;

    if (clip)
    {
        SkRect      bounds;
        SkIRect     ibounds;

        path.computeBounds(&bounds, SkPath::kFast_BoundsType);
        bounds.roundOut(&ibounds);
        ibounds.inset(-1, -1);

        if (clip->quickReject(ibounds))
            return;

        if (clip->quickContains(ibounds))
            clip = NULL;
        else
            clipR = &clip->getBounds();
    }

    SkPath::Iter    iter(path, false);
    SkPoint         pts[4];
    SkPath::Verb    verb;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb)
    {
        switch (verb) {
        case SkPath::kLine_Verb:
            lineproc(pts[0], pts[1], clip, blitter);
            break;
        case SkPath::kQuad_Verb: {
            int d = compute_int_quad_dist(pts);
            /*  quadratics approach the line connecting their start and end points
             4x closer with each subdivision, so we compute the number of
             subdivisions to be the minimum need to get that distance to be less
             than a pixel.
             */
            int level = (33 - SkCLZ(d)) >> 1;
//          SkDebugf("----- distance %d computedLevel %d\n", d, computedLevel);
            // sanity check on level (from the previous version)
            if (level > kMaxQuadSubdivideLevel) {
                level = kMaxQuadSubdivideLevel;
            }
            hairquad(pts, clip, blitter, level, lineproc);
            break;
        }
        case SkPath::kCubic_Verb:
            haircubic(pts, clip, blitter, kMaxCubicSubdivideLevel, lineproc);
            break;
        default:
            break;
        }
    }
}

void SkScan::HairPath(const SkPath& path, const SkRegion* clip, SkBlitter* blitter)
{
    hair_path(path, clip, blitter, SkScan::HairLine);
}

void SkScan::AntiHairPath(const SkPath& path, const SkRegion* clip, SkBlitter* blitter)
{
    hair_path(path, clip, blitter, SkScan::AntiHairLine);
}

////////////////////////////////////////////////////////////////////////////////

void SkScan::FrameRect(const SkRect& r, SkScalar diameter, const SkRegion* clip, SkBlitter* blitter)
{
    SkASSERT(diameter > 0);

    if (r.isEmpty())
        return;

    SkScalar radius = diameter / 2;
    SkRect   outer, tmp;

    outer.set(  r.fLeft - radius, r.fTop - radius,
                r.fRight + radius, r.fBottom + radius);

    if (r.width() <= diameter || r.height() <= diameter)
    {
        SkScan::FillRect(outer, clip, blitter);
        return;
    }

    tmp.set(outer.fLeft, outer.fTop, outer.fRight, outer.fTop + diameter);
    SkScan::FillRect(tmp, clip, blitter);
    tmp.fTop = outer.fBottom - diameter;
    tmp.fBottom = outer.fBottom;
    SkScan::FillRect(tmp, clip, blitter);

    tmp.set(outer.fLeft, outer.fTop + diameter, outer.fLeft + diameter, outer.fBottom - diameter);
    SkScan::FillRect(tmp, clip, blitter);
    tmp.fLeft = outer.fRight - diameter;
    tmp.fRight = outer.fRight;
    SkScan::FillRect(tmp, clip, blitter);
}

