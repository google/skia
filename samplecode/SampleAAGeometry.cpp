/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGeometry.h"
#include "SkIntersections.h"
#include "SkOpEdgeBuilder.h"
// #include "SkPathOpsSimplifyAA.h"
// #include "SkPathStroker.h"
#include "SkPointPriv.h"
#include "SkPathMeasure.h"
#include "SkStrokeRec.h"
#include "SkView.h"

struct CurveData;

struct ContourData {
    CurveData* fFirst;
    CurveData* fLast;
    int fPathIndex;  // index of first contour point within path (not sure if I need this)
};

enum class CurvePos {
    kUnknown,
    kFirst,
    kMid,
    kLast,
};

// one per non-degenerate curve
struct CurveData {
    SkPoint fPts[4];
    SkVector fTangents[2];
    SkScalar fWeight;
    SkScalar fStartT;
    SkScalar fEndT;
    int fPathIndex;  // index of fPts[0] within path (not sure if I need this)
    SkPath::Verb fVerb;
    CurvePos fPos;
};

// one per intersection of a pair of curves
struct SectData {
    CurveData* fCurve[2];
    SkPoint fL;   // mid angle formed by prev, next segs; length == stroke width; on the left
    SkPoint fR;   // mid angle formed by prev, next segs; length == stroke width; on the right
    SkPoint fBisects[2];
};

SkTDArray<ContourData> fContourData;
SkTDArray<CurveData> fCurveData;
SkTDArray<SectData> fSectData;

static inline bool degenerate_vector(const SkVector& v) {
    return !SkPointPriv::CanNormalize(v.fX, v.fY);
}

enum {
    kTangent_RecursiveLimit,
    kCubic_RecursiveLimit,
    kConic_RecursiveLimit,
    kQuad_RecursiveLimit
};

static const int kRecursiveLimits[] = { 5*3, 26*3, 11*3, 11*3 }; // 3x limits seen in practice

struct QuadConstruct {    // The state of the quad stroke under construction.
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

    bool initWithStart(QuadConstruct* parent) {
        if (!init(parent->fStartT, parent->fMidT)) {
            return false;
        }
        fQuad[0] = parent->fQuad[0];
        fTangentStart = parent->fTangentStart;
        fStartSet = true;
        return true;
    }

    bool initWithEnd(QuadConstruct* parent) {
        if (!init(parent->fMidT, parent->fEndT)) {
            return false;
        }
        fQuad[2] = parent->fQuad[2];
        fTangentEnd = parent->fTangentEnd;
        fEndSet = true;
        return true;
   }
};

#define DEBUG_QUAD_STROKER 0
#if DEBUG_QUAD_STROKER
    /* Enable to show the decisions made in subdividing the curve -- helpful when the resulting
        stroke has more than the optimal number of quadratics and lines */
    #define STROKER_RESULT(resultType, depth, quadPts, format, ...) \
            SkDebugf("[%d] %s " format "\n", depth, __FUNCTION__, __VA_ARGS__), \
            SkDebugf("  " #resultType " t=(%g,%g)\n", quadPts->fStartT, quadPts->fEndT), \
            resultType
#else
    #define STROKER_RESULT(resultType, depth, quadPts, format, ...) \
            resultType
#endif

class PathStroker {
public:
    enum StrokeType {
        kOuter_StrokeType = 1,      // use sign-opposite values later to flip perpendicular axis
        kInner_StrokeType = -1
    };

    PathStroker(SkScalar radius, StrokeType strokeType)
        : fRadius(radius)
        , fInvResScale(1)
        , fInvResScaleSquared(1)
        , fStrokeType(strokeType)
        , fRecursionDepth(0) {
    }

    bool quadStroke(const SkPoint quad[3], QuadConstruct* quadPts) {
        ResultType resultType = this->compareQuadQuad(quad, quadPts);
        if (kQuad_ResultType == resultType) {
            const SkPoint* stroke = quadPts->fQuad;
            SkPath* path = fStrokeType == kOuter_StrokeType ? &fOuter : &fInner;
            if (path->isEmpty()) {
                path->moveTo(stroke[0]);
            }
            path->quadTo(stroke[1], stroke[2]);
            return true;
        }
        if (kDegenerate_ResultType == resultType) {
            addDegenerateLine(quadPts);
            return true;
        }
    #if QUAD_STROKE_APPROX_EXTENDED_DEBUGGING
        SkDEBUGCODE(gMaxRecursion[kQuad_RecursiveLimit] = SkTMax(gMaxRecursion[kQuad_RecursiveLimit],
                fRecursionDepth + 1));
    #endif
        if (++fRecursionDepth > kRecursiveLimits[kQuad_RecursiveLimit]) {
            return false;  // just abort if projected quad isn't representable
        }
        QuadConstruct half;
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

    const SkPath& inner() const { return fInner; }
    const SkPath& outer() const { return fOuter; }

private:
    enum ResultType {
        kSplit_ResultType,          // the caller should split the quad stroke in two
        kDegenerate_ResultType,     // the caller should add a line
        kQuad_ResultType,           // the caller should (continue to try to) add a quad stroke
    };

    enum IntersectRayType {
        kCtrlPt_RayType,
        kResultType_RayType,
    };

    void addDegenerateLine(const QuadConstruct* quadPts) {
        const SkPoint* quad = quadPts->fQuad;
        SkPath* path = fStrokeType == kOuter_StrokeType ? &fOuter : &fInner;
        path->lineTo(quad[2].fX, quad[2].fY);
    }

    ResultType compareQuadQuad(const SkPoint quad[3], QuadConstruct* quadPts) {
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
        ResultType resultType = this->intersectRay(quadPts, kCtrlPt_RayType, fRecursionDepth);
        if (resultType != kQuad_ResultType) {
            return resultType;
        }
        // project a ray from the curve to the stroke
        SkPoint ray[2];
        this->quadPerpRay(quad, quadPts->fMidT, &ray[1], &ray[0], nullptr);
        return this->strokeCloseEnough(quadPts->fQuad, ray, quadPts, fRecursionDepth);
    }

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

    ResultType intersectRay(QuadConstruct* quadPts,
        IntersectRayType intersectRayType, int depth) const {
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
            quadPts->fOppositeTangents = aLen.dot(bLen) < 0;
            return STROKER_RESULT(kDegenerate_ResultType, depth, quadPts, "denom == 0");
        }
        quadPts->fOppositeTangents = false;
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
        // check to see if the denominator is teeny relative to the numerator
        // if the offset by one will be lost, the ratio is too large
        numerA /= denom;
        bool validDivide = numerA > numerA - 1;
        if (validDivide) {
            if (kCtrlPt_RayType == intersectRayType) {
                SkPoint* ctrlPt = &quadPts->fQuad[1];
                // the intersection of the tangents need not be on the tangent segment
                // so 0 <= numerA <= 1 is not necessarily true
                ctrlPt->fX = start.fX * (1 - numerA) + quadPts->fTangentStart.fX * numerA;
                ctrlPt->fY = start.fY * (1 - numerA) + quadPts->fTangentStart.fY * numerA;
            }
            return STROKER_RESULT(kQuad_ResultType, depth, quadPts,
                    "(numerA=%g >= 0) != (numerB=%g >= 0)", numerA, numerB);
        }
        quadPts->fOppositeTangents = aLen.dot(bLen) < 0;
        // if the lines are parallel, straight line is good enough
        return STROKER_RESULT(kDegenerate_ResultType, depth, quadPts,
                "SkScalarNearlyZero(denom=%g)", denom);
    }

    static bool points_within_dist(const SkPoint& nearPt, const SkPoint& farPt, SkScalar limit) {
        return SkPointPriv::DistanceToSqd(nearPt, farPt) <= limit * limit;
    }

    static SkScalar pt_to_line(const SkPoint& pt, const SkPoint& lineStart, const SkPoint& lineEnd) {
        SkVector dxy = lineEnd - lineStart;
        if (degenerate_vector(dxy)) {
            return SkPointPriv::DistanceToSqd(pt, lineStart);
        }
        SkVector ab0 = pt - lineStart;
        SkScalar numer = dxy.dot(ab0);
        SkScalar denom = dxy.dot(dxy);
        SkScalar t = numer / denom;
        SkPoint hit;
        hit.fX = lineStart.fX * (1 - t) + lineEnd.fX * t;
        hit.fY = lineStart.fY * (1 - t) + lineEnd.fY * t;
        return SkPointPriv::DistanceToSqd(hit, pt);
    }

    bool ptInQuadBounds(const SkPoint quad[3], const SkPoint& pt) const {
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

    void quadPerpRay(const SkPoint quad[3], SkScalar t, SkPoint* tPt, SkPoint* onPt,
        SkPoint* tangent) const {
        SkVector dxy;
        SkEvalQuadAt(quad, t, tPt, &dxy);
        if (dxy.fX == 0 && dxy.fY == 0) {
            dxy = quad[2] - quad[0];
        }
        setRayPts(*tPt, &dxy, onPt, tangent);
    }

    void setRayPts(const SkPoint& tPt, SkVector* dxy, SkPoint* onPt,
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

    static bool sharp_angle(const SkPoint quad[3]) {
        SkVector smaller = quad[1] - quad[0];
        SkVector larger = quad[1] - quad[2];
        SkScalar smallerLen = SkPointPriv::LengthSqd(smaller);
        SkScalar largerLen = SkPointPriv::LengthSqd(larger);
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

    ResultType strokeCloseEnough(const SkPoint stroke[3],
            const SkPoint ray[2], QuadConstruct* quadPts, int depth) const {
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



    SkPath  fInner, fOuter; // outer is our working answer, inner is temp
    SkScalar fRadius;
    SkScalar    fInvResScale;
    SkScalar    fInvResScaleSquared;
    StrokeType  fStrokeType;
    int fRecursionDepth;
};

#if 0
void SkStrokeSegment::dump() const {
    SkDebugf("{{{%1.9g,%1.9g}, {%1.9g,%1.9g}", fPts[0].fX, fPts[0].fY, fPts[1].fX, fPts[1].fY);
    if (SkPath::kQuad_Verb == fVerb) {
        SkDebugf(", {%1.9g,%1.9g}", fPts[2].fX, fPts[2].fY);
    }
    SkDebugf("}}");
#ifdef SK_DEBUG
    SkDebugf(" id=%d", fDebugID);
#endif
    SkDebugf("\n");
}

void SkStrokeSegment::dumpAll() const {
    const SkStrokeSegment* segment = this;
    while (segment) {
        segment->dump();
        segment = segment->fNext;
    }
}

void SkStrokeTriple::dump() const {
    SkDebugf("{{{%1.9g,%1.9g}, {%1.9g,%1.9g}", fPts[0].fX, fPts[0].fY, fPts[1].fX, fPts[1].fY);
    if (SkPath::kQuad_Verb <= fVerb) {
        SkDebugf(", {%1.9g,%1.9g}", fPts[2].fX, fPts[2].fY);
    }
    if (SkPath::kCubic_Verb == fVerb) {
        SkDebugf(", {%1.9g,%1.9g}", fPts[3].fX, fPts[3].fY);
    } else if (SkPath::kConic_Verb == fVerb) {
        SkDebugf(", %1.9g", weight());
    }
    SkDebugf("}}");
#ifdef SK_DEBUG
    SkDebugf(" triple id=%d", fDebugID);
#endif
    SkDebugf("\ninner:\n");
    fInner->dumpAll();
    SkDebugf("outer:\n");
    fOuter->dumpAll();
    SkDebugf("join:\n");
    fJoin->dumpAll();
}

void SkStrokeTriple::dumpAll() const {
    const SkStrokeTriple* triple = this;
    while (triple) {
        triple->dump();
        triple = triple->fNext;
    }
}

void SkStrokeContour::dump() const {
#ifdef SK_DEBUG
    SkDebugf("id=%d ", fDebugID);
#endif
    SkDebugf("head:\n");
    fHead->dumpAll();
    SkDebugf("head cap:\n");
    fHeadCap->dumpAll();
    SkDebugf("tail cap:\n");
    fTailCap->dumpAll();
}

void SkStrokeContour::dumpAll() const {
    const SkStrokeContour* contour = this;
    while (contour) {
        contour->dump();
        contour = contour->fNext;
    }
}
#endif

SkScalar gCurveDistance = 10;

#if 0  // unused
static SkPath::Verb get_path_verb(int index, const SkPath& path) {
    if (index < 0) {
        return SkPath::kMove_Verb;
    }
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (++counter < index) {
            continue;
        }
        return verb;
    }
    SkASSERT(0);
    return SkPath::kMove_Verb;
}
#endif

static SkScalar get_path_weight(int index, const SkPath& path) {
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (++counter < index) {
            continue;
        }
        return verb == SkPath::kConic_Verb ? iter.conicWeight() : 1;
    }
    SkASSERT(0);
    return 0;
}

static void set_path_pt(int index, const SkPoint& pt, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::RawIter iter(*path);
    int startIndex = 0;
    int endIndex = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                endIndex += 1;
                break;
            case SkPath::kLine_Verb:
                endIndex += 1;
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                endIndex += 2;
                break;
            case SkPath::kCubic_Verb:
                endIndex += 3;
                break;
            case SkPath::kClose_Verb:
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
        if (startIndex <= index && index < endIndex) {
            pts[index - startIndex] = pt;
            index = -1;
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                startIndex += 1;
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                startIndex += 2;
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], iter.conicWeight());
                startIndex += 2;
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                startIndex += 3;
                break;
            case SkPath::kClose_Verb:
                result.close();
                startIndex += 1;
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
    }
#if 0
    SkDebugf("\n\noriginal\n");
    path->dump();
    SkDebugf("\nedited\n");
    result.dump();
#endif
    *path = result;
}

static void add_path_segment(int index, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPoint firstPt = { 0, 0 };  // init to avoid warning
    SkPoint lastPt = { 0, 0 };  // init to avoid warning
    SkPath::Verb verb;
    SkPath::RawIter iter(*path);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        SkScalar weight  SK_INIT_TO_AVOID_WARNING;
        if (++counter == index) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    result.lineTo((pts[0].fX + pts[1].fX) / 2, (pts[0].fY + pts[1].fY) / 2);
                    break;
                case SkPath::kQuad_Verb: {
                    SkPoint chop[5];
                    SkChopQuadAtHalf(pts, chop);
                    result.quadTo(chop[1], chop[2]);
                    pts[1] = chop[3];
                    } break;
                case SkPath::kConic_Verb: {
                    SkConic chop[2];
                    SkConic conic;
                    conic.set(pts, iter.conicWeight());
                    if (!conic.chopAt(0.5f, chop)) {
                        return;
                    }
                    result.conicTo(chop[0].fPts[1], chop[0].fPts[2], chop[0].fW);
                    pts[1] = chop[1].fPts[1];
                    weight = chop[1].fW;
                    } break;
                case SkPath::kCubic_Verb: {
                    SkPoint chop[7];
                    SkChopCubicAtHalf(pts, chop);
                    result.cubicTo(chop[1], chop[2], chop[3]);
                    pts[1] = chop[4];
                    pts[2] = chop[5];
                    } break;
                case SkPath::kClose_Verb: {
                    result.lineTo((lastPt.fX + firstPt.fX) / 2, (lastPt.fY + firstPt.fY) / 2);
                    } break;
                default:
                    SkASSERT(0);
            }
        } else if (verb == SkPath::kConic_Verb) {
            weight = iter.conicWeight();
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(firstPt = pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(lastPt = pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], lastPt = pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], lastPt = pts[2], weight);
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], lastPt = pts[3]);
                break;
            case SkPath::kClose_Verb:
                result.close();
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
    }
    *path = result;
}

static void delete_path_segment(int index, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::RawIter iter(*path);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (++counter == index) {
            continue;
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPath::kClose_Verb:
                result.close();
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
    }
    *path = result;
}

static void set_path_weight(int index, SkScalar w, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(*path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        ++counter;
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], counter == index ? w : iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPath::kClose_Verb:
                result.close();
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
    }
    *path = result;
}

static void set_path_verb(int index, SkPath::Verb v, SkPath* path, SkScalar w) {
    SkASSERT(SkPath::kLine_Verb <= v && v <= SkPath::kCubic_Verb);
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(*path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        SkScalar weight = verb == SkPath::kConic_Verb ? iter.conicWeight() : 1;
        if (++counter == index && v != verb) {
            SkASSERT(SkPath::kLine_Verb <= verb && verb <= SkPath::kCubic_Verb);
            switch (verb) {
                case SkPath::kLine_Verb:
                    switch (v) {
                        case SkPath::kConic_Verb:
                            weight = w;
                        case SkPath::kQuad_Verb:
                            pts[2] = pts[1];
                            pts[1].fX = (pts[0].fX + pts[2].fX) / 2;
                            pts[1].fY = (pts[0].fY + pts[2].fY) / 2;
                            break;
                        case SkPath::kCubic_Verb:
                            pts[3] = pts[1];
                            pts[1].fX = (pts[0].fX * 2 + pts[3].fX) / 3;
                            pts[1].fY = (pts[0].fY * 2 + pts[3].fY) / 3;
                            pts[2].fX = (pts[0].fX + pts[3].fX * 2) / 3;
                            pts[2].fY = (pts[0].fY + pts[3].fY * 2) / 3;
                            break;
                         default:
                            SkASSERT(0);
                            break;
                    }
                    break;
                case SkPath::kQuad_Verb:
                case SkPath::kConic_Verb:
                    switch (v) {
                        case SkPath::kLine_Verb:
                            pts[1] = pts[2];
                            break;
                        case SkPath::kConic_Verb:
                            weight = w;
                        case SkPath::kQuad_Verb:
                            break;
                        case SkPath::kCubic_Verb: {
                            SkDQuad dQuad;
                            dQuad.set(pts);
                            SkDCubic dCubic = dQuad.debugToCubic();
                            pts[3] = pts[2];
                            pts[1] = dCubic[1].asSkPoint();
                            pts[2] = dCubic[2].asSkPoint();
                            } break;
                         default:
                            SkASSERT(0);
                            break;
                    }
                    break;
                case SkPath::kCubic_Verb:
                    switch (v) {
                        case SkPath::kLine_Verb:
                            pts[1] = pts[3];
                            break;
                        case SkPath::kConic_Verb:
                            weight = w;
                        case SkPath::kQuad_Verb: {
                            SkDCubic dCubic;
                            dCubic.set(pts);
                            SkDQuad dQuad = dCubic.toQuad();
                            pts[1] = dQuad[1].asSkPoint();
                            pts[2] = pts[3];
                            } break;
                        default:
                            SkASSERT(0);
                            break;
                    }
                    break;
                default:
                    SkASSERT(0);
                    break;
            }
            verb = v;
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], weight);
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPath::kClose_Verb:
                result.close();
                break;
            default:
                SkASSERT(0);
                break;
        }
    }
    *path = result;
}

static void add_to_map(SkScalar coverage, int x, int y, uint8_t* distanceMap, int w, int h) {
    int byteCoverage = (int) (coverage * 256);
    if (byteCoverage < 0) {
        byteCoverage = 0;
    } else if (byteCoverage > 255) {
        byteCoverage = 255;
    }
    SkASSERT(x < w);
    SkASSERT(y < h);
    distanceMap[y * w + x] = SkTMax(distanceMap[y * w + x], (uint8_t) byteCoverage);
}

static void filter_coverage(const uint8_t* map, int len, uint8_t min, uint8_t max,
        uint8_t* filter) {
    for (int index = 0; index < len; ++index) {
        uint8_t in = map[index];
        filter[index] = in < min ? 0 : max < in ? 0 : in;
    }
}

static void construct_path(SkPath& path) {
    path.reset();
    path.moveTo(442, 101.5f);
    path.quadTo(413.5f, 691, 772, 514);
    path.lineTo(346, 721.5f);
    path.lineTo(154, 209);
    path.lineTo(442, 101.5f);
    path.close();
}

struct ButtonPaints {
    static const int kMaxStateCount = 4;
    SkPaint fDisabled;
    SkPaint fStates[kMaxStateCount];
    SkPaint fLabel;

    ButtonPaints() {
        fStates[0].setAntiAlias(true);
        fStates[0].setStyle(SkPaint::kStroke_Style);
        fStates[0].setColor(0xFF3F0000);
        fStates[1] = fStates[0];
        fStates[1].setStrokeWidth(3);
        fStates[2] = fStates[1];
        fStates[2].setColor(0xFFcf0000);
        fLabel.setAntiAlias(true);
        fLabel.setTextSize(25.0f);
        fLabel.setTextAlign(SkPaint::kCenter_Align);
        fLabel.setStyle(SkPaint::kFill_Style);
    }
};

struct Button {
    SkRect fBounds;
    int fStateCount;
    int fState;
    char fLabel;
    bool fVisible;

    Button(char label) {
        fStateCount = 2;
        fState = 0;
        fLabel = label;
        fVisible = false;
    }

    Button(char label, int stateCount) {
        SkASSERT(stateCount <= ButtonPaints::kMaxStateCount);
        fStateCount = stateCount;
        fState = 0;
        fLabel = label;
        fVisible = false;
    }

    bool contains(const SkRect& rect) {
        return fVisible && fBounds.contains(rect);
    }

    bool enabled() {
        return SkToBool(fState);
    }

    void draw(SkCanvas* canvas, const ButtonPaints& paints) {
        if (!fVisible) {
            return;
        }
        canvas->drawRect(fBounds, paints.fStates[fState]);
        canvas->drawText(&fLabel, 1, fBounds.centerX(), fBounds.fBottom - 5, paints.fLabel);
    }

    void toggle() {
        if (++fState == fStateCount) {
            fState = 0;
        }
    }

    void setEnabled(bool enabled) {
        fState = (int) enabled;
    }
};

struct ControlPaints {
    SkPaint fOutline;
    SkPaint fIndicator;
    SkPaint fFill;
    SkPaint fLabel;
    SkPaint fValue;

    ControlPaints() {
        fOutline.setAntiAlias(true);
        fOutline.setStyle(SkPaint::kStroke_Style);
        fIndicator = fOutline;
        fIndicator.setColor(SK_ColorRED);
        fFill.setAntiAlias(true);
        fFill.setColor(0x7fff0000);
        fLabel.setAntiAlias(true);
        fLabel.setTextSize(13.0f);
        fValue.setAntiAlias(true);
        fValue.setTextSize(11.0f);
    }
};

struct UniControl {
    SkString fName;
    SkRect fBounds;
    SkScalar fMin;
    SkScalar fMax;
    SkScalar fValLo;
    SkScalar fYLo;
    bool fVisible;

    UniControl(const char* name, SkScalar min, SkScalar max) {
        fName = name;
        fValLo =  fMin = min;
        fMax = max;
        fVisible = false;

    }

    virtual ~UniControl() {}

    bool contains(const SkRect& rect) {
        return fVisible && fBounds.contains(rect);
    }

    virtual void draw(SkCanvas* canvas, const ControlPaints& paints) {
        if (!fVisible) {
            return;
        }
        canvas->drawRect(fBounds, paints.fOutline);
        fYLo = fBounds.fTop + (fValLo - fMin) * fBounds.height() / (fMax - fMin);
        canvas->drawLine(fBounds.fLeft - 5, fYLo, fBounds.fRight + 5, fYLo, paints.fIndicator);
        SkString label;
        label.printf("%0.3g", fValLo);
        canvas->drawString(label, fBounds.fLeft + 5, fYLo - 5, paints.fValue);
        canvas->drawString(fName, fBounds.fLeft, fBounds.bottom() + 11,
                paints.fLabel);
    }
};

struct BiControl : public UniControl {
    SkScalar fValHi;

    BiControl(const char* name, SkScalar min, SkScalar max)
        : UniControl(name, min, max)
        ,  fValHi(fMax) {
    }

    virtual ~BiControl() {}

    virtual void draw(SkCanvas* canvas, const ControlPaints& paints) {
        UniControl::draw(canvas, paints);
        if (!fVisible || fValHi == fValLo) {
            return;
        }
        SkScalar yPos = fBounds.fTop + (fValHi - fMin) * fBounds.height() / (fMax - fMin);
        canvas->drawLine(fBounds.fLeft - 5, yPos, fBounds.fRight + 5, yPos, paints.fIndicator);
        SkString label;
        label.printf("%0.3g", fValHi);
        if (yPos < fYLo + 10) {
            yPos = fYLo + 10;
        }
        canvas->drawString(label, fBounds.fLeft + 5, yPos - 5, paints.fValue);
        SkRect fill = { fBounds.fLeft, fYLo, fBounds.fRight, yPos };
        canvas->drawRect(fill, paints.fFill);
    }
};


class MyClick : public SampleView::Click {
public:
    enum ClickType {
        kInvalidType = -1,
        kPtType,
        kVerbType,
        kControlType,
        kPathType,
    } fType;

    enum ControlType {
        kInvalidControl = -1,
        kFirstControl,
        kFilterControl = kFirstControl,
        kResControl,
        kWeightControl,
        kWidthControl,
        kLastControl = kWidthControl,
        kFirstButton,
        kCubicButton = kFirstButton,
        kConicButton,
        kQuadButton,
        kLineButton,
        kLastVerbButton = kLineButton,
        kAddButton,
        kDeleteButton,
        kInOutButton,
        kFillButton,
        kSkeletonButton,
        kFilterButton,
        kBisectButton,
        kJoinButton,
        kLastButton = kJoinButton,
        kPathMove,
    } fControl;

    SkPath::Verb fVerb;
    SkScalar fWeight;

    MyClick(SkView* target, ClickType type, ControlType control)
        : Click(target)
        , fType(type)
        , fControl(control)
        , fVerb((SkPath::Verb) -1)
        , fWeight(1) {
    }

    MyClick(SkView* target, ClickType type, int index)
        : Click(target)
        , fType(type)
        , fControl((ControlType) index)
        , fVerb((SkPath::Verb) -1)
        , fWeight(1) {
    }

    MyClick(SkView* target, ClickType type, int index, SkPath::Verb verb, SkScalar weight)
        : Click(target)
        , fType(type)
        , fControl((ControlType) index)
        , fVerb(verb)
        , fWeight(weight) {
    }

    bool isButton() {
        return kFirstButton <= fControl && fControl <= kLastButton;
    }

    int ptHit() const {
        SkASSERT(fType == kPtType);
        return (int) fControl;
    }

    int verbHit() const {
        SkASSERT(fType == kVerbType);
        return (int) fControl;
    }
};

enum {
    kControlCount = MyClick::kLastControl - MyClick::kFirstControl + 1,
};

static struct ControlPair {
    UniControl* fControl;
    MyClick::ControlType fControlType;
} kControlList[kControlCount];

enum {
    kButtonCount = MyClick::kLastButton - MyClick::kFirstButton + 1,
    kVerbCount = MyClick::kLastVerbButton - MyClick::kFirstButton + 1,
};

static struct ButtonPair {
    Button* fButton;
    MyClick::ControlType fButtonType;
} kButtonList[kButtonCount];

static void enable_verb_button(MyClick::ControlType type) {
    for (int index = 0; index < kButtonCount; ++index) {
        MyClick::ControlType testType = kButtonList[index].fButtonType;
        if (MyClick::kFirstButton <= testType && testType <= MyClick::kLastVerbButton) {
            Button* button = kButtonList[index].fButton;
            button->setEnabled(testType == type);
        }
    }
}

struct Stroke;

struct Active {
    Active* fNext;
    Stroke* fParent;
    SkScalar fStart;
    SkScalar fEnd;

    void reset() {
        fNext = nullptr;
        fStart = 0;
        fEnd = 1;
    }
};

struct Stroke {
    SkPath fPath;
    Active fActive;
    bool fInner;

    void reset() {
        fPath.reset();
        fActive.reset();
    }
};

struct PathUndo {
    SkPath fPath;
    PathUndo* fNext;
};

class AAGeometryView : public SampleView {
    SkPaint fActivePaint;
    SkPaint fComplexPaint;
    SkPaint fCoveragePaint;
    SkPaint fLegendLeftPaint;
    SkPaint fLegendRightPaint;
    SkPaint fPointPaint;
    SkPaint fSkeletonPaint;
    SkPaint fDebugSkeletonPaint;
    SkPaint fLightSkeletonPaint;
    SkPath fPath;
    ControlPaints fControlPaints;
    UniControl fResControl;
    UniControl fWeightControl;
    UniControl fWidthControl;
    BiControl fFilterControl;
    ButtonPaints fButtonPaints;
    Button fCubicButton;
    Button fConicButton;
    Button fQuadButton;
    Button fLineButton;
    Button fAddButton;
    Button fDeleteButton;
    Button fFillButton;
    Button fSkeletonButton;
    Button fFilterButton;
    Button fBisectButton;
    Button fJoinButton;
    Button fInOutButton;
    SkTArray<Stroke> fStrokes;
    PathUndo* fUndo;
    SkPoint fFirstBisect[2];
    SkPoint fLastBisect[2];
    SkPoint fLastPt;
    SkScalar fBallInterval;
    int fActivePt;
    int fActiveVerb;
    SkPath::Verb fFirstVerb;
    SkPath::Verb fLastVerb;
    bool fHandlePathMove;
    bool fShowLegend;
    bool fHideAll;
    const int kHitToleranace = 25;
    bool fLastBisectSet;

public:

    AAGeometryView()
        : fResControl("error", 0, 10)
        , fWeightControl("weight", 0, 5)
        , fWidthControl("width", FLT_EPSILON, 100)
        , fFilterControl("filter", 0, 255)
        , fCubicButton('C')
        , fConicButton('K')
        , fQuadButton('Q')
        , fLineButton('L')
        , fAddButton('+')
        , fDeleteButton('x')
        , fFillButton('p')
        , fSkeletonButton('s')
        , fFilterButton('f', 4)
        , fBisectButton('b')
        , fJoinButton('j')
        , fInOutButton('|')
        , fUndo(nullptr)
        , fBallInterval(5)
        , fActivePt(-1)
        , fActiveVerb(-1)
        , fHandlePathMove(true)
        , fShowLegend(false)
        , fHideAll(false)
    {
        fCoveragePaint.setAntiAlias(true);
        fCoveragePaint.setColor(SK_ColorBLUE);
        SkPaint strokePaint;
        strokePaint.setAntiAlias(true);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        fPointPaint = strokePaint;
        fPointPaint.setColor(0x99ee3300);
        fSkeletonPaint = strokePaint;
        fSkeletonPaint.setColor(SK_ColorRED);
        fDebugSkeletonPaint = fSkeletonPaint;
        fDebugSkeletonPaint.setColor(SK_ColorBLUE);
        fLightSkeletonPaint = fSkeletonPaint;
        fLightSkeletonPaint.setColor(0xFFFF7f7f);
        fActivePaint = strokePaint;
        fActivePaint.setColor(0x99ee3300);
        fActivePaint.setStrokeWidth(5);
        fComplexPaint = fActivePaint;
        fComplexPaint.setColor(SK_ColorBLUE);
        fLegendLeftPaint.setAntiAlias(true);
        fLegendLeftPaint.setTextSize(13);
        fLegendRightPaint = fLegendLeftPaint;
        fLegendRightPaint.setTextAlign(SkPaint::kRight_Align);
        construct_path(fPath);
        fFillButton.fVisible = fSkeletonButton.fVisible = fFilterButton.fVisible
                = fBisectButton.fVisible = fJoinButton.fVisible = fInOutButton.fVisible = true;
        fSkeletonButton.setEnabled(true);
        fInOutButton.setEnabled(true);
        fJoinButton.setEnabled(true);
        fFilterControl.fValLo = 120;
        fFilterControl.fValHi = 141;
        fFilterControl.fVisible = fFilterButton.fState == 2;
        fResControl.fValLo = 5;
        fResControl.fVisible = true;
        fWidthControl.fValLo = 50;
        fWidthControl.fVisible = true;
        init_controlList();
        init_buttonList();
    }

    bool constructPath() {
        construct_path(fPath);
        this->inval(nullptr);
        return true;
    }

    void savePath(Click::State state) {
        if (state != Click::kDown_State) {
            return;
        }
        if (fUndo && fUndo->fPath == fPath) {
            return;
        }
        PathUndo* undo = new PathUndo;
        undo->fPath = fPath;
        undo->fNext = fUndo;
        fUndo = undo;
    }

    bool undo() {
        if (!fUndo) {
            return false;
        }
        fPath = fUndo->fPath;
        validatePath();
        PathUndo* next = fUndo->fNext;
        delete fUndo;
        fUndo = next;
        this->inval(nullptr);
        return true;
    }

    void validatePath() {
        PathUndo* undo = fUndo;
        int match = 0;
        while (undo) {
            match += fPath == undo->fPath;
            undo = undo->fNext;
        }
    }

    void set_controlList(int index, UniControl* control, MyClick::ControlType type) {
        kControlList[index].fControl = control;
        kControlList[index].fControlType = type;
    }

    #define SET_CONTROL(Name) set_controlList(index++, &f##Name##Control, \
        MyClick::k##Name##Control)

    bool hideAll() {
        fHideAll ^= true;
        this->inval(nullptr);
        return true;
    }

    void init_controlList() {
        int index = 0;
        SET_CONTROL(Width);
        SET_CONTROL(Res);
        SET_CONTROL(Filter);
        SET_CONTROL(Weight);
    }

    #undef SET_CONTROL

    void set_buttonList(int index, Button* button, MyClick::ControlType type) {
        kButtonList[index].fButton = button;
        kButtonList[index].fButtonType = type;
    }

    #define SET_BUTTON(Name) set_buttonList(index++, &f##Name##Button, \
            MyClick::k##Name##Button)

    void init_buttonList() {
        int index = 0;
        SET_BUTTON(Fill);
        SET_BUTTON(Skeleton);
        SET_BUTTON(Filter);
        SET_BUTTON(Bisect);
        SET_BUTTON(Join);
        SET_BUTTON(InOut);
        SET_BUTTON(Cubic);
        SET_BUTTON(Conic);
        SET_BUTTON(Quad);
        SET_BUTTON(Line);
        SET_BUTTON(Add);
        SET_BUTTON(Delete);
    }

    #undef SET_BUTTON

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override;

    void onSizeChange() override {
        setControlButtonsPos();
        this->INHERITED::onSizeChange();
    }

    bool pathDump() {
        fPath.dump();
        return true;
    }

    bool scaleDown() {
        SkMatrix matrix;
        SkRect bounds = fPath.getBounds();
        matrix.setScale(1.f / 1.5f, 1.f / 1.5f, bounds.centerX(), bounds.centerY());
        fPath.transform(matrix);
        validatePath();
        this->inval(nullptr);
        return true;
    }

    bool scaleToFit() {
        SkMatrix matrix;
        SkRect bounds = fPath.getBounds();
        SkScalar scale = SkTMin(this->width() / bounds.width(), this->height() / bounds.height())
                * 0.8f;
        matrix.setScale(scale, scale, bounds.centerX(), bounds.centerY());
        fPath.transform(matrix);
        bounds = fPath.getBounds();
        SkScalar offsetX = (this->width() - bounds.width()) / 2 - bounds.fLeft;
        SkScalar offsetY = (this->height() - bounds.height()) / 2 - bounds.fTop;
        fPath.offset(offsetX, offsetY);
        validatePath();
        this->inval(nullptr);
        return true;
    }

    bool scaleUp() {
        SkMatrix matrix;
        SkRect bounds = fPath.getBounds();
        matrix.setScale(1.5f, 1.5f, bounds.centerX(), bounds.centerY());
        fPath.transform(matrix);
        validatePath();
        this->inval(nullptr);
        return true;
    }

    void setControlButtonsPos() {
        SkScalar widthOffset = this->width() - 100;
        for (int index = 0; index < kControlCount; ++index) {
            if (kControlList[index].fControl->fVisible) {
                kControlList[index].fControl->fBounds.setXYWH(widthOffset, 30, 30, 400);
                widthOffset -= 50;
            }
        }
        SkScalar buttonOffset = 0;
        for (int index = 0; index < kButtonCount; ++index) {
            kButtonList[index].fButton->fBounds.setXYWH(this->width() - 50,
                    buttonOffset += 50, 30, 30);
        }
    }

    bool showLegend() {
        fShowLegend ^= true;
        this->inval(nullptr);
        return true;
    }

    void draw_bisect(SkCanvas* canvas, const SectData* sectData, const SectData* nextData,
            int counter) {
        const SkPoint& pt = sectData->fCurve[0]->fPts[1];
        if (fBisectButton.enabled()) {
            canvas->drawLine(pt, sectData->fBisects[1], fSkeletonPaint);
            if (nextData) {
                canvas->drawLine(nextData->fBisects[1], sectData->fBisects[1], fDebugSkeletonPaint);
            }
            canvas->drawLine(pt, sectData->fBisects[0], fSkeletonPaint);
            if (nextData) {
                canvas->drawLine(sectData->fBisects[0], nextData->fBisects[0], fSkeletonPaint);
            }
        }
        const SkVector& lastV = sectData->fCurve[0]->fTangents[1];
        const SkVector& nextV = sectData->fCurve[1]->fTangents[0];
        if (fBisectButton.enabled()) {
            canvas->drawLine(pt, {pt.fX - lastV.fY, pt.fY + lastV.fX}, fSkeletonPaint);
        }
        if (fBisectButton.enabled()) {
            canvas->drawLine(pt, {pt.fX + nextV.fY, pt.fY - nextV.fX}, fSkeletonPaint);
        }
        if (fJoinButton.enabled()) {
            SkScalar r = fWidthControl.fValLo;
            SkRect oval = { pt.fX - r, pt.fY - r, pt.fX + r, pt.fY + r};
            SkScalar startAngle = SkScalarATan2(lastV.fX, -lastV.fY) * 180.f / SK_ScalarPI;
            SkScalar endAngle = SkScalarATan2(-nextV.fX, nextV.fY) * 180.f / SK_ScalarPI;
            if (endAngle > startAngle) {
                canvas->drawArc(oval, startAngle, endAngle - startAngle, false, fSkeletonPaint);
            } else {
                canvas->drawArc(oval, startAngle, 360 - (startAngle - endAngle), false,
                        fSkeletonPaint);
            }
            canvas->drawLine(sectData->fBisects[1].fX, sectData->fBisects[1].fY,
                pt.fX - lastV.fY, pt.fY + lastV.fX, fSkeletonPaint);
            canvas->drawLine(sectData->fBisects[1].fX, sectData->fBisects[1].fY,
                pt.fX + nextV.fY, pt.fY - nextV.fX, fSkeletonPaint);
        }
    }

    SkScalar find_zero_crossing(SkPoint quad[3], SkScalar width, SkScalar min, SkScalar max) {
        SkScalar mid = (min + max) / 2;
        if (mid <= min) {
            return min;
        }
        if (mid >= max) {
            return max;
        }
        SkPoint pt;
        SkVector tangent;
        SkEvalQuadAt(quad, mid, &pt, &tangent);
        tangent.setLength(width);
        pt.fX += tangent.fY;
        if (pt.fX < -.5f) {
            return find_zero_crossing(quad, width, mid, max);
        } else if (pt.fX > .5f) {
            return find_zero_crossing(quad, width, min, mid);
        } else {
            return mid;
        }
    }

    // returns t value where quad[adjIndex] is fWidthControl.fValLo from nextEnd
    SkScalar adjust_quad(const SkPoint quad[3], int adjIndex, const SkPoint& nextEnd) {
        SkVector priorTangent = quad[adjIndex] - nextEnd;
        SkVector offset = { -priorTangent.fY, priorTangent.fX };
        if (adjIndex) {
            offset.negate();
        }
        offset.setLength(fWidthControl.fValLo);
        SkPoint parallelRay[] = { nextEnd + offset, quad[adjIndex] + offset };
        // rotate quad so ray is upright; translate quad so ray is on y-axis
        SkScalar rayLength = priorTangent.length();
        SkPoint uprightRay[] = { { 0, 0 }, { 0, rayLength } };
        SkMatrix uprightMatrix;
        uprightMatrix.setPolyToPoly(parallelRay, uprightRay, 2);
        SkPoint uprightQuad[3];
        uprightMatrix.mapPoints(uprightQuad, quad, 3);
        SkScalar start = 0;
        SkScalar end = 1;
        int debugIterations = 4;
        while ((uprightQuad[0].fX < 0) == (uprightQuad[2].fX < 0)) {
            if (--debugIterations <= 0) {
                SkASSERT(0);
            }
            SkPoint pieces[5];
            SkChopQuadAtHalf(uprightQuad, pieces);
            if (0 == adjIndex) {
                memcpy(uprightQuad, &pieces[0], sizeof(uprightQuad));
                end = (start + end) / 2;
            } else {
                SkASSERT(2 == adjIndex);
                memcpy(uprightQuad, &pieces[2], sizeof(uprightQuad));
                start = (start + end) / 2;
            }
        }
        // search for t where perpendicular of length width ends on y-axis
        bool swapped = false;
        if (uprightQuad[0].fX > 0) {
            SkTSwap(uprightQuad[0], uprightQuad[2]);
            swapped = true;
        }
        SkScalar result = find_zero_crossing(uprightQuad, fWidthControl.fValLo, 0, 1);
        if (swapped) {
            result = 1 - result;
        }
        if (end < 1) {
            result = start + (end - start) * result;
        }
        return result;
    }

    void set_point_array() {
        int verbCount = fPath.countVerbs();
        fCurveData.setCount(verbCount);
        fSectData.setCount(verbCount);
        SkPath::Iter iter(fPath, false);
        int ptIndex = 0;
        fContourData.reset();
        ContourData* curContour = nullptr;
        CurveData* curCurve = fCurveData.begin();
        CurvePos curPos = CurvePos::kUnknown;
        while ((curCurve->fVerb = iter.next(curCurve->fPts)) != SkPath::kDone_Verb) {
            int ptCount = 3;
            switch (curCurve->fVerb) {
            case SkPath::kMove_Verb:
                curPos = CurvePos::kFirst;
                curContour = fContourData.append();
                curContour->fFirst = curCurve;
                break;
            case SkPath::kClose_Verb:
                if (curCurve == curContour->fFirst) {
                    fContourData.pop();
                    break;
                }
                curCurve[-1].fPos = CurvePos::kLast;
                curContour->fLast = &curCurve[-1];
                break;
            case SkPath::kLine_Verb:
                ptCount -= 1;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                ptCount -= 1;
                curCurve->fWeight = iter.conicWeight();
            case SkPath::kCubic_Verb: {
                curCurve->fPos = curPos;
                curCurve->fStartT = 0;
                curCurve->fEndT = 1;
                curPos = CurvePos::kMid;
                curCurve->fPathIndex = ptIndex;
                ptIndex += ptCount;
                for (int index = 1; index <= ptCount; ++index) {
                    curCurve->fTangents[0] = curCurve->fPts[index] - curCurve->fPts[0];
                    if (!degenerate_vector(curCurve->fTangents[0])) {
                        break;
                    }
                }
                bool degenerate = !curCurve->fTangents[0].setLength(fWidthControl.fValLo);
                if (SkPath::kLine_Verb == curCurve->fVerb) {
                    curCurve->fTangents[1] = -curCurve->fTangents[0];
                } else {
                    for (int index = ptCount - 1; index >= 0; --index) {
                        curCurve->fTangents[1] = curCurve->fPts[ptCount] - curCurve->fPts[index];
                        if (!degenerate_vector(curCurve->fTangents[1])) {
                            break;
                        }
                    }
                    curCurve->fTangents[1].setLength(fWidthControl.fValLo);
                }
                if (!degenerate) {
                    ++curCurve;
                }
                } break;
            }
        }
        if (!fContourData.count()) {
            return;
        }
        SectData* curSect = &fSectData[0];
        for (curContour = fContourData.begin(); curContour != fContourData.end(); ++curContour) {
            curCurve = curContour->fFirst;
            SectData* firstSect = curSect;
            do {
                curSect->fCurve[0] = curCurve;
                curSect->fL.set(SK_ScalarNaN, SK_ScalarNaN);  // don't know yet
                curSect->fR.set(SK_ScalarNaN, SK_ScalarNaN);  // don't know yet
                if (firstSect != curSect) {
                    curSect[-1].fCurve[1] = curCurve;
                }
                if (curContour->fLast == curCurve) {
                    break;
                }
                ++curSect;
                ++curCurve;
            } while (true);
            curSect->fCurve[1] = curContour->fFirst;
        }
    }

    void calc_bisects(SkCanvas* canvas, bool activeOnly) {
        for (int ptIndex = 0; ptIndex < fSectData.count(); ++ptIndex) {
            SectData* curPtData = &fSectData[ptIndex];
            SkVector bisect;
            const SkVector& lastV = curPtData->fCurve[0]->fTangents[1];
            const SkVector& nextV = curPtData->fCurve[1]->fTangents[0];
            bisect.set(lastV.fX + nextV.fX, lastV.fY + nextV.fY);
            SkVector angleOpp = lastV - nextV;
            SkScalar oppLen = angleOpp.length() / 2;
            bisect.setLength(fWidthControl.fValLo * fWidthControl.fValLo / oppLen);
            curPtData->fL = curPtData->fCurve[1]->fPts[0] + bisect;
            curPtData->fR = curPtData->fCurve[1]->fPts[0] - bisect;
            if (SkPath::kLine_Verb != curPtData->fCurve[0]->fVerb &&
                    SkPath::kLine_Verb != curPtData->fCurve[1]->fVerb) {
                // need to compute intersection of inner pair
                SkASSERT(0);  // incomplete
                continue;
            }
            if (SkPath::kQuad_Verb == curPtData->fCurve[0]->fVerb &&
                    SkPath::kLine_Verb == curPtData->fCurve[1]->fVerb) {
                curPtData->fCurve[0]->fEndT = adjust_quad(curPtData->fCurve[0]->fPts, 2,
                        curPtData->fCurve[1]->fPts[1]);
            } else if (SkPath::kLine_Verb == curPtData->fCurve[0]->fVerb &&
                    SkPath::kQuad_Verb == curPtData->fCurve[1]->fVerb) {
                curPtData->fCurve[1]->fStartT = adjust_quad(curPtData->fCurve[1]->fPts, 0,
                        curPtData->fCurve[0]->fPts[0]);
            } else if (SkPath::kLine_Verb == curPtData->fCurve[0]->fVerb &&
                    SkPath::kLine_Verb == curPtData->fCurve[1]->fVerb) {
                SkVector lastV = curPtData->fCurve[0]->fTangents[1];
                SkScalar lastLen = lastV.length();
                SkVector nextV = curPtData->fCurve[1]->fTangents[0];
                SkScalar nextLen = nextV.length();
                SkScalar hypLen;
                if (lastLen < nextLen) {
                    lastV.setLength(nextLen);
                    hypLen = nextLen;
                } else {
                    nextV.setLength(lastLen);
                    hypLen = lastLen;
                }
                SkVector bisect = { lastV.fX + nextV.fX, lastV.fY + nextV.fY };
                SkVector angleOpp = lastV - nextV;
                angleOpp.scale(0.5);
                SkScalar oppLen = angleOpp.length();
                SkScalar oppSin = oppLen / hypLen;
                bisect.setLength(fWidthControl.fValLo * hypLen / oppLen);
                lastV.setLength(fWidthControl.fValLo);
                nextV.setLength(fWidthControl.fValLo);
                const SkPoint& pt = curPtData->fCurve[0]->fPts[1];
                curPtData->fBisects[0] = pt + bisect;
                curPtData->fBisects[1] = pt - bisect;
}
        }
    }

    void draw_bisects(SkCanvas* canvas, bool activeOnly) {
        int counter = -1;
        for (int ptIndex = 0; ptIndex < fSectData.count(); ++ptIndex) {
            const SectData* curPtData = &fSectData[ptIndex];
            ++counter;
            if (activeOnly && counter != fActiveVerb && counter - 1 != fActiveVerb
                    && counter + 1 != fActiveVerb
                    && (fActiveVerb != 1 || counter != fPath.countVerbs())) {
                continue;
            }
            if (!fBisectButton.enabled()) {
                continue;
            }
            if (SkPath::kQuad_Verb == curPtData->fCurve[0]->fVerb) {
                const SkPoint* pts = curPtData->fCurve[0]->fPts;
                SkScalar t = SkFindQuadMaxCurvature(pts);
                if (0 < t && t < 1) {
                    SkPoint maxPt = SkEvalQuadAt(pts, t);
                    SkVector tangent = SkEvalQuadTangentAt(pts, t);
                    tangent.setLength(fWidthControl.fValLo);
                    canvas->drawLine(maxPt, {maxPt.fX + tangent.fY, maxPt.fY - tangent.fX},
                                        fSkeletonPaint);
                    canvas->drawLine(maxPt, {maxPt.fX - tangent.fY, maxPt.fY + tangent.fX},
                                        fSkeletonPaint);
                }
                for (PathStroker::StrokeType strokeType : { PathStroker::kInner_StrokeType,
                        PathStroker::kOuter_StrokeType } ) {
                    PathStroker pathStroker(fWidthControl.fValLo, strokeType);
                    QuadConstruct quadConstruct;
                    // get next tangent
                    quadConstruct.init(curPtData->fCurve[0]->fStartT, curPtData->fCurve[0]->fEndT);
                    pathStroker.quadStroke(pts, &quadConstruct);
                    const SkPath& active = pathStroker.inner().isEmpty() ? pathStroker.outer() :
                            pathStroker.inner();
                    canvas->drawPath(active, fSkeletonPaint);
                }
            } else if (SkPath::kConic_Verb == curPtData->fCurve[0]->fVerb) {
                ;
                // FIXME : need max curvature or equivalent here ?
            } else if (SkPath::kCubic_Verb == curPtData->fCurve[0]->fVerb) {
                SkScalar tMax[2];
                int tMaxCount = SkFindCubicMaxCurvature(curPtData->fCurve[0]->fPts, tMax);
                for (int tIndex = 0; tIndex < tMaxCount; ++tIndex) {
                    if (0 >= tMax[tIndex] || tMax[tIndex] >= 1) {
                        continue;
                    }
                    SkPoint maxPt;
                    SkVector tangent;
                    SkEvalCubicAt(curPtData->fCurve[0]->fPts, tMax[tIndex], &maxPt, &tangent,
                            nullptr);
                    tangent.setLength(fWidthControl.fValLo * 2);
                    canvas->drawLine(maxPt, {maxPt.fX + tangent.fY, maxPt.fY - tangent.fX},
                            fSkeletonPaint);
                }
            }
            if (SkPath::kLine_Verb == curPtData->fCurve[0]->fVerb &&
                    SkPath::kLine_Verb == curPtData->fCurve[1]->fVerb) {
                draw_bisect(canvas, curPtData, &fSectData[(ptIndex + 1) % fSectData.count()],
                        counter);
            }
        }
    }

    void draw_legend(SkCanvas* canvas);

    void draw_segment(SkCanvas* canvas) {
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(fPath, true);
        int counter = -1;
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            if (++counter < fActiveVerb) {
                continue;
            }
            switch (verb) {
                case SkPath::kLine_Verb:
                    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, fActivePaint);
                    draw_points(canvas, pts, 2);
                    break;
                case SkPath::kQuad_Verb: {
                    SkPath qPath;
                    qPath.moveTo(pts[0]);
                    qPath.quadTo(pts[1], pts[2]);
                    canvas->drawPath(qPath, fActivePaint);
                    draw_points(canvas, pts, 3);
                    } break;
                case SkPath::kConic_Verb: {
                    SkPath conicPath;
                    conicPath.moveTo(pts[0]);
                    conicPath.conicTo(pts[1], pts[2], iter.conicWeight());
                    canvas->drawPath(conicPath, fActivePaint);
                    draw_points(canvas, pts, 3);
                    } break;
                case SkPath::kCubic_Verb: {
                    SkScalar loopT[3];
                    int complex = SkDCubic::ComplexBreak(pts, loopT);
                    SkPath cPath;
                    cPath.moveTo(pts[0]);
                    cPath.cubicTo(pts[1], pts[2], pts[3]);
                    canvas->drawPath(cPath, complex ? fComplexPaint : fActivePaint);
                    draw_points(canvas, pts, 4);
                    } break;
                default:
                    break;
            }
            return;
        }
    }

    void draw_points(SkCanvas* canvas, SkPoint* points, int count) {
        for (int index = 0; index < count; ++index) {
            canvas->drawCircle(points[index].fX, points[index].fY, 10, fPointPaint);
        }
    }

    int hittest_verb(SkPoint pt, SkPath::Verb* verbPtr, SkScalar* weight) {
        SkIntersections i;
        SkDLine hHit = {{{pt.fX - kHitToleranace, pt.fY }, {pt.fX + kHitToleranace, pt.fY}}};
        SkDLine vHit = {{{pt.fX, pt.fY - kHitToleranace }, {pt.fX, pt.fY + kHitToleranace}}};
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(fPath, true);
        int counter = -1;
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            ++counter;
            switch (verb) {
                case SkPath::kLine_Verb: {
                    SkDLine line;
                    line.set(pts);
                    if (i.intersect(line, hHit) || i.intersect(line, vHit)) {
                        *verbPtr = verb;
                        *weight = 1;
                        return counter;
                    }
                    } break;
                case SkPath::kQuad_Verb: {
                    SkDQuad quad;
                    quad.set(pts);
                    if (i.intersect(quad, hHit) || i.intersect(quad, vHit)) {
                        *verbPtr = verb;
                        *weight = 1;
                        return counter;
                    }
                    } break;
                case SkPath::kConic_Verb: {
                    SkDConic conic;
                    SkScalar w = iter.conicWeight();
                    conic.set(pts, w);
                    if (i.intersect(conic, hHit) || i.intersect(conic, vHit)) {
                        *verbPtr = verb;
                        *weight = w;
                        return counter;
                    }
                    } break;
                case SkPath::kCubic_Verb: {
                    SkDCubic cubic;
                    cubic.set(pts);
                    if (i.intersect(cubic, hHit) || i.intersect(cubic, vHit)) {
                        *verbPtr = verb;
                        *weight = 1;
                        return counter;
                    }
                    } break;
                default:
                    break;
            }
        }
        return -1;
    }

    SkScalar pt_to_line(SkPoint s, SkPoint e, int x, int y) {
        SkScalar radius = fWidthControl.fValLo;
        SkVector adjOpp = e - s;
        SkScalar lenSq = SkPointPriv::LengthSqd(adjOpp);
        SkPoint rotated = {
                (y - s.fY) * adjOpp.fY + (x - s.fX) * adjOpp.fX,
                (y - s.fY) * adjOpp.fX - (x - s.fX) * adjOpp.fY,
        };
        if (rotated.fX < 0 || rotated.fX > lenSq) {
                return -radius;
        }
        rotated.fY /= SkScalarSqrt(lenSq);
        return SkTMax(-radius, SkTMin(radius, rotated.fY));
    }

    // given a line, compute the interior and exterior gradient coverage
    bool coverage(SkPoint s, SkPoint e, uint8_t* distanceMap, int w, int h) {
        SkScalar radius = fWidthControl.fValLo;
        int minX = SkTMax(0, (int) (SkTMin(s.fX, e.fX) - radius));
        int minY = SkTMax(0, (int) (SkTMin(s.fY, e.fY) - radius));
        int maxX = SkTMin(w, (int) (SkTMax(s.fX, e.fX) + radius) + 1);
        int maxY = SkTMin(h, (int) (SkTMax(s.fY, e.fY) + radius) + 1);
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                SkScalar ptToLineDist = pt_to_line(s, e, x, y);
                if (ptToLineDist > -radius && ptToLineDist < radius) {
                    SkScalar coverage = ptToLineDist / radius;
                    add_to_map(1 - SkScalarAbs(coverage), x, y, distanceMap, w, h);
                }
                SkVector ptToS = { x - s.fX, y - s.fY };
                SkScalar dist = ptToS.length();
                if (dist < radius) {
                    SkScalar coverage = dist / radius;
                    add_to_map(1 - SkScalarAbs(coverage), x, y, distanceMap, w, h);
                }
                SkVector ptToE = { x - e.fX, y - e.fY };
                dist = ptToE.length();
                if (dist < radius) {
                    SkScalar coverage = dist / radius;
                    add_to_map(1 - SkScalarAbs(coverage), x, y, distanceMap, w, h);
                }
            }
        }
        return true;
    }

    void quad_coverage(SkPoint pts[3], uint8_t* distanceMap, int w, int h) {
        SkScalar dist = pts[0].Distance(pts[0], pts[2]);
        if (dist < gCurveDistance) {
            (void) coverage(pts[0], pts[2], distanceMap, w, h);
            return;
        }
        SkPoint split[5];
        SkChopQuadAt(pts, split, 0.5f);
        quad_coverage(&split[0], distanceMap, w, h);
        quad_coverage(&split[2], distanceMap, w, h);
    }

    void conic_coverage(SkPoint pts[3], SkScalar weight, uint8_t* distanceMap, int w, int h) {
        SkScalar dist = pts[0].Distance(pts[0], pts[2]);
        if (dist < gCurveDistance) {
            (void) coverage(pts[0], pts[2], distanceMap, w, h);
            return;
        }
        SkConic split[2];
        SkConic conic;
        conic.set(pts, weight);
        if (conic.chopAt(0.5f, split)) {
            conic_coverage(split[0].fPts, split[0].fW, distanceMap, w, h);
            conic_coverage(split[1].fPts, split[1].fW, distanceMap, w, h);
        }
    }

    void cubic_coverage(SkPoint pts[4], uint8_t* distanceMap, int w, int h) {
        SkScalar dist = pts[0].Distance(pts[0], pts[3]);
        if (dist < gCurveDistance) {
            (void) coverage(pts[0], pts[3], distanceMap, w, h);
            return;
        }
        SkPoint split[7];
        SkChopCubicAt(pts, split, 0.5f);
        cubic_coverage(&split[0], distanceMap, w, h);
        cubic_coverage(&split[3], distanceMap, w, h);
    }

    void path_coverage(const SkPath& path, uint8_t* distanceMap, int w, int h) {
        memset(distanceMap, 0, sizeof(distanceMap[0]) * w * h);
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(path, true);
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    (void) coverage(pts[0], pts[1], distanceMap, w, h);
                    break;
                case SkPath::kQuad_Verb:
                    quad_coverage(pts, distanceMap, w, h);
                    break;
                case SkPath::kConic_Verb:
                    conic_coverage(pts, iter.conicWeight(), distanceMap, w, h);
                    break;
                case SkPath::kCubic_Verb:
                    cubic_coverage(pts, distanceMap, w, h);
                    break;
                default:
                    break;
            }
        }
    }

    static uint8_t* set_up_dist_map(const SkImageInfo& imageInfo, SkBitmap* distMap) {
        distMap->setInfo(imageInfo);
        distMap->setIsVolatile(true);
        SkAssertResult(distMap->tryAllocPixels());
        SkASSERT((int) distMap->rowBytes() == imageInfo.width());
        return distMap->getAddr8(0, 0);
    }

    void path_stroke(int index, SkPath* inner, SkPath* outer) {
        #if 0
        SkPathStroker stroker(fPath, fWidthControl.fValLo, 0,
                SkPaint::kRound_Cap, SkPaint::kRound_Join, fResControl.fValLo);
        SkPoint pts[4], firstPt, lastPt;
        SkPath::Verb verb;
        SkPath::Iter iter(fPath, true);
        int counter = -1;
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            ++counter;
            switch (verb) {
                case SkPath::kMove_Verb:
                    firstPt = pts[0];
                    break;
                case SkPath::kLine_Verb:
                    if (counter == index) {
                        stroker.moveTo(pts[0]);
                        stroker.lineTo(pts[1]);
                        goto done;
                    }
                    lastPt = pts[1];
                    break;
                case SkPath::kQuad_Verb:
                    if (counter == index) {
                        stroker.moveTo(pts[0]);
                        stroker.quadTo(pts[1], pts[2]);
                        goto done;
                    }
                    lastPt = pts[2];
                    break;
                case SkPath::kConic_Verb:
                    if (counter == index) {
                        stroker.moveTo(pts[0]);
                        stroker.conicTo(pts[1], pts[2], iter.conicWeight());
                        goto done;
                    }
                    lastPt = pts[2];
                    break;
                case SkPath::kCubic_Verb:
                    if (counter == index) {
                        stroker.moveTo(pts[0]);
                        stroker.cubicTo(pts[1], pts[2], pts[3]);
                        goto done;
                    }
                    lastPt = pts[3];
                    break;
                case SkPath::kClose_Verb:
                    if (counter == index) {
                        stroker.moveTo(lastPt);
                        stroker.lineTo(firstPt);
                        goto done;
                    }
                    break;
                case SkPath::kDone_Verb:
                    break;
                default:
                    SkASSERT(0);
            }
        }
    done:
        *inner = stroker.fInner;
        *outer = stroker.fOuter;
#endif
    }

    void draw_stroke(SkCanvas* canvas, int active) {
        SkPath inner, outer;
        path_stroke(active, &inner, &outer);
        canvas->drawPath(inner, fSkeletonPaint);
        canvas->drawPath(outer, fSkeletonPaint);
    }

    void gather_strokes() {
        fStrokes.reset();
        for (int index = 0; index < fPath.countVerbs(); ++index) {
            Stroke& inner = fStrokes.push_back();
            inner.reset();
            inner.fInner = true;
            Stroke& outer = fStrokes.push_back();
            outer.reset();
            outer.fInner = false;
            path_stroke(index, &inner.fPath, &outer.fPath);
        }
    }

    void trim_strokes() {
        // eliminate self-itersecting loops
        // trim outside edges
        gather_strokes();
        for (int index = 0; index < fStrokes.count(); ++index) {
            SkPath& outPath = fStrokes[index].fPath;
            for (int inner = 0; inner < fStrokes.count(); ++inner) {
                if (index == inner) {
                    continue;
                }
                SkPath& inPath = fStrokes[inner].fPath;
                if (!outPath.getBounds().intersects(inPath.getBounds())) {
                    continue;
                }

            }
        }
    }

    void draw_coverage(SkCanvas* canvas) {
        SkRect bounds = fPath.getBounds();
        SkScalar radius = fWidthControl.fValLo;
        int w = (int) (bounds.fRight + radius + 1);
        int h = (int) (bounds.fBottom + radius + 1);
        SkImageInfo imageInfo = SkImageInfo::MakeA8(w, h);
        SkBitmap distMap;
        uint8_t* distanceMap = set_up_dist_map(imageInfo, &distMap);
        path_coverage(fPath, distanceMap, w, h);
        if (fFillButton.enabled()) {
            canvas->drawPath(fPath, fCoveragePaint);
        }
        if (fFilterButton.fState == 2
                && (0 < fFilterControl.fValLo || fFilterControl.fValHi < 255)) {
            SkBitmap filteredMap;
            uint8_t* filtered = set_up_dist_map(imageInfo, &filteredMap);
            filter_coverage(distanceMap, sizeof(uint8_t) * w * h, (uint8_t) fFilterControl.fValLo,
                    (uint8_t) fFilterControl.fValHi, filtered);
            canvas->drawBitmap(filteredMap, 0, 0, &fCoveragePaint);
        } else if (fFilterButton.enabled()) {
            canvas->drawBitmap(distMap, 0, 0, &fCoveragePaint);
        }
    }

    void draw_ball(SkCanvas* canvas) {
        // get dash positions along path
        // for each color in gradient
        // draw ball at the desired diameter and translucency to its own bitmap
        // keep max value
        // compute partial pixel coverage as well
        SkPath fWidth;
        fWidth.moveTo(0, .3f);
        fWidth.lineTo(1, .3f);
        fWidth.quadTo(1.5f, .6f, 2, .3f);
        fWidth.quadTo(2.5f, .8f, 3, .4f);
        fWidth.quadTo(3.5f, .9f, 4, .3f);
        fWidth.lineTo(5, .3f);
        SkPathMeasure measure(fPath, false);
        SkPathMeasure wMeasure(fWidth, false);
        SkScalar length = measure.getLength();
        SkScalar wLength = wMeasure.getLength();
        SkPaint paint(fCoveragePaint);
        for (SkScalar interval = 0; interval < length; interval += fBallInterval) {
            SkPoint position;
            if (!measure.getPosTan(interval, &position, nullptr)) {
                break;
            }
            SkPoint wPos;
            wMeasure.getPosTan(interval * wLength / length, &wPos, nullptr);
            int r = (int) (255 * interval * 4 / length) % 510;
            if (r > 255) r = 510 - r;
            int g = (int) (255 * wPos.fY * 6) % 510;
            if (g > 255) g = 510 - g;
            paint.setARGB(0xFF,  SkTMin(255, r), SkTMin(255, g), 0);
            canvas->drawCircle(position, fWidthControl.fValLo * wPos.fY, paint);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
#if 0
        SkDEBUGCODE(SkDebugStrokeGlobals debugGlobals);
        SkOpAA aaResult(fPath, fWidthControl.fValLo, fResControl.fValLo
                SkDEBUGPARAMS(&debugGlobals));
#endif
        SkPath strokePath;
//        aaResult.simplify(&strokePath);
        canvas->drawPath(strokePath, fSkeletonPaint);
        if (fFilterButton.fState == 1 || fFillButton.fState == 2) {
            draw_coverage(canvas);
        } else if (fFilterButton.fState == 3) {
            draw_ball(canvas);
        }
        if (fSkeletonButton.enabled()) {
            canvas->drawPath(fPath, fActiveVerb >= 0 ? fLightSkeletonPaint : fSkeletonPaint);
        }
        if (fActiveVerb >= 0) {
            draw_segment(canvas);
        }
        if (fBisectButton.enabled() || fJoinButton.enabled()) {
            draw_bisects(canvas, fActiveVerb >= 0);
        }
        if (fInOutButton.enabled()) {
            if (fActiveVerb >= 0) {
                draw_stroke(canvas, fActiveVerb);
            } else {
                for (int index = 0; index < fPath.countVerbs(); ++index) {
                    draw_stroke(canvas, index);
                }
            }
        }
        if (fHideAll) {
            return;
        }
        for (int index = 0; index < kControlCount; ++index) {
            kControlList[index].fControl->draw(canvas, fControlPaints);
        }
        for (int index = 0; index < kButtonCount; ++index) {
            kButtonList[index].fButton->draw(canvas, fButtonPaints);
        }
        if (fShowLegend) {
            draw_legend(canvas);
        }

#if 0
        SkPaint paint;
        paint.setARGB(255, 34, 31, 31);
        paint.setAntiAlias(true);

        SkPath path;
        path.moveTo(18,439);
        path.lineTo(414,439);
        path.lineTo(414,702);
        path.lineTo(18,702);
        path.lineTo(18,439);

        path.moveTo(19,701);
        path.lineTo(413,701);
        path.lineTo(413,440);
        path.lineTo(19,440);
        path.lineTo(19,701);
        path.close();
        canvas->drawPath(path, paint);

        canvas->scale(1.0f, -1.0f);
        canvas->translate(0.0f, -800.0f);
        canvas->drawPath(path, paint);
#endif

    }

    int hittest_pt(SkPoint pt) {
        for (int index = 0; index < fPath.countPoints(); ++index) {
            if (SkPoint::Distance(fPath.getPoint(index), pt) <= kHitToleranace * 2) {
                return index;
            }
        }
        return -1;
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        SkPoint pt = {x, y};
        int ptHit = hittest_pt(pt);
        if (ptHit >= 0) {
            return new MyClick(this, MyClick::kPtType, ptHit);
        }
        SkPath::Verb verb;
        SkScalar weight;
        int verbHit = hittest_verb(pt, &verb, &weight);
        if (verbHit >= 0) {
            return new MyClick(this, MyClick::kVerbType, verbHit, verb, weight);
        }
        if (!fHideAll) {
            const SkRect& rectPt = SkRect::MakeXYWH(x, y, 1, 1);
            for (int index = 0; index < kControlCount; ++index) {
                if (kControlList[index].fControl->contains(rectPt)) {
                    return new MyClick(this, MyClick::kControlType,
                            kControlList[index].fControlType);
                }
            }
            for (int index = 0; index < kButtonCount; ++index) {
                if (kButtonList[index].fButton->contains(rectPt)) {
                    return new MyClick(this, MyClick::kControlType, kButtonList[index].fButtonType);
                }
            }
        }
        fLineButton.fVisible = fQuadButton.fVisible = fConicButton.fVisible
                = fCubicButton.fVisible = fWeightControl.fVisible = fAddButton.fVisible
                = fDeleteButton.fVisible = false;
        fActiveVerb = -1;
        fActivePt = -1;
        if (fHandlePathMove) {
            return new MyClick(this, MyClick::kPathType, MyClick::kPathMove);
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    static SkScalar MapScreenYtoValue(int y, const UniControl& control) {
        return SkTMin(1.f, SkTMax(0.f,
                SkIntToScalar(y) - control.fBounds.fTop) / control.fBounds.height())
                * (control.fMax - control.fMin) + control.fMin;
    }

    bool onClick(Click* click) override {
        MyClick* myClick = (MyClick*) click;
        switch (myClick->fType) {
            case MyClick::kPtType: {
                savePath(click->fState);
                fActivePt = myClick->ptHit();
                SkPoint pt = fPath.getPoint((int) myClick->fControl);
                pt.offset(SkIntToScalar(click->fICurr.fX - click->fIPrev.fX),
                        SkIntToScalar(click->fICurr.fY - click->fIPrev.fY));
                set_path_pt(fActivePt, pt, &fPath);
                validatePath();
                this->inval(nullptr);
                return true;
                }
            case MyClick::kPathType:
                savePath(click->fState);
                fPath.offset(SkIntToScalar(click->fICurr.fX - click->fIPrev.fX),
                        SkIntToScalar(click->fICurr.fY - click->fIPrev.fY));
                validatePath();
                this->inval(nullptr);
                return true;
            case MyClick::kVerbType: {
                fActiveVerb = myClick->verbHit();
                fLineButton.fVisible = fQuadButton.fVisible = fConicButton.fVisible
                        = fCubicButton.fVisible = fAddButton.fVisible = fDeleteButton.fVisible
                        = true;
                fLineButton.setEnabled(myClick->fVerb == SkPath::kLine_Verb);
                fQuadButton.setEnabled(myClick->fVerb == SkPath::kQuad_Verb);
                fConicButton.setEnabled(myClick->fVerb == SkPath::kConic_Verb);
                fCubicButton.setEnabled(myClick->fVerb == SkPath::kCubic_Verb);
                fWeightControl.fValLo = myClick->fWeight;
                fWeightControl.fVisible = myClick->fVerb == SkPath::kConic_Verb;
                } break;
            case MyClick::kControlType: {
                if (click->fState != Click::kDown_State && myClick->isButton()) {
                    return true;
                }
                switch (myClick->fControl) {
                    case MyClick::kFilterControl: {
                        SkScalar val = MapScreenYtoValue(click->fICurr.fY, fFilterControl);
                        if (val - fFilterControl.fValLo < fFilterControl.fValHi - val) {
                            fFilterControl.fValLo = SkTMax(0.f, val);
                        } else {
                            fFilterControl.fValHi = SkTMin(255.f, val);
                        }
                        } break;
                    case MyClick::kResControl:
                        fResControl.fValLo = MapScreenYtoValue(click->fICurr.fY, fResControl);
                        break;
                    case MyClick::kWeightControl: {
                        savePath(click->fState);
                        SkScalar w = MapScreenYtoValue(click->fICurr.fY, fWeightControl);
                        set_path_weight(fActiveVerb, w, &fPath);
                        validatePath();
                        fWeightControl.fValLo = w;
                        } break;
                    case MyClick::kWidthControl:
                        fWidthControl.fValLo = MapScreenYtoValue(click->fICurr.fY, fWidthControl);
                        break;
                    case MyClick::kLineButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kLine_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClick::kQuadButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kQuad_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClick::kConicButton: {
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = true;
                        const SkScalar defaultConicWeight = 1.f / SkScalarSqrt(2);
                        set_path_verb(fActiveVerb, SkPath::kConic_Verb, &fPath, defaultConicWeight);
                        validatePath();
                        fWeightControl.fValLo = get_path_weight(fActiveVerb, fPath);
                        } break;
                    case MyClick::kCubicButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kCubic_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClick::kAddButton:
                        savePath(click->fState);
                        add_path_segment(fActiveVerb, &fPath);
                        validatePath();
                        if (fWeightControl.fVisible) {
                            fWeightControl.fValLo = get_path_weight(fActiveVerb, fPath);
                        }
                        break;
                    case MyClick::kDeleteButton:
                        savePath(click->fState);
                        delete_path_segment(fActiveVerb, &fPath);
                        validatePath();
                        break;
                    case MyClick::kFillButton:
                        fFillButton.toggle();
                        break;
                    case MyClick::kSkeletonButton:
                        fSkeletonButton.toggle();
                        break;
                    case MyClick::kFilterButton:
                        fFilterButton.toggle();
                        fFilterControl.fVisible = fFilterButton.fState == 2;
                        break;
                    case MyClick::kBisectButton:
                        fBisectButton.toggle();
                        break;
                    case MyClick::kJoinButton:
                        fJoinButton.toggle();
                        break;
                    case MyClick::kInOutButton:
                        fInOutButton.toggle();
                        break;
                    default:
                        SkASSERT(0);
                        break;
                }
            } break;
            default:
                SkASSERT(0);
                break;
        }
        setControlButtonsPos();
        this->inval(nullptr);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

static struct KeyCommand {
    char fKey;
    char fAlternate;
    const char* fDescriptionL;
    const char* fDescriptionR;
    bool (AAGeometryView::*fFunction)();
} kKeyCommandList[] = {
    { ' ',  0,  "space",   "center path", &AAGeometryView::scaleToFit },
    { '-',  0,  "-",          "zoom out", &AAGeometryView::scaleDown },
    { '+', '=', "+/=",         "zoom in", &AAGeometryView::scaleUp },
    { 'D',  0,  "D",   "dump to console", &AAGeometryView::pathDump },
    { 'H',  0,  "H",     "hide controls", &AAGeometryView::hideAll },
    { 'R',  0,  "R",        "reset path", &AAGeometryView::constructPath },
    { 'Z',  0,  "Z",              "undo", &AAGeometryView::undo },
    { '?',  0,  "?",       "show legend", &AAGeometryView::showLegend },
};

const int kKeyCommandCount = (int) SK_ARRAY_COUNT(kKeyCommandList);

void AAGeometryView::draw_legend(SkCanvas* canvas) {
    SkScalar bottomOffset = this->height() - 10;
    for (int index = kKeyCommandCount - 1; index >= 0; --index) {
        bottomOffset -= 15;
        canvas->drawString(kKeyCommandList[index].fDescriptionL,
                this->width() - 160, bottomOffset,
                fLegendLeftPaint);
        canvas->drawString(kKeyCommandList[index].fDescriptionR,
                this->width() - 20, bottomOffset,
                fLegendRightPaint);
    }
}

// overrides from SkEventSink
bool AAGeometryView::onQuery(SkEvent* evt) {
    if (SampleCode::TitleQ(*evt)) {
        SampleCode::TitleR(evt, "AAGeometry");
        return true;
    }
    SkUnichar uni;
    if (false) {
        return this->INHERITED::onQuery(evt);
    }
    if (SampleCode::CharQ(*evt, &uni)) {
        for (int index = 0; index < kButtonCount; ++index) {
            Button* button = kButtonList[index].fButton;
            if (button->fVisible && uni == button->fLabel) {
                MyClick click(this, MyClick::kControlType, kButtonList[index].fButtonType);
                click.fState = Click::kDown_State;
                (void) this->onClick(&click);
                return true;
            }
        }
        for (int index = 0; index < kKeyCommandCount; ++index) {
            KeyCommand& keyCommand = kKeyCommandList[index];
            if (uni == keyCommand.fKey || uni == keyCommand.fAlternate) {
                return (this->*keyCommand.fFunction)();
            }
        }
        if (('A' <= uni && uni <= 'Z') || ('a' <= uni && uni <= 'z')) {
            for (int index = 0; index < kButtonCount; ++index) {
                Button* button = kButtonList[index].fButton;
                if (button->fVisible && (uni & ~0x20) == (button->fLabel & ~0x20)) {
                    MyClick click(this, MyClick::kControlType, kButtonList[index].fButtonType);
                    click.fState = Click::kDown_State;
                    (void) this->onClick(&click);
                    return true;
                }
            }
        }
    }
    return this->INHERITED::onQuery(evt);
}

DEF_SAMPLE( return new AAGeometryView; )
