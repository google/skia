/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStrokerPriv.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"

enum {
    kTangent_RecursiveLimit,
    kCubic_RecursiveLimit,
    kConic_RecursiveLimit,
    kQuad_RecursiveLimit
};

// quads with extreme widths (e.g. (0,1) (1,6) (0,3) width=5e7) recurse to point of failure
// largest seen for normal cubics : 5, 26
// largest seen for normal quads : 11
static const int kRecursiveLimits[] = { 5*3, 26*3, 11*3, 11*3 }; // 3x limits seen in practice

SK_COMPILE_ASSERT(0 == kTangent_RecursiveLimit, cubic_stroke_relies_on_tangent_equalling_zero);
SK_COMPILE_ASSERT(1 == kCubic_RecursiveLimit, cubic_stroke_relies_on_cubic_equalling_one);
SK_COMPILE_ASSERT(SK_ARRAY_COUNT(kRecursiveLimits) == kQuad_RecursiveLimit + 1,
        recursive_limits_mismatch);

#ifdef SK_DEBUG
    int gMaxRecursion[SK_ARRAY_COUNT(kRecursiveLimits)] = { 0 };
#endif
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

static inline bool degenerate_vector(const SkVector& v) {
    return !SkPoint::CanNormalize(v.fX, v.fY);
}

static bool set_normal_unitnormal(const SkPoint& before, const SkPoint& after,
                                  SkScalar radius,
                                  SkVector* normal, SkVector* unitNormal) {
    if (!unitNormal->setNormalize(after.fX - before.fX, after.fY - before.fY)) {
        return false;
    }
    unitNormal->rotateCCW();
    unitNormal->scale(radius, normal);
    return true;
}

static bool set_normal_unitnormal(const SkVector& vec,
                                  SkScalar radius,
                                  SkVector* normal, SkVector* unitNormal) {
    if (!unitNormal->setNormalize(vec.fX, vec.fY)) {
        return false;
    }
    unitNormal->rotateCCW();
    unitNormal->scale(radius, normal);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

struct SkQuadConstruct {    // The state of the quad stroke under construction.
    SkPoint fQuad[3];       // the stroked quad parallel to the original curve
    SkPoint fTangentStart;  // a point tangent to fQuad[0]
    SkPoint fTangentEnd;    // a point tangent to fQuad[2]
    SkScalar fStartT;       // a segment of the original curve
    SkScalar fMidT;         //              "
    SkScalar fEndT;         //              "
    bool fStartSet;         // state to share common points across structs
    bool fEndSet;           //                     "

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
                  SkPaint::Join, SkScalar resScale);

    void moveTo(const SkPoint&);
    void lineTo(const SkPoint&);
    void quadTo(const SkPoint&, const SkPoint&);
    void conicTo(const SkPoint&, const SkPoint&, SkScalar weight);
    void cubicTo(const SkPoint&, const SkPoint&, const SkPoint&);
    void close(bool isLine) { this->finishContour(true, isLine); }

    void done(SkPath* dst, bool isLine) {
        this->finishContour(false, isLine);
        fOuter.addPath(fExtra);
        dst->swap(fOuter);
    }

    SkScalar getResScale() const { return fResScale; }

private:
    SkScalar    fRadius;
    SkScalar    fInvMiterLimit;
    SkScalar    fResScale;
    SkScalar    fInvResScale;
    SkScalar    fInvResScaleSquared;

    SkVector    fFirstNormal, fPrevNormal, fFirstUnitNormal, fPrevUnitNormal;
    SkPoint     fFirstPt, fPrevPt;  // on original path
    SkPoint     fFirstOuterPt;
    int         fSegmentCount;
    bool        fPrevIsLine;

    SkStrokerPriv::CapProc  fCapper;
    SkStrokerPriv::JoinProc fJoiner;

    SkPath  fInner, fOuter; // outer is our working answer, inner is temp
    SkPath  fExtra;         // added as extra complete contours

    enum StrokeType {
        kOuter_StrokeType = 1,      // use sign-opposite values later to flip perpendicular axis
        kInner_StrokeType = -1
    } fStrokeType;

    enum ResultType {
        kSplit_ResultType,          // the caller should split the quad stroke in two
        kDegenerate_ResultType,     // the caller should add a line
        kQuad_ResultType,           // the caller should (continue to try to) add a quad stroke
        kNormalError_ResultType,    // the cubic's normal couldn't be computed -- abort
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
    bool cubicPerpRay(const SkPoint cubic[4], SkScalar t, SkPoint* tPt, SkPoint* onPt,
                      SkPoint* tangent) const;
    bool cubicQuadEnds(const SkPoint cubic[4], SkQuadConstruct* );
    bool cubicQuadMid(const SkPoint cubic[4], const SkQuadConstruct* , SkPoint* mid) const;
    bool cubicStroke(const SkPoint cubic[4], SkQuadConstruct* );
    void init(StrokeType strokeType, SkQuadConstruct* , SkScalar tStart, SkScalar tEnd);
    ResultType intersectRay(SkQuadConstruct* , IntersectRayType  STROKER_DEBUG_PARAMS(int) ) const;
    bool ptInQuadBounds(const SkPoint quad[3], const SkPoint& pt) const;
    void quadPerpRay(const SkPoint quad[3], SkScalar t, SkPoint* tPt, SkPoint* onPt,
                     SkPoint* tangent) const;
    bool quadStroke(const SkPoint quad[3], SkQuadConstruct* );
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

///////////////////////////////////////////////////////////////////////////////

bool SkPathStroker::preJoinTo(const SkPoint& currPt, SkVector* normal,
                              SkVector* unitNormal, bool currIsLine) {
    SkASSERT(fSegmentCount >= 0);

    SkScalar    prevX = fPrevPt.fX;
    SkScalar    prevY = fPrevPt.fY;

    if (!set_normal_unitnormal(fPrevPt, currPt, fRadius, normal, unitNormal)) {
        return false;
    }

    if (fSegmentCount == 0) {
        fFirstNormal = *normal;
        fFirstUnitNormal = *unitNormal;
        fFirstOuterPt.set(prevX + normal->fX, prevY + normal->fY);

        fOuter.moveTo(fFirstOuterPt.fX, fFirstOuterPt.fY);
        fInner.moveTo(prevX - normal->fX, prevY - normal->fY);
    } else {    // we have a previous segment
        fJoiner(&fOuter, &fInner, fPrevUnitNormal, fPrevPt, *unitNormal,
                fRadius, fInvMiterLimit, fPrevIsLine, currIsLine);
    }
    fPrevIsLine = currIsLine;
    return true;
}

void SkPathStroker::postJoinTo(const SkPoint& currPt, const SkVector& normal,
                               const SkVector& unitNormal) {
    fPrevPt = currPt;
    fPrevUnitNormal = unitNormal;
    fPrevNormal = normal;
    fSegmentCount += 1;
}

void SkPathStroker::finishContour(bool close, bool currIsLine) {
    if (fSegmentCount > 0) {
        SkPoint pt;

        if (close) {
            fJoiner(&fOuter, &fInner, fPrevUnitNormal, fPrevPt,
                    fFirstUnitNormal, fRadius, fInvMiterLimit,
                    fPrevIsLine, currIsLine);
            fOuter.close();
            // now add fInner as its own contour
            fInner.getLastPt(&pt);
            fOuter.moveTo(pt.fX, pt.fY);
            fOuter.reversePathTo(fInner);
            fOuter.close();
        } else {    // add caps to start and end
            // cap the end
            fInner.getLastPt(&pt);
            fCapper(&fOuter, fPrevPt, fPrevNormal, pt,
                    currIsLine ? &fInner : NULL);
            fOuter.reversePathTo(fInner);
            // cap the start
            fCapper(&fOuter, fFirstPt, -fFirstNormal, fFirstOuterPt,
                    fPrevIsLine ? &fInner : NULL);
            fOuter.close();
        }
    }
    // since we may re-use fInner, we rewind instead of reset, to save on
    // reallocating its internal storage.
    fInner.rewind();
    fSegmentCount = -1;
}

///////////////////////////////////////////////////////////////////////////////

SkPathStroker::SkPathStroker(const SkPath& src,
                             SkScalar radius, SkScalar miterLimit,
                             SkPaint::Cap cap, SkPaint::Join join, SkScalar resScale)
        : fRadius(radius)
        , fResScale(resScale) {

    /*  This is only used when join is miter_join, but we initialize it here
        so that it is always defined, to fis valgrind warnings.
    */
    fInvMiterLimit = 0;

    if (join == SkPaint::kMiter_Join) {
        if (miterLimit <= SK_Scalar1) {
            join = SkPaint::kBevel_Join;
        } else {
            fInvMiterLimit = SkScalarInvert(miterLimit);
        }
    }
    fCapper = SkStrokerPriv::CapFactory(cap);
    fJoiner = SkStrokerPriv::JoinFactory(join);
    fSegmentCount = -1;
    fPrevIsLine = false;

    // Need some estimate of how large our final result (fOuter)
    // and our per-contour temp (fInner) will be, so we don't spend
    // extra time repeatedly growing these arrays.
    //
    // 3x for result == inner + outer + join (swag)
    // 1x for inner == 'wag' (worst contour length would be better guess)
    fOuter.incReserve(src.countPoints() * 3);
    fOuter.setIsVolatile(true);
    fInner.incReserve(src.countPoints());
    fInner.setIsVolatile(true);
    // TODO : write a common error function used by stroking and filling
    // The '4' below matches the fill scan converter's error term
    fInvResScale = SkScalarInvert(resScale * 4);
    fInvResScaleSquared = fInvResScale * fInvResScale;
    fRecursionDepth = 0;
}

void SkPathStroker::moveTo(const SkPoint& pt) {
    if (fSegmentCount > 0) {
        this->finishContour(false, false);
    }
    fSegmentCount = 0;
    fFirstPt = fPrevPt = pt;
}

void SkPathStroker::line_to(const SkPoint& currPt, const SkVector& normal) {
    fOuter.lineTo(currPt.fX + normal.fX, currPt.fY + normal.fY);
    fInner.lineTo(currPt.fX - normal.fX, currPt.fY - normal.fY);
}

void SkPathStroker::lineTo(const SkPoint& currPt) {
    if (SkPath::IsLineDegenerate(fPrevPt, currPt, false)) {
        return;
    }
    SkVector    normal, unitNormal;

    if (!this->preJoinTo(currPt, &normal, &unitNormal, true)) {
        return;
    }
    this->line_to(currPt, normal);
    this->postJoinTo(currPt, normal, unitNormal);
}

void SkPathStroker::setQuadEndNormal(const SkPoint quad[3], const SkVector& normalAB,
        const SkVector& unitNormalAB, SkVector* normalBC, SkVector* unitNormalBC) {
    if (!set_normal_unitnormal(quad[1], quad[2], fRadius, normalBC, unitNormalBC)) {
        *normalBC = normalAB;
        *unitNormalBC = unitNormalAB;
    }
}

void SkPathStroker::setConicEndNormal(const SkConic& conic, const SkVector& normalAB,
        const SkVector& unitNormalAB, SkVector* normalBC, SkVector* unitNormalBC) {
    setQuadEndNormal(conic.fPts, normalAB, unitNormalAB, normalBC, unitNormalBC);
}

void SkPathStroker::setCubicEndNormal(const SkPoint cubic[4], const SkVector& normalAB,
        const SkVector& unitNormalAB, SkVector* normalCD, SkVector* unitNormalCD) {
    SkVector    ab = cubic[1] - cubic[0];
    SkVector    cd = cubic[3] - cubic[2];

    bool    degenerateAB = degenerate_vector(ab);
    bool    degenerateCD = degenerate_vector(cd);

    if (degenerateAB && degenerateCD) {
        goto DEGENERATE_NORMAL;
    }

    if (degenerateAB) {
        ab = cubic[2] - cubic[0];
        degenerateAB = degenerate_vector(ab);
    }
    if (degenerateCD) {
        cd = cubic[3] - cubic[1];
        degenerateCD = degenerate_vector(cd);
    }
    if (degenerateAB || degenerateCD) {
DEGENERATE_NORMAL:
        *normalCD = normalAB;
        *unitNormalCD = unitNormalAB;
        return;
    }
    SkAssertResult(set_normal_unitnormal(cd, fRadius, normalCD, unitNormalCD));
}

void SkPathStroker::init(StrokeType strokeType, SkQuadConstruct* quadPts, SkScalar tStart,
        SkScalar tEnd) {
    fStrokeType = strokeType;
    fFoundTangents = false;
    quadPts->init(tStart, tEnd);
}

// returns the distance squared from the point to the line
static SkScalar pt_to_line(const SkPoint& pt, const SkPoint& lineStart, const SkPoint& lineEnd) {
    SkVector dxy = lineEnd - lineStart;
    if (degenerate_vector(dxy)) {
        return pt.distanceToSqd(lineStart);
    }
    SkVector ab0 = pt - lineStart;
    SkScalar numer = dxy.dot(ab0);
    SkScalar denom = dxy.dot(dxy);
    SkScalar t = numer / denom;
    SkPoint hit;
    hit.fX = lineStart.fX * (1 - t) + lineEnd.fX * t;
    hit.fY = lineStart.fY * (1 - t) + lineEnd.fY * t;
    return hit.distanceToSqd(pt);
}

/*  Given a cubic, determine if all four points are in a line.
    Return true if the inner points is close to a line connecting the outermost points.

    Find the outermost point by looking for the largest difference in X or Y.
    Given the indices of the outermost points, and that outer_1 is greater than outer_2,
    this table shows the index of the smaller of the remaining points:

                      outer_2
                  0    1    2    3
      outer_1     ----------------
         0     |  -    2    1    1
         1     |  -    -    0    0
         2     |  -    -    -    0
         3     |  -    -    -    -

    If outer_1 == 0 and outer_2 == 1, the smaller of the remaining indices (2 and 3) is 2.

    This table can be collapsed to: (1 + (2 >> outer_2)) >> outer_1

    Given three indices (outer_1 outer_2 mid_1) from 0..3, the remaining index is:

               mid_2 == (outer_1 ^ outer_2 ^ mid_1)
 */
static bool cubic_in_line(const SkPoint cubic[4]) {
    SkScalar ptMax = -1;
    int outer1 SK_INIT_TO_AVOID_WARNING;
    int outer2 SK_INIT_TO_AVOID_WARNING;
    for (int index = 0; index < 3; ++index) {
        for (int inner = index + 1; inner < 4; ++inner) {
            SkVector testDiff = cubic[inner] - cubic[index];
            SkScalar testMax = SkTMax(SkScalarAbs(testDiff.fX), SkScalarAbs(testDiff.fY));
            if (ptMax < testMax) {
                outer1 = index;
                outer2 = inner;
                ptMax = testMax;
            }
        }
    }
    SkASSERT(outer1 >= 0 && outer1 <= 2);
    SkASSERT(outer2 >= 1 && outer2 <= 3);
    SkASSERT(outer1 < outer2);
    int mid1 = (1 + (2 >> outer2)) >> outer1;
    SkASSERT(mid1 >= 0 && mid1 <= 2);
    SkASSERT(outer1 != mid1 && outer2 != mid1);
    int mid2 = outer1 ^ outer2 ^ mid1;
    SkASSERT(mid2 >= 1 && mid2 <= 3);
    SkASSERT(mid2 != outer1 && mid2 != outer2 && mid2 != mid1);
    SkASSERT(((1 << outer1) | (1 << outer2) | (1 << mid1) | (1 << mid2)) == 0x0f);
    SkScalar lineSlop = ptMax * ptMax * 0.00001f;  // this multiplier is pulled out of the air
    return pt_to_line(cubic[mid1], cubic[outer1], cubic[outer2]) <= lineSlop
            && pt_to_line(cubic[mid2], cubic[outer1], cubic[outer2]) <= lineSlop;
}

/* Given quad, see if all there points are in a line.
   Return true if the inside point is close to a line connecting the outermost points.

   Find the outermost point by looking for the largest difference in X or Y.
   Since the XOR of the indices is 3  (0 ^ 1 ^ 2)
   the missing index equals: outer_1 ^ outer_2 ^ 3
 */
static bool quad_in_line(const SkPoint quad[3]) {
    SkScalar ptMax = -1;
    int outer1 SK_INIT_TO_AVOID_WARNING;
    int outer2 SK_INIT_TO_AVOID_WARNING;
    for (int index = 0; index < 2; ++index) {
        for (int inner = index + 1; inner < 3; ++inner) {
            SkVector testDiff = quad[inner] - quad[index];
            SkScalar testMax = SkTMax(SkScalarAbs(testDiff.fX), SkScalarAbs(testDiff.fY));
            if (ptMax < testMax) {
                outer1 = index;
                outer2 = inner;
                ptMax = testMax;
            }
        }
    }
    SkASSERT(outer1 >= 0 && outer1 <= 1);
    SkASSERT(outer2 >= 1 && outer2 <= 2);
    SkASSERT(outer1 < outer2);
    int mid = outer1 ^ outer2 ^ 3;
    SkScalar lineSlop =  ptMax * ptMax * 0.00001f;  // this multiplier is pulled out of the air
    return pt_to_line(quad[mid], quad[outer1], quad[outer2]) <= lineSlop;
}

static bool conic_in_line(const SkConic& conic) {
    return quad_in_line(conic.fPts);
}

SkPathStroker::ReductionType SkPathStroker::CheckCubicLinear(const SkPoint cubic[4],
        SkPoint reduction[3], const SkPoint** tangentPtPtr) {
    bool degenerateAB = degenerate_vector(cubic[1] - cubic[0]);
    bool degenerateBC = degenerate_vector(cubic[2] - cubic[1]);
    bool degenerateCD = degenerate_vector(cubic[3] - cubic[2]);
    if (degenerateAB & degenerateBC & degenerateCD) {
        return kPoint_ReductionType;
    }
    if (degenerateAB + degenerateBC + degenerateCD == 2) {
        return kLine_ReductionType;
    }
    if (!cubic_in_line(cubic)) {
        *tangentPtPtr = degenerateAB ? &cubic[2] : &cubic[1];
        return kQuad_ReductionType;
    }
    SkScalar tValues[3];
    int count = SkFindCubicMaxCurvature(cubic, tValues);
    if (count == 0) {
        return kLine_ReductionType;
    }
    for (int index = 0; index < count; ++index) {
        SkScalar t = tValues[index];
        SkEvalCubicAt(cubic, t, &reduction[index], NULL, NULL);
    }
    SK_COMPILE_ASSERT(kQuad_ReductionType + 1 == kDegenerate_ReductionType, enum_out_of_whack);
    SK_COMPILE_ASSERT(kQuad_ReductionType + 2 == kDegenerate2_ReductionType, enum_out_of_whack);
    SK_COMPILE_ASSERT(kQuad_ReductionType + 3 == kDegenerate3_ReductionType, enum_out_of_whack);

    return (ReductionType) (kQuad_ReductionType + count);
}

SkPathStroker::ReductionType SkPathStroker::CheckConicLinear(const SkConic& conic,
        SkPoint* reduction) {
    bool degenerateAB = degenerate_vector(conic.fPts[1] - conic.fPts[0]);
    bool degenerateBC = degenerate_vector(conic.fPts[2] - conic.fPts[1]);
    if (degenerateAB & degenerateBC) {
        return kPoint_ReductionType;
    }
    if (degenerateAB | degenerateBC) {
        return kLine_ReductionType;
    }
    if (!conic_in_line(conic)) {
        return kQuad_ReductionType;
    }
#if 0   // once findMaxCurvature is implemented, this will be a better solution
    SkScalar t;
    if (!conic.findMaxCurvature(&t) || 0 == t) {
        return kLine_ReductionType;
    }
#else  // but for now, use extrema instead
    SkScalar xT = 0, yT = 0;
    (void) conic.findXExtrema(&xT);
    (void) conic.findYExtrema(&yT);
    SkScalar t = SkTMax(xT, yT); 
    if (0 == t) {
        return kLine_ReductionType;
    }
#endif
    conic.evalAt(t, reduction, NULL);
    return kDegenerate_ReductionType;
}

SkPathStroker::ReductionType SkPathStroker::CheckQuadLinear(const SkPoint quad[3],
        SkPoint* reduction) {
    bool degenerateAB = degenerate_vector(quad[1] - quad[0]);
    bool degenerateBC = degenerate_vector(quad[2] - quad[1]);
    if (degenerateAB & degenerateBC) {
        return kPoint_ReductionType;
    }
    if (degenerateAB | degenerateBC) {
        return kLine_ReductionType;
    }
    if (!quad_in_line(quad)) {
        return kQuad_ReductionType;
    }
    SkScalar t = SkFindQuadMaxCurvature(quad);
    if (0 == t) {
        return kLine_ReductionType;
    }
    *reduction = SkEvalQuadAt(quad, t);
    return kDegenerate_ReductionType;
}

void SkPathStroker::conicTo(const SkPoint& pt1, const SkPoint& pt2, SkScalar weight) {
    const SkConic conic(fPrevPt, pt1, pt2, weight);
    SkPoint reduction;
    ReductionType reductionType = CheckConicLinear(conic, &reduction);
    if (kPoint_ReductionType == reductionType) {
        return;
    }
    if (kLine_ReductionType == reductionType) {
        this->lineTo(pt2);
        return;
    }
    if (kDegenerate_ReductionType == reductionType) {
        this->lineTo(reduction);
        SkStrokerPriv::JoinProc saveJoiner = fJoiner;
        fJoiner = SkStrokerPriv::JoinFactory(SkPaint::kRound_Join);
        this->lineTo(pt2);
        fJoiner = saveJoiner;
        return;
    }
    SkASSERT(kQuad_ReductionType == reductionType);
    SkVector normalAB, unitAB, normalBC, unitBC;
    if (!this->preJoinTo(pt1, &normalAB, &unitAB, false)) {
        this->lineTo(pt2);
        return;
    }
    SkQuadConstruct quadPts;
    this->init(kOuter_StrokeType, &quadPts, 0, 1);
    (void) this->conicStroke(conic, &quadPts);
    this->init(kInner_StrokeType, &quadPts, 0, 1);
    (void) this->conicStroke(conic, &quadPts);
    this->setConicEndNormal(conic, normalAB, unitAB, &normalBC, &unitBC);
    this->postJoinTo(pt2, normalBC, unitBC);
}

void SkPathStroker::quadTo(const SkPoint& pt1, const SkPoint& pt2) {
    const SkPoint quad[3] = { fPrevPt, pt1, pt2 };
    SkPoint reduction;
    ReductionType reductionType = CheckQuadLinear(quad, &reduction);
    if (kPoint_ReductionType == reductionType) {
        return;
    }
    if (kLine_ReductionType == reductionType) {
        this->lineTo(pt2);
        return;
    }
    if (kDegenerate_ReductionType == reductionType) {
        this->lineTo(reduction);
        SkStrokerPriv::JoinProc saveJoiner = fJoiner;
        fJoiner = SkStrokerPriv::JoinFactory(SkPaint::kRound_Join);
        this->lineTo(pt2);
        fJoiner = saveJoiner;
        return;
    }
    SkASSERT(kQuad_ReductionType == reductionType);
    SkVector normalAB, unitAB, normalBC, unitBC;
    if (!this->preJoinTo(pt1, &normalAB, &unitAB, false)) {
        this->lineTo(pt2);
        return;
    }
    SkQuadConstruct quadPts;
    this->init(kOuter_StrokeType, &quadPts, 0, 1);
    (void) this->quadStroke(quad, &quadPts);
    this->init(kInner_StrokeType, &quadPts, 0, 1);
    (void) this->quadStroke(quad, &quadPts);
    this->setQuadEndNormal(quad, normalAB, unitAB, &normalBC, &unitBC);

    this->postJoinTo(pt2, normalBC, unitBC);
}

// Given a point on the curve and its derivative, scale the derivative by the radius, and
// compute the perpendicular point and its tangent.
void SkPathStroker::setRayPts(const SkPoint& tPt, SkVector* dxy, SkPoint* onPt,
        SkPoint* tangent) const {
    SkPoint oldDxy = *dxy;
    if (!dxy->setLength(fRadius)) {  // consider moving double logic into SkPoint::setLength
        double xx = oldDxy.fX;
        double yy = oldDxy.fY;
        double dscale = fRadius / sqrt(xx * xx + yy * yy);
        dxy->fX = SkDoubleToScalar(xx * dscale);
        dxy->fY = SkDoubleToScalar(yy * dscale);
    }
    SkScalar axisFlip = SkIntToScalar(fStrokeType);  // go opposite ways for outer, inner
    onPt->fX = tPt.fX + axisFlip * dxy->fY;
    onPt->fY = tPt.fY - axisFlip * dxy->fX;
    if (tangent) {
        tangent->fX = onPt->fX + dxy->fX;
        tangent->fY = onPt->fY + dxy->fY;
    }
}

// Given a conic and t, return the point on curve, its perpendicular, and the perpendicular tangent.
// Returns false if the perpendicular could not be computed (because the derivative collapsed to 0)
void SkPathStroker::conicPerpRay(const SkConic& conic, SkScalar t, SkPoint* tPt, SkPoint* onPt,
        SkPoint* tangent) const {
    SkVector dxy;
    conic.evalAt(t, tPt, &dxy);
    if (dxy.fX == 0 && dxy.fY == 0) {
        dxy = conic.fPts[2] - conic.fPts[0];
    }
    this->setRayPts(*tPt, &dxy, onPt, tangent);
}

// Given a conic and a t range, find the start and end if they haven't been found already.
void SkPathStroker::conicQuadEnds(const SkConic& conic, SkQuadConstruct* quadPts) const {
    if (!quadPts->fStartSet) {
        SkPoint conicStartPt;
        this->conicPerpRay(conic, quadPts->fStartT, &conicStartPt, &quadPts->fQuad[0],
                &quadPts->fTangentStart);
        quadPts->fStartSet = true;
    }
    if (!quadPts->fEndSet) {
        SkPoint conicEndPt;
        this->conicPerpRay(conic, quadPts->fEndT, &conicEndPt, &quadPts->fQuad[2],
                &quadPts->fTangentEnd);
        quadPts->fEndSet = true;
    }
}


// Given a cubic and t, return the point on curve, its perpendicular, and the perpendicular tangent.
// Returns false if the perpendicular could not be computed (because the derivative collapsed to 0)
bool SkPathStroker::cubicPerpRay(const SkPoint cubic[4], SkScalar t, SkPoint* tPt, SkPoint* onPt,
        SkPoint* tangent) const {
    SkVector dxy;
    SkEvalCubicAt(cubic, t, tPt, &dxy, NULL);
    if (dxy.fX == 0 && dxy.fY == 0) {
        if (SkScalarNearlyZero(t)) {
            dxy = cubic[2] - cubic[0];
        } else if (SkScalarNearlyZero(1 - t)) {
            dxy = cubic[3] - cubic[1];
        } else {
            return false;
        }
        if (dxy.fX == 0 && dxy.fY == 0) {
            dxy = cubic[3] - cubic[0];
        }
    }
    setRayPts(*tPt, &dxy, onPt, tangent);
    return true;
}

// Given a cubic and a t range, find the start and end if they haven't been found already.
bool SkPathStroker::cubicQuadEnds(const SkPoint cubic[4], SkQuadConstruct* quadPts) {
    if (!quadPts->fStartSet) {
        SkPoint cubicStartPt;
        if (!this->cubicPerpRay(cubic, quadPts->fStartT, &cubicStartPt, &quadPts->fQuad[0],
                &quadPts->fTangentStart)) {
            return false;
        }
        quadPts->fStartSet = true;
    }
    if (!quadPts->fEndSet) {
        SkPoint cubicEndPt;
        if (!this->cubicPerpRay(cubic, quadPts->fEndT, &cubicEndPt, &quadPts->fQuad[2],
                &quadPts->fTangentEnd)) {
            return false;
        }
        quadPts->fEndSet = true;
    }
    return true;
}

bool SkPathStroker::cubicQuadMid(const SkPoint cubic[4], const SkQuadConstruct* quadPts,
        SkPoint* mid) const {
    SkPoint cubicMidPt;
    return this->cubicPerpRay(cubic, quadPts->fMidT, &cubicMidPt, mid, NULL);
}

// Given a quad and t, return the point on curve, its perpendicular, and the perpendicular tangent.
void SkPathStroker::quadPerpRay(const SkPoint quad[3], SkScalar t, SkPoint* tPt, SkPoint* onPt,
        SkPoint* tangent) const {
    SkVector dxy;
    SkEvalQuadAt(quad, t, tPt, &dxy);
    if (dxy.fX == 0 && dxy.fY == 0) {
        dxy = quad[2] - quad[0];
    }
    setRayPts(*tPt, &dxy, onPt, tangent);
}

// Find the intersection of the stroke tangents to construct a stroke quad.
// Return whether the stroke is a degenerate (a line), a quad, or must be split.
// Optionally compute the quad's control point.
SkPathStroker::ResultType SkPathStroker::intersectRay(SkQuadConstruct* quadPts,
        IntersectRayType intersectRayType  STROKER_DEBUG_PARAMS(int depth)) const {
    const SkPoint& start = quadPts->fQuad[0];
    const SkPoint& end = quadPts->fQuad[2];
    SkVector aLen = quadPts->fTangentStart - start;
    SkVector bLen = quadPts->fTangentEnd - end;
    /* Slopes match when denom goes to zero:
                      axLen / ayLen ==                   bxLen / byLen
    (ayLen * byLen) * axLen / ayLen == (ayLen * byLen) * bxLen / byLen
             byLen  * axLen         ==  ayLen          * bxLen
             byLen  * axLen         -   ayLen          * bxLen         ( == denom )
     */
    SkScalar denom = aLen.cross(bLen);
    if (denom == 0 || !SkScalarIsFinite(denom)) {
        return STROKER_RESULT(kDegenerate_ResultType, depth, quadPts, "denom == 0");
    }
    SkVector ab0 = start - end;
    SkScalar numerA = bLen.cross(ab0);
    SkScalar numerB = aLen.cross(ab0);
    if ((numerA >= 0) == (numerB >= 0)) { // if the control point is outside the quad ends
        // if the perpendicular distances from the quad points to the opposite tangent line
        // are small, a straight line is good enough
        SkScalar dist1 = pt_to_line(start, end, quadPts->fTangentEnd);
        SkScalar dist2 = pt_to_line(end, start, quadPts->fTangentStart);
        if (SkTMax(dist1, dist2) <= fInvResScaleSquared) {
            return STROKER_RESULT(kDegenerate_ResultType, depth, quadPts,
                    "SkTMax(dist1=%g, dist2=%g) <= fInvResScaleSquared", dist1, dist2);
        }
        return STROKER_RESULT(kSplit_ResultType, depth, quadPts,
                "(numerA=%g >= 0) == (numerB=%g >= 0)", numerA, numerB);
    }
    // check to see if the denomerator is teeny relative to the numerator
    bool validDivide = SkScalarAbs(numerA) * SK_ScalarNearlyZero < SkScalarAbs(denom);
// the divide check is the same as checking if the scaled denom is nearly zero
// (commented out because on some platforms the two are not bit-identical)
//  SkASSERT(!SkScalarNearlyZero(denom / numerA) == validDivide);
    if (validDivide) {
        if (kCtrlPt_RayType == intersectRayType) {
            numerA /= denom;
            SkPoint* ctrlPt = &quadPts->fQuad[1];
            // the intersection of the tangents need not be on the tangent segment
            // so 0 <= numerA <= 1 is not necessarily true
            ctrlPt->fX = start.fX * (1 - numerA) + quadPts->fTangentStart.fX * numerA;
            ctrlPt->fY = start.fY * (1 - numerA) + quadPts->fTangentStart.fY * numerA;
        }
        return STROKER_RESULT(kQuad_ResultType, depth, quadPts,
                "(numerA=%g >= 0) != (numerB=%g >= 0)", numerA, numerB);
    }
    // if the lines are parallel, straight line is good enough
    return STROKER_RESULT(kDegenerate_ResultType, depth, quadPts,
            "SkScalarNearlyZero(denom=%g)", denom);
}

// Given a cubic and a t-range, determine if the stroke can be described by a quadratic.
SkPathStroker::ResultType SkPathStroker::tangentsMeet(const SkPoint cubic[4],
        SkQuadConstruct* quadPts) {
    if (!this->cubicQuadEnds(cubic, quadPts)) {
        return kNormalError_ResultType;
    }
    return this->intersectRay(quadPts, kResultType_RayType  STROKER_DEBUG_PARAMS(fRecursionDepth));
}

// Intersect the line with the quad and return the t values on the quad where the line crosses.
static int intersect_quad_ray(const SkPoint line[2], const SkPoint quad[3], SkScalar roots[2]) {
    SkVector vec = line[1] - line[0];
    SkScalar r[3];
    for (int n = 0; n < 3; ++n) {
        r[n] = (quad[n].fY - line[0].fY) * vec.fX - (quad[n].fX - line[0].fX) * vec.fY;
    }
    SkScalar A = r[2];
    SkScalar B = r[1];
    SkScalar C = r[0];
    A += C - 2 * B;  // A = a - 2*b + c
    B -= C;  // B = -(b - c)
    return SkFindUnitQuadRoots(A, 2 * B, C, roots);
}

// Return true if the point is close to the bounds of the quad. This is used as a quick reject.
bool SkPathStroker::ptInQuadBounds(const SkPoint quad[3], const SkPoint& pt) const {
    SkScalar xMin = SkTMin(SkTMin(quad[0].fX, quad[1].fX), quad[2].fX);
    if (pt.fX + fInvResScale < xMin) {
        return false;
    }
    SkScalar xMax = SkTMax(SkTMax(quad[0].fX, quad[1].fX), quad[2].fX);
    if (pt.fX - fInvResScale > xMax) {
        return false;
    }
    SkScalar yMin = SkTMin(SkTMin(quad[0].fY, quad[1].fY), quad[2].fY);
    if (pt.fY + fInvResScale < yMin) {
        return false;
    }
    SkScalar yMax = SkTMax(SkTMax(quad[0].fY, quad[1].fY), quad[2].fY);
    if (pt.fY - fInvResScale > yMax) {
        return false;
    }
    return true;
}

static bool points_within_dist(const SkPoint& nearPt, const SkPoint& farPt, SkScalar limit) {
    return nearPt.distanceToSqd(farPt) <= limit * limit;
}

static bool sharp_angle(const SkPoint quad[3]) {
    SkVector smaller = quad[1] - quad[0];
    SkVector larger = quad[1] - quad[2];
    SkScalar smallerLen = smaller.lengthSqd();
    SkScalar largerLen = larger.lengthSqd();
    if (smallerLen > largerLen) {
        SkTSwap(smaller, larger);
        largerLen = smallerLen;
    }
    if (!smaller.setLength(largerLen)) {
        return false;
    }
    SkScalar dot = smaller.dot(larger);
    return dot > 0;
}

SkPathStroker::ResultType SkPathStroker::strokeCloseEnough(const SkPoint stroke[3],
        const SkPoint ray[2], SkQuadConstruct* quadPts  STROKER_DEBUG_PARAMS(int depth)) const {
    SkPoint strokeMid = SkEvalQuadAt(stroke, SK_ScalarHalf);
    // measure the distance from the curve to the quad-stroke midpoint, compare to radius
    if (points_within_dist(ray[0], strokeMid, fInvResScale)) {  // if the difference is small
        if (sharp_angle(quadPts->fQuad)) {
            return STROKER_RESULT(kSplit_ResultType, depth, quadPts,
                    "sharp_angle (1) =%g,%g, %g,%g, %g,%g",
                    quadPts->fQuad[0].fX, quadPts->fQuad[0].fY,
                    quadPts->fQuad[1].fX, quadPts->fQuad[1].fY,
                    quadPts->fQuad[2].fX, quadPts->fQuad[2].fY);
        }
        return STROKER_RESULT(kQuad_ResultType, depth, quadPts,
                "points_within_dist(ray[0]=%g,%g, strokeMid=%g,%g, fInvResScale=%g)",
                ray[0].fX, ray[0].fY, strokeMid.fX, strokeMid.fY, fInvResScale);
    }
    // measure the distance to quad's bounds (quick reject)
        // an alternative : look for point in triangle
    if (!ptInQuadBounds(stroke, ray[0])) {  // if far, subdivide
        return STROKER_RESULT(kSplit_ResultType, depth, quadPts,
                "!pt_in_quad_bounds(stroke=(%g,%g %g,%g %g,%g), ray[0]=%g,%g)",
                stroke[0].fX, stroke[0].fY, stroke[1].fX, stroke[1].fY, stroke[2].fX, stroke[2].fY,
                ray[0].fX, ray[0].fY);
    }
    // measure the curve ray distance to the quad-stroke
    SkScalar roots[2];
    int rootCount = intersect_quad_ray(ray, stroke, roots);
    if (rootCount != 1) {
        return STROKER_RESULT(kSplit_ResultType, depth, quadPts,
                "rootCount=%d != 1", rootCount);
    }
    SkPoint quadPt = SkEvalQuadAt(stroke, roots[0]);
    SkScalar error = fInvResScale * (SK_Scalar1 - SkScalarAbs(roots[0] - 0.5f) * 2);
    if (points_within_dist(ray[0], quadPt, error)) {  // if the difference is small, we're done
        if (sharp_angle(quadPts->fQuad)) {
            return STROKER_RESULT(kSplit_ResultType, depth, quadPts,
                    "sharp_angle (2) =%g,%g, %g,%g, %g,%g",
                    quadPts->fQuad[0].fX, quadPts->fQuad[0].fY,
                    quadPts->fQuad[1].fX, quadPts->fQuad[1].fY,
                    quadPts->fQuad[2].fX, quadPts->fQuad[2].fY);
        }
        return STROKER_RESULT(kQuad_ResultType, depth, quadPts,
                "points_within_dist(ray[0]=%g,%g, quadPt=%g,%g, error=%g)",
                ray[0].fX, ray[0].fY, quadPt.fX, quadPt.fY, error);
    }
    // otherwise, subdivide
    return STROKER_RESULT(kSplit_ResultType, depth, quadPts, "%s", "fall through");
}

SkPathStroker::ResultType SkPathStroker::compareQuadCubic(const SkPoint cubic[4],
        SkQuadConstruct* quadPts) {
    // get the quadratic approximation of the stroke
    if (!this->cubicQuadEnds(cubic, quadPts)) {
        return kNormalError_ResultType;
    }
    ResultType resultType = this->intersectRay(quadPts, kCtrlPt_RayType
            STROKER_DEBUG_PARAMS(fRecursionDepth) );
    if (resultType != kQuad_ResultType) {
        return resultType;
    }
    // project a ray from the curve to the stroke
    SkPoint ray[2];  // points near midpoint on quad, midpoint on cubic
    if (!this->cubicPerpRay(cubic, quadPts->fMidT, &ray[1], &ray[0], NULL)) {
        return kNormalError_ResultType;
    }
    return this->strokeCloseEnough(quadPts->fQuad, ray, quadPts
            STROKER_DEBUG_PARAMS(fRecursionDepth));
}

SkPathStroker::ResultType SkPathStroker::compareQuadConic(const SkConic& conic,
        SkQuadConstruct* quadPts) const {
    // get the quadratic approximation of the stroke
    this->conicQuadEnds(conic, quadPts);
    ResultType resultType = this->intersectRay(quadPts, kCtrlPt_RayType
            STROKER_DEBUG_PARAMS(fRecursionDepth) );
    if (resultType != kQuad_ResultType) {
        return resultType;
    }
    // project a ray from the curve to the stroke
    SkPoint ray[2];  // points near midpoint on quad, midpoint on conic
    this->conicPerpRay(conic, quadPts->fMidT, &ray[1], &ray[0], NULL);
    return this->strokeCloseEnough(quadPts->fQuad, ray, quadPts
            STROKER_DEBUG_PARAMS(fRecursionDepth));
}

SkPathStroker::ResultType SkPathStroker::compareQuadQuad(const SkPoint quad[3],
        SkQuadConstruct* quadPts) {
    // get the quadratic approximation of the stroke
    if (!quadPts->fStartSet) {
        SkPoint quadStartPt;
        this->quadPerpRay(quad, quadPts->fStartT, &quadStartPt, &quadPts->fQuad[0],
                &quadPts->fTangentStart);
        quadPts->fStartSet = true;
    }
    if (!quadPts->fEndSet) {
        SkPoint quadEndPt;
        this->quadPerpRay(quad, quadPts->fEndT, &quadEndPt, &quadPts->fQuad[2],
                &quadPts->fTangentEnd);
        quadPts->fEndSet = true;
    }
    ResultType resultType = this->intersectRay(quadPts, kCtrlPt_RayType
            STROKER_DEBUG_PARAMS(fRecursionDepth));
    if (resultType != kQuad_ResultType) {
        return resultType;
    }
    // project a ray from the curve to the stroke
    SkPoint ray[2];
    this->quadPerpRay(quad, quadPts->fMidT, &ray[1], &ray[0], NULL);
    return this->strokeCloseEnough(quadPts->fQuad, ray, quadPts
            STROKER_DEBUG_PARAMS(fRecursionDepth));
}

void SkPathStroker::addDegenerateLine(const SkQuadConstruct* quadPts) {
    const SkPoint* quad = quadPts->fQuad;
    SkPath* path = fStrokeType == kOuter_StrokeType ? &fOuter : &fInner;
    path->lineTo(quad[2].fX, quad[2].fY);
}

bool SkPathStroker::cubicMidOnLine(const SkPoint cubic[4], const SkQuadConstruct* quadPts) const {
    SkPoint strokeMid;
    if (!cubicQuadMid(cubic, quadPts, &strokeMid)) {
        return false;
    }
    SkScalar dist = pt_to_line(strokeMid, quadPts->fQuad[0], quadPts->fQuad[2]);
    return dist < fInvResScaleSquared;
}

bool SkPathStroker::cubicStroke(const SkPoint cubic[4], SkQuadConstruct* quadPts) {
    if (!fFoundTangents) {
        ResultType resultType = this->tangentsMeet(cubic, quadPts);
        if (kQuad_ResultType != resultType) {
            if (kNormalError_ResultType == resultType) {
                return false;
            }
            if ((kDegenerate_ResultType == resultType
                    || points_within_dist(quadPts->fQuad[0], quadPts->fQuad[2],
                    fInvResScale)) && cubicMidOnLine(cubic, quadPts)) {
                addDegenerateLine(quadPts);
                return true;
            }
        } else {
            fFoundTangents = true;
        }
    }
    if (fFoundTangents) {
        ResultType resultType = this->compareQuadCubic(cubic, quadPts);
        if (kQuad_ResultType == resultType) {
            SkPath* path = fStrokeType == kOuter_StrokeType ? &fOuter : &fInner;
            const SkPoint* stroke = quadPts->fQuad;
            path->quadTo(stroke[1].fX, stroke[1].fY, stroke[2].fX, stroke[2].fY);
            return true;
        }
        if (kDegenerate_ResultType == resultType) {
            addDegenerateLine(quadPts);
            return true;
        }
        if (kNormalError_ResultType == resultType) {
            return false;
        }
    }
    if (!SkScalarIsFinite(quadPts->fQuad[2].fX) || !SkScalarIsFinite(quadPts->fQuad[2].fY)) {
        return false;  // just abort if projected quad isn't representable
    }
    SkDEBUGCODE(gMaxRecursion[fFoundTangents] = SkTMax(gMaxRecursion[fFoundTangents],
            fRecursionDepth + 1));
    if (++fRecursionDepth > kRecursiveLimits[fFoundTangents]) {
        return false;  // just abort if projected quad isn't representable
    }
    SkQuadConstruct half;
    if (!half.initWithStart(quadPts)) {
        addDegenerateLine(quadPts);
        return true;
    }
    if (!this->cubicStroke(cubic, &half)) {
        return false;
    }
    if (!half.initWithEnd(quadPts)) {
        addDegenerateLine(quadPts);
        return true;
    }
    if (!this->cubicStroke(cubic, &half)) {
        return false;
    }
    --fRecursionDepth;
    return true;
}

bool SkPathStroker::conicStroke(const SkConic& conic, SkQuadConstruct* quadPts) {
    ResultType resultType = this->compareQuadConic(conic, quadPts);
    if (kQuad_ResultType == resultType) {
        const SkPoint* stroke = quadPts->fQuad;
        SkPath* path = fStrokeType == kOuter_StrokeType ? &fOuter : &fInner;
        path->quadTo(stroke[1].fX, stroke[1].fY, stroke[2].fX, stroke[2].fY);
        return true;
    }
    if (kDegenerate_ResultType == resultType) {
        addDegenerateLine(quadPts);
        return true;
    }
    SkDEBUGCODE(gMaxRecursion[kConic_RecursiveLimit] = SkTMax(gMaxRecursion[kConic_RecursiveLimit],
            fRecursionDepth + 1));
    if (++fRecursionDepth > kRecursiveLimits[kConic_RecursiveLimit]) {
        return false;  // just abort if projected quad isn't representable
    }
    SkQuadConstruct half;
    (void) half.initWithStart(quadPts);
    if (!this->conicStroke(conic, &half)) {
        return false;
    }
    (void) half.initWithEnd(quadPts);
    if (!this->conicStroke(conic, &half)) {
        return false;
    }
    --fRecursionDepth;
    return true;
}

bool SkPathStroker::quadStroke(const SkPoint quad[3], SkQuadConstruct* quadPts) {
    ResultType resultType = this->compareQuadQuad(quad, quadPts);
    if (kQuad_ResultType == resultType) {
        const SkPoint* stroke = quadPts->fQuad;
        SkPath* path = fStrokeType == kOuter_StrokeType ? &fOuter : &fInner;
        path->quadTo(stroke[1].fX, stroke[1].fY, stroke[2].fX, stroke[2].fY);
        return true;
    }
    if (kDegenerate_ResultType == resultType) {
        addDegenerateLine(quadPts);
        return true;
    }
    SkDEBUGCODE(gMaxRecursion[kQuad_RecursiveLimit] = SkTMax(gMaxRecursion[kQuad_RecursiveLimit],
            fRecursionDepth + 1));
    if (++fRecursionDepth > kRecursiveLimits[kQuad_RecursiveLimit]) {
        return false;  // just abort if projected quad isn't representable
    }
    SkQuadConstruct half;
    (void) half.initWithStart(quadPts);
    if (!this->quadStroke(quad, &half)) {
        return false;
    }
    (void) half.initWithEnd(quadPts);
    if (!this->quadStroke(quad, &half)) {
        return false;
    }
    --fRecursionDepth;
    return true;
}

void SkPathStroker::cubicTo(const SkPoint& pt1, const SkPoint& pt2,
                            const SkPoint& pt3) {
    const SkPoint cubic[4] = { fPrevPt, pt1, pt2, pt3 };
    SkPoint reduction[3];
    const SkPoint* tangentPt;
    ReductionType reductionType = CheckCubicLinear(cubic, reduction, &tangentPt);
    if (kPoint_ReductionType == reductionType) {
        return;
    }
    if (kLine_ReductionType == reductionType) {
        this->lineTo(pt3);
        return;
    }
    if (kDegenerate_ReductionType <= reductionType && kDegenerate3_ReductionType >= reductionType) {
        this->lineTo(reduction[0]);
        SkStrokerPriv::JoinProc saveJoiner = fJoiner;
        fJoiner = SkStrokerPriv::JoinFactory(SkPaint::kRound_Join);
        if (kDegenerate2_ReductionType <= reductionType) {
            this->lineTo(reduction[1]);
        }
        if (kDegenerate3_ReductionType == reductionType) {
            this->lineTo(reduction[2]);
        }
        this->lineTo(pt3);
        fJoiner = saveJoiner;
        return;
    }
    SkASSERT(kQuad_ReductionType == reductionType);
    SkVector normalAB, unitAB, normalCD, unitCD;
    if (!this->preJoinTo(*tangentPt, &normalAB, &unitAB, false)) {
        this->lineTo(pt3);
        return;
    }
    SkScalar tValues[2];
    int count = SkFindCubicInflections(cubic, tValues);
    SkScalar lastT = 0;
    for (int index = 0; index <= count; ++index) {
        SkScalar nextT = index < count ? tValues[index] : 1;
        SkQuadConstruct quadPts;
        this->init(kOuter_StrokeType, &quadPts, lastT, nextT);
        (void) this->cubicStroke(cubic, &quadPts);
        this->init(kInner_StrokeType, &quadPts, lastT, nextT);
        (void) this->cubicStroke(cubic, &quadPts);
        lastT = nextT;
    }
    // emit the join even if one stroke succeeded but the last one failed
    // this avoids reversing an inner stroke with a partial path followed by another moveto
    this->setCubicEndNormal(cubic, normalAB, unitAB, &normalCD, &unitCD);

    this->postJoinTo(pt3, normalCD, unitCD);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkPaintDefaults.h"

SkStroke::SkStroke() {
    fWidth      = SK_Scalar1;
    fMiterLimit = SkPaintDefaults_MiterLimit;
    fCap        = SkPaint::kDefault_Cap;
    fJoin       = SkPaint::kDefault_Join;
    fDoFill     = false;
}

SkStroke::SkStroke(const SkPaint& p) {
    fWidth      = p.getStrokeWidth();
    fMiterLimit = p.getStrokeMiter();
    fCap        = (uint8_t)p.getStrokeCap();
    fJoin       = (uint8_t)p.getStrokeJoin();
    fDoFill     = SkToU8(p.getStyle() == SkPaint::kStrokeAndFill_Style);
}

SkStroke::SkStroke(const SkPaint& p, SkScalar width) {
    fWidth      = width;
    fMiterLimit = p.getStrokeMiter();
    fCap        = (uint8_t)p.getStrokeCap();
    fJoin       = (uint8_t)p.getStrokeJoin();
    fDoFill     = SkToU8(p.getStyle() == SkPaint::kStrokeAndFill_Style);
}

void SkStroke::setWidth(SkScalar width) {
    SkASSERT(width >= 0);
    fWidth = width;
}

void SkStroke::setMiterLimit(SkScalar miterLimit) {
    SkASSERT(miterLimit >= 0);
    fMiterLimit = miterLimit;
}

void SkStroke::setCap(SkPaint::Cap cap) {
    SkASSERT((unsigned)cap < SkPaint::kCapCount);
    fCap = SkToU8(cap);
}

void SkStroke::setJoin(SkPaint::Join join) {
    SkASSERT((unsigned)join < SkPaint::kJoinCount);
    fJoin = SkToU8(join);
}

///////////////////////////////////////////////////////////////////////////////

// If src==dst, then we use a tmp path to record the stroke, and then swap
// its contents with src when we're done.
class AutoTmpPath {
public:
    AutoTmpPath(const SkPath& src, SkPath** dst) : fSrc(src) {
        if (&src == *dst) {
            *dst = &fTmpDst;
            fSwapWithSrc = true;
        } else {
            (*dst)->reset();
            fSwapWithSrc = false;
        }
    }

    ~AutoTmpPath() {
        if (fSwapWithSrc) {
            fTmpDst.swap(*const_cast<SkPath*>(&fSrc));
        }
    }

private:
    SkPath          fTmpDst;
    const SkPath&   fSrc;
    bool            fSwapWithSrc;
};

void SkStroke::strokePath(const SkPath& src, SkPath* dst) const {
    SkASSERT(dst);

    SkScalar radius = SkScalarHalf(fWidth);

    AutoTmpPath tmp(src, &dst);

    if (radius <= 0) {
        return;
    }

    // If src is really a rect, call our specialty strokeRect() method
    {
        SkRect rect;
        bool isClosed;
        SkPath::Direction dir;
        if (src.isRect(&rect, &isClosed, &dir) && isClosed) {
            this->strokeRect(rect, dst, dir);
            // our answer should preserve the inverseness of the src
            if (src.isInverseFillType()) {
                SkASSERT(!dst->isInverseFillType());
                dst->toggleInverseFillType();
            }
            return;
        }
    }

    SkPathStroker   stroker(src, radius, fMiterLimit, this->getCap(), this->getJoin(), fResScale);
    SkPath::Iter    iter(src, false);
    SkPath::Verb    lastSegment = SkPath::kMove_Verb;

    for (;;) {
        SkPoint  pts[4];
        switch (iter.next(pts, false)) {
            case SkPath::kMove_Verb:
                stroker.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                stroker.lineTo(pts[1]);
                lastSegment = SkPath::kLine_Verb;
                break;
            case SkPath::kQuad_Verb:
                stroker.quadTo(pts[1], pts[2]);
                lastSegment = SkPath::kQuad_Verb;
                break;
            case SkPath::kConic_Verb: {
                stroker.conicTo(pts[1], pts[2], iter.conicWeight());
                lastSegment = SkPath::kConic_Verb;
                break;
            } break;
            case SkPath::kCubic_Verb:
                stroker.cubicTo(pts[1], pts[2], pts[3]);
                lastSegment = SkPath::kCubic_Verb;
                break;
            case SkPath::kClose_Verb:
                stroker.close(lastSegment == SkPath::kLine_Verb);
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
    }
DONE:
    stroker.done(dst, lastSegment == SkPath::kLine_Verb);

    if (fDoFill) {
        if (SkPathPriv::CheapIsFirstDirection(src, SkPathPriv::kCCW_FirstDirection)) {
            dst->reverseAddPath(src);
        } else {
            dst->addPath(src);
        }
    } else {
        //  Seems like we can assume that a 2-point src would always result in
        //  a convex stroke, but testing has proved otherwise.
        //  TODO: fix the stroker to make this assumption true (without making
        //  it slower that the work that will be done in computeConvexity())
#if 0
        // this test results in a non-convex stroke :(
        static void test(SkCanvas* canvas) {
            SkPoint pts[] = { 146.333328,  192.333328, 300.333344, 293.333344 };
            SkPaint paint;
            paint.setStrokeWidth(7);
            paint.setStrokeCap(SkPaint::kRound_Cap);
            canvas->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, paint);
        }
#endif
#if 0
        if (2 == src.countPoints()) {
            dst->setIsConvex(true);
        }
#endif
    }

    // our answer should preserve the inverseness of the src
    if (src.isInverseFillType()) {
        SkASSERT(!dst->isInverseFillType());
        dst->toggleInverseFillType();
    }
}

static SkPath::Direction reverse_direction(SkPath::Direction dir) {
    static const SkPath::Direction gOpposite[] = { SkPath::kCCW_Direction, SkPath::kCW_Direction };
    return gOpposite[dir];
}

static void addBevel(SkPath* path, const SkRect& r, const SkRect& outer, SkPath::Direction dir) {
    SkPoint pts[8];

    if (SkPath::kCW_Direction == dir) {
        pts[0].set(r.fLeft, outer.fTop);
        pts[1].set(r.fRight, outer.fTop);
        pts[2].set(outer.fRight, r.fTop);
        pts[3].set(outer.fRight, r.fBottom);
        pts[4].set(r.fRight, outer.fBottom);
        pts[5].set(r.fLeft, outer.fBottom);
        pts[6].set(outer.fLeft, r.fBottom);
        pts[7].set(outer.fLeft, r.fTop);
    } else {
        pts[7].set(r.fLeft, outer.fTop);
        pts[6].set(r.fRight, outer.fTop);
        pts[5].set(outer.fRight, r.fTop);
        pts[4].set(outer.fRight, r.fBottom);
        pts[3].set(r.fRight, outer.fBottom);
        pts[2].set(r.fLeft, outer.fBottom);
        pts[1].set(outer.fLeft, r.fBottom);
        pts[0].set(outer.fLeft, r.fTop);
    }
    path->addPoly(pts, 8, true);
}

void SkStroke::strokeRect(const SkRect& origRect, SkPath* dst,
                          SkPath::Direction dir) const {
    SkASSERT(dst != NULL);
    dst->reset();

    SkScalar radius = SkScalarHalf(fWidth);
    if (radius <= 0) {
        return;
    }

    SkScalar rw = origRect.width();
    SkScalar rh = origRect.height();
    if ((rw < 0) ^ (rh < 0)) {
        dir = reverse_direction(dir);
    }
    SkRect rect(origRect);
    rect.sort();
    // reassign these, now that we know they'll be >= 0
    rw = rect.width();
    rh = rect.height();

    SkRect r(rect);
    r.outset(radius, radius);

    SkPaint::Join join = (SkPaint::Join)fJoin;
    if (SkPaint::kMiter_Join == join && fMiterLimit < SK_ScalarSqrt2) {
        join = SkPaint::kBevel_Join;
    }

    switch (join) {
        case SkPaint::kMiter_Join:
            dst->addRect(r, dir);
            break;
        case SkPaint::kBevel_Join:
            addBevel(dst, rect, r, dir);
            break;
        case SkPaint::kRound_Join:
            dst->addRoundRect(r, radius, radius, dir);
            break;
        default:
            break;
    }

    if (fWidth < SkMinScalar(rw, rh) && !fDoFill) {
        r = rect;
        r.inset(radius, radius);
        dst->addRect(r, reverse_direction(dir));
    }
}
