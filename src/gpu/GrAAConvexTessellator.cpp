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

// Next steps:
//  use in AAConvexPathRenderer
//  add an interactive sample app slide
//  add debug check that all points are suitably far apart
//  test more degenerate cases

// The tolerance for fusing vertices and eliminating colinear lines (It is in device space).
static const SkScalar kClose = (SK_Scalar1 / 16);
static const SkScalar kCloseSqd = SkScalarMul(kClose, kClose);

static SkScalar intersect(const SkPoint& p0, const SkPoint& n0,
                          const SkPoint& p1, const SkPoint& n1) {
    const SkPoint v = p1 - p0;

    SkScalar perpDot = n0.fX * n1.fY - n0.fY * n1.fX;
    return (v.fX * n1.fY - v.fY * n1.fX) / perpDot;
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
                                 bool movable) {
    this->validate();

    int index = fPts.count();
    *fPts.push() = pt;
    *fDepths.push() = depth;
    *fMovable.push() = movable;

    this->validate();
    return index;
}

void GrAAConvexTessellator::popLastPt() {
    this->validate();

    fPts.pop();
    fDepths.pop();
    fMovable.pop();

    this->validate();
}

void GrAAConvexTessellator::popFirstPtShuffle() {
    this->validate();

    fPts.removeShuffle(0);
    fDepths.removeShuffle(0);
    fMovable.removeShuffle(0);

    this->validate();
}

void GrAAConvexTessellator::updatePt(int index,
                                     const SkPoint& pt,
                                     SkScalar depth) {
    this->validate();
    SkASSERT(fMovable[index]);

    fPts[index] = pt;
    fDepths[index] = depth;
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
    fDepths.rewind();
    fMovable.rewind();
    fIndices.rewind();
    fNorms.rewind();
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
        fBisectors[cur].normalize();
        fBisectors[cur].negate();      // make the bisector face in

        SkASSERT(SkScalarNearlyEqual(1.0f, fBisectors[cur].length()));
    }
}

// The general idea here is to, conceptually, start with the original polygon and slide
// the vertices along the bisectors until the first intersection. At that
// point two of the edges collapse and the process repeats on the new polygon.
// The polygon state is captured in the Ring class while the GrAAConvexTessellator
// controls the iteration. The CandidateVerts holds the formative points for the
// next ring.
bool GrAAConvexTessellator::tessellate(const SkMatrix& m, const SkPath& path) {
    static const int kMaxNumRings = 8;

    SkDEBUGCODE(fShouldCheckDepths = true;)

    if (!this->extractFromPath(m, path)) {
        return false;
    }

    this->createOuterRing();

    // the bisectors are only needed for the computation of the outer ring
    fBisectors.rewind();

    Ring* lastRing = &fInitialRing;
    int i;
    for (i = 0; i < kMaxNumRings; ++i) {
        Ring* nextRing = this->getNextRing(lastRing);

        if (this->createInsetRing(*lastRing, nextRing)) {
            break;
        }

        nextRing->init(*this);
        lastRing = nextRing;
    }

    if (kMaxNumRings == i) {
        // If we've exceeded the amount of time we want to throw at this, set
        // the depth of all points in the final ring to 'fTargetDepth' and
        // create a fan.
        this->terminate(*lastRing);
        SkDEBUGCODE(fShouldCheckDepths = false;)
    }

#ifdef SK_DEBUG
    this->validate();
    if (fShouldCheckDepths) {
        SkDEBUGCODE(this->checkAllDepths();)
    }
#endif
    return true;
}

SkScalar GrAAConvexTessellator::computeDepthFromEdge(int edgeIdx, const SkPoint& p) const {
    SkASSERT(edgeIdx < fNorms.count());

    SkPoint v = p - fPts[edgeIdx];
    SkScalar depth = -fNorms[edgeIdx].dot(v);
    SkASSERT(depth >= 0.0f);
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
        SkASSERT(startIdx < fNorms.count());
        newP = fPts[startIdx];
    } else if (t > 0.0f) {
        SkASSERT(t < 0.0f);
        newP = bisector;
        newP.scale(t);
        newP += fPts[startIdx];
    } else {
        return false;
    }

    // Then offset along the bisector from that point the correct distance
    t = -desiredDepth / bisector.dot(norm);
    SkASSERT(t > 0.0f);
    *result = bisector;
    result->scale(t);
    *result += newP;


    return true;
}

bool GrAAConvexTessellator::extractFromPath(const SkMatrix& m, const SkPath& path) {
    SkASSERT(SkPath::kLine_SegmentMask == path.getSegmentMasks());
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

    SkScalar minCross = SK_ScalarMax, maxCross = -SK_ScalarMax;

    // TODO: is there a faster way to extract the points from the path? Perhaps
    // get all the points via a new entry point, transform them all in bulk
    // and then walk them to find duplicates?
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                m.mapPoints(&pts[1], 1);
                if (this->numPts() > 0 && duplicate_pt(pts[1], this->lastPoint())) {
                    continue;
                }

                SkASSERT(fPts.count() <= 1 || fPts.count() == fNorms.count()+1);
                if (this->numPts() >= 2 && 
                    abs_dist_from_line(fPts.top(), fNorms.top(), pts[1]) < kClose) {
                    // The old last point is on the line from the second to last to the new point
                    this->popLastPt();
                    fNorms.pop();
                }

                this->addPt(pts[1], 0.0f, false);
                if (this->numPts() > 1) {
                    *fNorms.push() = fPts.top() - fPts[fPts.count()-2];
                    SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fNorms.top());
                    SkASSERT(len > 0.0f);
                    SkASSERT(SkScalarNearlyEqual(1.0f, fNorms.top().length()));
                }

                if (this->numPts() >= 3) {
                    int cur = this->numPts()-1;
                    SkScalar cross = SkPoint::CrossProduct(fNorms[cur-1], fNorms[cur-2]);
                    maxCross = SkTMax(maxCross, cross);
                    minCross = SkTMin(minCross, cross);
                }
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kCubic_Verb:
                SkASSERT(false);
                break;
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }

    if (this->numPts() < 3) {
        return false;
    }

    // check if last point is a duplicate of the first point. If so, remove it.
    if (duplicate_pt(fPts[this->numPts()-1], fPts[0])) {
        this->popLastPt();
        fNorms.pop();
    }

    SkASSERT(fPts.count() == fNorms.count()+1);
    if (this->numPts() >= 3 &&
        abs_dist_from_line(fPts.top(), fNorms.top(), fPts[0]) < kClose) {
        // The last point is on the line from the second to last to the first point.
        this->popLastPt();
        fNorms.pop();
    }

    if (this->numPts() < 3) {
        return false;
    }

    *fNorms.push() = fPts[0] - fPts.top();
    SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fNorms.top());
    SkASSERT(len > 0.0f);
    SkASSERT(fPts.count() == fNorms.count());

    if (abs_dist_from_line(fPts[0], fNorms.top(), fPts[1]) < kClose) {
        // The first point is on the line from the last to the second.
        this->popFirstPtShuffle();
        fNorms.removeShuffle(0);
        fNorms[0] = fPts[1] - fPts[0];
        SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fNorms[0]);
        SkASSERT(len > 0.0f);
        SkASSERT(SkScalarNearlyEqual(1.0f, fNorms[0].length()));
    }

    if (this->numPts() < 3) {
        return false;
    }

    // Check the cross produce of the final trio
    SkScalar cross = SkPoint::CrossProduct(fNorms[0], fNorms.top());
    maxCross = SkTMax(maxCross, cross);
    minCross = SkTMin(minCross, cross);

    if (maxCross > 0.0f) {
        SkASSERT(minCross >= 0.0f);
        fSide = SkPoint::kRight_Side;
    } else {
        SkASSERT(minCross <= 0.0f);
        fSide = SkPoint::kLeft_Side;
    }

    // Make all the normals face outwards rather than along the edge
    for (int cur = 0; cur < fNorms.count(); ++cur) {
        fNorms[cur].setOrthog(fNorms[cur], fSide);
        SkASSERT(SkScalarNearlyEqual(1.0f, fNorms[cur].length()));
    }

    this->computeBisectors();

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
    Ring* ring = *fRings.push() = SkNEW(Ring);
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
    for (int cur = 1; cur < ring.numPts()-1; ++cur) {
        this->addTri(ring.index(0), ring.index(cur), ring.index(cur+1));
    }
}

void GrAAConvexTessellator::createOuterRing() {
    // For now, we're only generating one outer ring (at the start). This
    // could be relaxed for stroking use cases.
    SkASSERT(0 == fIndices.count());  
    SkASSERT(fPts.count() == fNorms.count());

    const int numPts = fPts.count();

    // For each vertex of the original polygon we add three points to the 
    // outset polygon - one extending perpendicular to each impinging edge
    // and one along the bisector. Two triangles are added for each corner
    // and two are added along each edge.
    int prev = numPts - 1;
    int lastPerpIdx = -1, firstPerpIdx = -1, newIdx0, newIdx1, newIdx2;
    for (int cur = 0; cur < numPts; ++cur) {
        // The perpendicular point for the last edge
        SkPoint temp = fNorms[prev];
        temp.scale(fTargetDepth);
        temp += fPts[cur];

        // We know it isn't a duplicate of the prior point (since it and this
        // one are just perpendicular offsets from the non-merged polygon points)
        newIdx0 = this->addPt(temp, -fTargetDepth, false);

        // The bisector outset point
        temp = fBisectors[cur];
        temp.scale(-fTargetDepth);  // the bisectors point in
        temp += fPts[cur];

        // For very shallow angles all the corner points could fuse
        if (duplicate_pt(temp, this->point(newIdx0))) {
            newIdx1 = newIdx0;
        } else {
            newIdx1 = this->addPt(temp, -fTargetDepth, false);
        }

        // The perpendicular point for the next edge.
        temp = fNorms[cur];
        temp.scale(fTargetDepth);
        temp += fPts[cur];

        // For very shallow angles all the corner points could fuse.
        if (duplicate_pt(temp, this->point(newIdx1))) {
            newIdx2 = newIdx1;
        } else {
            newIdx2 = this->addPt(temp, -fTargetDepth, false);
        }

        if (0 == cur) {
            // Store the index of the first perpendicular point to finish up
            firstPerpIdx = newIdx0;
            SkASSERT(-1 == lastPerpIdx);
        } else {
            // The triangles for the previous edge
            this->addTri(prev, newIdx0, cur);
            this->addTri(prev, lastPerpIdx, newIdx0);
        }

        // The two triangles for the corner
        this->addTri(cur, newIdx0, newIdx1);
        this->addTri(cur, newIdx1, newIdx2);

        prev = cur;
        // Track the last perpendicular outset point so we can construct the
        // trailing edge triangles.
        lastPerpIdx = newIdx2;
    }

    // pick up the final edge rect
    this->addTri(numPts-1, firstPerpIdx, 0);
    this->addTri(numPts-1, lastPerpIdx, firstPerpIdx);

    this->validate();
}

// Something went wrong in the creation of the next ring. Mark the last good
// ring as being at the desired depth and fan it.
void GrAAConvexTessellator::terminate(const Ring& ring) {
    for (int i = 0; i < ring.numPts(); ++i) {
        fDepths[ring.index(i)] = fTargetDepth;
    }

    this->fanRing(ring);
}

// return true when processing is complete
bool GrAAConvexTessellator::createInsetRing(const Ring& lastRing, Ring* nextRing) {
    bool done = false;

    fCandidateVerts.rewind();

    // Loop through all the points in the ring and find the intersection with the smallest depth
    SkScalar minDist = SK_ScalarMax, minT = 0.0f;
    int minEdgeIdx = -1;

    for (int cur = 0; cur < lastRing.numPts(); ++cur) {
        int next = (cur + 1) % lastRing.numPts();

        SkScalar t = intersect(this->point(lastRing.index(cur)),  lastRing.bisector(cur),
                               this->point(lastRing.index(next)), lastRing.bisector(next));
        SkScalar dist = -t * lastRing.norm(cur).dot(lastRing.bisector(cur));

        if (minDist > dist) {
            minDist = dist;
            minT = t;
            minEdgeIdx = cur;
        }
    }

    SkPoint newPt = lastRing.bisector(minEdgeIdx);
    newPt.scale(minT);
    newPt += this->point(lastRing.index(minEdgeIdx));

    SkScalar depth = this->computeDepthFromEdge(lastRing.origEdgeID(minEdgeIdx), newPt);
    if (depth >= fTargetDepth) {
        // None of the bisectors intersect before reaching the desired depth.
        // Just step them all to the desired depth
        depth = fTargetDepth;
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
        SkDEBUGCODE(fShouldCheckDepths = false;)
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
            SkDEBUGCODE(fShouldCheckDepths = false;)
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
        SkDEBUGCODE(fShouldCheckDepths = false;)
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
            dst[cur] = dst[cur-1] = fCandidateVerts.fuseWithBoth();
        }
    }

    // Fold the new ring's points into the global pool
    for (int i = 0; i < fCandidateVerts.numPts(); ++i) {
        int newIdx;
        if (fCandidateVerts.needsToBeNew(i)) {
            // if the originating index is still valid then this point wasn't 
            // fused (and is thus movable)
            newIdx = this->addPt(fCandidateVerts.point(i), depth,
                                 fCandidateVerts.originatingIdx(i) != -1);
        } else {
            SkASSERT(fCandidateVerts.originatingIdx(i) != -1);
            this->updatePt(fCandidateVerts.originatingIdx(i), fCandidateVerts.point(i), depth);
            newIdx = fCandidateVerts.originatingIdx(i);
        }

        nextRing->addIdx(newIdx, fCandidateVerts.origEdge(i));
    }

    // 'dst' currently has indices into the ring. Remap these to be indices
    // into the global pool since the triangulation operates in that space.
    for (int i = 0; i < dst.count(); ++i) {
        dst[i] = nextRing->index(dst[i]);
    }

    for (int cur = 0; cur < lastRing.numPts(); ++cur) {
        int next = (cur + 1) % lastRing.numPts();

        this->addTri(lastRing.index(cur), lastRing.index(next), dst[next]);
        this->addTri(lastRing.index(cur), dst[next], dst[cur]);
    }

    if (done) {
        this->fanRing(*nextRing);
    }

    if (nextRing->numPts() < 3) {
        done = true;
    }

    return done;
}

void GrAAConvexTessellator::validate() const {
    SkASSERT(fPts.count() == fDepths.count());
    SkASSERT(fPts.count() == fMovable.count());
    SkASSERT(0 == (fIndices.count() % 3));
}

//////////////////////////////////////////////////////////////////////////////
void GrAAConvexTessellator::Ring::init(const GrAAConvexTessellator& tess) {
    this->computeNormals(tess);
    this->computeBisectors();
    SkASSERT(this->isConvex(tess));
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
        SkDEBUGCODE(SkScalar len =) SkPoint::Normalize(&fPts[cur].fNorm);
        SkASSERT(len > 0.0f);
        fPts[cur].fNorm.setOrthog(fPts[cur].fNorm, tess.side());

        SkASSERT(SkScalarNearlyEqual(1.0f, fPts[cur].fNorm.length()));
    }
}

void GrAAConvexTessellator::Ring::computeBisectors() {
    int prev = fPts.count() - 1;
    for (int cur = 0; cur < fPts.count(); prev = cur, ++cur) {
        fPts[cur].fBisector = fPts[cur].fNorm + fPts[prev].fNorm;
        fPts[cur].fBisector.normalize();
        fPts[cur].fBisector.negate();      // make the bisector face in

        SkASSERT(SkScalarNearlyEqual(1.0f, fPts[cur].fBisector.length()));
    }    
}

//////////////////////////////////////////////////////////////////////////////
#ifdef SK_DEBUG
// Is this ring convex?
bool GrAAConvexTessellator::Ring::isConvex(const GrAAConvexTessellator& tess) const {
    if (fPts.count() < 3) {
        return false;
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

    return (maxDot > 0.0f) == (minDot >= 0.0f);
}

static SkScalar capsule_depth(const SkPoint& p0, const SkPoint& p1,
                              const SkPoint& test, SkPoint::Side side,
                              int* sign) {
    *sign = -1;
    SkPoint edge = p1 - p0;
    SkScalar len = SkPoint::Normalize(&edge);

    SkPoint testVec = test - p0;

    SkScalar d0 = edge.dot(testVec);
    if (d0 < 0.0f) {
        return SkPoint::Distance(p0, test);
    }
    if (d0 > len) {
        return SkPoint::Distance(p1, test);
    }

    SkScalar perpDist = testVec.fY * edge.fX - testVec.fX * edge.fY;
    if (SkPoint::kRight_Side == side) {
        perpDist = -perpDist;
    }

    if (perpDist < 0.0f) {
        perpDist = -perpDist;
    } else {
        *sign = 1;
    }
    return perpDist;
}

SkScalar GrAAConvexTessellator::computeRealDepth(const SkPoint& p) const {
    SkScalar minDist = SK_ScalarMax;
    int closestSign, sign;

    for (int edge = 0; edge < fNorms.count(); ++edge) {
        SkScalar dist = capsule_depth(this->point(edge),
                                      this->point((edge+1) % fNorms.count()),
                                      p, fSide, &sign);
        SkASSERT(dist >= 0.0f);

        if (minDist > dist) {
            minDist = dist;
            closestSign = sign;
        }
    }

    return closestSign * minDist;
}

// Verify that the incrementally computed depths are close to the actual depths.
void GrAAConvexTessellator::checkAllDepths() const {
    for (int cur = 0; cur < this->numPts(); ++cur) {
        SkScalar realDepth = this->computeRealDepth(this->point(cur));
        SkScalar computedDepth = this->depth(cur);
        SkASSERT(SkScalarNearlyEqual(realDepth, computedDepth, 0.01f));
    }
}
#endif

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
        canvas->drawText(num.c_str(), num.size(), mid.fX, mid.fY, paint);

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
                   this->point(i), 0.5f + (this->depth(i)/(2*fTargetDepth)), 
                   !this->movable(i));

        SkPaint paint;
        paint.setTextSize(kPointTextSize);
        paint.setTextAlign(SkPaint::kCenter_Align);
        if (this->depth(i) <= -fTargetDepth) {
            paint.setColor(SK_ColorWHITE);
        }

        SkString num;
        num.printf("%d", i);
        canvas->drawText(num.c_str(), num.size(), 
                         this->point(i).fX, this->point(i).fY+(kPointRadius/2.0f), 
                         paint);
    }
}

#endif

