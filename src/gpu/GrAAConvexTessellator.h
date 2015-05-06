/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAConvexTessellator_DEFINED
#define GrAAConvexTessellator_DEFINED

#include "SkColor.h"
#include "SkPoint.h"
#include "SkScalar.h"
#include "SkTDArray.h"

class SkCanvas;
class SkMatrix;
class SkPath;

//#define GR_AA_CONVEX_TESSELLATOR_VIZ 1

class GrAAConvexTessellator;

// The AAConvexTessellator holds the global pool of points and the triangulation
// that connects them. It also drives the tessellation process.
// The outward facing normals of the original polygon are stored (in 'fNorms') to service
// computeDepthFromEdge requests.
class GrAAConvexTessellator {
public:
    GrAAConvexTessellator(SkScalar targetDepth = 0.5f)
        : fSide(SkPoint::kOn_Side)
        , fTargetDepth(targetDepth) {
    }

    void setTargetDepth(SkScalar targetDepth) { fTargetDepth = targetDepth; }
    SkScalar targetDepth() const { return fTargetDepth; }

    SkPoint::Side side() const { return fSide; }

    bool tessellate(const SkMatrix& m, const SkPath& path);

    // The next five should only be called after tessellate to extract the result
    int numPts() const { return fPts.count(); }
    int numIndices() const { return fIndices.count(); }

    const SkPoint& lastPoint() const { return fPts.top(); }
    const SkPoint& point(int index) const { return fPts[index]; }
    int index(int index) const { return fIndices[index]; }
    SkScalar depth(int index) const {return fDepths[index]; }

#if GR_AA_CONVEX_TESSELLATOR_VIZ
    void draw(SkCanvas* canvas) const;
#endif

    // The tessellator can be reused for multiple paths by rewinding in between
    void rewind();

private:
    // CandidateVerts holds the vertices for the next ring while they are 
    // being generated. Its main function is to de-dup the points.
    class CandidateVerts {
    public:
        void setReserve(int numPts) { fPts.setReserve(numPts); }
        void rewind() { fPts.rewind(); }

        int numPts() const { return fPts.count(); }

        const SkPoint& lastPoint() const { return fPts.top().fPt; }
        const SkPoint& firstPoint() const { return fPts[0].fPt; }
        const SkPoint& point(int index) const { return fPts[index].fPt; }

        int originatingIdx(int index) const { return fPts[index].fOriginatingIdx; }
        int origEdge(int index) const { return fPts[index].fOrigEdgeId; }
        bool needsToBeNew(int index) const { return fPts[index].fNeedsToBeNew; }

        int addNewPt(const SkPoint& newPt, int originatingIdx, int origEdge, bool needsToBeNew) {
            struct PointData* pt = fPts.push();
            pt->fPt = newPt;
            pt->fOrigEdgeId = origEdge;
            pt->fOriginatingIdx = originatingIdx;
            pt->fNeedsToBeNew = needsToBeNew;
            return fPts.count() - 1;
        }

        int fuseWithPrior(int origEdgeId) {
            fPts.top().fOrigEdgeId = origEdgeId;
            fPts.top().fOriginatingIdx = -1;
            fPts.top().fNeedsToBeNew = true;
            return fPts.count() - 1;
        }

        int fuseWithNext() {
            fPts[0].fOriginatingIdx = -1;
            fPts[0].fNeedsToBeNew = true;
            return 0;
        }

        int fuseWithBoth() {
            if (fPts.count() > 1) {
                fPts.pop();
            }

            fPts[0].fOriginatingIdx = -1;
            fPts[0].fNeedsToBeNew = true;
            return 0;
        }

    private:
        struct PointData {
            SkPoint fPt;
            int     fOriginatingIdx;
            int     fOrigEdgeId;
            bool    fNeedsToBeNew;
        };

        SkTDArray<struct PointData> fPts;
    };

    // The Ring holds a set of indices into the global pool that together define
    // a single polygon inset.
    class Ring {
    public:
        void setReserve(int numPts) { fPts.setReserve(numPts); }
        void rewind() { fPts.rewind(); }

        int numPts() const { return fPts.count(); }

        void addIdx(int index, int origEdgeId) {
            struct PointData* pt = fPts.push();
            pt->fIndex = index;
            pt->fOrigEdgeId = origEdgeId;
        }

        // init should be called after all the indices have been added (via addIdx)
        void init(const GrAAConvexTessellator& tess);
        void init(const SkTDArray<SkVector>& norms, const SkTDArray<SkVector>& bisectors);

        const SkPoint& norm(int index) const { return fPts[index].fNorm; }
        const SkPoint& bisector(int index) const { return fPts[index].fBisector; }
        int index(int index) const { return fPts[index].fIndex; }
        int origEdgeID(int index) const { return fPts[index].fOrigEdgeId; }

    #if GR_AA_CONVEX_TESSELLATOR_VIZ
        void draw(SkCanvas* canvas, const GrAAConvexTessellator& tess) const;
    #endif

    private:
        void computeNormals(const GrAAConvexTessellator& result);
        void computeBisectors();

        SkDEBUGCODE(bool isConvex(const GrAAConvexTessellator& tess) const;)

        struct PointData {
            SkPoint fNorm;
            SkPoint fBisector;
            int     fIndex;
            int     fOrigEdgeId;
        };

        SkTDArray<PointData> fPts;
    };

    bool movable(int index) const { return fMovable[index]; }

    // Movable points are those that can be slid along their bisector.
    // Basically, a point is immovable if it is part of the original
    // polygon or it results from the fusing of two bisectors.
    int addPt(const SkPoint& pt, SkScalar depth, bool movable);
    void popLastPt();
    void popFirstPtShuffle();

    void updatePt(int index, const SkPoint& pt, SkScalar depth);

    void addTri(int i0, int i1, int i2);

    void reservePts(int count) {
        fPts.setReserve(count);
        fDepths.setReserve(count);  
        fMovable.setReserve(count);
    }

    SkScalar computeDepthFromEdge(int edgeIdx, const SkPoint& p) const;

    bool computePtAlongBisector(int startIdx, const SkPoint& bisector,
                                int edgeIdx, SkScalar desiredDepth,
                                SkPoint* result) const;

    void terminate(const Ring& lastRing);

    // return false on failure/degenerate path
    bool extractFromPath(const SkMatrix& m, const SkPath& path);
    void computeBisectors();

    void fanRing(const Ring& ring);
    void createOuterRing();

    Ring* getNextRing(Ring* lastRing);

    bool createInsetRing(const Ring& lastRing, Ring* nextRing);

    void validate() const;


#ifdef SK_DEBUG
    SkScalar computeRealDepth(const SkPoint& p) const;
    void checkAllDepths() const;
#endif

    // fPts, fWeights & fMovable should always have the same # of elements
    SkTDArray<SkPoint>  fPts;
    SkTDArray<SkScalar> fDepths;
    // movable points are those that can be slid further along their bisector
    SkTDArray<bool>     fMovable;

    // The outward facing normals for the original polygon
    SkTDArray<SkVector> fNorms;
    // The inward facing bisector at each point in the original polygon. Only
    // needed for exterior ring creation and then handed off to the initial ring.
    SkTDArray<SkVector> fBisectors;
    SkPoint::Side       fSide;    // winding of the original polygon

    // The triangulation of the points
    SkTDArray<int>      fIndices;

    Ring                fInitialRing;
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    // When visualizing save all the rings
    SkTDArray<Ring*>    fRings;
#else
    Ring                fRings[2];
#endif
    CandidateVerts      fCandidateVerts;

    SkScalar            fTargetDepth;

    // If some goes wrong with the inset computation the tessellator will 
    // truncate the creation of the inset polygon. In this case the depth
    // check will complain.
    SkDEBUGCODE(bool fShouldCheckDepths;)
};


#endif

