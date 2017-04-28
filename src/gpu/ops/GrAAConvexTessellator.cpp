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
static const SkScalar kCloseSqd = kClose * kClose;

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
    fNorms.rewind();
    fCurveState.rewind();
    fInitialRing.rewind();
    fCandidateVerts.rewind();
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    fRings.rewind();        // TODO: leak in this case!
#else
    fRings[0].rewind();
    fRings[1].rewind();
#endif
}

void GrAAConvexTessellator::computeBisectors() {
    fBisectors.setCount(fNorms.count());

    int prev = fBisectors.count() - 1;
    for (int cur = 0; cur < fBisectors.count(); prev = cur, ++cur) {
        fBisectors[cur] = fNorms[cur] + fNorms[prev];
        if (!fBisectors[cur].normalize()) {
            SkASSERT(SkPoint::kLeft_Side == fSide || SkPoint::kRight_Side == fSide);
            fBisectors[cur].setOrthog(fNorms[cur], (SkPoint::Side)-fSide);
            SkVector other;
            other.setOrthog(fNorms[prev], fSide);
            fBisectors[cur] += other;
            SkAssertResult(fBisectors[cur].normalize());
        } else {
            fBisectors[cur].negate();      // make the bisector face in
        }
        if (fCurveState[prev] == kIndeterminate_CurveState) {
            if (fCurveState[cur] == kSharp_CurveState) {
                fCurveState[prev] = kSharp_CurveState;
            } else {
                if (SkScalarAbs(fNorms[cur].dot(fNorms[prev])) > kCurveConnectionThreshold) {
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
bool GrAAConvexTessellator::createInsetRings(Ring& previousRing, SkScalar initialDepth,
                                             SkScalar initialCoverage, SkScalar targetDepth,
                                             SkScalar targetCoverage, Ring** finalRing) {
    static const int kMaxNumRings = 8;

    if (previousRing.numPts() < 3) {
        return false;
    }
    Ring* currentRing = &previousRing;
    int i;
    for (i = 0; i < kMaxNumRings; ++i) {
        Ring* nextRing = this->getNextRing(currentRing);
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

    SkScalar coverage = 1.0f;
    SkScalar scaleFactor = 0.0f;

    if (SkStrokeRec::kStrokeAndFill_Style == fStyle) {
        SkASSERT(m.isSimilarity());
        scaleFactor = m.getMaxScale(); // x and y scale are the same
        SkScalar effectiveStrokeWidth = scaleFactor * fStrokeWidth;
        Ring outerStrokeAndAARing;
        this->createOuterRing(fInitialRing,
                              effectiveStrokeWidth / 2 + kAntialiasingRadius, 0.0,
                              &outerStrokeAndAARing);

        // discard all the triangles added between the originating ring and the new outer ring
        fIndices.rewind();

        outerStrokeAndAARing.init(*this);

        outerStrokeAndAARing.makeOriginalRing();

        // Add the outer stroke ring's normals to the originating ring's normals
        // so it can also act as an originating ring
        fNorms.setCount(fNorms.count() + outerStrokeAndAARing.numPts());
        for (int i = 0; i < outerStrokeAndAARing.numPts(); ++i) {
            SkASSERT(outerStrokeAndAARing.index(i) < fNorms.count());
            fNorms[outerStrokeAndAARing.index(i)] = outerStrokeAndAARing.norm(i);
        }

        // the bisectors are only needed for the computation of the outer ring
        fBisectors.rewind();

        Ring* insetAARing;
        this->createInsetRings(outerStrokeAndAARing,
                               0.0f, 0.0f, 2*kAntialiasingRadius, 1.0f,
                               &insetAARing);

        SkDEBUGCODE(this->validate();)
        return true;
    }

    if (SkStrokeRec::kStroke_Style == fStyle) {
        SkASSERT(fStrokeWidth >= 0.0f);
        SkASSERT(m.isSimilarity());
        scaleFactor = m.getMaxScale(); // x and y scale are the same
        SkScalar effectiveStrokeWidth = scaleFactor * fStrokeWidth;
        Ring outerStrokeRing;
        this->createOuterRing(fInitialRing, effectiveStrokeWidth / 2 - kAntialiasingRadius,
                              coverage, &outerStrokeRing);
        outerStrokeRing.init(*this);
        Ring outerAARing;
        this->createOuterRing(outerStrokeRing, kAntialiasingRadius * 2, 0.0f, &outerAARing);
    } else {
        Ring outerAARing;
        this->createOuterRing(fInitialRing, kAntialiasingRadius, 0.0f, &outerAARing);
    }

    // the bisectors are only needed for the computation of the outer ring
    fBisectors.rewind();
    if (SkStrokeRec::kStroke_Style == fStyle && fInitialRing.numPts() > 2) {
        SkASSERT(fStrokeWidth >= 0.0f);
        SkScalar effectiveStrokeWidth = scaleFactor * fStrokeWidth;
        Ring* insetStrokeRing;
        SkScalar strokeDepth = effectiveStrokeWidth / 2 - kAntialiasingRadius;
        if (this->createInsetRings(fInitialRing, 0.0f, coverage, strokeDepth, coverage,
                                   &insetStrokeRing)) {
            Ring* insetAARing;
            this->createInsetRings(*insetStrokeRing, strokeDepth, coverage, strokeDepth +
                                   kAntialiasingRadius * 2, 0.0f, &insetAARing);
        }
    } else {
        Ring* insetAARing;
        this->createInsetRings(fInitialRing, 0.0f, 0.5f, kAntialiasingRadius, 1.0f, &insetAARing);
    }

    SkDEBUGCODE(this->validate();)
    return true;
}

SkScalar GrAAConvexTessellator::computeDepthFromEdge(int edgeIdx, const SkPoint& p) const {
    SkASSERT(edgeIdx < fNorms.count());

    SkPoint v = p - fPts[edgeIdx];
    SkScalar depth = -fNorms[edgeIdx].dot(v);
    return depth;
}

// Find a point that is 'desiredDepth' away from the 'edgeIdx'-th edge and lies
// along the 'bisector' from the 'startIdx'-th point.
bool GrAAConvexTessellator::computePtAlongBisector(int startIdx,
                                                   const SkVector& bisector,
                                                   int edgeIdx,
                                                   SkScalar desiredDepth,
                                                   SkPoint* result) const {
    const SkPoint& norm = fNorms[edgeIdx];

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

    fNorms.setReserve(path.countPoints());

    // TODO: is there a faster way to extract the points from the path? Perhaps
    // get all the points via a new entry point, transform them all in bulk
    // and then walk them to find duplicates?
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, true, true)) != SkPath::kDone_Verb) {
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

    if (this->numPts() < 2) {
        return false;
    }

    // check if last point is a duplicate of the first point. If so, remove it.
    if (duplicate_pt(fPts[this->numPts()-1], fPts[0])) {
        this->popLastPt();
        fNorms.pop();
    }

    SkASSERT(fPts.count() == fNorms.count()+1);
    if (this->numPts() >= 3) {
        if (abs_dist_from_line(fPts.top(), fNorms.top(), fPts[0]) < kClose) {
            // The last point is on the line from the second to last to the first point.
            this->popLastPt();
            fNorms.pop();
        }

        *fNorms.push() = fPts[0] - fPts.top();
        SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fNorms.top());
        SkASSERT(len > 0.0f);
        SkASSERT(fPts.count() == fNorms.count());
    }

    if (this->numPts() >= 3 && abs_dist_from_line(fPts[0], fNorms.top(), fPts[1]) < kClose) {
        // The first point is on the line from the last to the second.
        this->popFirstPtShuffle();
        fNorms.removeShuffle(0);
        fNorms[0] = fPts[1] - fPts[0];
        SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fNorms[0]);
        SkASSERT(len > 0.0f);
        SkASSERT(SkScalarNearlyEqual(1.0f, fNorms[0].length()));
    }

    if (this->numPts() >= 3) {
        // Check the cross product of the final trio
        SkScalar cross = SkPoint::CrossProduct(fNorms[0], fNorms.top());
        if (cross > 0.0f) {
            fSide = SkPoint::kRight_Side;
        } else {
            fSide = SkPoint::kLeft_Side;
        }

        // Make all the normals face outwards rather than along the edge
        for (int cur = 0; cur < fNorms.count(); ++cur) {
            fNorms[cur].setOrthog(fNorms[cur], fSide);
            SkASSERT(SkScalarNearlyEqual(1.0f, fNorms[cur].length()));
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
        for (int cur = 0; cur < fNorms.count(); ++cur) {
            fNorms[cur].setOrthog(fNorms[cur], fSide);
            SkASSERT(SkScalarNearlyEqual(1.0f, fNorms[cur].length()));
        }

        fNorms.push(SkPoint::Make(-fNorms[0].fX, -fNorms[0].fY));
        // we won't actually use the bisectors, so just push zeroes
        fBisectors.push(SkPoint::Make(0.0, 0.0));
        fBisectors.push(SkPoint::Make(0.0, 0.0));
    } else {
        return false;
    }

    fCandidateVerts.setReserve(this->numPts());
    fInitialRing.setReserve(this->numPts());
    for (int i = 0; i < this->numPts(); ++i) {
        fInitialRing.addIdx(i, i);
    }
    fInitialRing.init(fNorms, fBisectors);

    this->validate();
    return true;
}

GrAAConvexTessellator::Ring* GrAAConvexTessellator::getNextRing(Ring* lastRing) {
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    Ring* ring = *fRings.push() = new Ring;
    ring->setReserve(fInitialRing.numPts());
    ring->rewind();
    return ring;
#else
    // Flip flop back and forth between fRings[0] & fRings[1]
    int nextRing = (lastRing == &fRings[0]) ? 1 : 0;
    fRings[nextRing].setReserve(fInitialRing.numPts());
    fRings[nextRing].rewind();
    return &fRings[nextRing];
#endif
}

void GrAAConvexTessellator::fanRing(const Ring& ring) {
    // fan out from point 0
    int startIdx = ring.index(0);
    for (int cur = ring.numPts() - 2; cur >= 0; --cur) {
        this->addTri(startIdx, ring.index(cur), ring.index(cur + 1));
    }
}

void GrAAConvexTessellator::createOuterRing(const Ring& previousRing, SkScalar outset,
                                            SkScalar coverage, Ring* nextRing) {
    const int numPts = previousRing.numPts();
    if (numPts == 0) {
        return;
    }

    int prev = numPts - 1;
    int lastPerpIdx = -1, firstPerpIdx = -1;

    const SkScalar outsetSq = outset * outset;
    SkScalar miterLimitSq = outset * fMiterLimit;
    miterLimitSq = miterLimitSq * miterLimitSq;
    for (int cur = 0; cur < numPts; ++cur) {
        int originalIdx = previousRing.index(cur);
        // For each vertex of the original polygon we add at least two points to the
        // outset polygon - one extending perpendicular to each impinging edge. Connecting these
        // two points yields a bevel join. We need one additional point for a mitered join, and
        // a round join requires one or more points depending upon curvature.

        // The perpendicular point for the last edge
        SkPoint normal1 = previousRing.norm(prev);
        SkPoint perp1 = normal1;
        perp1.scale(outset);
        perp1 += this->point(originalIdx);

        // The perpendicular point for the next edge.
        SkPoint normal2 = previousRing.norm(cur);
        SkPoint perp2 = normal2;
        perp2.scale(outset);
        perp2 += fPts[originalIdx];

        CurveState curve = fCurveState[originalIdx];

        // We know it isn't a duplicate of the prior point (since it and this
        // one are just perpendicular offsets from the non-merged polygon points)
        int perp1Idx = this->addPt(perp1, -outset, coverage, false, curve);
        nextRing->addIdx(perp1Idx, originalIdx);

        int perp2Idx;
        // For very shallow angles all the corner points could fuse.
        if (duplicate_pt(perp2, this->point(perp1Idx))) {
            perp2Idx = perp1Idx;
        } else {
            perp2Idx = this->addPt(perp2, -outset, coverage, false, curve);
        }

        if (perp2Idx != perp1Idx) {
            if (curve == kCurve_CurveState) {
                // bevel or round depending upon curvature
                SkScalar dotProd = normal1.dot(normal2);
                if (dotProd < kRoundCapThreshold) {
                    // Currently we "round" by creating a single extra point, which produces
                    // good results for common cases. For thick strokes with high curvature, we will
                    // need to add more points; for the time being we simply fall back to software
                    // rendering for thick strokes.
                    SkPoint miter = previousRing.bisector(cur);
                    miter.setLength(-outset);
                    miter += fPts[originalIdx];

                    // For very shallow angles all the corner points could fuse
                    if (!duplicate_pt(miter, this->point(perp1Idx))) {
                        int miterIdx;
                        miterIdx = this->addPt(miter, -outset, coverage, false, kSharp_CurveState);
                        nextRing->addIdx(miterIdx, originalIdx);
                        // The two triangles for the corner
                        this->addTri(originalIdx, perp1Idx, miterIdx);
                        this->addTri(originalIdx, miterIdx, perp2Idx);
                    }
                } else {
                    this->addTri(originalIdx, perp1Idx, perp2Idx);
                }
            } else {
                switch (fJoin) {
                    case SkPaint::Join::kMiter_Join: {
                        // The bisector outset point
                        SkPoint miter = previousRing.bisector(cur);
                        SkScalar dotProd = normal1.dot(normal2);
                        SkScalar sinHalfAngleSq = SkScalarHalf(SK_Scalar1 + dotProd);
                        SkScalar lengthSq = outsetSq / sinHalfAngleSq;
                        if (lengthSq > miterLimitSq) {
                            // just bevel it
                            this->addTri(originalIdx, perp1Idx, perp2Idx);
                            break;
                        }
                        miter.setLength(-SkScalarSqrt(lengthSq));
                        miter += fPts[originalIdx];

                        // For very shallow angles all the corner points could fuse
                        if (!duplicate_pt(miter, this->point(perp1Idx))) {
                            int miterIdx;
                            miterIdx = this->addPt(miter, -outset, coverage, false, 
                                                   kSharp_CurveState);
                            nextRing->addIdx(miterIdx, originalIdx);
                            // The two triangles for the corner
                            this->addTri(originalIdx, perp1Idx, miterIdx);
                            this->addTri(originalIdx, miterIdx, perp2Idx);
                        }
                        break;
                    }
                    case SkPaint::Join::kBevel_Join:
                        this->addTri(originalIdx, perp1Idx, perp2Idx);
                        break;
                    default:
                        // kRound_Join is unsupported for now. GrAALinearizingConvexPathRenderer is
                        // only willing to draw mitered or beveled, so we should never get here.
                        SkASSERT(false);
                }
            }

            nextRing->addIdx(perp2Idx, originalIdx);
        }

        if (0 == cur) {
            // Store the index of the first perpendicular point to finish up
            firstPerpIdx = perp1Idx;
            SkASSERT(-1 == lastPerpIdx);
        } else {
            // The triangles for the previous edge
            int prevIdx = previousRing.index(prev);
            this->addTri(prevIdx, perp1Idx, originalIdx);
            this->addTri(prevIdx, lastPerpIdx, perp1Idx);
        }

        // Track the last perpendicular outset point so we can construct the
        // trailing edge triangles.
        lastPerpIdx = perp2Idx;
        prev = cur;
    }

    // pick up the final edge rect
    int lastIdx = previousRing.index(numPts - 1);
    this->addTri(lastIdx, firstPerpIdx, previousRing.index(0));
    this->addTri(lastIdx, lastPerpIdx, firstPerpIdx);

    this->validate();
}

// Something went wrong in the creation of the next ring. If we're filling the shape, just go ahead
// and fan it.
void GrAAConvexTessellator::terminate(const Ring& ring) {
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
bool GrAAConvexTessellator::createInsetRing(const Ring& lastRing, Ring* nextRing,
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
        bool result = intersect(this->point(lastRing.index(cur)),  lastRing.bisector(cur),
                                this->point(lastRing.index(next)), lastRing.bisector(next),
                                &t);
        if (!result) {
            continue;
        }
        SkScalar dist = -t * lastRing.norm(cur).dot(lastRing.bisector(cur));

        if (minDist > dist) {
            minDist = dist;
            minT = t;
            minEdgeIdx = cur;
        }
    }

    if (minEdgeIdx == -1) {
        return false;
    }
    SkPoint newPt = lastRing.bisector(minEdgeIdx);
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
                                      lastRing.bisector(0),
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
                                          lastRing.bisector(cur),
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
                                       lastRing.bisector(cur),
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
    SkASSERT(!fBisectors.count() || fBisectors.count() == fNorms.count());
}

//////////////////////////////////////////////////////////////////////////////
void GrAAConvexTessellator::Ring::init(const GrAAConvexTessellator& tess) {
    this->computeNormals(tess);
    this->computeBisectors(tess);
}

void GrAAConvexTessellator::Ring::init(const SkTDArray<SkVector>& norms,
                                       const SkTDArray<SkVector>& bisectors) {
    for (int i = 0; i < fPts.count(); ++i) {
        fPts[i].fNorm = norms[i];
        fPts[i].fBisector = bisectors[i];
    }
}

// Compute the outward facing normal at each vertex.
void GrAAConvexTessellator::Ring::computeNormals(const GrAAConvexTessellator& tess) {
    for (int cur = 0; cur < fPts.count(); ++cur) {
        int next = (cur + 1) % fPts.count();

        fPts[cur].fNorm = tess.point(fPts[next].fIndex) - tess.point(fPts[cur].fIndex);
        SkPoint::Normalize(&fPts[cur].fNorm);
        fPts[cur].fNorm.setOrthog(fPts[cur].fNorm, tess.side());
    }
}

void GrAAConvexTessellator::Ring::computeBisectors(const GrAAConvexTessellator& tess) {
    int prev = fPts.count() - 1;
    for (int cur = 0; cur < fPts.count(); prev = cur, ++cur) {
        fPts[cur].fBisector = fPts[cur].fNorm + fPts[prev].fNorm;
        if (!fPts[cur].fBisector.normalize()) {
            SkASSERT(SkPoint::kLeft_Side == tess.side() || SkPoint::kRight_Side == tess.side());
            fPts[cur].fBisector.setOrthog(fPts[cur].fNorm, (SkPoint::Side)-tess.side());
            SkVector other;
            other.setOrthog(fPts[prev].fNorm, tess.side());
            fPts[cur].fBisector += other;
            SkAssertResult(fPts[cur].fBisector.normalize());
        } else {
            fPts[cur].fBisector.negate();      // make the bisector face in
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
#ifdef SK_DEBUG
// Is this ring convex?
bool GrAAConvexTessellator::Ring::isConvex(const GrAAConvexTessellator& tess) const {
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
    if (this->numPts() > 0 && duplicate_pt(p, this->lastPoint())) {
        return;
    }

    SkASSERT(fPts.count() <= 1 || fPts.count() == fNorms.count()+1);
    if (this->numPts() >= 2 && abs_dist_from_line(fPts.top(), fNorms.top(), p) < kClose) {
        // The old last point is on the line from the second to last to the new point
        this->popLastPt();
        fNorms.pop();
        // double-check that the new last point is not a duplicate of the new point. In an ideal
        // world this wouldn't be necessary (since it's only possible for non-convex paths), but
        // floating point precision issues mean it can actually happen on paths that were
        // determined to be convex.
        if (duplicate_pt(p, this->lastPoint())) {
            return;
        }
    }
    SkScalar initialRingCoverage = (SkStrokeRec::kFill_Style == fStyle) ? 0.5f : 1.0f;
    this->addPt(p, 0.0f, initialRingCoverage, false, curve);
    if (this->numPts() > 1) {
        *fNorms.push() = fPts.top() - fPts[fPts.count()-2];
        SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fNorms.top());
        SkASSERT(len > 0.0f);
        SkASSERT(SkScalarNearlyEqual(1.0f, fNorms.top().length()));
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
static const SkScalar kPointRadius = 0.02f;
static const SkScalar kArrowStrokeWidth = 0.0f;
static const SkScalar kArrowLength = 0.2f;
static const SkScalar kEdgeTextSize = 0.1f;
static const SkScalar kPointTextSize = 0.02f;

static void draw_point(SkCanvas* canvas, const SkPoint& p, SkScalar paramValue, bool stroke) {
    SkPaint paint;
    SkASSERT(paramValue <= 1.0f);
    int gs = int(255*paramValue);
    paint.setARGB(255, gs, gs, gs);

    canvas->drawCircle(p.fX, p.fY, kPointRadius, paint);

    if (stroke) {
        SkPaint stroke;
        stroke.setColor(SK_ColorYELLOW);
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(kPointRadius/3.0f);
        canvas->drawCircle(p.fX, p.fY, kPointRadius, stroke);
    }
}

static void draw_line(SkCanvas* canvas, const SkPoint& p0, const SkPoint& p1, SkColor color) {
    SkPaint p;
    p.setColor(color);

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

void GrAAConvexTessellator::Ring::draw(SkCanvas* canvas, const GrAAConvexTessellator& tess) const {
    SkPaint paint;
    paint.setTextSize(kEdgeTextSize);

    for (int cur = 0; cur < fPts.count(); ++cur) {
        int next = (cur + 1) % fPts.count();

        draw_line(canvas,
                  tess.point(fPts[cur].fIndex),
                  tess.point(fPts[next].fIndex),
                  SK_ColorGREEN);

        SkPoint mid = tess.point(fPts[cur].fIndex) + tess.point(fPts[next].fIndex);
        mid.scale(0.5f);

        if (fPts.count()) {
            draw_arrow(canvas, mid, fPts[cur].fNorm, kArrowLength, SK_ColorRED);
            mid.fX += (kArrowLength/2) * fPts[cur].fNorm.fX;
            mid.fY += (kArrowLength/2) * fPts[cur].fNorm.fY;
        }

        SkString num;
        num.printf("%d", this->origEdgeID(cur));
        canvas->drawString(num, mid.fX, mid.fY, paint);

        if (fPts.count()) {
            draw_arrow(canvas, tess.point(fPts[cur].fIndex), fPts[cur].fBisector,
                       kArrowLength, SK_ColorBLUE);
        }
    }
}

void GrAAConvexTessellator::draw(SkCanvas* canvas) const {
    for (int i = 0; i < fIndices.count(); i += 3) {
        SkASSERT(fIndices[i] < this->numPts()) ;
        SkASSERT(fIndices[i+1] < this->numPts()) ;
        SkASSERT(fIndices[i+2] < this->numPts()) ;

        draw_line(canvas,
                  this->point(this->fIndices[i]), this->point(this->fIndices[i+1]),
                  SK_ColorBLACK);
        draw_line(canvas,
                  this->point(this->fIndices[i+1]), this->point(this->fIndices[i+2]),
                  SK_ColorBLACK);
        draw_line(canvas,
                  this->point(this->fIndices[i+2]), this->point(this->fIndices[i]),
                  SK_ColorBLACK);
    }

    fInitialRing.draw(canvas, *this);
    for (int i = 0; i < fRings.count(); ++i) {
        fRings[i]->draw(canvas, *this);
    }

    for (int i = 0; i < this->numPts(); ++i) {
        draw_point(canvas,
                   this->point(i), 0.5f + (this->depth(i)/(2 * kAntialiasingRadius)),
                   !this->movable(i));

        SkPaint paint;
        paint.setTextSize(kPointTextSize);
        paint.setTextAlign(SkPaint::kCenter_Align);
        if (this->depth(i) <= -kAntialiasingRadius) {
            paint.setColor(SK_ColorWHITE);
        }

        SkString num;
        num.printf("%d", i);
        canvas->drawString(num,
                         this->point(i).fX, this->point(i).fY+(kPointRadius/2.0f),
                         paint);
    }
}

#endif
