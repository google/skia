/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPathMeasure.h"
#include "SkPathMeasurePriv.h"
#include "SkGeometry.h"
#include "SkPath.h"
#include "SkTSearch.h"

#define kMaxTValue  0x3FFFFFFF

static inline SkScalar tValue2Scalar(int t) {
    SkASSERT((unsigned)t <= kMaxTValue);
    const SkScalar kMaxTReciprocal = 1.0f / kMaxTValue;
    return t * kMaxTReciprocal;
}

SkScalar SkPathMeasure::Segment::getScalarT() const {
    return tValue2Scalar(fTValue);
}

const SkPathMeasure::Segment* SkPathMeasure::NextSegment(const Segment* seg) {
    unsigned ptIndex = seg->fPtIndex;

    do {
        ++seg;
    } while (seg->fPtIndex == ptIndex);
    return seg;
}

void SkPathMeasure_segTo(const SkPoint pts[], unsigned segType,
                   SkScalar startT, SkScalar stopT, SkPath* dst) {
    SkASSERT(startT >= 0 && startT <= SK_Scalar1);
    SkASSERT(stopT >= 0 && stopT <= SK_Scalar1);
    SkASSERT(startT <= stopT);

    if (startT == stopT) {
        /* if the dash as a zero-length on segment, add a corresponding zero-length line.
           The stroke code will add end caps to zero length lines as appropriate */
        SkPoint lastPt;
        SkAssertResult(dst->getLastPt(&lastPt));
        dst->lineTo(lastPt);
        return;
    }

    SkPoint tmp0[7], tmp1[7];

    switch (segType) {
        case kLine_SegType:
            if (SK_Scalar1 == stopT) {
                dst->lineTo(pts[1]);
            } else {
                dst->lineTo(SkScalarInterp(pts[0].fX, pts[1].fX, stopT),
                            SkScalarInterp(pts[0].fY, pts[1].fY, stopT));
            }
            break;
        case kQuad_SegType:
            if (0 == startT) {
                if (SK_Scalar1 == stopT) {
                    dst->quadTo(pts[1], pts[2]);
                } else {
                    SkChopQuadAt(pts, tmp0, stopT);
                    dst->quadTo(tmp0[1], tmp0[2]);
                }
            } else {
                SkChopQuadAt(pts, tmp0, startT);
                if (SK_Scalar1 == stopT) {
                    dst->quadTo(tmp0[3], tmp0[4]);
                } else {
                    SkChopQuadAt(&tmp0[2], tmp1, (stopT - startT) / (1 - startT));
                    dst->quadTo(tmp1[1], tmp1[2]);
                }
            }
            break;
        case kConic_SegType: {
            SkConic conic(pts[0], pts[2], pts[3], pts[1].fX);

            if (0 == startT) {
                if (SK_Scalar1 == stopT) {
                    dst->conicTo(conic.fPts[1], conic.fPts[2], conic.fW);
                } else {
                    SkConic tmp[2];
                    if (conic.chopAt(stopT, tmp)) {
                        dst->conicTo(tmp[0].fPts[1], tmp[0].fPts[2], tmp[0].fW);
                    }
                }
            } else {
                if (SK_Scalar1 == stopT) {
                    SkConic tmp1[2];
                    if (conic.chopAt(startT, tmp1)) {
                        dst->conicTo(tmp1[1].fPts[1], tmp1[1].fPts[2], tmp1[1].fW);
                    }
                } else {
                    SkConic tmp;
                    conic.chopAt(startT, stopT, &tmp);
                    dst->conicTo(tmp.fPts[1], tmp.fPts[2], tmp.fW);
                }
            }
        } break;
        case kCubic_SegType:
            if (0 == startT) {
                if (SK_Scalar1 == stopT) {
                    dst->cubicTo(pts[1], pts[2], pts[3]);
                } else {
                    SkChopCubicAt(pts, tmp0, stopT);
                    dst->cubicTo(tmp0[1], tmp0[2], tmp0[3]);
                }
            } else {
                SkChopCubicAt(pts, tmp0, startT);
                if (SK_Scalar1 == stopT) {
                    dst->cubicTo(tmp0[4], tmp0[5], tmp0[6]);
                } else {
                    SkChopCubicAt(&tmp0[3], tmp1, (stopT - startT) / (1 - startT));
                    dst->cubicTo(tmp1[1], tmp1[2], tmp1[3]);
                }
            }
            break;
        default:
            SkDEBUGFAIL("unknown segType");
            sk_throw();
    }
}

///////////////////////////////////////////////////////////////////////////////

static inline int tspan_big_enough(int tspan) {
    SkASSERT((unsigned)tspan <= kMaxTValue);
    return tspan >> 10;
}

// can't use tangents, since we need [0..1..................2] to be seen
// as definitely not a line (it is when drawn, but not parametrically)
// so we compare midpoints
#define CHEAP_DIST_LIMIT    (SK_Scalar1/2)  // just made this value up

bool SkPathMeasure::quad_too_curvy(const SkPoint pts[3]) {
    // diff = (a/4 + b/2 + c/4) - (a/2 + c/2)
    // diff = -a/4 + b/2 - c/4
    SkScalar dx = SkScalarHalf(pts[1].fX) -
                        SkScalarHalf(SkScalarHalf(pts[0].fX + pts[2].fX));
    SkScalar dy = SkScalarHalf(pts[1].fY) -
                        SkScalarHalf(SkScalarHalf(pts[0].fY + pts[2].fY));

    SkScalar dist = SkMaxScalar(SkScalarAbs(dx), SkScalarAbs(dy));
    return dist > fTolerance;
}

bool SkPathMeasure::conic_too_curvy(const SkPoint& firstPt, const SkPoint& midTPt,
                            const SkPoint& lastPt) {
    SkPoint midEnds = firstPt + lastPt;
    midEnds *= 0.5f;
    SkVector dxy = midTPt - midEnds;
    SkScalar dist = SkMaxScalar(SkScalarAbs(dxy.fX), SkScalarAbs(dxy.fY));
    return dist > fTolerance;
}

bool SkPathMeasure::cheap_dist_exceeds_limit(const SkPoint& pt,
                                     SkScalar x, SkScalar y) {
    SkScalar dist = SkMaxScalar(SkScalarAbs(x - pt.fX), SkScalarAbs(y - pt.fY));
    // just made up the 1/2
    return dist > fTolerance;
}

bool SkPathMeasure::cubic_too_curvy(const SkPoint pts[4]) {
    return  cheap_dist_exceeds_limit(pts[1],
                         SkScalarInterp(pts[0].fX, pts[3].fX, SK_Scalar1/3),
                         SkScalarInterp(pts[0].fY, pts[3].fY, SK_Scalar1/3))
                         ||
            cheap_dist_exceeds_limit(pts[2],
                         SkScalarInterp(pts[0].fX, pts[3].fX, SK_Scalar1*2/3),
                         SkScalarInterp(pts[0].fY, pts[3].fY, SK_Scalar1*2/3));
}

static SkScalar quad_folded_len(const SkPoint pts[3]) {
    SkScalar t = SkFindQuadMaxCurvature(pts);
    SkPoint pt = SkEvalQuadAt(pts, t);
    SkVector a = pts[2] - pt;
    SkScalar result = a.length();
    if (0 != t) {
        SkVector b = pts[0] - pt;
        result += b.length();
    }
    SkASSERT(SkScalarIsFinite(result));
    return result;
}

/* from http://www.malczak.linuxpl.com/blog/quadratic-bezier-curve-length/ */
/* This works -- more needs to be done to see if it is performant on all platforms.
   To use this to measure parts of quads requires recomputing everything -- perhaps
   a chop-like interface can start from a larger measurement and get two new measurements
   with one call here.
 */
static SkScalar compute_quad_len(const SkPoint pts[3]) {
    SkPoint a,b;
    a.fX = pts[0].fX - 2 * pts[1].fX + pts[2].fX;
    a.fY = pts[0].fY - 2 * pts[1].fY + pts[2].fY;
    SkScalar A = 4 * (a.fX * a.fX + a.fY * a.fY);
    if (0 == A) {
        a = pts[2] - pts[0];
        return a.length();
    }
    b.fX = 2 * (pts[1].fX - pts[0].fX);
    b.fY = 2 * (pts[1].fY - pts[0].fY);
    SkScalar B = 4 * (a.fX * b.fX + a.fY * b.fY);
    SkScalar C =      b.fX * b.fX + b.fY * b.fY;
    SkScalar Sabc = 2 * SkScalarSqrt(A + B + C);
    SkScalar A_2  = SkScalarSqrt(A);
    SkScalar A_32 = 2 * A * A_2;
    SkScalar C_2  = 2 * SkScalarSqrt(C);
    SkScalar BA   = B / A_2;
    if (0 == BA + C_2) {
        return quad_folded_len(pts);
    }
    SkScalar J = A_32 * Sabc + A_2 * B * (Sabc - C_2);
    SkScalar K = 4 * C * A - B * B;
    SkScalar L = (2 * A_2 + BA + Sabc) / (BA + C_2);
    if (L <= 0) {
        return quad_folded_len(pts);
    }
    SkScalar M = SkScalarLog(L);
    SkScalar result = (J + K * M) / (4 * A_32);
    SkASSERT(SkScalarIsFinite(result));
    return result;
}

SkScalar SkPathMeasure::compute_quad_segs(const SkPoint pts[3],
                          SkScalar distance, int mint, int maxt, int ptIndex) {
    if (tspan_big_enough(maxt - mint) && quad_too_curvy(pts)) {
        SkPoint tmp[5];
        int     halft = (mint + maxt) >> 1;

        SkChopQuadAtHalf(pts, tmp);
        distance = this->compute_quad_segs(tmp, distance, mint, halft, ptIndex);
        distance = this->compute_quad_segs(&tmp[2], distance, halft, maxt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(pts[0], pts[2]);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kQuad_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;
}

SkScalar SkPathMeasure::compute_conic_segs(const SkConic& conic, SkScalar distance,
                                           int mint, const SkPoint& minPt,
                                           int maxt, const SkPoint& maxPt, int ptIndex) {
    int halft = (mint + maxt) >> 1;
    SkPoint halfPt = conic.evalAt(tValue2Scalar(halft));
    if (tspan_big_enough(maxt - mint) && conic_too_curvy(minPt, halfPt, maxPt)) {
        distance = this->compute_conic_segs(conic, distance, mint, minPt, halft, halfPt, ptIndex);
        distance = this->compute_conic_segs(conic, distance, halft, halfPt, maxt, maxPt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(minPt, maxPt);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kConic_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;
}

SkScalar SkPathMeasure::compute_cubic_segs(const SkPoint pts[4],
                           SkScalar distance, int mint, int maxt, int ptIndex) {
    if (tspan_big_enough(maxt - mint) && cubic_too_curvy(pts)) {
        SkPoint tmp[7];
        int     halft = (mint + maxt) >> 1;

        SkChopCubicAtHalf(pts, tmp);
        distance = this->compute_cubic_segs(tmp, distance, mint, halft, ptIndex);
        distance = this->compute_cubic_segs(&tmp[3], distance, halft, maxt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(pts[0], pts[3]);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kCubic_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;
}

void SkPathMeasure::buildSegments() {
    SkPoint         pts[4];
    int             ptIndex = fFirstPtIndex;
    SkScalar        distance = 0;
    bool            isClosed = fForceClosed;
    bool            firstMoveTo = ptIndex < 0;
    Segment*        seg;

    /*  Note:
     *  as we accumulate distance, we have to check that the result of +=
     *  actually made it larger, since a very small delta might be > 0, but
     *  still have no effect on distance (if distance >>> delta).
     *
     *  We do this check below, and in compute_quad_segs and compute_cubic_segs
     */
    fSegments.reset();
    bool done = false;
    do {
        switch (fIter.next(pts)) {
            case SkPath::kMove_Verb:
                ptIndex += 1;
                fPts.append(1, pts);
                if (!firstMoveTo) {
                    done = true;
                    break;
                }
                firstMoveTo = false;
                break;

            case SkPath::kLine_Verb: {
                SkScalar d = SkPoint::Distance(pts[0], pts[1]);
                SkASSERT(d >= 0);
                SkScalar prevD = distance;
                distance += d;
                if (distance > prevD) {
                    seg = fSegments.append();
                    seg->fDistance = distance;
                    seg->fPtIndex = ptIndex;
                    seg->fType = kLine_SegType;
                    seg->fTValue = kMaxTValue;
                    fPts.append(1, pts + 1);
                    ptIndex++;
                }
            } break;

            case SkPath::kQuad_Verb: {
                SkScalar prevD = distance;
                if (false) {
                    SkScalar length = compute_quad_len(pts);
                    if (length) {
                        distance += length;
                        Segment* seg = fSegments.append();
                        seg->fDistance = distance;
                        seg->fPtIndex = ptIndex;
                        seg->fType = kQuad_SegType;
                        seg->fTValue = kMaxTValue;
                    }
                } else {
                    distance = this->compute_quad_segs(pts, distance, 0, kMaxTValue, ptIndex);
                }
                if (distance > prevD) {
                    fPts.append(2, pts + 1);
                    ptIndex += 2;
                }
            } break;

            case SkPath::kConic_Verb: {
                const SkConic conic(pts, fIter.conicWeight());
                SkScalar prevD = distance;
                distance = this->compute_conic_segs(conic, distance, 0, conic.fPts[0],
                                                    kMaxTValue, conic.fPts[2], ptIndex);
                if (distance > prevD) {
                    // we store the conic weight in our next point, followed by the last 2 pts
                    // thus to reconstitue a conic, you'd need to say
                    // SkConic(pts[0], pts[2], pts[3], weight = pts[1].fX)
                    fPts.append()->set(conic.fW, 0);
                    fPts.append(2, pts + 1);
                    ptIndex += 3;
                }
            } break;

            case SkPath::kCubic_Verb: {
                SkScalar prevD = distance;
                distance = this->compute_cubic_segs(pts, distance, 0, kMaxTValue, ptIndex);
                if (distance > prevD) {
                    fPts.append(3, pts + 1);
                    ptIndex += 3;
                }
            } break;

            case SkPath::kClose_Verb:
                isClosed = true;
                break;

            case SkPath::kDone_Verb:
                done = true;
                break;
        }
    } while (!done);

    fLength = distance;
    fIsClosed = isClosed;
    fFirstPtIndex = ptIndex;

#ifdef SK_DEBUG
    {
        const Segment* seg = fSegments.begin();
        const Segment* stop = fSegments.end();
        unsigned        ptIndex = 0;
        SkScalar        distance = 0;
        // limit the loop to a reasonable number; pathological cases can run for minutes
        int             maxChecks = 10000000;  // set to INT_MAX to defeat the check
        while (seg < stop) {
            SkASSERT(seg->fDistance > distance);
            SkASSERT(seg->fPtIndex >= ptIndex);
            SkASSERT(seg->fTValue > 0);

            const Segment* s = seg;
            while (s < stop - 1 && s[0].fPtIndex == s[1].fPtIndex && --maxChecks > 0) {
                SkASSERT(s[0].fType == s[1].fType);
                SkASSERT(s[0].fTValue < s[1].fTValue);
                s += 1;
            }

            distance = seg->fDistance;
            ptIndex = seg->fPtIndex;
            seg += 1;
        }
    //  SkDebugf("\n");
    }
#endif
}

static void compute_pos_tan(const SkPoint pts[], unsigned segType,
                            SkScalar t, SkPoint* pos, SkVector* tangent) {
    switch (segType) {
        case kLine_SegType:
            if (pos) {
                pos->set(SkScalarInterp(pts[0].fX, pts[1].fX, t),
                         SkScalarInterp(pts[0].fY, pts[1].fY, t));
            }
            if (tangent) {
                tangent->setNormalize(pts[1].fX - pts[0].fX, pts[1].fY - pts[0].fY);
            }
            break;
        case kQuad_SegType:
            SkEvalQuadAt(pts, t, pos, tangent);
            if (tangent) {
                tangent->normalize();
            }
            break;
        case kConic_SegType: {
            SkConic(pts[0], pts[2], pts[3], pts[1].fX).evalAt(t, pos, tangent);
            if (tangent) {
                tangent->normalize();
            }
        } break;
        case kCubic_SegType:
            SkEvalCubicAt(pts, t, pos, tangent, nullptr);
            if (tangent) {
                tangent->normalize();
            }
            break;
        default:
            SkDEBUGFAIL("unknown segType");
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

SkPathMeasure::SkPathMeasure() {
    fPath = nullptr;
    fTolerance = CHEAP_DIST_LIMIT;
    fLength = -1;   // signal we need to compute it
    fForceClosed = false;
    fFirstPtIndex = -1;
}

SkPathMeasure::SkPathMeasure(const SkPath& path, bool forceClosed, SkScalar resScale) {
    fPath = &path;
    fTolerance = CHEAP_DIST_LIMIT * SkScalarInvert(resScale);
    fLength = -1;   // signal we need to compute it
    fForceClosed = forceClosed;
    fFirstPtIndex = -1;

    fIter.setPath(path, forceClosed);
}

SkPathMeasure::~SkPathMeasure() {}

/** Assign a new path, or null to have none.
*/
void SkPathMeasure::setPath(const SkPath* path, bool forceClosed) {
    fPath = path;
    fLength = -1;   // signal we need to compute it
    fForceClosed = forceClosed;
    fFirstPtIndex = -1;

    if (path) {
        fIter.setPath(*path, forceClosed);
    }
    fSegments.reset();
    fPts.reset();
}

SkScalar SkPathMeasure::getLength() {
    if (fPath == nullptr) {
        return 0;
    }
    if (fLength < 0) {
        this->buildSegments();
    }
    SkASSERT(fLength >= 0);
    return fLength;
}

template <typename T, typename K>
int SkTKSearch(const T base[], int count, const K& key) {
    SkASSERT(count >= 0);
    if (count <= 0) {
        return ~0;
    }

    SkASSERT(base != nullptr); // base may be nullptr if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        if (base[mid].fDistance < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (base[hi].fDistance < key) {
        hi += 1;
        hi = ~hi;
    } else if (key < base[hi].fDistance) {
        hi = ~hi;
    }
    return hi;
}

const SkPathMeasure::Segment* SkPathMeasure::distanceToSegment(
                                            SkScalar distance, SkScalar* t) {
    SkDEBUGCODE(SkScalar length = ) this->getLength();
    SkASSERT(distance >= 0 && distance <= length);

    const Segment*  seg = fSegments.begin();
    int             count = fSegments.count();

    int index = SkTKSearch<Segment, SkScalar>(seg, count, distance);
    // don't care if we hit an exact match or not, so we xor index if it is negative
    index ^= (index >> 31);
    seg = &seg[index];

    // now interpolate t-values with the prev segment (if possible)
    SkScalar    startT = 0, startD = 0;
    // check if the prev segment is legal, and references the same set of points
    if (index > 0) {
        startD = seg[-1].fDistance;
        if (seg[-1].fPtIndex == seg->fPtIndex) {
            SkASSERT(seg[-1].fType == seg->fType);
            startT = seg[-1].getScalarT();
        }
    }

    SkASSERT(seg->getScalarT() > startT);
    SkASSERT(distance >= startD);
    SkASSERT(seg->fDistance > startD);

    *t = startT + (seg->getScalarT() - startT) * (distance - startD) / (seg->fDistance - startD);
    return seg;
}

bool SkPathMeasure::getPosTan(SkScalar distance, SkPoint* pos, SkVector* tangent) {
    if (nullptr == fPath) {
        return false;
    }

    SkScalar    length = this->getLength(); // call this to force computing it
    int         count = fSegments.count();

    if (count == 0 || length == 0) {
        return false;
    }

    // pin the distance to a legal range
    if (distance < 0) {
        distance = 0;
    } else if (distance > length) {
        distance = length;
    }

    SkScalar        t;
    const Segment*  seg = this->distanceToSegment(distance, &t);

    compute_pos_tan(&fPts[seg->fPtIndex], seg->fType, t, pos, tangent);
    return true;
}

bool SkPathMeasure::getMatrix(SkScalar distance, SkMatrix* matrix,
                              MatrixFlags flags) {
    if (nullptr == fPath) {
        return false;
    }

    SkPoint     position;
    SkVector    tangent;

    if (this->getPosTan(distance, &position, &tangent)) {
        if (matrix) {
            if (flags & kGetTangent_MatrixFlag) {
                matrix->setSinCos(tangent.fY, tangent.fX, 0, 0);
            } else {
                matrix->reset();
            }
            if (flags & kGetPosition_MatrixFlag) {
                matrix->postTranslate(position.fX, position.fY);
            }
        }
        return true;
    }
    return false;
}

bool SkPathMeasure::getSegment(SkScalar startD, SkScalar stopD, SkPath* dst,
                               bool startWithMoveTo) {
    SkASSERT(dst);

    SkScalar length = this->getLength();    // ensure we have built our segments

    if (startD < 0) {
        startD = 0;
    }
    if (stopD > length) {
        stopD = length;
    }
    if (startD > stopD) {
        return false;
    }
    if (!fSegments.count()) {
        return false;
    }

    SkPoint  p;
    SkScalar startT, stopT;
    const Segment* seg = this->distanceToSegment(startD, &startT);
    const Segment* stopSeg = this->distanceToSegment(stopD, &stopT);
    SkASSERT(seg <= stopSeg);

    if (startWithMoveTo) {
        compute_pos_tan(&fPts[seg->fPtIndex], seg->fType, startT, &p, nullptr);
        dst->moveTo(p);
    }

    if (seg->fPtIndex == stopSeg->fPtIndex) {
        SkPathMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, startT, stopT, dst);
    } else {
        do {
            SkPathMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, startT, SK_Scalar1, dst);
            seg = SkPathMeasure::NextSegment(seg);
            startT = 0;
        } while (seg->fPtIndex < stopSeg->fPtIndex);
        SkPathMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, 0, stopT, dst);
    }
    return true;
}

bool SkPathMeasure::isClosed() {
    (void)this->getLength();
    return fIsClosed;
}

/** Move to the next contour in the path. Return true if one exists, or false if
    we're done with the path.
*/
bool SkPathMeasure::nextContour() {
    fLength = -1;
    return this->getLength() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkPathMeasure::dump() {
    SkDebugf("pathmeas: length=%g, segs=%d\n", fLength, fSegments.count());

    for (int i = 0; i < fSegments.count(); i++) {
        const Segment* seg = &fSegments[i];
        SkDebugf("pathmeas: seg[%d] distance=%g, point=%d, t=%g, type=%d\n",
                i, seg->fDistance, seg->fPtIndex, seg->getScalarT(),
                 seg->fType);
    }
}

#endif
