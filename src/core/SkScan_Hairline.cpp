
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRasterClip.h"
#include "SkFDot6.h"
#include "SkLineClipper.h"

static void horiline(int x, int stopx, SkFixed fy, SkFixed dy,
                     SkBlitter* blitter) {
    SkASSERT(x < stopx);

    do {
        blitter->blitH(x, fy >> 16, 1);
        fy += dy;
    } while (++x < stopx);
}

static void vertline(int y, int stopy, SkFixed fx, SkFixed dx,
                     SkBlitter* blitter) {
    SkASSERT(y < stopy);

    do {
        blitter->blitH(fx >> 16, y, 1);
        fx += dx;
    } while (++y < stopy);
}

#ifdef SK_DEBUG
static bool canConvertFDot6ToFixed(SkFDot6 x) {
    const int maxDot6 = SK_MaxS32 >> (16 - 6);
    return SkAbs32(x) <= maxDot6;
}
#endif

void SkScan::HairLineRgn(const SkPoint& pt0, const SkPoint& pt1,
                         const SkRegion* clip, SkBlitter* blitter) {
    SkBlitterClipper    clipper;
    SkRect  r;
    SkIRect clipR, ptsR;
    SkPoint pts[2] = { pt0, pt1 };

#ifdef SK_SCALAR_IS_FLOAT
    // We have to pre-clip the line to fit in a SkFixed, so we just chop
    // the line. TODO find a way to actually draw beyond that range.
    {
        SkRect fixedBounds;
        const SkScalar max = SkIntToScalar(32767);
        fixedBounds.set(-max, -max, max, max);
        if (!SkLineClipper::IntersectLine(pts, fixedBounds, pts)) {
            return;
        }
    }
#endif

    if (clip) {
        // Perform a clip in scalar space, so we catch huge values which might
        // be missed after we convert to SkFDot6 (overflow)
        r.set(clip->getBounds());
        if (!SkLineClipper::IntersectLine(pts, r, pts)) {
            return;
        }
    }

    SkFDot6 x0 = SkScalarToFDot6(pts[0].fX);
    SkFDot6 y0 = SkScalarToFDot6(pts[0].fY);
    SkFDot6 x1 = SkScalarToFDot6(pts[1].fX);
    SkFDot6 y1 = SkScalarToFDot6(pts[1].fY);

    SkASSERT(canConvertFDot6ToFixed(x0));
    SkASSERT(canConvertFDot6ToFixed(y0));
    SkASSERT(canConvertFDot6ToFixed(x1));
    SkASSERT(canConvertFDot6ToFixed(y1));

    if (clip) {
        // now perform clipping again, as the rounding to dot6 can wiggle us
        // our rects are really dot6 rects, but since we've already used
        // lineclipper, we know they will fit in 32bits (26.6)
        const SkIRect& bounds = clip->getBounds();

        clipR.set(SkIntToFDot6(bounds.fLeft), SkIntToFDot6(bounds.fTop),
                  SkIntToFDot6(bounds.fRight), SkIntToFDot6(bounds.fBottom));
        ptsR.set(x0, y0, x1, y1);
        ptsR.sort();

        // outset the right and bottom, to account for how hairlines are
        // actually drawn, which may hit the pixel to the right or below of
        // the coordinate
        ptsR.fRight += SK_FDot6One;
        ptsR.fBottom += SK_FDot6One;

        if (!SkIRect::Intersects(ptsR, clipR)) {
            return;
        }
        if (clip->isRect() && clipR.contains(ptsR)) {
            clip = NULL;
        } else {
            blitter = clipper.apply(blitter, clip);
        }
    }

    SkFDot6 dx = x1 - x0;
    SkFDot6 dy = y1 - y0;

    if (SkAbs32(dx) > SkAbs32(dy)) { // mostly horizontal
        if (x0 > x1) {   // we want to go left-to-right
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }
        int ix0 = SkFDot6Round(x0);
        int ix1 = SkFDot6Round(x1);
        if (ix0 == ix1) {// too short to draw
            return;
        }

        SkFixed slope = SkFixedDiv(dy, dx);
        SkFixed startY = SkFDot6ToFixed(y0) + (slope * ((32 - x0) & 63) >> 6);

        horiline(ix0, ix1, startY, slope, blitter);
    } else {              // mostly vertical
        if (y0 > y1) {   // we want to go top-to-bottom
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }
        int iy0 = SkFDot6Round(y0);
        int iy1 = SkFDot6Round(y1);
        if (iy0 == iy1) { // too short to draw
            return;
        }

        SkFixed slope = SkFixedDiv(dx, dy);
        SkFixed startX = SkFDot6ToFixed(x0) + (slope * ((32 - y0) & 63) >> 6);

        vertline(iy0, iy1, startX, slope, blitter);
    }
}

// we don't just draw 4 lines, 'cause that can leave a gap in the bottom-right
// and double-hit the top-left.
// TODO: handle huge coordinates on rect (before calling SkScalarToFixed)
void SkScan::HairRect(const SkRect& rect, const SkRasterClip& clip,
                      SkBlitter* blitter) {
    SkAAClipBlitterWrapper wrapper;
    SkBlitterClipper    clipper;
    SkIRect             r;

    r.set(SkScalarToFixed(rect.fLeft) >> 16,
          SkScalarToFixed(rect.fTop) >> 16,
          (SkScalarToFixed(rect.fRight) >> 16) + 1,
          (SkScalarToFixed(rect.fBottom) >> 16) + 1);

    if (clip.quickReject(r)) {
        return;
    }
    if (!clip.quickContains(r)) {
        const SkRegion* clipRgn;
        if (clip.isBW()) {
            clipRgn = &clip.bwRgn();
        } else {
            wrapper.init(clip, blitter);
            clipRgn = &wrapper.getRgn();
            blitter = wrapper.getBlitter();
        }
        blitter = clipper.apply(blitter, clipRgn);
    }

    int width = r.width();
    int height = r.height();

    if ((width | height) == 0) {
        return;
    }
    if (width <= 2 || height <= 2) {
        blitter->blitRect(r.fLeft, r.fTop, width, height);
        return;
    }
    // if we get here, we know we have 4 segments to draw
    blitter->blitH(r.fLeft, r.fTop, width);                     // top
    blitter->blitRect(r.fLeft, r.fTop + 1, 1, height - 2);      // left
    blitter->blitRect(r.fRight - 1, r.fTop + 1, 1, height - 2); // right
    blitter->blitH(r.fLeft, r.fBottom - 1, width);              // bottom
}

///////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"
#include "SkGeometry.h"

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
    if (level > 0)
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

static void haircubic(const SkPoint pts[4], const SkRegion* clip, SkBlitter* blitter, int level,
                      void (*lineproc)(const SkPoint&, const SkPoint&, const SkRegion*, SkBlitter*))
{
    if (level > 0)
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

static void hair_path(const SkPath& path, const SkRasterClip& rclip, SkBlitter* blitter,
                      void (*lineproc)(const SkPoint&, const SkPoint&, const SkRegion*, SkBlitter*))
{
    if (path.isEmpty()) {
        return;
    }

    SkAAClipBlitterWrapper wrap;
    const SkRegion* clip = NULL;

    {
        SkIRect ibounds;
        path.getBounds().roundOut(&ibounds);
        ibounds.inset(-1, -1);

        if (rclip.quickReject(ibounds)) {
            return;
        }
        if (!rclip.quickContains(ibounds)) {
            if (rclip.isBW()) {
                clip = &rclip.bwRgn();
            } else {
                wrap.init(rclip, blitter);
                blitter = wrap.getBlitter();
                clip = &wrap.getRgn();
            }
        }
    }

    SkPath::Iter    iter(path, false);
    SkPoint         pts[4];
    SkPath::Verb    verb;

    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
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

void SkScan::HairPath(const SkPath& path, const SkRasterClip& clip,
                      SkBlitter* blitter) {
    hair_path(path, clip, blitter, SkScan::HairLineRgn);
}

void SkScan::AntiHairPath(const SkPath& path, const SkRasterClip& clip,
                          SkBlitter* blitter) {
    hair_path(path, clip, blitter, SkScan::AntiHairLineRgn);
}

///////////////////////////////////////////////////////////////////////////////

void SkScan::FrameRect(const SkRect& r, const SkPoint& strokeSize,
                       const SkRasterClip& clip, SkBlitter* blitter) {
    SkASSERT(strokeSize.fX >= 0 && strokeSize.fY >= 0);

    if (strokeSize.fX < 0 || strokeSize.fY < 0) {
        return;
    }

    const SkScalar dx = strokeSize.fX;
    const SkScalar dy = strokeSize.fY;
    SkScalar rx = SkScalarHalf(dx);
    SkScalar ry = SkScalarHalf(dy);
    SkRect   outer, tmp;

    outer.set(r.fLeft - rx, r.fTop - ry,
                r.fRight + rx, r.fBottom + ry);

    if (r.width() <= dx || r.height() <= dx) {
        SkScan::FillRect(outer, clip, blitter);
        return;
    }

    tmp.set(outer.fLeft, outer.fTop, outer.fRight, outer.fTop + dy);
    SkScan::FillRect(tmp, clip, blitter);
    tmp.fTop = outer.fBottom - dy;
    tmp.fBottom = outer.fBottom;
    SkScan::FillRect(tmp, clip, blitter);

    tmp.set(outer.fLeft, outer.fTop + dy, outer.fLeft + dx, outer.fBottom - dy);
    SkScan::FillRect(tmp, clip, blitter);
    tmp.fLeft = outer.fRight - dx;
    tmp.fRight = outer.fRight;
    SkScan::FillRect(tmp, clip, blitter);
}

void SkScan::HairLine(const SkPoint& p0, const SkPoint& p1,
                      const SkRasterClip& clip, SkBlitter* blitter) {
    if (clip.isBW()) {
        HairLineRgn(p0, p1, &clip.bwRgn(), blitter);
    } else {
        const SkRegion* clipRgn = NULL;
        SkRect r;
        SkIRect ir;
        r.set(p0.fX, p0.fY, p1.fX, p1.fY);
        r.sort();
        r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
        r.roundOut(&ir);

        SkAAClipBlitterWrapper wrap;
        if (!clip.quickContains(ir)) {
            wrap.init(clip, blitter);
            blitter = wrap.getBlitter();
            clipRgn = &wrap.getRgn();
        }
        HairLineRgn(p0, p1, clipRgn, blitter);
    }
}

void SkScan::AntiHairLine(const SkPoint& p0, const SkPoint& p1,
                          const SkRasterClip& clip, SkBlitter* blitter) {
    if (clip.isBW()) {
        AntiHairLineRgn(p0, p1, &clip.bwRgn(), blitter);
    } else {
        const SkRegion* clipRgn = NULL;
        SkRect r;
        SkIRect ir;
        r.set(p0.fX, p0.fY, p1.fX, p1.fY);
        r.sort();
        r.roundOut(&ir);
        ir.inset(-1, -1);

        SkAAClipBlitterWrapper wrap;
        if (!clip.quickContains(ir)) {
            wrap.init(clip, blitter);
            blitter = wrap.getBlitter();
            clipRgn = &wrap.getRgn();
        }
        AntiHairLineRgn(p0, p1, clipRgn, blitter);
    }
}
