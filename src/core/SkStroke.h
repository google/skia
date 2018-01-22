/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStroke_DEFINED
#define SkStroke_DEFINED

#include "SkGeometry.h"
#include "SkStrokerPriv.h"

#ifndef DEBUG_QUAD_STROKER
    #define DEBUG_QUAD_STROKER 0
#endif

#if DEBUG_QUAD_STROKER
    /* Enable to show the decisions made in subdividing the curve -- helpful when the resulting
        stroke has more than the optimal number of quadratics and lines */
    #define STROKER_RESULT(resultType, depth, quadPts, format, ...) \
            SkDebugf("[%d] %s " format "\n", depth, __FUNCTION__, __VA_ARGS__), \
            SkDebugf("  " #resultType " t=(%g,%g)\n", quadPts->fStartT, quadPts->fEndT), \
            resultType
    #define STROKER_DEBUG_PARAMS(...) , __VA_ARGS__
#else
    #define STROKER_RESULT(resultType, depth, quadPts, format, ...) \
            resultType
    #define STROKER_DEBUG_PARAMS(...)
#endif

#ifdef SK_DEBUG
extern bool gDebugStrokerErrorSet;
extern SkScalar gDebugStrokerError;
extern int gMaxRecursion[];
#endif

/** \class SkStroke
    SkStroke is the utility class that constructs paths by stroking
    geometries (lines, rects, ovals, roundrects, paths). This is
    invoked when a geometry or text is drawn in a canvas with the
    kStroke_Mask bit set in the paint.
*/
class SkStroke {
public:
    SkStroke();
    SkStroke(const SkPaint&);
    SkStroke(const SkPaint&, SkScalar width);   // width overrides paint.getStrokeWidth()

    SkPaint::Cap    getCap() const { return (SkPaint::Cap)fCap; }
    void        setCap(SkPaint::Cap);

    SkPaint::Join   getJoin() const { return (SkPaint::Join)fJoin; }
    void        setJoin(SkPaint::Join);

    void    setMiterLimit(SkScalar);
    void    setWidth(SkScalar);

    bool    getDoFill() const { return SkToBool(fDoFill); }
    void    setDoFill(bool doFill) { fDoFill = SkToU8(doFill); }

    /**
     *  ResScale is the "intended" resolution for the output.
     *      Default is 1.0.
     *      Larger values (res > 1) indicate that the result should be more precise, since it will
     *          be zoomed up, and small errors will be magnified.
     *      Smaller values (0 < res < 1) indicate that the result can be less precise, since it will
     *          be zoomed down, and small errors may be invisible.
     */
    SkScalar getResScale() const { return fResScale; }
    void setResScale(SkScalar rs) {
        SkASSERT(rs > 0 && SkScalarIsFinite(rs));
        fResScale = rs;
    }

    /**
     *  Stroke the specified rect, winding it in the specified direction..
     */
    void    strokeRect(const SkRect& rect, SkPath* result,
                       SkPath::Direction = SkPath::kCW_Direction) const;
    void    strokePath(const SkPath& path, SkPath*) const;

    ////////////////////////////////////////////////////////////////

private:
    SkScalar    fWidth, fMiterLimit;
    SkScalar    fResScale;
    uint8_t     fCap, fJoin;
    SkBool8     fDoFill;

    friend class SkPaint;
};

struct SkQuadConstruct {    // The state of the quad stroke under construction.
    SkPoint fQuad[3];       // the stroked quad parallel to the original curve
    SkPoint fTangentStart;  // a point tangent to fQuad[0]
    SkPoint fTangentEnd;    // a point tangent to fQuad[2]
    SkScalar fStartT;       // a segment of the original curve
    SkScalar fMidT;         //              "
    SkScalar fEndT;         //              "
    bool fStartSet;         // state to share common points across structs
    bool fEndSet;           //                     "
    bool fOppositeTangents; // set if coincident tangents have opposite directions

    // return false if start and end are too close to have a unique middle
    bool init(SkScalar start, SkScalar end) {
        fStartT = start;
        fMidT = (start + end) * SK_ScalarHalf;
        fEndT = end;
        fStartSet = fEndSet = false;
        return fStartT < fMidT && fMidT < fEndT;
    }

    bool initWithStart(SkQuadConstruct* parent) {
        if (!init(parent->fStartT, parent->fMidT)) {
            return false;
        }
        fQuad[0] = parent->fQuad[0];
        fTangentStart = parent->fTangentStart;
        fStartSet = true;
        return true;
    }

    bool initWithEnd(SkQuadConstruct* parent) {
        if (!init(parent->fMidT, parent->fEndT)) {
            return false;
        }
        fQuad[2] = parent->fQuad[2];
        fTangentEnd = parent->fTangentEnd;
        fEndSet = true;
        return true;
   }
};

class SkPathStroker {
public:
    SkPathStroker(const SkPath& src,
                  SkScalar radius, SkScalar miterLimit, SkPaint::Cap,
                  SkPaint::Join, SkScalar resScale,
                  bool canIgnoreCenter);

    enum StrokeType {
        kOuter_StrokeType = 1,      // use sign-opposite values later to flip perpendicular axis
        kInner_StrokeType = -1
    } fStrokeType;

    bool hasOnlyMoveTo() const { return 0 == fSegmentCount; }
    SkPoint moveToPt() const { return fFirstPt; }

    void moveTo(const SkPoint&);
    void lineTo(const SkPoint&, const SkPath::Iter* iter = nullptr);
    void quadTo(const SkPoint&, const SkPoint&);
    void conicTo(const SkPoint&, const SkPoint&, SkScalar weight);
    void cubicTo(const SkPoint&, const SkPoint&, const SkPoint&);
    void close(bool isLine) { this->finishContour(true, isLine); }

    void done(SkPath* dst, bool isLine) {
        this->finishContour(false, isLine);
        dst->swap(fOuter);
    }

    SkScalar getResScale() const { return fResScale; }
    SkPath* inner() { return &fInner; }
    void init(StrokeType strokeType, SkQuadConstruct* , SkScalar tStart, SkScalar tEnd);

    bool isCurrentContourEmpty() const {
        return fInner.isZeroLengthSincePoint(0) &&
               fOuter.isZeroLengthSincePoint(fFirstOuterPtIndexInContour);
    }

    SkPath* outer() { return &fOuter; }
    bool quadStroke(const SkPoint quad[3], SkQuadConstruct* );

private:
    SkScalar    fRadius;
    SkScalar    fInvMiterLimit;
    SkScalar    fResScale;
    SkScalar    fInvResScale;
    SkScalar    fInvResScaleSquared;

    SkVector    fFirstNormal, fPrevNormal, fFirstUnitNormal, fPrevUnitNormal;
    SkPoint     fFirstPt, fPrevPt;  // on original path
    SkPoint     fFirstOuterPt;
    int         fFirstOuterPtIndexInContour;
    int         fSegmentCount;
    bool        fPrevIsLine;
    bool        fCanIgnoreCenter;

    SkStrokerPriv::CapProc  fCapper;
    SkStrokerPriv::JoinProc fJoiner;

    SkPath  fInner, fOuter; // outer is our working answer, inner is temp

    enum ResultType {
        kSplit_ResultType,          // the caller should split the quad stroke in two
        kDegenerate_ResultType,     // the caller should add a line
        kQuad_ResultType,           // the caller should (continue to try to) add a quad stroke
    };

    enum ReductionType {
        kPoint_ReductionType,       // all curve points are practically identical
        kLine_ReductionType,        // the control point is on the line between the ends
        kQuad_ReductionType,        // the control point is outside the line between the ends
        kDegenerate_ReductionType,  // the control point is on the line but outside the ends
        kDegenerate2_ReductionType, // two control points are on the line but outside ends (cubic)
        kDegenerate3_ReductionType, // three areas of max curvature found (for cubic)
    };

    enum IntersectRayType {
        kCtrlPt_RayType,
        kResultType_RayType,
    };

    int fRecursionDepth;            // track stack depth to abort if numerics run amok
    bool fFoundTangents;            // do less work until tangents meet (cubic)
    bool fJoinCompleted;            // previous join was not degenerate

    void addDegenerateLine(const SkQuadConstruct* );
    static ReductionType CheckConicLinear(const SkConic& , SkPoint* reduction);
    static ReductionType CheckCubicLinear(const SkPoint cubic[4], SkPoint reduction[3],
                                   const SkPoint** tanPtPtr);
    static ReductionType CheckQuadLinear(const SkPoint quad[3], SkPoint* reduction);
    ResultType compareQuadConic(const SkConic& , SkQuadConstruct* ) const;
    ResultType compareQuadCubic(const SkPoint cubic[4], SkQuadConstruct* );
    ResultType compareQuadQuad(const SkPoint quad[3], SkQuadConstruct* );
    void conicPerpRay(const SkConic& , SkScalar t, SkPoint* tPt, SkPoint* onPt,
                      SkPoint* tangent) const;
    void conicQuadEnds(const SkConic& , SkQuadConstruct* ) const;
    bool conicStroke(const SkConic& , SkQuadConstruct* );
    bool cubicMidOnLine(const SkPoint cubic[4], const SkQuadConstruct* ) const;
    void cubicPerpRay(const SkPoint cubic[4], SkScalar t, SkPoint* tPt, SkPoint* onPt,
                      SkPoint* tangent) const;
    void cubicQuadEnds(const SkPoint cubic[4], SkQuadConstruct* );
    void cubicQuadMid(const SkPoint cubic[4], const SkQuadConstruct* , SkPoint* mid) const;
    bool cubicStroke(const SkPoint cubic[4], SkQuadConstruct* );
    ResultType intersectRay(SkQuadConstruct* , IntersectRayType  STROKER_DEBUG_PARAMS(int) ) const;
    bool ptInQuadBounds(const SkPoint quad[3], const SkPoint& pt) const;
    void quadPerpRay(const SkPoint quad[3], SkScalar t, SkPoint* tPt, SkPoint* onPt,
                     SkPoint* tangent) const;
    void setConicEndNormal(const SkConic& ,
                           const SkVector& normalAB, const SkVector& unitNormalAB,
                           SkVector* normalBC, SkVector* unitNormalBC);
    void setCubicEndNormal(const SkPoint cubic[4],
                           const SkVector& normalAB, const SkVector& unitNormalAB,
                           SkVector* normalCD, SkVector* unitNormalCD);
    void setQuadEndNormal(const SkPoint quad[3],
                          const SkVector& normalAB, const SkVector& unitNormalAB,
                          SkVector* normalBC, SkVector* unitNormalBC);
    void setRayPts(const SkPoint& tPt, SkVector* dxy, SkPoint* onPt, SkPoint* tangent) const;
    static bool SlightAngle(SkQuadConstruct* );
    ResultType strokeCloseEnough(const SkPoint stroke[3], const SkPoint ray[2],
                                 SkQuadConstruct*  STROKER_DEBUG_PARAMS(int depth) ) const;
    ResultType tangentsMeet(const SkPoint cubic[4], SkQuadConstruct* );

    void    finishContour(bool close, bool isLine);
    bool    preJoinTo(const SkPoint&, SkVector* normal, SkVector* unitNormal,
                      bool isLine);
    void    postJoinTo(const SkPoint&, const SkVector& normal,
                       const SkVector& unitNormal);

    void    line_to(const SkPoint& currPt, const SkVector& normal);
};

#endif
