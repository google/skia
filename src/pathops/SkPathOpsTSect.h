/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChunkAlloc.h"
#include "SkPathOpsBounds.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsQuad.h"
#include "SkIntersections.h"
#include "SkTSort.h"

/* TCurve is either SkDQuadratic or SkDCubic */
template<typename TCurve>
class SkTCoincident {
public:
    SkTCoincident()
        : fCoincident(false) {
    }

    void clear() {
        fPerpT = -1;
        fCoincident = false;
    }

    bool isCoincident() const {
        return fCoincident;
    }

    void init() {
        fCoincident = false;
        SkDEBUGCODE(fPerpPt.fX = fPerpPt.fY = SK_ScalarNaN);
        SkDEBUGCODE(fPerpT = SK_ScalarNaN);
    }

    void markCoincident() {
        if (!fCoincident) {
            fPerpT = -1;
        }
        fCoincident = true;
    }

    const SkDPoint& perpPt() const {
        return fPerpPt;
    }

    double perpT() const {
        return fPerpT;
    }

    void setPerp(const TCurve& c1, double t, const SkDPoint& cPt, const TCurve& );

private:
    SkDPoint fPerpPt;
    double fPerpT;  // perpendicular intersection on opposite curve
    bool fCoincident;
};

template<typename TCurve> class SkTSect;
template<typename TCurve> class SkTSpan;

template<typename TCurve>
struct SkTSpanBounded {
    SkTSpan<TCurve>* fBounded;
    SkTSpanBounded* fNext;
};

/* Curve is either TCurve or SkDCubic */
template<typename TCurve>
class SkTSpan {
public:
    void addBounded(SkTSpan* , SkChunkAlloc* );
    double closestBoundedT(const SkDPoint& pt) const;
    bool contains(double t) const;

    const SkTSect<TCurve>* debugOpp() const;
    const SkTSpan* debugSpan(int ) const;
    const SkTSpan* debugT(double t) const;
#ifdef SK_DEBUG
    bool debugIsBefore(const SkTSpan* span) const;
#endif
    void dump() const;
    void dumpBounds(int id) const;

    double endT() const {
        return fEndT;
    }

    SkTSpan* findOppSpan(const SkTSpan* opp) const;

    SkTSpan* findOppT(double t) const {
        SkTSpan* result = oppT(t);
        SkASSERT(result);
        return result;
    }

    bool hasOppT(double t) const {
        return SkToBool(oppT(t));
    }

    int hullsIntersect(SkTSpan* span, bool* start, bool* oppStart);
    void init(const TCurve& );
    void initBounds(const TCurve& );

    bool isBounded() const {
        return fBounded != NULL;
    }

    bool linearsIntersect(SkTSpan* span);
    double linearT(const SkDPoint& ) const;

    void markCoincident() {
        fCoinStart.markCoincident();
        fCoinEnd.markCoincident();
    }

    const SkTSpan* next() const {
        return fNext;
    }

    bool onlyEndPointsInCommon(const SkTSpan* opp, bool* start, bool* oppStart, bool* ptsInCommon);

    const TCurve& part() const {
        return fPart;
    }

    bool removeAllBounded();
    bool removeBounded(const SkTSpan* opp);

    void reset() {
        fBounded = NULL;
    }

    void resetBounds(const TCurve& curve) {
        fIsLinear = fIsLine = false;
        initBounds(curve);
    }

    bool split(SkTSpan* work, SkChunkAlloc* heap) {
        return splitAt(work, (work->fStartT + work->fEndT) * 0.5, heap);
    }

    bool splitAt(SkTSpan* work, double t, SkChunkAlloc* heap);

    double startT() const {
        return fStartT;
    }

private:

    // implementation is for testing only
    int debugID() const {
        return PATH_OPS_DEBUG_T_SECT_RELEASE(fID, -1);
    }

    void dumpID() const;

    int hullCheck(const SkTSpan* opp, bool* start, bool* oppStart);
    int linearIntersects(const TCurve& ) const;
    SkTSpan* oppT(double t) const;

    void validate() const;
    void validateBounded() const;
    void validatePerpT(double oppT) const;
    void validatePerpPt(double t, const SkDPoint& ) const;

    TCurve fPart;
    SkTCoincident<TCurve> fCoinStart;
    SkTCoincident<TCurve> fCoinEnd;
    SkTSpanBounded<TCurve>* fBounded;
    SkTSpan* fPrev;
    SkTSpan* fNext;
    SkDRect fBounds;
    double fStartT;
    double fEndT;
    double fBoundsMax;
    bool fCollapsed;
    bool fHasPerp;
    bool fIsLinear;
    bool fIsLine;
    bool fDeleted;
    PATH_OPS_DEBUG_CODE(SkTSect<TCurve>* fDebugSect);
    PATH_OPS_DEBUG_T_SECT_CODE(int fID);
    friend class SkTSect<TCurve>;
};

template<typename TCurve>
class SkTSect {
public:
    SkTSect(const TCurve& c  PATH_OPS_DEBUG_T_SECT_PARAMS(int id));
    static void BinarySearch(SkTSect* sect1, SkTSect* sect2, SkIntersections* intersections);

    // for testing only
    bool debugHasBounded(const SkTSpan<TCurve>* ) const;

    const SkTSect* debugOpp() const {
        return PATH_OPS_DEBUG_RELEASE(fOppSect, NULL);
    }

    const SkTSpan<TCurve>* debugSpan(int id) const;
    const SkTSpan<TCurve>* debugT(double t) const;
    void dump() const;
    void dumpBoth(SkTSect* ) const;
    void dumpBounds(int id) const;
    void dumpCoin() const;
    void dumpCoinCurves() const;
    void dumpCurves() const;

private:
    enum {
        kZeroS1Set = 1,
        kOneS1Set = 2,
        kZeroS2Set = 4,
        kOneS2Set = 8
    };

    SkTSpan<TCurve>* addFollowing(SkTSpan<TCurve>* prior);
    void addForPerp(SkTSpan<TCurve>* span, double t);
    SkTSpan<TCurve>* addOne();
    
    SkTSpan<TCurve>* addSplitAt(SkTSpan<TCurve>* span, double t) {
        SkTSpan<TCurve>* result = this->addOne();
        result->splitAt(span, t, &fHeap);
        result->initBounds(fCurve);
        span->initBounds(fCurve);
        return result;
    }

    bool binarySearchCoin(SkTSect* , double tStart, double tStep, double* t, double* oppT);
    SkTSpan<TCurve>* boundsMax() const;
    void coincidentCheck(SkTSect* sect2);
    bool coincidentHasT(double t);
    void computePerpendiculars(SkTSect* sect2, SkTSpan<TCurve>* first, SkTSpan<TCurve>* last);
    int countConsecutiveSpans(SkTSpan<TCurve>* first, SkTSpan<TCurve>** last) const;

    int debugID() const {
        return PATH_OPS_DEBUG_T_SECT_RELEASE(fID, -1);
    }

    void deleteEmptySpans();
    void dumpCommon(const SkTSpan<TCurve>* ) const;
    void dumpCommonCurves(const SkTSpan<TCurve>* ) const;
    static int EndsEqual(const SkTSect* sect1, const SkTSect* sect2, SkIntersections* );
    SkTSpan<TCurve>* extractCoincident(SkTSect* sect2, SkTSpan<TCurve>* first,
                                       SkTSpan<TCurve>* last);
    SkTSpan<TCurve>* findCoincidentRun(SkTSpan<TCurve>* first, SkTSpan<TCurve>** lastPtr,
                                       const SkTSect* sect2);
    int intersects(SkTSpan<TCurve>* span, const SkTSect* opp,
                   SkTSpan<TCurve>* oppSpan, int* oppResult) const;
    int linesIntersect(const SkTSpan<TCurve>* span, const SkTSect* opp,
                       const SkTSpan<TCurve>* oppSpan, SkIntersections* ) const;
    void markSpanGone(SkTSpan<TCurve>* span);
    bool matchedDirection(double t, const SkTSect* sect2, double t2) const;
    void matchedDirCheck(double t, const SkTSect* sect2, double t2,
                         bool* calcMatched, bool* oppMatched) const;
    void mergeCoincidence(SkTSect* sect2);
    SkTSpan<TCurve>* prev(SkTSpan<TCurve>* ) const;
    void removeByPerpendicular(SkTSect* opp);
    void recoverCollapsed();
    void removeCoincident(SkTSpan<TCurve>* span, bool isBetween);
    void removeAllBut(const SkTSpan<TCurve>* keep, SkTSpan<TCurve>* span, SkTSect* opp);
    void removeSpan(SkTSpan<TCurve>* span);
    void removeSpanRange(SkTSpan<TCurve>* first, SkTSpan<TCurve>* last);
    void removeSpans(SkTSpan<TCurve>* span, SkTSect* opp);
    SkTSpan<TCurve>* spanAtT(double t, SkTSpan<TCurve>** priorSpan);
    SkTSpan<TCurve>* tail();
    void trim(SkTSpan<TCurve>* span, SkTSect* opp);
    void unlinkSpan(SkTSpan<TCurve>* span);
    bool updateBounded(SkTSpan<TCurve>* first, SkTSpan<TCurve>* last, SkTSpan<TCurve>* oppFirst);
    void validate() const;
    void validateBounded() const;

    const TCurve& fCurve;
    SkChunkAlloc fHeap;
    SkTSpan<TCurve>* fHead;
    SkTSpan<TCurve>* fCoincident;
    SkTSpan<TCurve>* fDeleted;
    int fActiveCount;
    PATH_OPS_DEBUG_CODE(SkTSect* fOppSect);
    PATH_OPS_DEBUG_T_SECT_CODE(int fID);
    PATH_OPS_DEBUG_T_SECT_CODE(int fDebugCount);
#if DEBUG_T_SECT
    int fDebugAllocatedCount;
#endif
    friend class SkTSpan<TCurve>;  // only used by debug id
};

#define COINCIDENT_SPAN_COUNT 9

template<typename TCurve>
void SkTCoincident<TCurve>::setPerp(const TCurve& c1, double t,
        const SkDPoint& cPt, const TCurve& c2) {
    SkDVector dxdy = c1.dxdyAtT(t);
    SkDLine perp = {{ cPt, {cPt.fX + dxdy.fY, cPt.fY - dxdy.fX} }};
    SkIntersections i;
    int used = i.intersectRay(c2, perp);
    // only keep closest
    if (used == 0 || used == 3) {
        this->clear();
        return;
    } 
    fPerpT = i[0][0];
    fPerpPt = i.pt(0);
    SkASSERT(used <= 2);
    if (used == 2) {
        double distSq = (fPerpPt - cPt).lengthSquared();
        double dist2Sq = (i.pt(1) - cPt).lengthSquared();
        if (dist2Sq < distSq) {
            fPerpT = i[0][1];
            fPerpPt = i.pt(1);
        }
    }
#if DEBUG_T_SECT
    SkDebugf("%s cPt=(%1.9g,%1.9g) %s fPerpPt=(%1.9g,%1.9g)\n", __FUNCTION__, cPt.fX, cPt.fY,
            cPt.approximatelyEqual(fPerpPt) ? "==" : "!=", fPerpPt.fX, fPerpPt.fY);
#endif
    fCoincident = cPt.approximatelyEqual(fPerpPt);
#if DEBUG_T_SECT
    if (fCoincident) {
        SkDebugf("");  // allow setting breakpoint
    }
#endif
}

template<typename TCurve>
void SkTSpan<TCurve>::addBounded(SkTSpan* span, SkChunkAlloc* heap) {
    SkTSpanBounded<TCurve>* bounded = SkNEW_PLACEMENT(heap->allocThrow(
            sizeof(SkTSpanBounded<TCurve>)), SkTSpanBounded<TCurve>);
    bounded->fBounded = span;
    bounded->fNext = fBounded;
    fBounded = bounded;
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::addFollowing(SkTSpan<TCurve>* prior) {
    SkTSpan<TCurve>* result = this->addOne();
    result->fStartT = prior ? prior->fEndT : 0;
    SkTSpan<TCurve>* next = prior ? prior->fNext : fHead;
    result->fEndT = next ? next->fStartT : 1;
    result->fPrev = prior;
    result->fNext = next;
    if (prior) {
        prior->fNext = result;
    } else {
        fHead = result;
    }
    if (next) {
        next->fPrev = result;
    }
    result->resetBounds(fCurve);
    return result;
}

template<typename TCurve>
void SkTSect<TCurve>::addForPerp(SkTSpan<TCurve>* span, double t) {
    if (!span->hasOppT(t)) {
        SkTSpan<TCurve>* priorSpan;
        SkTSpan<TCurve>* opp = this->spanAtT(t, &priorSpan);
        if (!opp) {
            opp = this->addFollowing(priorSpan);
#if DEBUG_PERP
            SkDebugf("%s priorSpan=%d t=%1.9g opp=%d\n", __FUNCTION__, priorSpan->debugID(), t,
                    opp->debugID());
#endif
        }
#if DEBUG_PERP
        opp->dump(); SkDebugf("\n");
        SkDebugf("%s addBounded span=%d opp=%d\n", __FUNCTION__, priorSpan->debugID(),
                opp->debugID());
#endif
        opp->addBounded(span, &fHeap);
        span->addBounded(opp, &fHeap);
    }
    this->validate();
    span->validatePerpT(t);
}

template<typename TCurve>
double SkTSpan<TCurve>::closestBoundedT(const SkDPoint& pt) const {
    double result = -1;
    double closest = FLT_MAX;
    const SkTSpanBounded<TCurve>* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* test = testBounded->fBounded;
        double startDist = test->fPart[0].distanceSquared(pt);
        if (closest > startDist) {
            closest = startDist;
            result = test->fStartT;
        }
        double endDist = test->fPart[TCurve::kPointLast].distanceSquared(pt);
        if (closest > endDist) {
            closest = endDist;
            result = test->fEndT;
        }
        testBounded = testBounded->fNext;
    }
    SkASSERT(between(0, result, 1));
    return result;
}

#ifdef SK_DEBUG
template<typename TCurve>
bool SkTSpan<TCurve>::debugIsBefore(const SkTSpan* span) const {
    const SkTSpan* work = this;
    do {
        if (span == work) {
            return true;
        }
    } while ((work = work->fNext));
    return false;
}
#endif

template<typename TCurve>
bool SkTSpan<TCurve>::contains(double t) const {
    const SkTSpan* work = this;
    do {
        if (between(work->fStartT, t, work->fEndT)) {
            return true;
        }
    } while ((work = work->fNext));
    return false;
}

template<typename TCurve>
const SkTSect<TCurve>* SkTSpan<TCurve>::debugOpp() const {
    return PATH_OPS_DEBUG_RELEASE(fDebugSect->debugOpp(), NULL);
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSpan<TCurve>::findOppSpan(const SkTSpan* opp) const {
    SkTSpanBounded<TCurve>* bounded = fBounded;
    while (bounded) {
        SkTSpan* test = bounded->fBounded;
        if (opp == test) {
            return test;
        }
        bounded = bounded->fNext;
    }
    return NULL;
}

// returns 0 if no hull intersection
//         1 if hulls intersect
//         2 if hulls only share a common endpoint
//        -1 if linear and further checking is required
template<typename TCurve>
int SkTSpan<TCurve>::hullCheck(const SkTSpan* opp, bool* start, bool* oppStart) {
    if (fIsLinear) {
        return -1;
    }
    bool ptsInCommon;
    if (onlyEndPointsInCommon(opp, start, oppStart, &ptsInCommon)) {
        SkASSERT(ptsInCommon);
        return 2;
    }
    bool linear;
    if (fPart.hullIntersects(opp->fPart, &linear)) {
        if (!linear) {  // check set true if linear
            return 1;
        }
        fIsLinear = true;
        fIsLine = fPart.controlsInside();
        return ptsInCommon ? 2 : -1;
    } else {  // hull is not linear; check set true if intersected at the end points
        return ((int) ptsInCommon) << 1;  // 0 or 2
    }
    return 0;
}

// OPTIMIZE ? If at_most_end_pts_in_common detects that one quad is near linear,
// use line intersection to guess a better split than 0.5
// OPTIMIZE Once at_most_end_pts_in_common detects linear, mark span so all future splits are linear
template<typename TCurve>
int SkTSpan<TCurve>::hullsIntersect(SkTSpan* opp, bool* start, bool* oppStart) {
    if (!fBounds.intersects(opp->fBounds)) {
        return 0;
    }
    int hullSect = this->hullCheck(opp, start, oppStart);
    if (hullSect >= 0) {
        return hullSect;
    }
    hullSect = opp->hullCheck(this, oppStart, start);
    if (hullSect >= 0) {
        return hullSect;
    }
    return -1;
}

template<typename TCurve>
void SkTSpan<TCurve>::init(const TCurve& c) {
    fPrev = fNext = NULL;
    fStartT = 0;
    fEndT = 1;
    fBounded = NULL;
    resetBounds(c);
}

template<typename TCurve>
void SkTSpan<TCurve>::initBounds(const TCurve& c) {
    fPart = c.subDivide(fStartT, fEndT);
    fBounds.setBounds(fPart);
    fCoinStart.init();
    fCoinEnd.init();
    fBoundsMax = SkTMax(fBounds.width(), fBounds.height());
    fCollapsed = fPart.collapsed();
    fHasPerp = false;
    fDeleted = false;
#if DEBUG_T_SECT
    if (fCollapsed) {
        SkDebugf("");  // for convenient breakpoints
    }
#endif
}

template<typename TCurve>
bool SkTSpan<TCurve>::linearsIntersect(SkTSpan* span) {
    int result = this->linearIntersects(span->fPart);
    if (result <= 1) {
        return SkToBool(result);
    }
    SkASSERT(span->fIsLinear);
    result = span->linearIntersects(this->fPart);
//    SkASSERT(result <= 1);
    return SkToBool(result);
}

template<typename TCurve>
double SkTSpan<TCurve>::linearT(const SkDPoint& pt) const {
    SkDVector len = fPart[TCurve::kPointLast] - fPart[0];
    return fabs(len.fX) > fabs(len.fY)
            ? (pt.fX - fPart[0].fX) / len.fX
            : (pt.fY - fPart[0].fY) / len.fY;
}

template<typename TCurve>
int SkTSpan<TCurve>::linearIntersects(const TCurve& q2) const {
    // looks like q1 is near-linear
    int start = 0, end = TCurve::kPointLast;  // the outside points are usually the extremes
    if (!fPart.controlsInside()) {
        double dist = 0;  // if there's any question, compute distance to find best outsiders
        for (int outer = 0; outer < TCurve::kPointCount - 1; ++outer) {
            for (int inner = outer + 1; inner < TCurve::kPointCount; ++inner) {
                double test = (fPart[outer] - fPart[inner]).lengthSquared();
                if (dist > test) {
                    continue;
                }
                dist = test;
                start = outer;
                end = inner;
            }
        }
    }
    // see if q2 is on one side of the line formed by the extreme points
    double origX = fPart[start].fX;
    double origY = fPart[start].fY;
    double adj = fPart[end].fX - origX;
    double opp = fPart[end].fY - origY;
    double maxPart = SkTMax(fabs(adj), fabs(opp));
    double sign = 0;  // initialization to shut up warning in release build
    for (int n = 0; n < TCurve::kPointCount; ++n) {
        double dx = q2[n].fY - origY;
        double dy = q2[n].fX - origX;
        double maxVal = SkTMax(maxPart, SkTMax(fabs(dx), fabs(dy)));
        double test = (q2[n].fY - origY) * adj - (q2[n].fX - origX) * opp;
        if (precisely_zero_when_compared_to(test, maxVal)) {
            return 1;
        }
        if (approximately_zero_when_compared_to(test, maxVal)) {
            return 3;
        }
        if (n == 0) {
            sign = test;
            continue;
        }
        if (test * sign < 0) {
            return 1;
        }
    }
    return 0;
}

template<typename TCurve>
bool SkTSpan<TCurve>::onlyEndPointsInCommon(const SkTSpan* opp, bool* start, bool* oppStart,
        bool* ptsInCommon) {
    if (opp->fPart[0] == fPart[0]) {
        *start = *oppStart = true;
    } else if (opp->fPart[0] == fPart[TCurve::kPointLast]) {
        *start = false;
        *oppStart = true;
    } else if (opp->fPart[TCurve::kPointLast] == fPart[0]) {
        *start = true;
        *oppStart = false;
    } else if (opp->fPart[TCurve::kPointLast] == fPart[TCurve::kPointLast]) {
        *start = *oppStart = false;
    } else {
        *ptsInCommon = false;
        return false;
    }
    *ptsInCommon = true;
    const SkDPoint* o1Pts[TCurve::kPointCount - 1], * o2Pts[TCurve::kPointCount - 1];
    int baseIndex = *start ? 0 : TCurve::kPointLast;
    fPart.otherPts(baseIndex, o1Pts);
    opp->fPart.otherPts(*oppStart ? 0 : TCurve::kPointLast, o2Pts);
    const SkDPoint& base = fPart[baseIndex];
    for (int o1 = 0; o1 < (int) SK_ARRAY_COUNT(o1Pts); ++o1) {
        SkDVector v1 = *o1Pts[o1] - base;
        for (int o2 = 0; o2 < (int) SK_ARRAY_COUNT(o2Pts); ++o2) {
            SkDVector v2 = *o2Pts[o2] - base;
            if (v2.dot(v1) >= 0) {
                return false;
            }
        }
    }
    return true;
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSpan<TCurve>::oppT(double t) const {
    SkTSpanBounded<TCurve>* bounded = fBounded;
    while (bounded) {
        SkTSpan* test = bounded->fBounded;
        if (between(test->fStartT, t, test->fEndT)) {
            return test;
        }
        bounded = bounded->fNext;
    }
    return NULL;
}

template<typename TCurve>
bool SkTSpan<TCurve>::removeAllBounded() {
    bool deleteSpan = false;
    SkTSpanBounded<TCurve>* bounded = fBounded;
    while (bounded) {
        SkTSpan* opp = bounded->fBounded;
        deleteSpan |= opp->removeBounded(this);
        bounded = bounded->fNext;
    }
    return deleteSpan;
}

template<typename TCurve>
bool SkTSpan<TCurve>::removeBounded(const SkTSpan* opp) {
    if (fHasPerp) {
        bool foundStart = false;
        bool foundEnd = false;
        SkTSpanBounded<TCurve>* bounded = fBounded;
        while (bounded) {
            SkTSpan* test = bounded->fBounded;
            if (opp != test) {
                foundStart |= between(test->fStartT, fCoinStart.perpT(), test->fEndT);
                foundEnd |= between(test->fStartT, fCoinEnd.perpT(), test->fEndT);
            }
            bounded = bounded->fNext;
        }
        if (!foundStart || !foundEnd) {
            fHasPerp = false;
            fCoinStart.init();
            fCoinEnd.init();
        }
    }
    SkTSpanBounded<TCurve>* bounded = fBounded;
    SkTSpanBounded<TCurve>* prev = NULL;
    while (bounded) {
        SkTSpanBounded<TCurve>* boundedNext = bounded->fNext;
        if (opp == bounded->fBounded) {
            if (prev) {
                prev->fNext = boundedNext;
                return false;
            } else {
                fBounded = boundedNext;
                return fBounded == NULL;
            }
        }
        prev = bounded;
        bounded = boundedNext;
    }
    SkASSERT(0);
    return false;
}

template<typename TCurve>
bool SkTSpan<TCurve>::splitAt(SkTSpan* work, double t, SkChunkAlloc* heap) {
    fStartT = t;
    fEndT = work->fEndT;
    if (fStartT == fEndT) {
        fCollapsed = true;
        return false;
    }
    work->fEndT = t;
    if (work->fStartT == work->fEndT) {
        work->fCollapsed = true;
        return false;
    }
    fPrev = work;
    fNext = work->fNext;
    fIsLinear = work->fIsLinear;
    fIsLine = work->fIsLine;

    work->fNext = this;
    if (fNext) {
        fNext->fPrev = this;
    }
    SkTSpanBounded<TCurve>* bounded = work->fBounded;
    fBounded = NULL;
    while (bounded) {
        this->addBounded(bounded->fBounded, heap);
        bounded = bounded->fNext;
    }
    bounded = fBounded;
    while (bounded) {
        bounded->fBounded->addBounded(this, heap);
        bounded = bounded->fNext;
    }
    return true;
}

template<typename TCurve>
void SkTSpan<TCurve>::validate() const {
#if DEBUG_T_SECT
    SkASSERT(fNext == NULL || fNext != fPrev);
    SkASSERT(fNext == NULL || this == fNext->fPrev);
    SkASSERT(fPrev == NULL || this == fPrev->fNext);
    SkASSERT(fBounds.width() || fBounds.height() || fCollapsed);
    SkASSERT(fBoundsMax == SkTMax(fBounds.width(), fBounds.height()));
    SkASSERT(0 <= fStartT);
    SkASSERT(fEndT <= 1);
    SkASSERT(fStartT <= fEndT);
    SkASSERT(fBounded);
    this->validateBounded();
    if (fHasPerp) {
        if (fCoinStart.isCoincident()) {
            validatePerpT(fCoinStart.perpT());
            validatePerpPt(fCoinStart.perpT(), fCoinStart.perpPt());
        }
        if (fCoinEnd.isCoincident()) {
            validatePerpT(fCoinEnd.perpT());
            validatePerpPt(fCoinEnd.perpT(), fCoinEnd.perpPt());
        }
    }
#endif
}

template<typename TCurve>
void SkTSpan<TCurve>::validateBounded() const {
#if DEBUG_VALIDATE
    const SkTSpanBounded<TCurve>* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* overlap = testBounded->fBounded;
        SkASSERT(!overlap->fDeleted);
        SkASSERT(((this->debugID() ^ overlap->debugID()) & 1) == 1);
        SkASSERT(overlap->findOppSpan(this));
        testBounded = testBounded->fNext;
    }
#endif
}

template<typename TCurve>
void SkTSpan<TCurve>::validatePerpT(double oppT) const {
#if DEBUG_VALIDATE
    const SkTSpanBounded<TCurve>* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* overlap = testBounded->fBounded;
        if (between(overlap->fStartT, oppT, overlap->fEndT)) {
            return;
        }
        testBounded = testBounded->fNext;
    }
    SkASSERT(0);
#endif
}

template<typename TCurve>
void SkTSpan<TCurve>::validatePerpPt(double t, const SkDPoint& pt) const {
#if DEBUG_T_SECT
    PATH_OPS_DEBUG_CODE(SkASSERT(fDebugSect->fOppSect->fCurve.ptAtT(t) == pt));
#endif
}


template<typename TCurve>
SkTSect<TCurve>::SkTSect(const TCurve& c PATH_OPS_DEBUG_T_SECT_PARAMS(int id))
    : fCurve(c)
    , fHeap(sizeof(SkTSpan<TCurve>) * 4)
    , fCoincident(NULL)
    , fDeleted(NULL)
    , fActiveCount(0)
    PATH_OPS_DEBUG_T_SECT_PARAMS(fID(id))
    PATH_OPS_DEBUG_T_SECT_PARAMS(fDebugCount(0))
    PATH_OPS_DEBUG_T_SECT_PARAMS(fDebugAllocatedCount(0))
{
    fHead = addOne();
    fHead->init(c);
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::addOne() {
    SkTSpan<TCurve>* result;
    if (fDeleted) {
        result = fDeleted;
        result->reset();
        fDeleted = result->fNext;
    } else {
        result = SkNEW_PLACEMENT(fHeap.allocThrow(sizeof(SkTSpan<TCurve>)), SkTSpan<TCurve>);
        result->fBounded = NULL;
#if DEBUG_T_SECT
        ++fDebugAllocatedCount;
#endif
    }
    ++fActiveCount; 
    PATH_OPS_DEBUG_T_SECT_CODE(result->fID = fDebugCount++ * 2 + fID);
    PATH_OPS_DEBUG_CODE(result->fDebugSect = this);
    return result;
}

template<typename TCurve>
bool SkTSect<TCurve>::binarySearchCoin(SkTSect* sect2, double tStart, double tStep,
        double* resultT, double* oppT) {
    SkTSpan<TCurve> work;
    double result = work.fStartT = work.fEndT = tStart;
    PATH_OPS_DEBUG_CODE(work.fDebugSect = this);
    SkDPoint last = fCurve.ptAtT(tStart);
    SkDPoint oppPt;
    bool flip = false;
    SkDEBUGCODE(bool down = tStep < 0);
    const TCurve& opp = sect2->fCurve;
    do {
        tStep *= 0.5;
        work.fStartT += tStep;
        if (flip) {
            tStep = -tStep;
            flip = false;
        }
        work.initBounds(fCurve);
        if (work.fCollapsed) {
            return false;
        }
        if (last.approximatelyEqual(work.fPart[0])) {
            break;
        }
        last = work.fPart[0];
        work.fCoinStart.setPerp(fCurve, work.fStartT, last, opp);
        if (work.fCoinStart.isCoincident()) {
#if DEBUG_T_SECT
            work.validatePerpPt(work.fCoinStart.perpT(), work.fCoinStart.perpPt());
#endif
            double oppTTest = work.fCoinStart.perpT();
            if (sect2->fHead->contains(oppTTest)) {
                *oppT = oppTTest;
                oppPt = work.fCoinStart.perpPt();
                SkASSERT(down ? result > work.fStartT : result < work.fStartT);
                result = work.fStartT;
                continue;
            }
        }
        tStep = -tStep;
        flip = true;
    } while (true);
    if (last.approximatelyEqual(fCurve[0])) {
        result = 0;
    } else if (last.approximatelyEqual(fCurve[TCurve::kPointLast])) {
        result = 1;
    }
    if (oppPt.approximatelyEqual(opp[0])) {
        *oppT = 0;
    } else if (oppPt.approximatelyEqual(opp[TCurve::kPointLast])) {
        *oppT = 1;
    }
    *resultT = result;
    return true;
}

// OPTIMIZE ? keep a sorted list of sizes in the form of a doubly-linked list in quad span
//            so that each quad sect has a pointer to the largest, and can update it as spans
//            are split
template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::boundsMax() const {
    SkTSpan<TCurve>* test = fHead;
    SkTSpan<TCurve>* largest = fHead;
    bool lCollapsed = largest->fCollapsed;
    while ((test = test->fNext)) {
        bool tCollapsed = test->fCollapsed;
        if ((lCollapsed && !tCollapsed) || (lCollapsed == tCollapsed &&
                largest->fBoundsMax < test->fBoundsMax)) {
            largest = test;
        }
    }
    return largest;
}

template<typename TCurve>
void SkTSect<TCurve>::coincidentCheck(SkTSect* sect2) {
    SkTSpan<TCurve>* first = fHead;
    SkTSpan<TCurve>* last, * next;
    do {
        int consecutive = this->countConsecutiveSpans(first, &last);
        next = last->fNext;
        if (consecutive < COINCIDENT_SPAN_COUNT) {
            continue;
        }
        this->validate();
        sect2->validate();
        this->computePerpendiculars(sect2, first, last);
        this->validate();
        sect2->validate();
        // check to see if a range of points are on the curve
        SkTSpan<TCurve>* coinStart = first;
        do {
            coinStart = this->extractCoincident(sect2, coinStart, last);
        } while (coinStart && !last->fDeleted);
    } while ((first = next));
}

template<typename TCurve>
bool SkTSect<TCurve>::coincidentHasT(double t) {
    SkTSpan<TCurve>* test = fCoincident;
    while (test) {
        if (between(test->fStartT, t, test->fEndT)) {
            return true;
        }
        test = test->fNext;
    }
    return false;
}

template<typename TCurve>
void SkTSect<TCurve>::computePerpendiculars(SkTSect* sect2, SkTSpan<TCurve>* first,
        SkTSpan<TCurve>* last) {
    const TCurve& opp = sect2->fCurve;
        SkTSpan<TCurve>* work = first;
    SkTSpan<TCurve>* prior = NULL;
    do {
        if (!work->fHasPerp && !work->fCollapsed) {
            if (prior) {
                work->fCoinStart = prior->fCoinEnd;
            } else {
                work->fCoinStart.setPerp(fCurve, work->fStartT, work->fPart[0], opp);
            }
            if (work->fCoinStart.isCoincident()) {
                double perpT = work->fCoinStart.perpT();
                if (sect2->coincidentHasT(perpT)) {
                    work->fCoinStart.clear();
                } else {
                    sect2->addForPerp(work, perpT);
                }
            }
            work->fCoinEnd.setPerp(fCurve, work->fEndT, work->fPart[TCurve::kPointLast], opp);
            if (work->fCoinEnd.isCoincident()) {
                double perpT = work->fCoinEnd.perpT();
                if (sect2->coincidentHasT(perpT)) {
                    work->fCoinEnd.clear();
                } else {
                    sect2->addForPerp(work, perpT);
                }
            }
            work->fHasPerp = true;
        }
        if (work == last) {
            break;
        }
        prior = work;
        work = work->fNext;
        SkASSERT(work);
    } while (true);
}

template<typename TCurve>
int SkTSect<TCurve>::countConsecutiveSpans(SkTSpan<TCurve>* first,
        SkTSpan<TCurve>** lastPtr) const {
    int consecutive = 1;
    SkTSpan<TCurve>* last = first;
    do {
        SkTSpan<TCurve>* next = last->fNext;
        if (!next) {
            break;
        }
        if (next->fStartT > last->fEndT) {
            break;
        }
        ++consecutive;
        last = next;
    } while (true);
    *lastPtr = last;
    return consecutive;
}

template<typename TCurve>
bool SkTSect<TCurve>::debugHasBounded(const SkTSpan<TCurve>* span) const {
    const SkTSpan<TCurve>* test = fHead;
    if (!test) {
        return false;
    }
    do {
        if (test->findOppSpan(span)) {
            return true;
        }
    } while ((test = test->next()));
    return false;
}

template<typename TCurve>
void SkTSect<TCurve>::deleteEmptySpans() {
    SkTSpan<TCurve>* test;
    SkTSpan<TCurve>* next = fHead;
    while ((test = next)) {
        next = test->fNext;
        if (!test->fBounded) {
            this->removeSpan(test);
        }
    }
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::extractCoincident(SkTSect* sect2, SkTSpan<TCurve>* first,
        SkTSpan<TCurve>* last) {
    first = findCoincidentRun(first, &last, sect2);
    if (!first) {
        return NULL;
    }
    // march outwards to find limit of coincidence from here to previous and next spans
    double startT = first->fStartT;
    double oppStartT SK_INIT_TO_AVOID_WARNING;
    double oppEndT SK_INIT_TO_AVOID_WARNING;
    SkTSpan<TCurve>* prev = first->fPrev;
    SkASSERT(first->fCoinStart.isCoincident());
    SkTSpan<TCurve>* oppFirst = first->findOppT(first->fCoinStart.perpT());
    SkASSERT(last->fCoinEnd.isCoincident());
    bool oppMatched = first->fCoinStart.perpT() < first->fCoinEnd.perpT();
    double coinStart;
    SkDEBUGCODE(double coinEnd);
    if (prev && prev->fEndT == startT
            && this->binarySearchCoin(sect2, startT, prev->fStartT - startT, &coinStart,
                                      &oppStartT)
            && prev->fStartT < coinStart && coinStart < startT) {
        oppFirst = prev->findOppT(oppStartT);  // find opp start before splitting prev
        SkASSERT(oppFirst);
        first = this->addSplitAt(prev, coinStart);
        first->markCoincident();
        prev->fCoinEnd.markCoincident();
        if (oppFirst->fStartT < oppStartT && oppStartT < oppFirst->fEndT) {
            SkTSpan<TCurve>* oppHalf = sect2->addSplitAt(oppFirst, oppStartT);
            if (oppMatched) {
                oppFirst->fCoinEnd.markCoincident();
                oppHalf->markCoincident();
                oppFirst = oppHalf;
            } else {
                oppFirst->markCoincident();
                oppHalf->fCoinStart.markCoincident();
            }
        }
    } else {
        SkDEBUGCODE(coinStart = first->fStartT);
        SkDEBUGCODE(oppStartT = oppMatched ? oppFirst->fStartT : oppFirst->fEndT);
    }
    SkTSpan<TCurve>* oppLast;
    SkASSERT(last->fCoinEnd.isCoincident());
    oppLast = last->findOppT(last->fCoinEnd.perpT());
    SkDEBUGCODE(coinEnd = last->fEndT);
    SkDEBUGCODE(oppEndT = oppMatched ? oppLast->fEndT : oppLast->fStartT);
    if (!oppMatched) {
        SkTSwap(oppFirst, oppLast);
        SkTSwap(oppStartT, oppEndT);
    }
    SkASSERT(oppStartT < oppEndT);
    SkASSERT(coinStart == first->fStartT);
    SkASSERT(coinEnd == last->fEndT);
    SkASSERT(oppStartT == oppFirst->fStartT);
    SkASSERT(oppEndT == oppLast->fEndT);
    // reduce coincident runs to single entries
    this->validate();
    sect2->validate();
    bool deleteThisSpan = this->updateBounded(first, last, oppFirst);
    bool deleteSect2Span = sect2->updateBounded(oppFirst, oppLast, first);
    this->removeSpanRange(first, last);
    sect2->removeSpanRange(oppFirst, oppLast);
    first->fEndT = last->fEndT;
    first->resetBounds(this->fCurve);
    first->fCoinStart.setPerp(fCurve, first->fStartT, first->fPart[0], sect2->fCurve);
    first->fCoinEnd.setPerp(fCurve, first->fEndT, first->fPart[TCurve::kPointLast], sect2->fCurve);
    oppStartT = first->fCoinStart.perpT();
    oppEndT = first->fCoinEnd.perpT();
    if (between(0, oppStartT, 1) && between(0, oppEndT, 1)) {
        if (!oppMatched) {
            SkTSwap(oppStartT, oppEndT);
        }
        oppFirst->fStartT = oppStartT;
        oppFirst->fEndT = oppEndT;
        oppFirst->resetBounds(sect2->fCurve);
    }
    this->validateBounded();
    sect2->validateBounded();
    last = first->fNext;
    this->removeCoincident(first, false);
    sect2->removeCoincident(oppFirst, true);
    if (deleteThisSpan) {
        this->deleteEmptySpans();
    }
    if (deleteSect2Span) {
        sect2->deleteEmptySpans();
    }
    this->validate();
    sect2->validate();
    return last && !last->fDeleted ? last : NULL;
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::findCoincidentRun(SkTSpan<TCurve>* first,
        SkTSpan<TCurve>** lastPtr, const SkTSect* sect2) {
    SkTSpan<TCurve>* work = first;
    SkTSpan<TCurve>* lastCandidate = NULL;
    first = NULL;
    // find the first fully coincident span
    do {
        if (work->fCoinStart.isCoincident()) {
            work->validatePerpT(work->fCoinStart.perpT());
            work->validatePerpPt(work->fCoinStart.perpT(), work->fCoinStart.perpPt());
            SkASSERT(work->hasOppT(work->fCoinStart.perpT()));
            if (!work->fCoinEnd.isCoincident()) {
                break;
            }
            lastCandidate = work;
            if (!first) {
                first = work;
            }
        } else {
            lastCandidate = NULL;
            SkASSERT(!first);
        }
        if (work == *lastPtr) {
            return first;
        }
        work = work->fNext;
        SkASSERT(work);
    } while (true);
    if (lastCandidate) {
        *lastPtr = lastCandidate;
    }
    return first;
}

template<typename TCurve>
int SkTSect<TCurve>::intersects(SkTSpan<TCurve>* span, const SkTSect* opp,
        SkTSpan<TCurve>* oppSpan, int* oppResult) const {
    bool spanStart, oppStart;
    int hullResult = span->hullsIntersect(oppSpan, &spanStart, &oppStart);
    if (hullResult >= 0) {
        if (hullResult == 2) {  // hulls have one point in common
            if (!span->fBounded || !span->fBounded->fNext) {
                SkASSERT(!span->fBounded || span->fBounded->fBounded == oppSpan);
                if (spanStart) {
                    span->fEndT = span->fStartT;
                } else {
                    span->fStartT = span->fEndT;
                }
            } else {
                hullResult = 1;
            }
            if (!oppSpan->fBounded || !oppSpan->fBounded->fNext) {
                SkASSERT(!oppSpan->fBounded || oppSpan->fBounded->fBounded == span);
                if (oppStart) {
                    oppSpan->fEndT = oppSpan->fStartT;
                } else {
                    oppSpan->fStartT = oppSpan->fEndT;
                }
                *oppResult = 2;
            } else {
                *oppResult = 1;
            }
        } else {
            *oppResult = 1;
        }
        return hullResult;
    }
    if (span->fIsLine && oppSpan->fIsLine) {
        SkIntersections i;
        int sects = this->linesIntersect(span, opp, oppSpan, &i);
        if (!sects) {
            return -1;
        }
        span->fStartT = span->fEndT = i[0][0];
        oppSpan->fStartT = oppSpan->fEndT = i[1][0];
        return *oppResult = 2;
    }
    if (span->fIsLinear || oppSpan->fIsLinear) {
        return *oppResult = (int) span->linearsIntersect(oppSpan);
    }
    return *oppResult = 1;
}

// while the intersection points are sufficiently far apart:
// construct the tangent lines from the intersections
// find the point where the tangent line intersects the opposite curve
template<typename TCurve>
int SkTSect<TCurve>::linesIntersect(const SkTSpan<TCurve>* span, const SkTSect* opp,
            const SkTSpan<TCurve>* oppSpan, SkIntersections* i) const {
    SkIntersections thisRayI, oppRayI;
    SkDLine thisLine = {{ span->fPart[0], span->fPart[TCurve::kPointLast] }};
    SkDLine oppLine = {{ oppSpan->fPart[0], oppSpan->fPart[TCurve::kPointLast] }};
    int loopCount = 0;
    double bestDistSq = DBL_MAX;
    do {
        if (!thisRayI.intersectRay(opp->fCurve, thisLine)) {
            return 0;
        }
        if (!oppRayI.intersectRay(this->fCurve, oppLine)) {
            return 0;
        }
        // pick the closest pair of points
        double closest = DBL_MAX;
        int closeIndex SK_INIT_TO_AVOID_WARNING;
        int oppCloseIndex SK_INIT_TO_AVOID_WARNING;
        for (int index = 0; index < oppRayI.used(); ++index) {
            if (!roughly_between(span->fStartT, oppRayI[0][index], span->fEndT)) {
                continue;
            }
            for (int oIndex = 0; oIndex < thisRayI.used(); ++oIndex) {
                if (!roughly_between(oppSpan->fStartT, thisRayI[0][oIndex], oppSpan->fEndT)) {
                    continue;
                }
                double distSq = thisRayI.pt(index).distanceSquared(oppRayI.pt(oIndex));
                if (closest > distSq) {
                    closest = distSq;
                    closeIndex = index;
                    oppCloseIndex = oIndex;
                }
            }
        }
        if (closest == DBL_MAX) {
            return 0;
        }
        const SkDPoint& oppIPt = thisRayI.pt(oppCloseIndex);
        const SkDPoint& iPt = oppRayI.pt(closeIndex);
        if (between(span->fStartT, oppRayI[0][closeIndex], span->fEndT)
                && between(oppSpan->fStartT, thisRayI[0][oppCloseIndex], oppSpan->fEndT)
                && oppIPt.approximatelyEqual(iPt)) {
            i->merge(oppRayI, closeIndex, thisRayI, oppCloseIndex);
            return i->used();
        }
        double distSq = oppIPt.distanceSquared(iPt);
        if (bestDistSq < distSq || ++loopCount > 5) {
            break;
        }
        bestDistSq = distSq;
        thisLine[0] = fCurve.ptAtT(oppRayI[0][closeIndex]);
        thisLine[1] = thisLine[0] + fCurve.dxdyAtT(oppRayI[0][closeIndex]);
        oppLine[0] = opp->fCurve.ptAtT(thisRayI[0][oppCloseIndex]);
        oppLine[1] = oppLine[0] + opp->fCurve.dxdyAtT(thisRayI[0][oppCloseIndex]);
    } while (true);
    return false;
}


template<typename TCurve>
void SkTSect<TCurve>::markSpanGone(SkTSpan<TCurve>* span) {
    --fActiveCount;
    span->fNext = fDeleted;
    fDeleted = span;
    SkASSERT(!span->fDeleted);
    span->fDeleted = true;
}

template<typename TCurve>
bool SkTSect<TCurve>::matchedDirection(double t, const SkTSect* sect2, double t2) const {
    SkDVector dxdy = this->fCurve.dxdyAtT(t);
    SkDVector dxdy2 = sect2->fCurve.dxdyAtT(t2);
    return dxdy.dot(dxdy2) >= 0;
}

template<typename TCurve>
void SkTSect<TCurve>::matchedDirCheck(double t, const SkTSect* sect2, double t2,
        bool* calcMatched, bool* oppMatched) const {
    if (*calcMatched) {
        SkASSERT(*oppMatched == this->matchedDirection(t, sect2, t2)); 
    } else {
        *oppMatched = this->matchedDirection(t, sect2, t2);
        *calcMatched = true;
    }
}

template<typename TCurve>
void SkTSect<TCurve>::mergeCoincidence(SkTSect* sect2) {
    double smallLimit = 0;
    do {
        // find the smallest unprocessed span
        SkTSpan<TCurve>* smaller = NULL;
        SkTSpan<TCurve>* test = fCoincident;
        do {
            if (test->fStartT < smallLimit) {
                continue;
            }
            if (smaller && smaller->fEndT < test->fStartT) {
                continue;
            }
            smaller = test;
        } while ((test = test->fNext));
        if (!smaller) {
            return;
        }
        smallLimit = smaller->fEndT;
        // find next larger span
        SkTSpan<TCurve>* prior = NULL;
        SkTSpan<TCurve>* larger = NULL;
        SkTSpan<TCurve>* largerPrior = NULL;
        test = fCoincident;
        do {
            if (test->fStartT < smaller->fEndT) {
                continue;
            }
            SkASSERT(test->fStartT != smaller->fEndT);
            if (larger && larger->fStartT < test->fStartT) {
                continue;
            }
            largerPrior = prior;
            larger = test;
        } while ((prior = test), (test = test->fNext));
        if (!larger) {
            continue;
        }
        // check middle t value to see if it is coincident as well
        double midT = (smaller->fEndT + larger->fStartT) / 2;
        SkDPoint midPt = fCurve.ptAtT(midT);
        SkTCoincident<TCurve> coin;
        coin.setPerp(fCurve, midT, midPt, sect2->fCurve);
        if (coin.isCoincident()) {
            smaller->fEndT = larger->fEndT;
            smaller->fCoinEnd = larger->fCoinEnd;
            if (largerPrior) {
                largerPrior->fNext = larger->fNext;
            } else {
                fCoincident = larger->fNext;
            }
        }
    } while (true);
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::prev(SkTSpan<TCurve>* span) const {
    SkTSpan<TCurve>* result = NULL;
    SkTSpan<TCurve>* test = fHead;
    while (span != test) {
        result = test;
        test = test->fNext;
        SkASSERT(test);
    }
    return result; 
}

template<typename TCurve>
void SkTSect<TCurve>::recoverCollapsed() {
    SkTSpan<TCurve>* deleted = fDeleted;
    while (deleted) {
        SkTSpan<TCurve>* delNext = deleted->fNext;
        if (deleted->fCollapsed) {
            SkTSpan<TCurve>** spanPtr = &fHead;
            while (*spanPtr && (*spanPtr)->fEndT <= deleted->fStartT) {
                spanPtr = &(*spanPtr)->fNext;
            }
            deleted->fNext = *spanPtr;
            *spanPtr = deleted;
        }
        deleted = delNext;
    }
}

template<typename TCurve>
void SkTSect<TCurve>::removeAllBut(const SkTSpan<TCurve>* keep, SkTSpan<TCurve>* span,
        SkTSect* opp) {
    const SkTSpanBounded<TCurve>* testBounded = span->fBounded;
    while (testBounded) {
        SkTSpan<TCurve>* bounded = testBounded->fBounded;
        const SkTSpanBounded<TCurve>* next = testBounded->fNext;
        // may have been deleted when opp did 'remove all but'
        if (bounded != keep && !bounded->fDeleted) {
            SkAssertResult(SkDEBUGCODE(!) span->removeBounded(bounded));
            if (bounded->removeBounded(span)) {
                opp->removeSpan(bounded);
            }
        }
        testBounded = next;
    }
    SkASSERT(!span->fDeleted);
    SkASSERT(span->findOppSpan(keep));
    SkASSERT(keep->findOppSpan(span));
}

template<typename TCurve>
void SkTSect<TCurve>::removeByPerpendicular(SkTSect<TCurve>* opp) {
    SkTSpan<TCurve>* test = fHead;
    SkTSpan<TCurve>* next;
    do {
        next = test->fNext;
        if (test->fCoinStart.perpT() < 0 || test->fCoinEnd.perpT() < 0) {
            continue;
        }
        SkDVector startV = test->fCoinStart.perpPt() - test->fPart[0];
        SkDVector endV = test->fCoinEnd.perpPt() - test->fPart[TCurve::kPointLast];
#if DEBUG_T_SECT
        SkDebugf("%s startV=(%1.9g,%1.9g) endV=(%1.9g,%1.9g) dot=%1.9g\n", __FUNCTION__,
                startV.fX, startV.fY, endV.fX, endV.fY, startV.dot(endV));
#endif
        if (startV.dot(endV) <= 0) {
            continue;
        }
        this->removeSpans(test, opp);
    } while ((test = next));
}

template<typename TCurve>
void SkTSect<TCurve>::removeCoincident(SkTSpan<TCurve>* span, bool isBetween) {
    this->unlinkSpan(span);
    if (isBetween || between(0, span->fCoinStart.perpT(), 1)) {
        --fActiveCount;
        span->fNext = fCoincident;
        fCoincident = span;
    } else {
        this->markSpanGone(span);
    }
}

template<typename TCurve>
void SkTSect<TCurve>::removeSpan(SkTSpan<TCurve>* span) {
    this->unlinkSpan(span);
    this->markSpanGone(span);
}

template<typename TCurve>
void SkTSect<TCurve>::removeSpanRange(SkTSpan<TCurve>* first, SkTSpan<TCurve>* last) {
    if (first == last) {
        return;
    }
    SkTSpan<TCurve>* span = first;
    SkASSERT(span);
    SkTSpan<TCurve>* final = last->fNext;
    SkTSpan<TCurve>* next = span->fNext;
    while ((span = next) && span != final) {
        next = span->fNext;
        this->markSpanGone(span);
    }
    if (final) {
        final->fPrev = first;
    }
    first->fNext = final;
}

template<typename TCurve>
void SkTSect<TCurve>::removeSpans(SkTSpan<TCurve>* span, SkTSect<TCurve>* opp) {
    SkTSpanBounded<TCurve>* bounded = span->fBounded;
    while (bounded) {
        SkTSpan<TCurve>* spanBounded = bounded->fBounded;
        SkTSpanBounded<TCurve>* next = bounded->fNext;
        if (span->removeBounded(spanBounded)) {  // shuffles last into position 0
            this->removeSpan(span);
        }
        if (spanBounded->removeBounded(span)) {
            opp->removeSpan(spanBounded);
        }
        SkASSERT(!span->fDeleted || !opp->debugHasBounded(span));
        bounded = next;
    }
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::spanAtT(double t, SkTSpan<TCurve>** priorSpan) {
    SkTSpan<TCurve>* test = fHead;
    SkTSpan<TCurve>* prev = NULL;
    while (test && test->fEndT < t) {
        prev = test;
        test = test->fNext;
    }
    *priorSpan = prev;
    return test && test->fStartT <= t ? test : NULL;
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSect<TCurve>::tail() {
    SkTSpan<TCurve>* result = fHead;
    SkTSpan<TCurve>* next = fHead;
    while ((next = next->fNext)) {
        if (next->fEndT > result->fEndT) {
            result = next;
        }
    }
    return result;
}

/* Each span has a range of opposite spans it intersects. After the span is split in two,
    adjust the range to its new size */
template<typename TCurve>
void SkTSect<TCurve>::trim(SkTSpan<TCurve>* span, SkTSect* opp) {
    span->initBounds(fCurve);
    const SkTSpanBounded<TCurve>* testBounded = span->fBounded;
    while (testBounded) {
        SkTSpan<TCurve>* test = testBounded->fBounded;
        const SkTSpanBounded<TCurve>* next = testBounded->fNext;
        int oppSects, sects = this->intersects(span, opp, test, &oppSects);
        if (sects >= 1) {
            if (oppSects == 2) {
                test->initBounds(opp->fCurve);
                opp->removeAllBut(span, test, this);
            }
            if (sects == 2) {
                span->initBounds(fCurve);
                this->removeAllBut(test, span, opp);
                return;
            }
        } else {
            if (span->removeBounded(test)) {
                this->removeSpan(span);
            }
            if (test->removeBounded(span)) {
                opp->removeSpan(test);
            }
        }
        testBounded = next;
    }
}

template<typename TCurve>
void SkTSect<TCurve>::unlinkSpan(SkTSpan<TCurve>* span) {
    SkTSpan<TCurve>* prev = span->fPrev;
    SkTSpan<TCurve>* next = span->fNext;
    if (prev) {
        prev->fNext = next;
        if (next) {
            next->fPrev = prev;
        }
    } else {
        fHead = next;
        if (next) {
            next->fPrev = NULL;
        }
    }
}

template<typename TCurve>
bool SkTSect<TCurve>::updateBounded(SkTSpan<TCurve>* first, SkTSpan<TCurve>* last,
        SkTSpan<TCurve>* oppFirst) {
    SkTSpan<TCurve>* test = first;
    const SkTSpan<TCurve>* final = last->next();
    bool deleteSpan = false;
    do {
        deleteSpan |= test->removeAllBounded();
    } while ((test = test->fNext) != final);
    first->fBounded = NULL;
    first->addBounded(oppFirst, &fHeap);
    // cannot call validate until remove span range is called
    return deleteSpan;
}


template<typename TCurve>
void SkTSect<TCurve>::validate() const {
#if DEBUG_T_SECT
    int count = 0;
    if (fHead) {
        const SkTSpan<TCurve>* span = fHead;
        SkASSERT(!span->fPrev);
        SkDEBUGCODE(double last = 0);
        do {
            span->validate();
            SkASSERT(span->fStartT >= last);
            SkDEBUGCODE(last = span->fEndT);
            ++count;
        } while ((span = span->fNext) != NULL);
    }
    SkASSERT(count == fActiveCount);
    SkASSERT(fActiveCount <= fDebugAllocatedCount);
    int deletedCount = 0;
    const SkTSpan<TCurve>* deleted = fDeleted;
    while (deleted) {
        ++deletedCount;
        deleted = deleted->fNext;
    }
    const SkTSpan<TCurve>* coincident = fCoincident;
    while (coincident) {
        ++deletedCount;
        coincident = coincident->fNext;
    }
    SkASSERT(fActiveCount + deletedCount == fDebugAllocatedCount);
#endif
}

template<typename TCurve>
void SkTSect<TCurve>::validateBounded() const {
#if DEBUG_T_SECT
    if (!fHead) {
        return;
    }
    const SkTSpan<TCurve>* span = fHead;
    do {
        span->validateBounded();
    } while ((span = span->fNext) != NULL);
#endif
}

template<typename TCurve>
int SkTSect<TCurve>::EndsEqual(const SkTSect* sect1, const SkTSect* sect2,
        SkIntersections* intersections) {
    int zeroOneSet = 0;
    if (sect1->fCurve[0] == sect2->fCurve[0]) {
        zeroOneSet |= kZeroS1Set | kZeroS2Set;
        intersections->insert(0, 0, sect1->fCurve[0]);
    }
    if (sect1->fCurve[0] == sect2->fCurve[TCurve::kPointLast]) {
        zeroOneSet |= kZeroS1Set | kOneS2Set;
        intersections->insert(0, 1, sect1->fCurve[0]);
    }
    if (sect1->fCurve[TCurve::kPointLast] == sect2->fCurve[0]) {
        zeroOneSet |= kOneS1Set | kZeroS2Set;
        intersections->insert(1, 0, sect1->fCurve[TCurve::kPointLast]);
    }
    if (sect1->fCurve[TCurve::kPointLast] == sect2->fCurve[TCurve::kPointLast]) {
        zeroOneSet |= kOneS1Set | kOneS2Set;
            intersections->insert(1, 1, sect1->fCurve[TCurve::kPointLast]);
    }
    // check for zero
    if (!(zeroOneSet & (kZeroS1Set | kZeroS2Set))
            && sect1->fCurve[0].approximatelyEqual(sect2->fCurve[0])) {
        zeroOneSet |= kZeroS1Set | kZeroS2Set;
        intersections->insertNear(0, 0, sect1->fCurve[0], sect2->fCurve[0]);
    }
    if (!(zeroOneSet & (kZeroS1Set | kOneS2Set))
            && sect1->fCurve[0].approximatelyEqual(sect2->fCurve[TCurve::kPointLast])) {
        zeroOneSet |= kZeroS1Set | kOneS2Set;
        intersections->insertNear(0, 1, sect1->fCurve[0], sect2->fCurve[TCurve::kPointLast]);
    }
    // check for one
    if (!(zeroOneSet & (kOneS1Set | kZeroS2Set))
            && sect1->fCurve[TCurve::kPointLast].approximatelyEqual(sect2->fCurve[0])) {
        zeroOneSet |= kOneS1Set | kZeroS2Set;
        intersections->insertNear(1, 0, sect1->fCurve[TCurve::kPointLast], sect2->fCurve[0]);
    }
    if (!(zeroOneSet & (kOneS1Set | kOneS2Set))
            && sect1->fCurve[TCurve::kPointLast].approximatelyEqual(sect2->fCurve[
            TCurve::kPointLast])) {
        zeroOneSet |= kOneS1Set | kOneS2Set;
        intersections->insertNear(1, 1, sect1->fCurve[TCurve::kPointLast],
                sect2->fCurve[TCurve::kPointLast]);
    }
    return zeroOneSet;
}

template<typename TCurve>
struct SkClosestRecord {
    bool operator<(const SkClosestRecord& rh) const {
        return fClosest < rh.fClosest;
    }

    void addIntersection(SkIntersections* intersections) const {
        double r1t = fC1Index ? fC1Span->endT() : fC1Span->startT();
        double r2t = fC2Index ? fC2Span->endT() : fC2Span->startT();
        intersections->insert(r1t, r2t, fC1Span->part()[fC1Index]);
    }

    void findEnd(const SkTSpan<TCurve>* span1, const SkTSpan<TCurve>* span2,
            int c1Index, int c2Index) {
        const TCurve& c1 = span1->part();
        const TCurve& c2 = span2->part();
        if (!c1[c1Index].approximatelyEqual(c2[c2Index])) {
            return;
        }
        double dist = c1[c1Index].distanceSquared(c2[c2Index]);
        if (fClosest < dist) {
            return;
        }
        fC1Span = span1;
        fC2Span = span2;
        fC1StartT = span1->startT();
        fC1EndT = span1->endT();
        fC2StartT = span2->startT();
        fC2EndT = span2->endT();
        fC1Index = c1Index;
        fC2Index = c2Index;
        fClosest = dist;
    }

    bool matesWith(const SkClosestRecord& mate) const {
        SkASSERT(fC1Span == mate.fC1Span || fC1Span->endT() <= mate.fC1Span->startT()
                || mate.fC1Span->endT() <= fC1Span->startT());
        SkASSERT(fC2Span == mate.fC2Span || fC2Span->endT() <= mate.fC2Span->startT()
                || mate.fC2Span->endT() <= fC2Span->startT());
        return fC1Span == mate.fC1Span || fC1Span->endT() == mate.fC1Span->startT()
                || fC1Span->startT() == mate.fC1Span->endT()
                || fC2Span == mate.fC2Span
                || fC2Span->endT() == mate.fC2Span->startT()
                || fC2Span->startT() == mate.fC2Span->endT();
    }

    void merge(const SkClosestRecord& mate) {
        fC1Span = mate.fC1Span;
        fC2Span = mate.fC2Span;
        fClosest = mate.fClosest;
        fC1Index = mate.fC1Index;
        fC2Index = mate.fC2Index;
    }
    
    void reset() {
        fClosest = FLT_MAX;
        SkDEBUGCODE(fC1Span = fC2Span = NULL);
        SkDEBUGCODE(fC1Index = fC2Index = -1);
    }

    void update(const SkClosestRecord& mate) {
        fC1StartT = SkTMin(fC1StartT, mate.fC1StartT);
        fC1EndT = SkTMax(fC1EndT, mate.fC1EndT);
        fC2StartT = SkTMin(fC2StartT, mate.fC2StartT);
        fC2EndT = SkTMax(fC2EndT, mate.fC2EndT);
    }

    const SkTSpan<TCurve>* fC1Span;
    const SkTSpan<TCurve>* fC2Span;
    double fC1StartT;
    double fC1EndT;
    double fC2StartT;
    double fC2EndT;
    double fClosest;
    int fC1Index;
    int fC2Index;
};

template<typename TCurve>
struct SkClosestSect {
    SkClosestSect()
        : fUsed(0) {
        fClosest.push_back().reset();
    }

    bool find(const SkTSpan<TCurve>* span1, const SkTSpan<TCurve>* span2) {
        SkClosestRecord<TCurve>* record = &fClosest[fUsed];
        record->findEnd(span1, span2, 0, 0);
        record->findEnd(span1, span2, 0, TCurve::kPointLast);
        record->findEnd(span1, span2, TCurve::kPointLast, 0);
        record->findEnd(span1, span2, TCurve::kPointLast, TCurve::kPointLast);
        if (record->fClosest == FLT_MAX) {
            return false;
        }
        for (int index = 0; index < fUsed; ++index) {
            SkClosestRecord<TCurve>* test = &fClosest[index];
            if (test->matesWith(*record)) {
                if (test->fClosest > record->fClosest) {
                    test->merge(*record);
                }
                test->update(*record);
                record->reset();
                return false;
            }
        }
        ++fUsed;
        fClosest.push_back().reset();
        return true;
    }

    void finish(SkIntersections* intersections) const {
        SkSTArray<TCurve::kMaxIntersections * 2, const SkClosestRecord<TCurve>*, true> closestPtrs;
        for (int index = 0; index < fUsed; ++index) {
            closestPtrs.push_back(&fClosest[index]);
        }
        SkTQSort<const SkClosestRecord<TCurve> >(closestPtrs.begin(), closestPtrs.end() - 1);
        for (int index = 0; index < fUsed; ++index) {
            const SkClosestRecord<TCurve>* test = closestPtrs[index];
            test->addIntersection(intersections);
        }
    }

    // this is oversized so that an extra records can merge into final one
    SkSTArray<TCurve::kMaxIntersections * 2, SkClosestRecord<TCurve>, true> fClosest;
    int fUsed;
};

// returns true if the rect is too small to consider
template<typename TCurve>
void SkTSect<TCurve>::BinarySearch(SkTSect* sect1, SkTSect* sect2, SkIntersections* intersections) {
#if DEBUG_T_SECT_DUMP > 1
    gDumpTSectNum = 0;
#endif
    PATH_OPS_DEBUG_CODE(sect1->fOppSect = sect2);
    PATH_OPS_DEBUG_CODE(sect2->fOppSect = sect1);
    intersections->reset();
    intersections->setMax(TCurve::kMaxIntersections * 2);  // give extra for slop
    SkTSpan<TCurve>* span1 = sect1->fHead;
    SkTSpan<TCurve>* span2 = sect2->fHead;
    int oppSect, sect = sect1->intersects(span1, sect2, span2, &oppSect);
//    SkASSERT(between(0, sect, 2));
    if (!sect) {
        return;
    }
    if (sect == 2 && oppSect == 2) {
        (void) EndsEqual(sect1, sect2, intersections);
        return;
    }
    span1->addBounded(span2, &sect1->fHeap);
    span2->addBounded(span1, &sect2->fHeap);
    do {
        // find the largest bounds
        SkTSpan<TCurve>* largest1 = sect1->boundsMax();
        if (!largest1) {
            break;
        }
        SkTSpan<TCurve>* largest2 = sect2->boundsMax();
        bool split1 = !largest2 || (largest1 && (largest1->fBoundsMax > largest2->fBoundsMax
            || (!largest1->fCollapsed && largest2->fCollapsed)));
        // split it
        SkTSpan<TCurve>* half1 = split1 ? largest1 : largest2;
        SkASSERT(half1);
        if (half1->fCollapsed) {
            break;
        }
        SkTSect* splitSect = split1 ? sect1 : sect2;
        // trim parts that don't intersect the opposite
        SkTSpan<TCurve>* half2 = splitSect->addOne();
        SkTSect* unsplitSect = split1 ? sect2 : sect1;
        if (!half2->split(half1, &splitSect->fHeap)) {
            break;
        }
        splitSect->trim(half1, unsplitSect);
        splitSect->trim(half2, unsplitSect);
        sect1->validate();
        sect2->validate();
        // if there are 9 or more continuous spans on both sects, suspect coincidence
        if (sect1->fActiveCount >= COINCIDENT_SPAN_COUNT
                && sect2->fActiveCount >= COINCIDENT_SPAN_COUNT) {
            sect1->coincidentCheck(sect2);
            sect1->validate();
            sect2->validate();
        }
        if (sect1->fActiveCount >= COINCIDENT_SPAN_COUNT
                && sect2->fActiveCount >= COINCIDENT_SPAN_COUNT) {
            sect1->computePerpendiculars(sect2, sect1->fHead, sect1->tail());
            sect2->computePerpendiculars(sect1, sect2->fHead, sect2->tail());
            sect1->removeByPerpendicular(sect2);
            sect1->validate();
            sect2->validate();
        }
#if DEBUG_T_SECT_DUMP
        sect1->dumpBoth(sect2);
#endif
        if (!sect1->fHead || !sect2->fHead) {
            break;
        }
    } while (true);
    SkTSpan<TCurve>* coincident = sect1->fCoincident;
    if (coincident) {
        // if there is more than one coincident span, check loosely to see if they should be joined
        if (coincident->fNext) {
            sect1->mergeCoincidence(sect2);
            coincident = sect1->fCoincident;
        }
        SkASSERT(sect2->fCoincident);  // courtesy check : coincidence only looks at sect 1
        do {
            SkASSERT(coincident->fCoinStart.isCoincident());
            SkASSERT(coincident->fCoinEnd.isCoincident());
            int index = intersections->insertCoincident(coincident->fStartT,
                    coincident->fCoinStart.perpT(), coincident->fPart[0]);
            if ((intersections->insertCoincident(coincident->fEndT,
                    coincident->fCoinEnd.perpT(),
                    coincident->fPart[TCurve::kPointLast]) < 0) && index >= 0) {
                intersections->clearCoincidence(index);
            }
        } while ((coincident = coincident->fNext));
    }
    if (!sect1->fHead || !sect2->fHead) {
        return;
    }
    int zeroOneSet = EndsEqual(sect1, sect2, intersections);
    sect1->recoverCollapsed();
    sect2->recoverCollapsed();
    SkTSpan<TCurve>* result1 = sect1->fHead;
    // check heads and tails for zero and ones and insert them if we haven't already done so
    const SkTSpan<TCurve>* head1 = result1;
    if (!(zeroOneSet & kZeroS1Set) && approximately_less_than_zero(head1->fStartT)) {
        const SkDPoint& start1 = sect1->fCurve[0];
        if (head1->isBounded()) {
            double t = head1->closestBoundedT(start1);
            if (sect2->fCurve.ptAtT(t).approximatelyEqual(start1)) {
                intersections->insert(0, t, start1);
            }
        }
    }
    const SkTSpan<TCurve>* head2 = sect2->fHead;
    if (!(zeroOneSet & kZeroS2Set) && approximately_less_than_zero(head2->fStartT)) {
        const SkDPoint& start2 = sect2->fCurve[0];
        if (head2->isBounded()) {
            double t = head2->closestBoundedT(start2);
            if (sect1->fCurve.ptAtT(t).approximatelyEqual(start2)) {
                intersections->insert(t, 0, start2);
            }
        }
    }
    const SkTSpan<TCurve>* tail1 = sect1->tail();
    if (!(zeroOneSet & kOneS1Set) && approximately_greater_than_one(tail1->fEndT)) {
        const SkDPoint& end1 = sect1->fCurve[TCurve::kPointLast];
        if (tail1->isBounded()) {
            double t = tail1->closestBoundedT(end1);
            if (sect2->fCurve.ptAtT(t).approximatelyEqual(end1)) {
                intersections->insert(1, t, end1);
            }
        }
    }
    const SkTSpan<TCurve>* tail2 = sect2->tail();
    if (!(zeroOneSet & kOneS2Set) && approximately_greater_than_one(tail2->fEndT)) {
        const SkDPoint& end2 = sect2->fCurve[TCurve::kPointLast];
        if (tail2->isBounded()) {
            double t = tail2->closestBoundedT(end2);
            if (sect1->fCurve.ptAtT(t).approximatelyEqual(end2)) {
                intersections->insert(t, 1, end2);
            }
        }
    }
    SkClosestSect<TCurve> closest;
    do {
        while (result1 && result1->fCoinStart.isCoincident() && result1->fCoinEnd.isCoincident()) {
            result1 = result1->fNext;
        }
        if (!result1) {
            break;
        }
        SkTSpan<TCurve>* result2 = sect2->fHead;
        bool found = false;
        while (result2) {
            found |= closest.find(result1, result2);
            result2 = result2->fNext;
        }
    } while ((result1 = result1->fNext));
    closest.finish(intersections);
    // if there is more than one intersection and it isn't already coincident, check
    int last = intersections->used() - 1;
    for (int index = 0; index < last; ) {
        if (intersections->isCoincident(index) && intersections->isCoincident(index + 1)) {
            ++index;
            continue;
        }
        double midT = ((*intersections)[0][index] + (*intersections)[0][index + 1]) / 2;
        SkDPoint midPt = sect1->fCurve.ptAtT(midT);
        // intersect perpendicular with opposite curve
        SkTCoincident<TCurve> perp;
        perp.setPerp(sect1->fCurve, midT, midPt, sect2->fCurve);
        if (!perp.isCoincident()) {
            ++index;
            continue;
        }
        if (intersections->isCoincident(index)) {
            intersections->removeOne(index);
            --last;
        } else if (intersections->isCoincident(index + 1)) {
            intersections->removeOne(index + 1);
            --last;
        } else {
            intersections->setCoincident(index++);
        }
        intersections->setCoincident(index);
    }
    SkASSERT(intersections->used() <= TCurve::kMaxIntersections);
}
