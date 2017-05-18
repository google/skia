/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScan.h"
#include "SkBlitter.h"
#include "SkMathPriv.h"
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

void SkScan::HairLineRgn(const SkPoint array[], int arrayCount, const SkRegion* clip,
                         SkBlitter* origBlitter) {
    SkBlitterClipper    clipper;
    SkIRect clipR, ptsR;

    const SkScalar max = SkIntToScalar(32767);
    const SkRect fixedBounds = SkRect::MakeLTRB(-max, -max, max, max);

    SkRect clipBounds;
    if (clip) {
        clipBounds.set(clip->getBounds());
    }

    for (int i = 0; i < arrayCount - 1; ++i) {
        SkBlitter* blitter = origBlitter;

        SkPoint pts[2];

        // We have to pre-clip the line to fit in a SkFixed, so we just chop
        // the line. TODO find a way to actually draw beyond that range.
        if (!SkLineClipper::IntersectLine(&array[i], fixedBounds, pts)) {
            continue;
        }

        // Perform a clip in scalar space, so we catch huge values which might
        // be missed after we convert to SkFDot6 (overflow)
        if (clip && !SkLineClipper::IntersectLine(pts, clipBounds, pts)) {
            continue;
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
                continue;
            }
            if (!clip->isRect() || !clipR.contains(ptsR)) {
                blitter = clipper.apply(origBlitter, clip);
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
                continue;
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
                continue;
            }

            SkFixed slope = SkFixedDiv(dx, dy);
            SkFixed startX = SkFDot6ToFixed(x0) + (slope * ((32 - y0) & 63) >> 6);

            vertline(iy0, iy1, startX, slope, blitter);
        }
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
#include "SkNx.h"

#define kMaxCubicSubdivideLevel 9
#define kMaxQuadSubdivideLevel  5

static int compute_int_quad_dist(const SkPoint pts[3]) {
    // compute the vector between the control point ([1]) and the middle of the
    // line connecting the start and end ([0] and [2])
    SkScalar dx = SkScalarHalf(pts[0].fX + pts[2].fX) - pts[1].fX;
    SkScalar dy = SkScalarHalf(pts[0].fY + pts[2].fY) - pts[1].fY;
    // we want everyone to be positive
    dx = SkScalarAbs(dx);
    dy = SkScalarAbs(dy);
    // convert to whole pixel values (use ceiling to be conservative)
    int idx = SkScalarCeilToInt(dx);
    int idy = SkScalarCeilToInt(dy);
    // use the cheap approx for distance
    if (idx > idy) {
        return idx + (idy >> 1);
    } else {
        return idy + (idx >> 1);
    }
}

static void hair_quad(const SkPoint pts[3], const SkRegion* clip,
                     SkBlitter* blitter, int level, SkScan::HairRgnProc lineproc) {
    SkASSERT(level <= kMaxQuadSubdivideLevel);

    SkQuadCoeff coeff(pts);

    const int lines = 1 << level;
    Sk2s t(0);
    Sk2s dt(SK_Scalar1 / lines);

    SkPoint tmp[(1 << kMaxQuadSubdivideLevel) + 1];
    SkASSERT((unsigned)lines < SK_ARRAY_COUNT(tmp));

    tmp[0] = pts[0];
    Sk2s A = coeff.fA;
    Sk2s B = coeff.fB;
    Sk2s C = coeff.fC;
    for (int i = 1; i < lines; ++i) {
        t = t + dt;
        ((A * t + B) * t + C).store(&tmp[i]);
    }
    tmp[lines] = pts[2];
    lineproc(tmp, lines + 1, clip, blitter);
}

static SkRect compute_nocheck_quad_bounds(const SkPoint pts[3]) {
    SkASSERT(SkScalarsAreFinite(&pts[0].fX, 6));

    Sk2s min = Sk2s::Load(pts);
    Sk2s max = min;
    for (int i = 1; i < 3; ++i) {
        Sk2s pair = Sk2s::Load(pts+i);
        min = Sk2s::Min(min, pair);
        max = Sk2s::Max(max, pair);
    }
    return { min[0], min[1], max[0], max[1] };
}

static bool is_inverted(const SkRect& r) {
    return r.fLeft > r.fRight || r.fTop > r.fBottom;
}

// Can't call SkRect::intersects, since it cares about empty, and we don't (since we tracking
// something to be stroked, so empty can still draw something (e.g. horizontal line)
static bool geometric_overlap(const SkRect& a, const SkRect& b) {
    SkASSERT(!is_inverted(a) && !is_inverted(b));
    return a.fLeft < b.fRight && b.fLeft < a.fRight &&
            a.fTop < b.fBottom && b.fTop < a.fBottom;
}

// Can't call SkRect::contains, since it cares about empty, and we don't (since we tracking
// something to be stroked, so empty can still draw something (e.g. horizontal line)
static bool geometric_contains(const SkRect& outer, const SkRect& inner) {
    SkASSERT(!is_inverted(outer) && !is_inverted(inner));
    return inner.fRight <= outer.fRight && inner.fLeft >= outer.fLeft &&
            inner.fBottom <= outer.fBottom && inner.fTop >= outer.fTop;
}

static inline void hairquad(const SkPoint pts[3], const SkRegion* clip, const SkRect* insetClip, const SkRect* outsetClip,
    SkBlitter* blitter, int level, SkScan::HairRgnProc lineproc) {
    if (insetClip) {
        SkASSERT(outsetClip);
        SkRect bounds = compute_nocheck_quad_bounds(pts);
        if (!geometric_overlap(*outsetClip, bounds)) {
            return;
        } else if (geometric_contains(*insetClip, bounds)) {
            clip = nullptr;
        }
    }

    hair_quad(pts, clip, blitter, level, lineproc);
}

static inline Sk2s abs(const Sk2s& value) {
    return Sk2s::Max(value, Sk2s(0)-value);
}

static inline SkScalar max_component(const Sk2s& value) {
    SkScalar components[2];
    value.store(components);
    return SkTMax(components[0], components[1]);
}

static inline int compute_cubic_segs(const SkPoint pts[4]) {
    Sk2s p0 = from_point(pts[0]);
    Sk2s p1 = from_point(pts[1]);
    Sk2s p2 = from_point(pts[2]);
    Sk2s p3 = from_point(pts[3]);

    const Sk2s oneThird(1.0f / 3.0f);
    const Sk2s twoThird(2.0f / 3.0f);

    Sk2s p13 = oneThird * p3 + twoThird * p0;
    Sk2s p23 = oneThird * p0 + twoThird * p3;

    SkScalar diff = max_component(Sk2s::Max(abs(p1 - p13), abs(p2 - p23)));
    SkScalar tol = SK_Scalar1 / 8;

    for (int i = 0; i < kMaxCubicSubdivideLevel; ++i) {
        if (diff < tol) {
            return 1 << i;
        }
        tol *= 4;
    }
    return 1 << kMaxCubicSubdivideLevel;
}

static bool lt_90(SkPoint p0, SkPoint pivot, SkPoint p2) {
    return SkVector::DotProduct(p0 - pivot, p2 - pivot) >= 0;
}

// The off-curve points are "inside" the limits of the on-curve pts
static bool quick_cubic_niceness_check(const SkPoint pts[4]) {
    return lt_90(pts[1], pts[0], pts[3]) &&
           lt_90(pts[2], pts[0], pts[3]) &&
           lt_90(pts[1], pts[3], pts[0]) &&
           lt_90(pts[2], pts[3], pts[0]);
}

static void hair_cubic(const SkPoint pts[4], const SkRegion* clip, SkBlitter* blitter,
                       SkScan::HairRgnProc lineproc) {
    const int lines = compute_cubic_segs(pts);
    SkASSERT(lines > 0);
    if (1 == lines) {
        SkPoint tmp[2] = { pts[0], pts[3] };
        lineproc(tmp, 2, clip, blitter);
        return;
    }

    SkCubicCoeff coeff(pts);

    const Sk2s dt(SK_Scalar1 / lines);
    Sk2s t(0);

    SkPoint tmp[(1 << kMaxCubicSubdivideLevel) + 1];
    SkASSERT((unsigned)lines < SK_ARRAY_COUNT(tmp));

    tmp[0] = pts[0];
    Sk2s A = coeff.fA;
    Sk2s B = coeff.fB;
    Sk2s C = coeff.fC;
    Sk2s D = coeff.fD;
    for (int i = 1; i < lines; ++i) {
        t = t + dt;
        (((A * t + B) * t + C) * t + D).store(&tmp[i]);
    }
    tmp[lines] = pts[3];
    lineproc(tmp, lines + 1, clip, blitter);
}

static SkRect compute_nocheck_cubic_bounds(const SkPoint pts[4]) {
    SkASSERT(SkScalarsAreFinite(&pts[0].fX, 8));

    Sk2s min = Sk2s::Load(pts);
    Sk2s max = min;
    for (int i = 1; i < 4; ++i) {
        Sk2s pair = Sk2s::Load(pts+i);
        min = Sk2s::Min(min, pair);
        max = Sk2s::Max(max, pair);
    }
    return { min[0], min[1], max[0], max[1] };
}

static inline void haircubic(const SkPoint pts[4], const SkRegion* clip, const SkRect* insetClip, const SkRect* outsetClip,
                      SkBlitter* blitter, int level, SkScan::HairRgnProc lineproc) {
    if (insetClip) {
        SkASSERT(outsetClip);
        SkRect bounds = compute_nocheck_cubic_bounds(pts);
        if (!geometric_overlap(*outsetClip, bounds)) {
            return;
        } else if (geometric_contains(*insetClip, bounds)) {
            clip = nullptr;
        }
    }

    if (quick_cubic_niceness_check(pts)) {
        hair_cubic(pts, clip, blitter, lineproc);
    } else {
        SkPoint  tmp[13];
        SkScalar tValues[3];

        int count = SkChopCubicAtMaxCurvature(pts, tmp, tValues);
        for (int i = 0; i < count; i++) {
            hair_cubic(&tmp[i * 3], clip, blitter, lineproc);
        }
    }
}

static int compute_quad_level(const SkPoint pts[3]) {
    int d = compute_int_quad_dist(pts);
    /*  quadratics approach the line connecting their start and end points
     4x closer with each subdivision, so we compute the number of
     subdivisions to be the minimum need to get that distance to be less
     than a pixel.
     */
    int level = (33 - SkCLZ(d)) >> 1;
    // sanity check on level (from the previous version)
    if (level > kMaxQuadSubdivideLevel) {
        level = kMaxQuadSubdivideLevel;
    }
    return level;
}

/* Extend the points in the direction of the starting or ending tangent by 1/2 unit to
   account for a round or square cap. If there's no distance between the end point and
   the control point, use the next control point to create a tangent. If the curve
   is degenerate, move the cap out 1/2 unit horizontally. */
template <SkPaint::Cap capStyle>
void extend_pts(SkPath::Verb prevVerb, SkPath::Verb nextVerb, SkPoint* pts, int ptCount) {
    SkASSERT(SkPaint::kSquare_Cap == capStyle || SkPaint::kRound_Cap == capStyle);
    // The area of a circle is PI*R*R. For a unit circle, R=1/2, and the cap covers half of that.
    const SkScalar capOutset = SkPaint::kSquare_Cap == capStyle ? 0.5f : SK_ScalarPI / 8;
    if (SkPath::kMove_Verb == prevVerb) {
        SkPoint* first = pts;
        SkPoint* ctrl = first;
        int controls = ptCount - 1;
        SkVector tangent;
        do {
            tangent = *first - *++ctrl;
        } while (tangent.isZero() && --controls > 0);
        if (tangent.isZero()) {
            tangent.set(1, 0);
            controls = ptCount - 1;  // If all points are equal, move all but one
        } else {
            tangent.normalize();
        }
        do {    // If the end point and control points are equal, loop to move them in tandem.
            first->fX += tangent.fX * capOutset;
            first->fY += tangent.fY * capOutset;
            ++first;
        } while (++controls < ptCount);
    }
    if (SkPath::kMove_Verb == nextVerb || SkPath::kDone_Verb == nextVerb) {
        SkPoint* last = &pts[ptCount - 1];
        SkPoint* ctrl = last;
        int controls = ptCount - 1;
        SkVector tangent;
        do {
            tangent = *last - *--ctrl;
        } while (tangent.isZero() && --controls > 0);
        if (tangent.isZero()) {
            tangent.set(-1, 0);
            controls = ptCount - 1;
        } else {
            tangent.normalize();
        }
        do {
            last->fX += tangent.fX * capOutset;
            last->fY += tangent.fY * capOutset;
            --last;
        } while (++controls < ptCount);
    }
}

template <SkPaint::Cap capStyle>
void hair_path(const SkPath& path, const SkRasterClip& rclip, SkBlitter* blitter,
                      SkScan::HairRgnProc lineproc) {
    if (path.isEmpty()) {
        return;
    }

    SkAAClipBlitterWrapper wrap;
    const SkRegion* clip = nullptr;
    SkRect insetStorage, outsetStorage;
    const SkRect* insetClip = nullptr;
    const SkRect* outsetClip = nullptr;

    {
        const int capOut = SkPaint::kButt_Cap == capStyle ? 1 : 2;
        const SkIRect ibounds = path.getBounds().roundOut().makeOutset(capOut, capOut);
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

            /*
             *  We now cache two scalar rects, to use for culling per-segment (e.g. cubic).
             *  Since we're hairlining, the "bounds" of the control points isn't necessairly the
             *  limit of where a segment can draw (it might draw up to 1 pixel beyond in aa-hairs).
             *
             *  Compute the pt-bounds per segment is easy, so we do that, and then inversely adjust
             *  the culling bounds so we can just do a straight compare per segment.
             *
             *  insetClip is use for quick-accept (i.e. the segment is not clipped), so we inset
             *  it from the clip-bounds (since segment bounds can be off by 1).
             *
             *  outsetClip is used for quick-reject (i.e. the segment is entirely outside), so we
             *  outset it from the clip-bounds.
             */
            insetStorage.set(clip->getBounds());
            outsetStorage = insetStorage.makeOutset(1, 1);
            insetStorage.inset(1, 1);
            if (is_inverted(insetStorage)) {
                /*
                 *  our bounds checks assume the rects are never inverted. If insetting has
                 *  created that, we assume that the area is too small to safely perform a
                 *  quick-accept, so we just mark the rect as empty (so the quick-accept check
                 *  will always fail.
                 */
                insetStorage.setEmpty();    // just so we don't pass an inverted rect
            }
            if (rclip.isRect()) {
                insetClip = &insetStorage;
            }
            outsetClip = &outsetStorage;
        }
    }

    SkPath::RawIter     iter(path);
    SkPoint             pts[4], firstPt, lastPt;
    SkPath::Verb        verb, prevVerb;
    SkAutoConicToQuads  converter;

    if (SkPaint::kButt_Cap != capStyle) {
        prevVerb = SkPath::kDone_Verb;
    }
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                firstPt = lastPt = pts[0];
                break;
            case SkPath::kLine_Verb:
                if (SkPaint::kButt_Cap != capStyle) {
                    extend_pts<capStyle>(prevVerb, iter.peek(), pts, 2);
                }
                lineproc(pts, 2, clip, blitter);
                lastPt = pts[1];
                break;
            case SkPath::kQuad_Verb:
                if (SkPaint::kButt_Cap != capStyle) {
                    extend_pts<capStyle>(prevVerb, iter.peek(), pts, 3);
                }
                hairquad(pts, clip, insetClip, outsetClip, blitter, compute_quad_level(pts), lineproc);
                lastPt = pts[2];
                break;
            case SkPath::kConic_Verb: {
                if (SkPaint::kButt_Cap != capStyle) {
                    extend_pts<capStyle>(prevVerb, iter.peek(), pts, 3);
                }
                // how close should the quads be to the original conic?
                const SkScalar tol = SK_Scalar1 / 4;
                const SkPoint* quadPts = converter.computeQuads(pts,
                                                       iter.conicWeight(), tol);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    int level = compute_quad_level(quadPts);
                    hairquad(quadPts, clip, insetClip, outsetClip, blitter, level, lineproc);
                    quadPts += 2;
                }
                lastPt = pts[2];
                break;
            }
            case SkPath::kCubic_Verb: {
                if (SkPaint::kButt_Cap != capStyle) {
                    extend_pts<capStyle>(prevVerb, iter.peek(), pts, 4);
                }
                haircubic(pts, clip, insetClip, outsetClip, blitter, kMaxCubicSubdivideLevel, lineproc);
                lastPt = pts[3];
            } break;
            case SkPath::kClose_Verb:
                pts[0] = lastPt;
                pts[1] = firstPt;
                if (SkPaint::kButt_Cap != capStyle && prevVerb == SkPath::kMove_Verb) {
                    // cap moveTo/close to match svg expectations for degenerate segments
                    extend_pts<capStyle>(prevVerb, iter.peek(), pts, 2);
                }
                lineproc(pts, 2, clip, blitter);
                break;
            case SkPath::kDone_Verb:
                break;
        }
        if (SkPaint::kButt_Cap != capStyle) {
            if (prevVerb == SkPath::kMove_Verb &&
                    verb >= SkPath::kLine_Verb && verb <= SkPath::kCubic_Verb) {
                firstPt = pts[0];  // the curve moved the initial point, so close to it instead
            }
            prevVerb = verb;
        }
    }
}

void SkScan::HairPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    hair_path<SkPaint::kButt_Cap>(path, clip, blitter, SkScan::HairLineRgn);
}

void SkScan::AntiHairPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    hair_path<SkPaint::kButt_Cap>(path, clip, blitter, SkScan::AntiHairLineRgn);
}

void SkScan::HairSquarePath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    hair_path<SkPaint::kSquare_Cap>(path, clip, blitter, SkScan::HairLineRgn);
}

void SkScan::AntiHairSquarePath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    hair_path<SkPaint::kSquare_Cap>(path, clip, blitter, SkScan::AntiHairLineRgn);
}

void SkScan::HairRoundPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    hair_path<SkPaint::kRound_Cap>(path, clip, blitter, SkScan::HairLineRgn);
}

void SkScan::AntiHairRoundPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    hair_path<SkPaint::kRound_Cap>(path, clip, blitter, SkScan::AntiHairLineRgn);
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

    if (r.width() <= dx || r.height() <= dy) {
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

void SkScan::HairLine(const SkPoint pts[], int count, const SkRasterClip& clip,
                      SkBlitter* blitter) {
    if (clip.isBW()) {
        HairLineRgn(pts, count, &clip.bwRgn(), blitter);
    } else {
        const SkRegion* clipRgn = nullptr;

        SkRect r;
        r.set(pts, count);
        r.outset(SK_ScalarHalf, SK_ScalarHalf);

        SkAAClipBlitterWrapper wrap;
        if (!clip.quickContains(r.roundOut())) {
            wrap.init(clip, blitter);
            blitter = wrap.getBlitter();
            clipRgn = &wrap.getRgn();
        }
        HairLineRgn(pts, count, clipRgn, blitter);
    }
}

void SkScan::AntiHairLine(const SkPoint pts[], int count, const SkRasterClip& clip,
                          SkBlitter* blitter) {
    if (clip.isBW()) {
        AntiHairLineRgn(pts, count, &clip.bwRgn(), blitter);
    } else {
        const SkRegion* clipRgn = nullptr;

        SkRect r;
        r.set(pts, count);

        SkAAClipBlitterWrapper wrap;
        if (!clip.quickContains(r.roundOut().makeOutset(1, 1))) {
            wrap.init(clip, blitter);
            blitter = wrap.getBlitter();
            clipRgn = &wrap.getRgn();
        }
        AntiHairLineRgn(pts, count, clipRgn, blitter);
    }
}
