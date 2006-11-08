/* libs/graphics/sgl/SkScan_Hairline.cpp
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
        SkRect16    ir;
        SkPoint     pts[2];

        pts[0] = pt0;
        pts[1] = pt1;
        r.set(pts, 2);
        r.roundOut(&ir);

        if (clip->quickReject(ir))
            return;
        if (clip->quickContains(ir))
            clip = nil;
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
        SkFixed startY = SkFDot6ToFixed(y0 + SkFixedMul(slope, (32 - x0) & 63));

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
        SkFixed startX = SkFDot6ToFixed(x0 + SkFixedMul(slope, (32 - y0) & 63));

        vertline(iy0, iy1, startX, slope, blitter);
    }
}

void SkScan::HairRect(const SkRect& rect, const SkRegion* clip, SkBlitter* blitter)
{
    SkPoint p0, p1;

    p0.set(rect.fLeft, rect.fTop);
    p1.set(rect.fRight, rect.fTop);
    SkScan::HairLine(p0, p1, clip, blitter);
    p0.set(rect.fRight, rect.fBottom);
    SkScan::HairLine(p0, p1, clip, blitter);
    p1.set(rect.fLeft, rect.fBottom);
    SkScan::HairLine(p0, p1, clip, blitter);
    p0.set(rect.fLeft, rect.fTop);
    SkScan::HairLine(p0, p1, clip, blitter);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"
#include "SkGeometry.h"

static bool quad_too_curvy(const SkPoint pts[3])
{
    return true;
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
    lineproc(pts[0], pts[1], clip, blitter);
    lineproc(pts[1], pts[2], clip, blitter);
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

    const SkRect16* clipR = nil;

    if (clip)
    {
        SkRect      bounds;
        SkRect16    ibounds;

        path.computeBounds(&bounds, SkPath::kFast_BoundsType);
        bounds.roundOut(&ibounds);
        ibounds.inset(-1, -1);

        if (clip->quickReject(ibounds))
            return;

        if (clip->quickContains(ibounds))
            clip = nil;
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
        case SkPath::kQuad_Verb:
            hairquad(pts, clip, blitter, kMaxQuadSubdivideLevel, lineproc);
            break;
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

