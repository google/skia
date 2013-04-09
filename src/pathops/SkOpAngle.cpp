/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkOpAngle.h"
#include "SkPathOpsCurve.h"

// FIXME: this is bogus for quads and cubics
// if the quads and cubics' line from end pt to ctrl pt are coincident,
// there's no obvious way to determine the curve ordering from the
// derivatives alone. In particular, if one quadratic's coincident tangent
// is longer than the other curve, the final control point can place the
// longer curve on either side of the shorter one.
// Using Bezier curve focus http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf
// may provide some help, but nothing has been figured out yet.

/*(
for quads and cubics, set up a parameterized line (e.g. LineParameters )
for points [0] to [1]. See if point [2] is on that line, or on one side
or the other. If it both quads' end points are on the same side, choose
the shorter tangent. If the tangents are equal, choose the better second
tangent angle

maybe I could set up LineParameters lazily
*/
bool SkOpAngle::operator<(const SkOpAngle& rh) const {
    double y = dy();
    double ry = rh.dy();
    if ((y < 0) ^ (ry < 0)) {  // OPTIMIZATION: better to use y * ry < 0 ?
        return y < 0;
    }
    double x = dx();
    double rx = rh.dx();
    if (y == 0 && ry == 0 && x * rx < 0) {
        return x < rx;
    }
    double x_ry = x * ry;
    double rx_y = rx * y;
    double cmp = x_ry - rx_y;
    if (!approximately_zero(cmp)) {
        return cmp < 0;
    }
    if (approximately_zero(x_ry) && approximately_zero(rx_y)
            && !approximately_zero_squared(cmp)) {
        return cmp < 0;
    }
    // at this point, the initial tangent line is coincident
    // see if edges curl away from each other
    if (fSide * rh.fSide <= 0 && (!approximately_zero(fSide)
            || !approximately_zero(rh.fSide))) {
        // FIXME: running demo will trigger this assertion
        // (don't know if commenting out will trigger further assertion or not)
        // commenting it out allows demo to run in release, though
        return fSide < rh.fSide;
    }
    // see if either curve can be lengthened and try the tangent compare again
    if (cmp && (*fSpans)[fEnd].fOther != rh.fSegment  // tangents not absolutely identical
            && (*rh.fSpans)[rh.fEnd].fOther != fSegment) {  // and not intersecting
        SkOpAngle longer = *this;
        SkOpAngle rhLonger = rh;
        if (longer.lengthen() | rhLonger.lengthen()) {
            return longer < rhLonger;
        }
    }
    if ((fVerb == SkPath::kLine_Verb && approximately_zero(x) && approximately_zero(y))
            || (rh.fVerb == SkPath::kLine_Verb
            && approximately_zero(rx) && approximately_zero(ry))) {
        // See general unsortable comment below. This case can happen when
        // one line has a non-zero change in t but no change in x and y.
        fUnsortable = true;
        rh.fUnsortable = true;
        return this < &rh;  // even with no solution, return a stable sort
    }
    if ((*rh.fSpans)[SkMin32(rh.fStart, rh.fEnd)].fTiny
            || (*fSpans)[SkMin32(fStart, fEnd)].fTiny) {
        fUnsortable = true;
        rh.fUnsortable = true;
        return this < &rh;  // even with no solution, return a stable sort
    }
    SkASSERT(fVerb >= SkPath::kQuad_Verb);
    SkASSERT(rh.fVerb >= SkPath::kQuad_Verb);
    // FIXME: until I can think of something better, project a ray from the
    // end of the shorter tangent to midway between the end points
    // through both curves and use the resulting angle to sort
    // FIXME: some of this setup can be moved to set() if it works, or cached if it's expensive
    double len = fTangent1.normalSquared();
    double rlen = rh.fTangent1.normalSquared();
    SkDLine ray;
    SkIntersections i, ri;
    int roots, rroots;
    bool flip = false;
    do {
        bool useThis = (len < rlen) ^ flip;
        const SkDCubic& part = useThis ? fCurvePart : rh.fCurvePart;
        SkPath::Verb partVerb = useThis ? fVerb : rh.fVerb;
        ray[0] = partVerb == SkPath::kCubic_Verb && part[0].approximatelyEqual(part[1]) ?
            part[2] : part[1];
        ray[1].fX = (part[0].fX + part[partVerb].fX) / 2;
        ray[1].fY = (part[0].fY + part[partVerb].fY) / 2;
        SkASSERT(ray[0] != ray[1]);
        roots = (i.*CurveRay[fVerb])(fPts, ray);
        rroots = (ri.*CurveRay[rh.fVerb])(rh.fPts, ray);
    } while ((roots == 0 || rroots == 0) && (flip ^= true));
    if (roots == 0 || rroots == 0) {
        // FIXME: we don't have a solution in this case. The interim solution
        // is to mark the edges as unsortable, exclude them from this and
        // future computations, and allow the returned path to be fragmented
        fUnsortable = true;
        rh.fUnsortable = true;
        return this < &rh;  // even with no solution, return a stable sort
    }
    SkDPoint loc;
    double best = SK_ScalarInfinity;
    SkDVector dxy;
    double dist;
    int index;
    for (index = 0; index < roots; ++index) {
        loc = (*CurveDPointAtT[fVerb])(fPts, i[0][index]);
        dxy = loc - ray[0];
        dist = dxy.lengthSquared();
        if (best > dist) {
            best = dist;
        }
    }
    for (index = 0; index < rroots; ++index) {
        loc = (*CurveDPointAtT[rh.fVerb])(rh.fPts, ri[0][index]);
        dxy = loc - ray[0];
        dist = dxy.lengthSquared();
        if (best > dist) {
            return fSide < 0;
        }
    }
    return fSide > 0;
}

bool SkOpAngle::lengthen() {
    int newEnd = fEnd;
    if (fStart < fEnd ? ++newEnd < fSpans->count() : --newEnd >= 0) {
        fEnd = newEnd;
        setSpans();
        return true;
    }
    return false;
}

bool SkOpAngle::reverseLengthen() {
    if (fReversed) {
        return false;
    }
    int newEnd = fStart;
    if (fStart > fEnd ? ++newEnd < fSpans->count() : --newEnd >= 0) {
        fEnd = newEnd;
        fReversed = true;
        setSpans();
        return true;
    }
    return false;
}

void SkOpAngle::set(const SkPoint* orig, SkPath::Verb verb, const SkOpSegment* segment,
        int start, int end, const SkTDArray<SkOpSpan>& spans) {
    fSegment = segment;
    fStart = start;
    fEnd = end;
    fPts = orig;
    fVerb = verb;
    fSpans = &spans;
    fReversed = false;
    fUnsortable = false;
    setSpans();
}


void SkOpAngle::setSpans() {
    double startT = (*fSpans)[fStart].fT;
    double endT = (*fSpans)[fEnd].fT;
    switch (fVerb) {
    case SkPath::kLine_Verb: {
        SkDLine l = SkDLine::SubDivide(fPts, startT, endT);
        // OPTIMIZATION: for pure line compares, we never need fTangent1.c
        fTangent1.lineEndPoints(l);
        fSide = 0;
        } break;
    case SkPath::kQuad_Verb: {
        SkDQuad& quad = *SkTCast<SkDQuad*>(&fCurvePart);
        quad = SkDQuad::SubDivide(fPts, startT, endT);
        fTangent1.quadEndPoints(quad, 0, 1);
        if (dx() == 0 && dy() == 0) {
            fTangent1.quadEndPoints(quad);
        }
        fSide = -fTangent1.pointDistance(fCurvePart[2]);  // not normalized -- compare sign only
        } break;
    case SkPath::kCubic_Verb: {
        int nextC = 2;
        fCurvePart = SkDCubic::SubDivide(fPts, startT, endT);
        fTangent1.cubicEndPoints(fCurvePart, 0, 1);
        if (dx() == 0 && dy() == 0) {
            fTangent1.cubicEndPoints(fCurvePart, 0, 2);
            nextC = 3;
            if (dx() == 0 && dy() == 0) {
                fTangent1.cubicEndPoints(fCurvePart, 0, 3);
            }
        }
        fSide = -fTangent1.pointDistance(fCurvePart[nextC]);  // compare sign only
        if (nextC == 2 && approximately_zero(fSide)) {
            fSide = -fTangent1.pointDistance(fCurvePart[3]);
        }
        } break;
    default:
        SkASSERT(0);
    }
    fUnsortable = dx() == 0 && dy() == 0;
    if (fUnsortable) {
        return;
    }
    SkASSERT(fStart != fEnd);
    int step = fStart < fEnd ? 1 : -1;  // OPTIMIZE: worth fStart - fEnd >> 31 type macro?
    for (int index = fStart; index != fEnd; index += step) {
#if 1
        const SkOpSpan& thisSpan = (*fSpans)[index];
        const SkOpSpan& nextSpan = (*fSpans)[index + step];
        if (thisSpan.fTiny || precisely_equal(thisSpan.fT, nextSpan.fT)) {
            continue;
        }
        fUnsortable = step > 0 ? thisSpan.fUnsortableStart : nextSpan.fUnsortableEnd;
#if DEBUG_UNSORTABLE
        if (fUnsortable) {
            SkPoint iPt = (*CurvePointAtT[fVerb])(fPts, thisSpan.fT);
            SkPoint ePt = (*CurvePointAtT[fVerb])(fPts, nextSpan.fT);
            SkDebugf("%s unsortable [%d] (%1.9g,%1.9g) [%d] (%1.9g,%1.9g)\n", __FUNCTION__,
                    index, iPt.fX, iPt.fY, fEnd, ePt.fX, ePt.fY);
        }
#endif
        return;
#else
        if ((*fSpans)[index].fUnsortableStart) {
            fUnsortable = true;
            return;
        }
#endif
    }
#if 1
#if DEBUG_UNSORTABLE
    SkPoint iPt = (*CurvePointAtT[fVerb])(fPts, startT);
    SkPoint ePt = (*CurvePointAtT[fVerb])(fPts, endT);
    SkDebugf("%s all tiny unsortable [%d] (%1.9g,%1.9g) [%d] (%1.9g,%1.9g)\n", __FUNCTION__,
        fStart, iPt.fX, iPt.fY, fEnd, ePt.fX, ePt.fY);
#endif
    fUnsortable = true;
#endif
}
