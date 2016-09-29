/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAConvexTessellator.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkString.h"
#include "GrPathUtils.h"

// Next steps:
//  add an interactive sample app slide
//  add debug check that all points are suitably far apart
//  test more degenerate cases

// The tolerance for fusing vertices and eliminating colinear lines (It is in device space).
static const SkScalar kClose = (SK_Scalar1 / 16);
static const SkScalar kCloseSqd = SkScalarMul(kClose, kClose);

// tesselation tolerance values, in device space pixels
static const SkScalar kQuadTolerance = 0.2f;
static const SkScalar kCubicTolerance = 0.2f;
static const SkScalar kConicTolerance = 0.5f;

// dot product below which we use a round cap between curve segments
static const SkScalar kRoundCapThreshold = 0.8f;

// dot product above which we consider two adjacent curves to be part of the "same" curve
static const SkScalar kCurveConnectionThreshold = 0.8f;

static bool intersect(const SkPoint& p0, const SkPoint& n0,
                      const SkPoint& p1, const SkPoint& n1,
                      SkScalar* t) {
    const SkPoint v = p1 - p0;
    SkScalar perpDot = n0.fX * n1.fY - n0.fY * n1.fX;
    if (SkScalarNearlyZero(perpDot)) {
        return false;
    }
    *t = (v.fX * n1.fY - v.fY * n1.fX) / perpDot;
    SkASSERT(SkScalarIsFinite(*t));
    return true;
}

// This is a special case version of intersect where we have the vector
// perpendicular to the second line rather than the vector parallel to it.
static SkScalar perp_intersect(const SkPoint& p0, const SkPoint& n0,
                               const SkPoint& p1, const SkPoint& perp) {
    const SkPoint v = p1 - p0;
    SkScalar perpDot = n0.dot(perp);
    return v.dot(perp) / perpDot;
}

static bool duplicate_pt(const SkPoint& p0, const SkPoint& p1) {
    SkScalar distSq = p0.distanceToSqd(p1);
    return distSq < kCloseSqd;
}

static bool is_duplicate_outset_pt(const SkPoint& p0, const SkPoint& p1) {
    SkScalar distSq = p0.distanceToSqd(p1);
    return SkScalarNearlyZero(distSq, SK_ScalarNearlyZero*SK_ScalarNearlyZero);

}

static SkScalar abs_dist_from_line(const SkPoint& p0, const SkVector& v, const SkPoint& test) {
    SkPoint testV = test - p0;
    SkScalar dist = testV.fX * v.fY - testV.fY * v.fX;
    return SkScalarAbs(dist);
}

int GrAAConvexTessellator::addPt(const SkPoint& pt,
                                 SkScalar depth,
                                 SkScalar coverage,
                                 bool movable,
                                 CurveState curve) {
    this->validate();

    int index = fPts.count();
    *fPts.push() = pt;
    *fCoverages.push() = coverage;
    *fMovable.push() = movable;
    *fCurveState.push() = curve;

    this->validate();
    return index;
}

void GrAAConvexTessellator::popLastPt() {
    this->validate();

    fPts.pop();
    fCoverages.pop();
    fMovable.pop();
    fCurveState.pop();

    this->validate();
}

void GrAAConvexTessellator::popFirstPtShuffle() {
    this->validate();

    fPts.removeShuffle(0);
    fCoverages.removeShuffle(0);
    fMovable.removeShuffle(0);
    fCurveState.removeShuffle(0);

    this->validate();
}

void GrAAConvexTessellator::updatePt(int index,
                                     const SkPoint& pt,
                                     SkScalar depth,
                                     SkScalar coverage) {
    this->validate();
    SkASSERT(fMovable[index]);

    fPts[index] = pt;
    fCoverages[index] = coverage;
}

void GrAAConvexTessellator::addTri(int i0, int i1, int i2) {
    if (i0 == i1 || i1 == i2 || i2 == i0) {
        return;
    }

    *fIndices.push() = i0;
    *fIndices.push() = i1;
    *fIndices.push() = i2;
}

void GrAAConvexTessellator::rewind() {
    fPts.rewind();
    fCoverages.rewind();
    fMovable.rewind();
    fIndices.rewind();
    fNorms1.rewind();
    fCurveState.rewind();
    fInitialInsetRing.rewind();
    fInitialOutsetRing.rewind();
    fCandidateVerts.rewind();
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    fInsetRings.rewind();        // TODO: leak in this case!
    fOutsetRings.rewind();
#else
    fInsetRings[0].rewind();
    fInsetRings[1].rewind();
#endif
}

void GrAAConvexTessellator::computeBisectors() {
    fBisectors.setCount(fNorms1.count());

    int prev = fBisectors.count() - 1;
    for (int cur = 0; cur < fBisectors.count(); prev = cur, ++cur) {
        fBisectors[cur] = fNorms1[cur] + fNorms1[prev];
        if (!fBisectors[cur].normalize()) {
            SkASSERT(SkPoint::kLeft_Side == fSide || SkPoint::kRight_Side == fSide);
            fBisectors[cur].setOrthog(fNorms1[cur], (SkPoint::Side)-fSide);
            SkVector other;
            other.setOrthog(fNorms1[prev], fSide);
            fBisectors[cur] += other;
            SkAssertResult(fBisectors[cur].normalize());
        } else {
            fBisectors[cur].negate();      // make the bisector face in
        }
        if (fCurveState[prev] == kIndeterminate_CurveState) {
            if (fCurveState[cur] == kSharp_CurveState) {
                fCurveState[prev] = kSharp_CurveState;
            } else {
                if (SkScalarAbs(fNorms1[cur].dot(fNorms1[prev])) > kCurveConnectionThreshold) {
                    fCurveState[prev] = kCurve_CurveState;
                    fCurveState[cur]  = kCurve_CurveState;
                } else {
                    fCurveState[prev] = kSharp_CurveState;
                    fCurveState[cur]  = kSharp_CurveState;
                }
            }
        }

        SkASSERT(SkScalarNearlyEqual(1.0f, fBisectors[cur].length()));
    }
}

// Create as many rings as we need to (up to a predefined limit) to reach the specified target
// depth. If we are in fill mode, the final ring will automatically be fanned.
bool GrAAConvexTessellator::createInsetRings(InsetRing& previousRing, SkScalar initialDepth,
                                             SkScalar initialCoverage, SkScalar targetDepth,
                                             SkScalar targetCoverage, InsetRing** finalRing) {
    static const int kMaxNumRings = 8;

    if (previousRing.numPts() < 3) {
        return false;
    }
    InsetRing* currentRing = &previousRing;
    int i;
    for (i = 0; i < kMaxNumRings; ++i) {
        InsetRing* nextRing = this->getNextInsetRing(currentRing);
        SkASSERT(nextRing != currentRing);

        bool done = this->createInsetRing(*currentRing, nextRing, initialDepth, initialCoverage,
                                          targetDepth, targetCoverage, i == 0);
        currentRing = nextRing;
        if (done) {
            break;
        }
        currentRing->init(*this);
    }

    if (kMaxNumRings == i) {
        // Bail if we've exceeded the amount of time we want to throw at this.
        this->terminate(*currentRing);
        return false;
    }
    bool done = currentRing->numPts() >= 3;
    if (done) {
        currentRing->init(*this);
    }
    *finalRing = currentRing;
    return done;
}

// The general idea here is to, conceptually, start with the original polygon and slide
// the vertices along the bisectors until the first intersection. At that
// point two of the edges collapse and the process repeats on the new polygon.
// The polygon state is captured in the Ring class while the GrAAConvexTessellator
// controls the iteration. The CandidateVerts holds the formative points for the
// next ring.
bool GrAAConvexTessellator::tessellate(const SkMatrix& m, const SkPath& path) {
    if (!this->extractFromPath(m, path)) {
        return false;
    }

    static const SkScalar kFullCoverage = 1.0f;
    static const SkScalar kHalfCoverage = 0.5f;
    static const SkScalar kZeroCoverage = 0.0f;
    SkScalar scaleFactor = 0.0f;

    if (SkStrokeRec::kStrokeAndFill_Style == fStyle) {
        SkASSERT(m.isSimilarity());
        scaleFactor = m.getMaxScale(); // x and y scale are the same
        SkScalar effectiveStrokeWidth = scaleFactor * fStrokeWidth;
        OutsetRing* outerStrokeRing = this->getNextOutsetRing(true);
        fInitialOutsetRing.createOuterRing(*this,
                                           effectiveStrokeWidth / 2 - kAntialiasingRadius,
                                           kFullCoverage,
                                           outerStrokeRing);

        this->fanRing(fInitialInsetRing);

#if 1
        outerStrokeRing->init1(this->side());
        OutsetRing* outerAARing = this->getNextOutsetRing(true);
        outerStrokeRing->createOuterRing(*this, kAntialiasingRadius * 60,
                                         kZeroCoverage, outerAARing);
#endif

#if 0
        // discard all the triangles added between the originating ring and the new outer ring
        fIndices.rewind();

        outerStrokeAndAARing.init(*this);

        outerStrokeAndAARing.makeOriginalRing();

        // Add the outer stroke ring's normals to the originating ring's normals
        // so it can also act as an originating ring
        fNorms1.setCount(fNorms1.count() + outerStrokeAndAARing.numPts());
        for (int i = 0; i < outerStrokeAndAARing.numPts(); ++i) {
            int foo = outerStrokeAndAARing.index(i);
            SkASSERT(foo < fNorms1.count());
            fNorms1[outerStrokeAndAARing.index(i)] = outerStrokeAndAARing.norm17(i);
        }

        // the bisectors are only needed for the computation of the outer ring
        fBisectors.rewind();

        InsetRing* insetAARing;
        this->createInsetRings(outerStrokeAndAARing,
                               0.0f, 0.0f, 2*kAntialiasingRadius, 1.0f,
                               &insetAARing);
#endif

        SkDEBUGCODE(this->validate();)
        return true;
    }

    if (SkStrokeRec::kStroke_Style == fStyle) {
        SkASSERT(fStrokeWidth >= 0.0f);
        SkASSERT(m.isSimilarity());
        scaleFactor = m.getMaxScale(); // x and y scale are the same
        SkScalar effectiveStrokeWidth = scaleFactor * fStrokeWidth;
        OutsetRing outerStrokeRing(true);
        fInitialOutsetRing.createOuterRing(*this, effectiveStrokeWidth / 2 - kAntialiasingRadius,
                                           kFullCoverage, &outerStrokeRing);
        outerStrokeRing.init1(this->side());
        OutsetRing outerAARing(true);
        outerStrokeRing.createOuterRing(*this, kAntialiasingRadius * 2,
                                        kZeroCoverage, &outerAARing);
    } else {
        OutsetRing outerAARing(true);
        fInitialOutsetRing.createOuterRing(*this, kAntialiasingRadius,
                                           kZeroCoverage, &outerAARing);
    }

    // the bisectors are only needed for the computation of the outer ring
    fBisectors.rewind();
    if (SkStrokeRec::kStroke_Style == fStyle && fInitialInsetRing.numPts() > 2) {
        SkASSERT(fStrokeWidth >= 0.0f);
        SkScalar effectiveStrokeWidth = scaleFactor * fStrokeWidth;
        InsetRing* insetStrokeRing;
        SkScalar strokeDepth = effectiveStrokeWidth / 2 - kAntialiasingRadius;
        if (this->createInsetRings(fInitialInsetRing, 0.0f, kFullCoverage, strokeDepth, kFullCoverage,
                                   &insetStrokeRing)) {
            InsetRing* insetAARing;
            this->createInsetRings(*insetStrokeRing, strokeDepth, kFullCoverage, strokeDepth +
                                   kAntialiasingRadius * 2, kZeroCoverage, &insetAARing);
        }
    } else {
        InsetRing* insetAARing;
        this->createInsetRings(fInitialInsetRing, 0.0f, kHalfCoverage, kAntialiasingRadius,
                               kFullCoverage, &insetAARing);
    }

    SkDEBUGCODE(this->validate();)
    return true;
}

static SkVector compute_normal(const SkVector& p0, const SkVector& p1) {
    SkVector n = p0 - p1;
    SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&n);
    SkASSERT(len > 0.0f);
    SkASSERT(SkScalarNearlyEqual(1.0f, n.length()));
    return n;
}

SkScalar GrAAConvexTessellator::computeDepthFromEdge(int edgeIdx, const SkPoint& p) const {
    SkASSERT(edgeIdx < fNorms1.count());

    SkPoint v = p - fPts[edgeIdx];
    SkScalar depth = -fNorms1[edgeIdx].dot(v);
    return depth;
}

// Find a point that is 'desiredDepth' away from the 'edgeIdx'-th edge and lies
// along the 'bisector' from the 'startIdx'-th point.
bool GrAAConvexTessellator::computePtAlongBisector(int startIdx,
                                                   const SkVector& bisector,
                                                   int edgeIdx,
                                                   SkScalar desiredDepth,
                                                   SkPoint* result) const {
    const SkPoint& norm = fNorms1[edgeIdx];

    // First find the point where the edge and the bisector intersect
    SkPoint newP;

    SkScalar t = perp_intersect(fPts[startIdx], bisector, fPts[edgeIdx], norm);
    if (SkScalarNearlyEqual(t, 0.0f)) {
        // the start point was one of the original ring points
        SkASSERT(startIdx < fPts.count());
        newP = fPts[startIdx];
    } else if (t < 0.0f) {
        newP = bisector;
        newP.scale(t);
        newP += fPts[startIdx];
    } else {
        return false;
    }

    // Then offset along the bisector from that point the correct distance
    SkScalar dot = bisector.dot(norm);
    t = -desiredDepth / dot;
    *result = bisector;
    result->scale(t);
    *result += newP;

    return true;
}

bool GrAAConvexTessellator::extractFromPath(const SkMatrix& m, const SkPath& path) {
    SkASSERT(SkPath::kConvex_Convexity == path.getConvexity());

    // Outer ring: 3*numPts
    // Middle ring: numPts
    // Presumptive inner ring: numPts
    this->reservePts(5*path.countPoints());
    // Outer ring: 12*numPts
    // Middle ring: 0
    // Presumptive inner ring: 6*numPts + 6
    fIndices.setReserve(18*path.countPoints() + 6);

    fNorms1.setReserve(path.countPoints());

    fInitialOutsetRing.setReserve(path.countPoints());

    // TODO: is there a faster way to extract the points from the path? Perhaps
    // get all the points via a new entry point, transform them all in bulk
    // and then walk them to find duplicates?
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->lineTo(m, pts[1], kSharp_CurveState);
                break;
            case SkPath::kQuad_Verb:
                this->quadTo(m, pts);
                break;
            case SkPath::kCubic_Verb:
                this->cubicTo(m, pts);
                break;
            case SkPath::kConic_Verb:
                this->conicTo(m, pts, iter.conicWeight());
                break;
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }

    // This isn't quite right for stroking
    if (this->numPts() < 2) {
        return false;
    }

    if (is_duplicate_outset_pt(fInitialOutsetRing.lastPt(), fInitialOutsetRing.point(0))) {
        fInitialOutsetRing.pop();
    }

    // check if last point is a duplicate of the first point. If so, remove it.
    if (duplicate_pt(fPts[this->numPts()-1], fPts[0])) {
        this->popLastPt();
        fNorms1.pop();
        fInitialOutsetRing.fuse(this->numPts(), 0);
    }

    SkASSERT(fPts.count() == fNorms1.count()+1);
    if (this->numPts() >= 3) {
        if (abs_dist_from_line(fPts.top(), fNorms1.top(), fPts[0]) < kClose) {
            // The last point is on the line from the second to last to the first point.
            this->popLastPt();
            fNorms1.pop();
            fInitialOutsetRing.rmLinearPt(this->numPts(), this->numPts()-1, 0);
        }

        *fNorms1.push() = compute_normal(fPts[0], fPts.top());
        SkASSERT(fPts.count() == fNorms1.count());
    }

    if (this->numPts() >= 3 && abs_dist_from_line(fPts[0], fNorms1.top(), fPts[1]) < kClose) {
        // The first point is on the line from the last to the second.
        this->popFirstPtShuffle();
        fInitialOutsetRing.rmLinearPt(0, this->numPts(), 1);
        fNorms1.removeShuffle(0);
        fNorms1[0] = compute_normal(fPts[1], fPts[0]);
    }

    if (this->numPts() >= 3) {
        // Check the cross product of the final trio
        SkScalar cross = SkPoint::CrossProduct(fNorms1[0], fNorms1.top());
        if (cross > 0.0f) {
            fSide = SkPoint::kRight_Side;
        } else {
            fSide = SkPoint::kLeft_Side;
        }

        // Make all the normals face outwards rather than along the edge
        for (int cur = 0; cur < fNorms1.count(); ++cur) {
            fNorms1[cur].setOrthog(fNorms1[cur], fSide);
            SkASSERT(SkScalarNearlyEqual(1.0f, fNorms1[cur].length()));
        }

        this->computeBisectors();
    } else if (this->numPts() == 2) {
        // We've got two points, so we're degenerate.
        if (fStyle == SkStrokeRec::kFill_Style) {
            // it's a fill, so we don't need to worry about degenerate paths
            return false;
        }
        // For stroking, we still need to process the degenerate path, so fix it up
        fSide = SkPoint::kLeft_Side;

        // Make all the normals face outwards rather than along the edge
        for (int cur = 0; cur < fNorms1.count(); ++cur) {
            fNorms1[cur].setOrthog(fNorms1[cur], fSide);
            SkASSERT(SkScalarNearlyEqual(1.0f, fNorms1[cur].length()));
        }

        fNorms1.push(SkPoint::Make(-fNorms1[0].fX, -fNorms1[0].fY));
        // we won't actually use the bisectors, so just push zeroes
        fBisectors.push(SkPoint::Make(0.0, 0.0));
        fBisectors.push(SkPoint::Make(0.0, 0.0));
    } else {
        return false;
    }

    fCandidateVerts.setReserve(this->numPts());
    fInitialInsetRing.setReserve(this->numPts());
    for (int i = 0; i < this->numPts(); ++i) {
        fInitialInsetRing.addIdx(i, i);
    }
    fInitialInsetRing.init(fNorms1, fBisectors);

    fInitialOutsetRing.init1(this->side());

    this->validate();
    return true;
}

GrAAConvexTessellator::InsetRing* GrAAConvexTessellator::getNextInsetRing(InsetRing* lastRing) {
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    InsetRing* ring = *fInsetRings.push() = new InsetRing;
    ring->setReserve(fInitialInsetRing.numPts());
    ring->rewind();
    return ring;
#else
    // Flip flop back and forth between fRings[0] & fRings[1]
    int nextRing = (lastRing == &fRings[0]) ? 1 : 0;
    fRings[nextRing].setReserve(fInitialInsetRing.numPts());
    fRings[nextRing].rewind();
    return &fRings[nextRing];
#endif
}

GrAAConvexTessellator::OutsetRing* GrAAConvexTessellator::getNextOutsetRing(bool passOnPts) {
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    OutsetRing* ring = *fOutsetRings.push() = new OutsetRing(passOnPts);
    //ring->setReserve(fInitialInsetRing.numPts());
    ring->rewind();
    return ring;
#else
    // Flip flop back and forth between fRings[0] & fRings[1]
    int nextRing = (lastRing == &fRings[0]) ? 1 : 0;
    fRings[nextRing].setReserve(fInitialInsetRing.numPts());
    fRings[nextRing].rewind();
    return &fRings[nextRing];
#endif
}

void GrAAConvexTessellator::fanRing(const InsetRing& ring) {
    // fan out from point 0
    int startIdx = ring.index(0);
    for (int cur = ring.numPts() - 2; cur >= 0; --cur) {
        this->addTri(startIdx, ring.index(cur), ring.index(cur + 1));
    }
}

int GrAAConvexTessellator::OutsetRing::bevel(GrAAConvexTessellator& tess,
                                             OutsetRing* nextRing,
                                             const SkVector& perp2,
                                             int perp1Idx,
                                             int cur,
                                             SkScalar coverage,
                                             CurveState curve) {
    int perp2Idx = nextRing->add(tess, perp2, coverage, curve, -1, -1);
    this->addTri(tess, cur, perp1Idx, perp2Idx);
    SkASSERT(-1 != perp2Idx);
    return perp2Idx;
}

void GrAAConvexTessellator::OutsetRing::createOuterRing(GrAAConvexTessellator & tess, SkScalar outset,
                                                        SkScalar coverage, OutsetRing* nextRing) {
    this->validate1(tess);

    const int numPts = this->numPts();
    if (numPts == 0) {
        return;
    }

    int prev = numPts - 1;
    // Track the last perpendicular outset point so we can construct the
    // trailing edge triangles.
    int lastPerpIdx = -1, firstPerpIdx = -1;

    const SkScalar outsetSq = SkScalarMul(outset, outset);
    SkScalar miterLimitSq = SkScalarMul(outset, tess.miterLimit());
    miterLimitSq = SkScalarMul(miterLimitSq, miterLimitSq);

    for (int cur = 0; cur < numPts; ++cur) {
        // For each vertex of the original polygon we add at least two points to the
        // outset polygon - one extending perpendicular to each impinging edge. Connecting these
        // two points yields a bevel join. We need one additional point for a mitered join, and
        // a round join requires one or more points depending upon curvature.

        const SkPoint& basePt = this->point(cur);

        // The perpendicular point for the last edge
        const SkPoint& normal1 = this->norm(prev);
        SkPoint perp1 = normal1;
        perp1.scale(outset);
        perp1 += basePt;

        // The perpendicular point for the next edge.
        const SkPoint& normal2 = this->norm(cur);
        SkPoint perp2 = normal2;
        perp2.scale(outset);
        perp2 += basePt;

        CurveState curve = this->curveState(cur);

        // TODO: this isn't true anymore!
        // We know it isn't a duplicate of the prior point (since it and this
        // one are just perpendicular offsets from the non-merged polygon points)
        int perp1Idx = nextRing->add(tess, perp1, coverage, curve, -1, -1);
        //nextRing->addIdx(perp1Idx, -1);

//        int perp2Idx;
        // For very shallow angles all the corner points could fuse.
//        if (duplicate_pt(perp2, perp1)) {
//            perp2Idx = perp1Idx;
//        } else {
//            int perp2Idx = nextRing->add(tess, perp2, coverage, curve, -1, -1);
//        }

        int perp2Idx = -1;
        if (!duplicate_pt(perp2, perp1)) {
            if (curve == kCurve_CurveState) {
                // bevel or round depending upon curvature
                SkScalar dotProd = normal1.dot(normal2);
                if (dotProd < kRoundCapThreshold) {
                    // Currently we "round" by creating a single extra point, which produces
                    // good results for common cases. For thick strokes with high curvature, we will
                    // need to add more points; for the time being we simply fall back to software
                    // rendering for thick strokes.
                    SkPoint miter = this->bisector(cur);
                    miter.setLength(-outset);
                    miter += basePt;

                    // For very shallow angles all the corner points could fuse
                    if (!duplicate_pt(miter, perp1)) {
                        int miterIdx = nextRing->add(tess, miter, coverage,
                                                     kSharp_CurveState, -1, -1);
                        //nextRing->addIdx(miterIdx, -1);
                        perp2Idx = nextRing->add(tess, perp2, coverage, curve, -1, -1);

                        // The two triangles for the corner
                        this->addTrisForCorner(tess, cur, perp1Idx, miterIdx, perp2Idx);
                    } else {
                        perp2Idx = this->bevel(tess, nextRing, perp2, perp1Idx, cur, coverage, curve);
                    }
                } else {
                    perp2Idx = this->bevel(tess, nextRing, perp2, perp1Idx, cur, coverage, curve);
                }
            } else {
                switch (tess.join()) {
                    case SkPaint::Join::kMiter_Join: {
                        // The bisector outset point
                        SkScalar dotProd = normal1.dot(normal2);
                        SkScalar sinHalfAngleSq = SkScalarHalf(SK_Scalar1 + dotProd);
                        SkScalar lengthSq = outsetSq / sinHalfAngleSq;
                        if (lengthSq > miterLimitSq) {
                            // just bevel it
                            perp2Idx = this->bevel(tess, nextRing, perp2, perp1Idx, cur, coverage, curve);
                            break;
                        }
                        SkPoint miter = this->bisector(cur);
                        miter.setLength(-SkScalarSqrt(lengthSq));
                        miter += basePt;

                        // For very shallow angles all the corner points could fuse
                        if (!duplicate_pt(miter, perp1)) {
                            int miterIdx = nextRing->add(tess, miter, coverage,
                                                         kSharp_CurveState, -1, -1);
                            //nextRing->addIdx(miterIdx, -1);
                            perp2Idx = nextRing->add(tess, perp2, coverage, curve, -1, -1);

                            // The two triangles for the corner
                            this->addTrisForCorner(tess, cur, perp1Idx, miterIdx, perp2Idx);
                        } else {
                            perp2Idx = nextRing->bevel(tess, nextRing, perp2, perp1Idx, cur, coverage, curve);
                        }
                        break;
                    }
                    case SkPaint::Join::kBevel_Join:
                        perp2Idx = this->bevel(tess, nextRing, perp2, perp1Idx, cur, coverage, curve);
                        break;
                    default:
                        // kRound_Join is unsupported for now. GrAALinearizingConvexPathRenderer is
                        // only willing to draw mitered or beveled, so we should never get here.
                        SkASSERT(false);
                }
            }

            SkASSERT(-1 != perp2Idx);
            //nextRing->addIdx(perp2Idx, -1);
        } else {
            perp2Idx = perp1Idx;
        }

        if (0 == cur) {
            // Store the index of the first perpendicular point to finish up
            firstPerpIdx = perp1Idx;
            //SkASSERT(-1 == lastPerpIdx);
        } else {
            // The triangles for the previous edge
            this->addTrisForEdge(tess, prev, cur, perp1Idx, lastPerpIdx);
        }

        // Track the last perpendicular outset point so we can construct the
        // trailing edge triangles.
        lastPerpIdx = perp2Idx;
        prev = cur;
    }

    // pick up the final edge rect
    this->addTrisForEdge(tess, numPts-1, 0, firstPerpIdx, lastPerpIdx);

    nextRing->validate1(tess);
    tess.validate();
}

// Something went wrong in the creation of the next ring. If we're filling the shape, just go ahead
// and fan it.
void GrAAConvexTessellator::terminate(const InsetRing& ring) {
    if (fStyle != SkStrokeRec::kStroke_Style) {
        this->fanRing(ring);
    }
}

static SkScalar compute_coverage(SkScalar depth, SkScalar initialDepth, SkScalar initialCoverage,
                                SkScalar targetDepth, SkScalar targetCoverage) {
    if (SkScalarNearlyEqual(initialDepth, targetDepth)) {
        return targetCoverage;
    }
    SkScalar result = (depth - initialDepth) / (targetDepth - initialDepth) *
            (targetCoverage - initialCoverage) + initialCoverage;
    return SkScalarClampMax(result, 1.0f);
}

// return true when processing is complete
bool GrAAConvexTessellator::createInsetRing(const InsetRing& lastRing, InsetRing* nextRing,
                                            SkScalar initialDepth, SkScalar initialCoverage,
                                            SkScalar targetDepth, SkScalar targetCoverage,
                                            bool forceNew) {
    bool done = false;

    fCandidateVerts.rewind();

    // Loop through all the points in the ring and find the intersection with the smallest depth
    SkScalar minDist = SK_ScalarMax, minT = 0.0f;
    int minEdgeIdx = -1;

    for (int cur = 0; cur < lastRing.numPts(); ++cur) {
        int next = (cur + 1) % lastRing.numPts();

        SkScalar t;
        bool result = intersect(this->point(lastRing.index(cur)),  lastRing.bisector17(cur),
                                this->point(lastRing.index(next)), lastRing.bisector17(next),
                                &t);
        if (!result) {
            continue;
        }
        SkScalar dist = -t * lastRing.norm17(cur).dot(lastRing.bisector17(cur));

        if (minDist > dist) {
            minDist = dist;
            minT = t;
            minEdgeIdx = cur;
        }
    }

    if (minEdgeIdx == -1) {
        return false;
    }
    SkPoint newPt = lastRing.bisector17(minEdgeIdx);
    newPt.scale(minT);
    newPt += this->point(lastRing.index(minEdgeIdx));

    SkScalar depth = this->computeDepthFromEdge(lastRing.origEdgeID(minEdgeIdx), newPt);
    if (depth >= targetDepth) {
        // None of the bisectors intersect before reaching the desired depth.
        // Just step them all to the desired depth
        depth = targetDepth;
        done = true;
    }

    // 'dst' stores where each point in the last ring maps to/transforms into
    // in the next ring.
    SkTDArray<int> dst;
    dst.setCount(lastRing.numPts());

    // Create the first point (who compares with no one)
    if (!this->computePtAlongBisector(lastRing.index(0),
                                      lastRing.bisector17(0),
                                      lastRing.origEdgeID(0),
                                      depth, &newPt)) {
        this->terminate(lastRing);
        return true;
    }
    dst[0] = fCandidateVerts.addNewPt(newPt,
                                      lastRing.index(0), lastRing.origEdgeID(0),
                                      !this->movable(lastRing.index(0)));

    // Handle the middle points (who only compare with the prior point)
    for (int cur = 1; cur < lastRing.numPts()-1; ++cur) {
        if (!this->computePtAlongBisector(lastRing.index(cur),
                                          lastRing.bisector17(cur),
                                          lastRing.origEdgeID(cur),
                                          depth, &newPt)) {
            this->terminate(lastRing);
            return true;
        }
        if (!duplicate_pt(newPt, fCandidateVerts.lastPoint())) {
            dst[cur] = fCandidateVerts.addNewPt(newPt,
                                                lastRing.index(cur), lastRing.origEdgeID(cur),
                                                !this->movable(lastRing.index(cur)));
        } else {
            dst[cur] = fCandidateVerts.fuseWithPrior(lastRing.origEdgeID(cur));
        }
    }

    // Check on the last point (handling the wrap around)
    int cur = lastRing.numPts()-1;
    if  (!this->computePtAlongBisector(lastRing.index(cur),
                                       lastRing.bisector17(cur),
                                       lastRing.origEdgeID(cur),
                                       depth, &newPt)) {
        this->terminate(lastRing);
        return true;
    }
    bool dupPrev = duplicate_pt(newPt, fCandidateVerts.lastPoint());
    bool dupNext = duplicate_pt(newPt, fCandidateVerts.firstPoint());

    if (!dupPrev && !dupNext) {
        dst[cur] = fCandidateVerts.addNewPt(newPt,
                                            lastRing.index(cur), lastRing.origEdgeID(cur),
                                            !this->movable(lastRing.index(cur)));
    } else if (dupPrev && !dupNext) {
        dst[cur] = fCandidateVerts.fuseWithPrior(lastRing.origEdgeID(cur));
    } else if (!dupPrev && dupNext) {
        dst[cur] = fCandidateVerts.fuseWithNext();
    } else {
        bool dupPrevVsNext = duplicate_pt(fCandidateVerts.firstPoint(), fCandidateVerts.lastPoint());

        if (!dupPrevVsNext) {
            dst[cur] = fCandidateVerts.fuseWithPrior(lastRing.origEdgeID(cur));
        } else {
            const int fused = fCandidateVerts.fuseWithBoth();
            dst[cur] = fused;
            const int targetIdx = dst[cur - 1];
            for (int i = cur - 1; i >= 0 && dst[i] == targetIdx; i--) {
                dst[i] = fused;
            }
        }
    }

    // Fold the new ring's points into the global pool
    for (int i = 0; i < fCandidateVerts.numPts(); ++i) {
        int newIdx;
        if (fCandidateVerts.needsToBeNew(i) || forceNew) {
            // if the originating index is still valid then this point wasn't
            // fused (and is thus movable)
            SkScalar coverage = compute_coverage(depth, initialDepth, initialCoverage,
                                                 targetDepth, targetCoverage);
            newIdx = this->addPt(fCandidateVerts.point(i), depth, coverage,
                                 fCandidateVerts.originatingIdx(i) != -1, kSharp_CurveState);
        } else {
            SkASSERT(fCandidateVerts.originatingIdx(i) != -1);
            this->updatePt(fCandidateVerts.originatingIdx(i), fCandidateVerts.point(i), depth,
                           targetCoverage);
            newIdx = fCandidateVerts.originatingIdx(i);
        }

        nextRing->addIdx(newIdx, fCandidateVerts.origEdge(i));
    }

    // 'dst' currently has indices into the ring. Remap these to be indices
    // into the global pool since the triangulation operates in that space.
    for (int i = 0; i < dst.count(); ++i) {
        dst[i] = nextRing->index(dst[i]);
    }

    for (int i = 0; i < lastRing.numPts(); ++i) {
        int next = (i + 1) % lastRing.numPts();

        this->addTri(lastRing.index(i), lastRing.index(next), dst[next]);
        this->addTri(lastRing.index(i), dst[next], dst[i]);
    }

    if (done && fStyle != SkStrokeRec::kStroke_Style) {
        // fill or stroke-and-fill
        this->fanRing(*nextRing);
    }

    if (nextRing->numPts() < 3) {
        done = true;
    }
    return done;
}

void GrAAConvexTessellator::validate() const {
    SkASSERT(fPts.count() == fMovable.count());
    SkASSERT(fPts.count() == fCoverages.count());
    SkASSERT(fPts.count() == fCurveState.count());
    SkASSERT(0 == (fIndices.count() % 3));
    SkASSERT(!fBisectors.count() || fBisectors.count() == fNorms1.count());
}

//////////////////////////////////////////////////////////////////////////////
void GrAAConvexTessellator::InsetRing::init(const GrAAConvexTessellator& tess) {
    this->computeNormals(tess);
    this->computeBisectors(tess);
}

void GrAAConvexTessellator::InsetRing::init(const SkTDArray<SkVector>& norms,
                                       const SkTDArray<SkVector>& bisectors) {
    for (int i = 0; i < fPts.count(); ++i) {
        fPts[i].fNorm17 = norms[i];
        fPts[i].fBisector17 = bisectors[i];
    }
}

// Compute the outward facing normal at each vertex.
void GrAAConvexTessellator::InsetRing::computeNormals(const GrAAConvexTessellator& tess) {
    for (int cur = 0; cur < fPts.count(); ++cur) {
        int next = (cur + 1) % fPts.count();

        fPts[cur].fNorm17 = tess.point(fPts[next].fIndex) - tess.point(fPts[cur].fIndex);
        SkPoint::Normalize(&fPts[cur].fNorm17);
        fPts[cur].fNorm17.setOrthog(fPts[cur].fNorm17, tess.side());
    }
}

void GrAAConvexTessellator::InsetRing::computeBisectors(const GrAAConvexTessellator& tess) {
    int prev = fPts.count() - 1;
    for (int cur = 0; cur < fPts.count(); prev = cur, ++cur) {
        fPts[cur].fBisector17 = fPts[cur].fNorm17 + fPts[prev].fNorm17;
        if (!fPts[cur].fBisector17.normalize()) {
            SkASSERT(SkPoint::kLeft_Side == tess.side() || SkPoint::kRight_Side == tess.side());
            fPts[cur].fBisector17.setOrthog(fPts[cur].fNorm17, (SkPoint::Side)-tess.side());
            SkVector other;
            other.setOrthog(fPts[prev].fNorm17, tess.side());
            fPts[cur].fBisector17 += other;
            SkAssertResult(fPts[cur].fBisector17.normalize());
        } else {
            fPts[cur].fBisector17.negate();      // make the bisector face in
        }
    }
}

void GrAAConvexTessellator::OutsetRing::init1(SkVector::Side side) {
    this->computeNormals1(side);
    this->computeBisectors1(side);  
}

// Compute the outward facing normal at each vertex.
void GrAAConvexTessellator::OutsetRing::computeNormals1(SkVector::Side side) {
    for (int cur = 0; cur < fPts7.count(); ++cur) {
        int next = (cur + 1) % fPts7.count();

        fPts7[cur].fNorm = fPts7[next].fPt - fPts7[cur].fPt;
        SkPoint::Normalize(&fPts7[cur].fNorm);
        fPts7[cur].fNorm.setOrthog(fPts7[cur].fNorm, side);
    }
}

void GrAAConvexTessellator::OutsetRing::computeBisectors1(SkVector::Side side) {
    int prev = fPts7.count() - 1;
    for (int cur = 0; cur < fPts7.count(); prev = cur, ++cur) {
        const PointData& p0 = fPts7[cur];
        const PointData& p1 = fPts7[prev];
        const SkVector b = p0.fNorm + p1.fNorm;

        fPts7[cur].fBisector = b;
        if (!fPts7[cur].fBisector.normalize()) {
            SkASSERT(SkPoint::kLeft_Side == side || SkPoint::kRight_Side == side);
            fPts7[cur].fBisector.setOrthog(fPts7[cur].fNorm, (SkPoint::Side)-side);
            SkVector other;
            other.setOrthog(fPts7[prev].fNorm, side);
            fPts7[cur].fBisector += other;
            SkAssertResult(fPts7[cur].fBisector.normalize());
        } else {
            fPts7[cur].fBisector.negate();      // make the bisector face in
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
#ifdef SK_DEBUG
// Is this ring convex?
bool GrAAConvexTessellator::InsetRing::isConvex(const GrAAConvexTessellator& tess) const {
    if (fPts.count() < 3) {
        return true;
    }

    SkPoint prev = tess.point(fPts[0].fIndex) - tess.point(fPts.top().fIndex);
    SkPoint cur  = tess.point(fPts[1].fIndex) - tess.point(fPts[0].fIndex);
    SkScalar minDot = prev.fX * cur.fY - prev.fY * cur.fX;
    SkScalar maxDot = minDot;

    prev = cur;
    for (int i = 1; i < fPts.count(); ++i) {
        int next = (i + 1) % fPts.count();

        cur  = tess.point(fPts[next].fIndex) - tess.point(fPts[i].fIndex);
        SkScalar dot = prev.fX * cur.fY - prev.fY * cur.fX;

        minDot = SkMinScalar(minDot, dot);
        maxDot = SkMaxScalar(maxDot, dot);

        prev = cur;
    }

    if (SkScalarNearlyEqual(maxDot, 0.0f, 0.005f)) {
        maxDot = 0;
    }
    if (SkScalarNearlyEqual(minDot, 0.0f, 0.005f)) {
        minDot = 0;
    }
    return (maxDot >= 0.0f) == (minDot >= 0.0f);
}

#endif

void GrAAConvexTessellator::lineTo(const SkPoint& p, CurveState curve) {
    if (fInitialOutsetRing.numPts() > 0 &&
        is_duplicate_outset_pt(p, fInitialOutsetRing.lastPt())) {
        // If it is a duplicate outset point it is definitely an duplicate inset point
        return;
    }

    SkScalar initialRingCoverage = (SkStrokeRec::kFill_Style == fStyle) ? 0.5f : 1.0f;
    const int origMapGuess = this->numPts();

    fInitialOutsetRing.add(*this, p, initialRingCoverage, curve, origMapGuess, -1);

    if (this->numPts() > 0 && duplicate_pt(p, this->lastPoint())) {
        // The new outer point maps back to the last inner point
        fInitialOutsetRing.fuse(origMapGuess, this->numPts()-1);
        return;
    }

    SkASSERT(fPts.count() <= 1 || fPts.count() == fNorms1.count()+1);
    if (this->numPts() >= 2 && abs_dist_from_line(fPts.top(), fNorms1.top(), p) < kClose) {
        // The old last point is on the line from the second to last to the new point
        this->popLastPt();
        fNorms1.pop();
        // double-check that the new last point is not a duplicate of the new point. In an ideal
        // world this wouldn't be necessary (since it's only possible for non-convex paths), but
        // floating point precision issues mean it can actually happen on paths that were
        // determined to be convex.
        if (duplicate_pt(p, this->lastPoint())) {
            // The new outer point maps back to the last inner point
            fInitialOutsetRing.fuse(origMapGuess, this->numPts()-1);
            return;
        } else {
            // What to do here? It should actually map to two points and emit a triangle.
            //this->fix(this->numPts(), this->numPts()-1, this->numPts());
            fInitialOutsetRing.rmLinearPt(this->numPts(), this->numPts(), this->numPts()-1);
        }
    }
    this->addPt(p, 0.0f, initialRingCoverage, false, curve);
    if (this->numPts() > 1) {
        *fNorms1.push() = compute_normal(fPts.top(), fPts[fPts.count()-2]);
    }
}

void GrAAConvexTessellator::lineTo(const SkMatrix& m, SkPoint p, CurveState curve) {
    m.mapPoints(&p, 1);
    this->lineTo(p, curve);
}

void GrAAConvexTessellator::quadTo(const SkPoint pts[3]) {
    int maxCount = GrPathUtils::quadraticPointCount(pts, kQuadTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     kQuadTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count - 1; i++) {
        this->lineTo(fPointBuffer[i], kCurve_CurveState);
    }
    this->lineTo(fPointBuffer[count - 1], kIndeterminate_CurveState);
}

void GrAAConvexTessellator::quadTo(const SkMatrix& m, SkPoint pts[3]) {
    m.mapPoints(pts, 3);
    this->quadTo(pts);
}

void GrAAConvexTessellator::cubicTo(const SkMatrix& m, SkPoint pts[4]) {
    m.mapPoints(pts, 4);
    int maxCount = GrPathUtils::cubicPointCount(pts, kCubicTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateCubicPoints(pts[0], pts[1], pts[2], pts[3],
            kCubicTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count - 1; i++) {
        this->lineTo(fPointBuffer[i], kCurve_CurveState);
    }
    this->lineTo(fPointBuffer[count - 1], kIndeterminate_CurveState);
}

// include down here to avoid compilation errors caused by "-" overload in SkGeometry.h
#include "SkGeometry.h"

void GrAAConvexTessellator::conicTo(const SkMatrix& m, SkPoint pts[3], SkScalar w) {
    m.mapPoints(pts, 3);
    SkAutoConicToQuads quadder;
    const SkPoint* quads = quadder.computeQuads(pts, w, kConicTolerance);
    SkPoint lastPoint = *(quads++);
    int count = quadder.countQuads();
    for (int i = 0; i < count; ++i) {
        SkPoint quadPts[3];
        quadPts[0] = lastPoint;
        quadPts[1] = quads[0];
        quadPts[2] = i == count - 1 ? pts[2] : quads[1];
        this->quadTo(quadPts);
        lastPoint = quadPts[2];
        quads += 2;
    }
}

//////////////////////////////////////////////////////////////////////////////
#if GR_AA_CONVEX_TESSELLATOR_VIZ
static const SkScalar kPointRadius = 2.0f;
static const SkScalar kArrowStrokeWidth = 3.0f;
static const SkScalar kArrowLength = 30.0f;
static const SkScalar kEdgeTextSize = 20.0f;
static const SkScalar kPointTextSize = 10.0f;

static void draw_point(SkCanvas* canvas, const SkPoint& p, SkColor color) {
    SkPaint paint;
    paint.setColor(color);

    canvas->drawCircle(p.fX, p.fY, kPointRadius, paint);
}

static void draw_line(SkCanvas* canvas, const SkPoint& p0, const SkPoint& p1, SkColor color, float strokeWidth) {
    SkPaint p;
    p.setColor(color);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(strokeWidth);

    canvas->drawLine(p0.fX, p0.fY, p1.fX, p1.fY, p);
}

static void draw_arrow(SkCanvas*canvas, const SkPoint& p, const SkPoint &n,
                       SkScalar len, SkColor color) {
    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(kArrowStrokeWidth);
    paint.setStyle(SkPaint::kStroke_Style);

    canvas->drawLine(p.fX, p.fY,
                     p.fX + len * n.fX, p.fY + len * n.fY,
                     paint);
}

void GrAAConvexTessellator::InsetRing::draw(SkCanvas* canvas, const GrAAConvexTessellator& tess) const {
    SkPaint paint;
    paint.setTextSize(kEdgeTextSize);

    for (int cur = 0; cur < fPts.count(); ++cur) {
        const SkVector& n = fPts[cur].fNorm17;

        int next = (cur + 1) % fPts.count();

        draw_line(canvas,
                  tess.point(fPts[cur].fIndex),
                  tess.point(fPts[next].fIndex),
                  SK_ColorGREEN, 3);

        SkPoint mid = tess.point(fPts[cur].fIndex) + tess.point(fPts[next].fIndex);
        mid.scale(0.5f);

        draw_arrow(canvas, mid, n, 1.5f*kArrowLength, SK_ColorRED);
        mid.fX += (kArrowLength/2) * n.fX;
        mid.fY += (kArrowLength/2) * n.fY;

        SkString num;
        num.printf("%d", this->origEdgeID(cur));
        canvas->drawText(num.c_str(), num.size(), mid.fX, mid.fY, paint);

        draw_arrow(canvas, tess.point(fPts[cur].fIndex), fPts[cur].fBisector17,
                   1.5f*kArrowLength, SK_ColorBLUE);
    }
}

void GrAAConvexTessellator::OutsetRing::draw(SkCanvas* canvas, const GrAAConvexTessellator& tess) const {
    SkPaint paint;
    paint.setTextSize(kEdgeTextSize);

    for (int i = 0; i < this->numPts(); ++i) {

    }

    for (int cur = 0; cur < fPts7.count(); ++cur) {
        const SkVector& n = fPts7[cur].fNorm;
        const SkPoint& p = fPts7[cur].fPt;

        SkPaint paint;
        paint.setTextSize(kPointTextSize);
        paint.setTextAlign(SkPaint::kCenter_Align);

        SkString num;
        num.printf("%d", fPts7[cur].fI1);
        canvas->drawText(num.c_str(), num.size(), p.fX+10.0f, p.fY-5.0f, paint);

        int next = (cur + 1) % fPts7.count();

        draw_line(canvas, fPts7[cur].fPt, fPts7[next].fPt, SK_ColorCYAN, 1.5f);

        SkPoint mid = fPts7[cur].fPt + fPts7[next].fPt;
        mid.scale(0.5f);

        draw_arrow(canvas, mid, n, kArrowLength, SK_ColorMAGENTA);
        mid.fX += (kArrowLength/2) * n.fY;
        mid.fY += (kArrowLength/2) * n.fY;

//        SkString num;
//        num.printf("%d", this->origEdgeID(cur));
//        canvas->drawText(num.c_str(), num.size(), mid.fX, mid.fY, paint);

        draw_arrow(canvas, fPts7[cur].fPt, fPts7[cur].fBisector,
                    kArrowLength, SK_ColorGRAY);
    }
}

void GrAAConvexTessellator::draw(SkCanvas* canvas) const {
//    fInitialInsetRing.draw(canvas, *this);
    fInitialOutsetRing.draw(canvas, *this);

    for (int i = 0; i < fInsetRings.count(); ++i) {
//        fInsetRings[i]->draw(canvas, *this);
    }
    for (int i = 0; i < fOutsetRings.count(); ++i) {
        fOutsetRings[i]->draw(canvas, *this);
    }

#if 1
    for (int i = 0; i < fIndices.count(); i += 3) {
        SkASSERT(fIndices[i] < this->numPts()) ;
        SkASSERT(fIndices[i+1] < this->numPts()) ;
        SkASSERT(fIndices[i+2] < this->numPts()) ;

        draw_line(canvas,
                  this->point(this->fIndices[i]), this->point(this->fIndices[i+1]),
                  SK_ColorGRAY, 0.0f);
        draw_line(canvas,
                  this->point(this->fIndices[i+1]), this->point(this->fIndices[i+2]),
                  SK_ColorGRAY, 0.0f);
        draw_line(canvas,
                  this->point(this->fIndices[i+2]), this->point(this->fIndices[i]),
                  SK_ColorGRAY, 0.0f);
    }
#endif

#if 1
    for (int i = 0; i < this->numPts(); ++i) {
        draw_point(canvas, this->point(i), SK_ColorRED);

        SkPaint paint;
        paint.setTextSize(kPointTextSize);
        paint.setTextAlign(SkPaint::kCenter_Align);

        SkString num;
        num.printf("%d", i);
        canvas->drawText(num.c_str(), num.size(),
                         this->point(i).fX+5.0f,
                         this->point(i).fY-5.0f,
                         paint);
    }
#endif

}

#endif
