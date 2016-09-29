/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAConvexTessellator_DEFINED
#define GrAAConvexTessellator_DEFINED

#include "SkColor.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkScalar.h"
#include "SkStrokeRec.h"
#include "SkTDArray.h"

class SkCanvas;
class SkMatrix;
class SkPath;

#define GR_AA_CONVEX_TESSELLATOR_VIZ 1

// device space distance which we inset / outset points in order to create the soft antialiased edge
static const SkScalar kAntialiasingRadius = 0.5f;

class GrAAConvexTessellator;

// The AAConvexTessellator holds the global pool of points and the triangulation
// that connects them. It also drives the tessellation process.
// The outward facing normals of the original polygon are stored (in 'fNorms') to service
// computeDepthFromEdge requests.
class GrAAConvexTessellator {
public:
    // Represents whether a given point is within a curve. A point is inside a curve only if it is
    // an interior point within a quad, cubic, or conic, or if it is the endpoint of a quad, cubic,
    // or conic with another curve meeting it at (more or less) the same angle.
    enum CurveState {
        // point is a sharp vertex
        kSharp_CurveState,
        // endpoint of a curve with the other side's curvature not yet determined
        kIndeterminate_CurveState,
        // point is in the interior of a curve
        kCurve_CurveState
    };

    GrAAConvexTessellator(SkStrokeRec::Style style = SkStrokeRec::kFill_Style,
                          SkScalar strokeWidth = -1.0f,
                          SkPaint::Join join = SkPaint::Join::kBevel_Join,
                          SkScalar miterLimit = 0.0f)
        : fSide(SkPoint::kOn_Side)
        , fInitialOutsetRing(false)
        , fStrokeWidth(strokeWidth)
        , fStyle(style)
        , fJoin(join)
        , fMiterLimit(miterLimit) {
    }

    SkPoint::Side side() const { return fSide; }

    SkScalar miterLimit() const { return fMiterLimit; }
    SkPaint::Join join() const { return fJoin; }

    bool tessellate(const SkMatrix& m, const SkPath& path);

    // The next five should only be called after tessellate to extract the result
    int numPts() const { return fPts.count(); }
    int numIndices() const { return fIndices.count(); }

    const SkPoint& lastPoint() const { return fPts.top(); }
    const SkPoint& point(int index) const { return fPts[index]; }
    int index(int index) const { return fIndices[index]; }
    SkScalar coverage(int index) const { return fCoverages[index]; }

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
    class InsetRing {
    public:
        void setReserve(int numPts) { fPts.setReserve(numPts); }
        void rewind() { fPts.rewind(); }

        int numPts() const { return fPts.count(); }

        void addIdx(int index, int origEdgeId) {
            struct PointData* pt = fPts.push();
            pt->fIndex = index;
            pt->fOrigEdgeId = origEdgeId;
        }

        // Upgrade this ring so that it can behave like an originating ring
        void makeOriginalRing() {
            for (int i = 0; i < fPts.count(); ++i) {
                fPts[i].fOrigEdgeId = fPts[i].fIndex;
            }            
        }

        // init should be called after all the indices have been added (via addIdx)
        void init(const GrAAConvexTessellator& tess);
        void init(const SkTDArray<SkVector>& norms, const SkTDArray<SkVector>& bisectors);

        const SkPoint& norm17(int index) const { return fPts[index].fNorm17; }
        const SkPoint& bisector17(int index) const { return fPts[index].fBisector17; }
        int index(int index) const { return fPts[index].fIndex; }
        int origEdgeID(int index) const { return fPts[index].fOrigEdgeId; }
        void setOrigEdgeId(int index, int id) { fPts[index].fOrigEdgeId = id; }

    #if GR_AA_CONVEX_TESSELLATOR_VIZ
        void draw(SkCanvas* canvas, const GrAAConvexTessellator& tess) const;
    #endif

    private:
        void computeNormals(const GrAAConvexTessellator& result);
        void computeBisectors(const GrAAConvexTessellator& tess);

        SkDEBUGCODE(bool isConvex(const GrAAConvexTessellator& tess) const;)

        struct PointData {
            SkPoint fNorm17;
            SkPoint fBisector17;
            int     fIndex;
            int     fOrigEdgeId;
        };

        SkTDArray<PointData> fPts;
    };

    class OutsetRing {
    public:
        OutsetRing(bool passOnPts) : fPassOnPts(passOnPts) { }

        void setReserve(int numPts) { fPts7.setReserve(numPts); }
        void rewind() { fPts7.rewind(); }

        int numPts() const { return fPts7.count(); }

        int add(GrAAConvexTessellator& tess, const SkPoint& p,
                SkScalar coverage, CurveState curve, int i1, int i2) {

            int index = -1;
            if (fPassOnPts) {
                index = tess.addPt(p, -1.0f, coverage, false, curve);
                fPts7.push()->set(p, curve, index, -1);
            } else {
                fPts7.push()->set(p, curve, i1, i2);            
            }

            return index;
        }

        void validate1(const GrAAConvexTessellator& tess) {
            for (int i = 0; i < fPts7.count(); ++i) {
                SkASSERT(fPts7[i].fI1 < tess.numPts());
                SkASSERT(fPts7[i].fI2 < tess.numPts());
            }
        }

        void fuse(int disappearingId, int stayingId) {
            for (int i = fPts7.count()-1; i >= 0; --i) {
                if (fPts7[i].fI1 == disappearingId) {
                    fPts7[i].fI1 = stayingId;
                } else {
                    break;
                }
            }
        }
        void rmLinearPt(int disapearingPt, int firstEndPt, int secondEndPt) {
            for (int i = fPts7.count()-1; i >= 0; --i) {
        
            }
        }

        // init should be called after all the points have been added (via add)
        void init1(SkVector::Side side);

        void pop() {
            fPts7.pop();
        }

        const SkPoint& lastPt() const { return fPts7.top().fPt; }
        const SkPoint& point(int index) const { return fPts7[index].fPt; }
        const SkPoint& norm(int index) const { return fPts7[index].fNorm; }
        const SkPoint& bisector(int index) const { return fPts7[index].fBisector; }
        CurveState curveState(int index) const { return fPts7[index].fCurveState; }
        int index1(int index) const { return fPts7[index].fI1; }
        int index2(int index) const { return fPts7[index].fI2; }

        void createOuterRing(GrAAConvexTessellator& tess, SkScalar outset,
                             SkScalar coverage, OutsetRing* nextRing);

   #if GR_AA_CONVEX_TESSELLATOR_VIZ
        void draw(SkCanvas* canvas, const GrAAConvexTessellator& tess) const;
    #endif

    private:
        void computeNormals1(SkVector::Side side);
        void computeBisectors1(SkVector::Side side);

        int bevel(GrAAConvexTessellator& tess,
                  OutsetRing* nextRing,
                  const SkVector& perp2,
                  int perp1Idx,
                  int cur,
                  SkScalar coverage,
                  CurveState curve);

        void addTrisForEdge(GrAAConvexTessellator& tess, int first, int second, int i0, int i1) {
            SkASSERT(fPts7[first].fI1 != -1 && fPts7[first].fI2 == -1);
            SkASSERT(fPts7[second].fI1 != -1 && fPts7[second].fI2 == -1);

            tess.addTri(fPts7[first].fI1, i0, fPts7[second].fI1);
            tess.addTri(fPts7[first].fI1, i1, i0);        
        }

        void addTrisForCorner(GrAAConvexTessellator& tess, int cur, int i0, int i1, int i2) {
            SkASSERT(fPts7[cur].fI1 != -1 && fPts7[cur].fI2 == -1);
            tess.addTri(fPts7[cur].fI1, i0, i1);
            tess.addTri(fPts7[cur].fI1, i1, i2);
        }

        void addTri(GrAAConvexTessellator& tess, int cur, int i0, int i1) {
            SkASSERT(fPts7[cur].fI1 != -1 && fPts7[cur].fI2 == -1);
            tess.addTri(fPts7[cur].fI1, i0, i1);
        }

        struct PointData {
            void set(const SkPoint& p, CurveState curve, int i1, int i2) {
                fPt = p;
                //fNorm = n;
                fCurveState = curve;
                fI1 = i1;
                fI2 = i2;
            }

            SkPoint fPt;
            SkVector fNorm;
            SkVector fBisector;
            CurveState fCurveState;
            int fI1;
            int fI2;
        };

        SkTDArray<PointData> fPts7;
        bool fPassOnPts;
    };

    bool movable(int index) const { return fMovable[index]; }

    // Movable points are those that can be slid along their bisector.
    // Basically, a point is immovable if it is part of the original
    // polygon or it results from the fusing of two bisectors.
    int addPt(const SkPoint& pt, SkScalar depth, SkScalar coverage, bool movable, CurveState curve);
    void popLastPt();
    void popFirstPtShuffle();

    void updatePt(int index, const SkPoint& pt, SkScalar depth, SkScalar coverage);

    void addTri(int i0, int i1, int i2);

    void reservePts(int count) {
        fPts.setReserve(count);
        fCoverages.setReserve(count);
        fMovable.setReserve(count);
    }

    SkScalar computeDepthFromEdge(int edgeIdx, const SkPoint& p) const;

    bool computePtAlongBisector(int startIdx, const SkPoint& bisector,
                                int edgeIdx, SkScalar desiredDepth,
                                SkPoint* result) const;

    void lineTo(const SkPoint& p, CurveState curve);

    void lineTo(const SkMatrix& m, SkPoint p, CurveState curve);

    void quadTo(const SkPoint pts[3]);

    void quadTo(const SkMatrix& m, SkPoint pts[3]);

    void cubicTo(const SkMatrix& m, SkPoint pts[4]);

    void conicTo(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    void terminate(const InsetRing& lastRing);

    // return false on failure/degenerate path
    bool extractFromPath(const SkMatrix& m, const SkPath& path);
    void computeBisectors();

    void fanRing(const InsetRing& ring);

    InsetRing* getNextInsetRing(InsetRing* lastRing);
    OutsetRing* getNextOutsetRing(bool passOnPts);

    bool createInsetRings(InsetRing& previousRing, SkScalar initialDepth, SkScalar initialCoverage,
                          SkScalar targetDepth, SkScalar targetCoverage, InsetRing** finalRing);

    bool createInsetRing(const InsetRing& lastRing, InsetRing* nextRing,
                         SkScalar initialDepth, SkScalar initialCoverage, SkScalar targetDepth,
                         SkScalar targetCoverage, bool forceNew);

    void validate() const;

    // fPts, fCoverages, fMovable & fCurveState should always have the same # of elements
    SkTDArray<SkPoint>    fPts;
    SkTDArray<SkScalar>   fCoverages;
    // movable points are those that can be slid further along their bisector
    SkTDArray<bool>       fMovable;
    // Tracks whether a given point is interior to a curve. Such points are
    // assumed to have shallow curvature.
    SkTDArray<CurveState> fCurveState;

    // The outward facing normals for the original polygon
    SkTDArray<SkVector>   fNorms1;
    // The inward facing bisector at each point in the original polygon. Only
    // needed for exterior ring creation and then handed off to the initial ring.
    SkTDArray<SkVector>   fBisectors;

    SkPoint::Side         fSide;    // winding of the original polygon

    // The triangulation of the points
    SkTDArray<int>        fIndices;

    InsetRing             fInitialInsetRing;
    OutsetRing            fInitialOutsetRing;
#if GR_AA_CONVEX_TESSELLATOR_VIZ
    // When visualizing save all the rings
    SkTDArray<InsetRing*>      fInsetRings;
    SkTDArray<OutsetRing*>     fOutsetRings;
#else
    InsetRing                  fInsetRings[2];
#endif
    CandidateVerts        fCandidateVerts;

    // the stroke width is only used for stroke or stroke-and-fill styles
    SkScalar              fStrokeWidth;
    SkStrokeRec::Style    fStyle;

    SkPaint::Join         fJoin;

    SkScalar              fMiterLimit;

    SkTDArray<SkPoint>    fPointBuffer;
};


#endif
