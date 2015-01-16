/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChunkAlloc.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsQuad.h"
#include "SkIntersections.h"
#include "SkTArray.h"

/* TCurve is either SkDQuadratic or SkDCubic */
template<typename TCurve>
class SkTCoincident {
public:
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

/* Curve is either TCurve or SkDCubic */
template<typename TCurve>
class SkTSpan {
public:
    void init(const TCurve& );
    void initBounds(const TCurve& );

    double closestBoundedT(const SkDPoint& pt) const;

    bool contains(double t) const {
        return !! const_cast<SkTSpan*>(this)->innerFind(t);
    }

    bool contains(const SkTSpan* span) const;

    double endT() const {
        return fEndT;
    }

    SkTSpan* find(double t) {
        SkTSpan* result = innerFind(t);
        SkASSERT(result);
        return result;
    }

    bool intersects(const SkTSpan* span, bool* check);

    const SkTSpan* next() const {
        return fNext;
    }

    const TCurve& part() const {
        return fPart;
    }

    void reset() {
        fBounded.reset();
    }

    bool split(SkTSpan* work) {
        return splitAt(work, (work->fStartT + work->fEndT) * 0.5);
    }

    bool splitAt(SkTSpan* work, double t);

    double startT() const {
        return fStartT;
    }

    bool tightBoundsIntersects(const SkTSpan* span) const;

    // implementation is for testing only
    void dump() const {
        dump(NULL);
    }

private:
    SkTSpan* innerFind(double t);
    bool linearIntersects(const TCurve& ) const;

    // implementation is for testing only
#if DEBUG_T_SECT
    int debugID(const SkTSect<TCurve>* ) const { return fDebugID; }
#else
    int debugID(const SkTSect<TCurve>* ) const;
#endif
    void dump(const SkTSect<TCurve>* ) const;
    void dumpID(const SkTSect<TCurve>* ) const;

#if DEBUG_T_SECT
    void validate() const;
#endif

    TCurve fPart;
    SkTCoincident<TCurve> fCoinStart;
    SkTCoincident<TCurve> fCoinEnd;
    SkSTArray<4, SkTSpan*, true> fBounded;
    SkTSpan* fPrev;
    SkTSpan* fNext;
    SkDRect fBounds;
    double fStartT;
    double fEndT;
    double fBoundsMax;
    bool fCollapsed;
    bool fHasPerp;
    bool fIsLinear;
#if DEBUG_T_SECT
    int fDebugID;
    bool fDebugDeleted;
#endif
    friend class SkTSect<TCurve>;
};

template<typename TCurve>
class SkTSect {
public:
    SkTSect(const TCurve& c  PATH_OPS_DEBUG_PARAMS(int id));
    static void BinarySearch(SkTSect* sect1, SkTSect* sect2, SkIntersections* intersections);

    // for testing only
    void dump() const;
    void dumpBoth(const SkTSect& opp) const;
    void dumpBoth(const SkTSect* opp) const;
    void dumpCurves() const;

private:
    enum {
        kZeroS1Set = 1,
        kOneS1Set = 2,
        kZeroS2Set = 4,
        kOneS2Set = 8
    };

    SkTSpan<TCurve>* addOne();
    bool binarySearchCoin(const SkTSect& , double tStart, double tStep, double* t, double* oppT);
    SkTSpan<TCurve>* boundsMax() const;
    void coincidentCheck(SkTSect* sect2);
    static int EndsEqual(const SkTSect* sect1, const SkTSect* sect2, SkIntersections* );
    bool intersects(SkTSpan<TCurve>* span, const SkTSect* opp,
            const SkTSpan<TCurve>* oppSpan) const;
    void onCurveCheck(SkTSect* sect2, SkTSpan<TCurve>* first, SkTSpan<TCurve>* last);
    void recoverCollapsed();
    void removeSpan(SkTSpan<TCurve>* span);
    void removeOne(const SkTSpan<TCurve>* test, SkTSpan<TCurve>* span);
    void removeSpans(SkTSpan<TCurve>* span, SkTSect* opp);
    void setPerp(const TCurve& opp, SkTSpan<TCurve>* first, SkTSpan<TCurve>* last);
    const SkTSpan<TCurve>* tail() const;
    void trim(SkTSpan<TCurve>* span, SkTSect* opp);

#if DEBUG_T_SECT
    int debugID() const { return fDebugID; }
    void validate() const;
#else
    int debugID() const { return 0; }
#endif
    const TCurve& fCurve;
    SkChunkAlloc fHeap;
    SkTSpan<TCurve>* fHead;
    SkTSpan<TCurve>* fDeleted;
    int fActiveCount;
#if DEBUG_T_SECT
    int fDebugID;
    int fDebugCount;
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
    if (used == 0) {
        fPerpT = -1;
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
    fCoincident = cPt.approximatelyEqual(fPerpPt);
#if DEBUG_T_SECT
    if (fCoincident) {
        SkDebugf("");  // allow setting breakpoint
    }
#endif
}

template<typename TCurve>
void SkTSpan<TCurve>::init(const TCurve& c) {
    fPrev = fNext = NULL;
    fIsLinear = false;
    fStartT = 0;
    fEndT = 1;
    initBounds(c);
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
#if DEBUG_T_SECT
    fDebugDeleted = false;
    if (fCollapsed) {
        SkDebugf("");  // for convenient breakpoints
    }
#endif
}

template<typename TCurve>
double SkTSpan<TCurve>::closestBoundedT(const SkDPoint& pt) const {
    int count = fBounded.count();
    double result = -1;
    double closest = FLT_MAX;
    for (int index = 0; index < count; ++index) {
        const SkTSpan* test = fBounded[index];
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
    }
    SkASSERT(between(0, result, 1));
    return result;
}

template<typename TCurve>
bool SkTSpan<TCurve>::contains(const SkTSpan* span) const {
    int count = fBounded.count();
    for (int index = 0; index < count; ++index) {
        const SkTSpan* test = fBounded[index];
        if (span == test) {
            return true;
        }
    }
    return false;
}

template<typename TCurve>
SkTSpan<TCurve>* SkTSpan<TCurve>::innerFind(double t) {
    SkTSpan* work = this;
    do {
        if (between(work->fStartT, t, work->fEndT)) {
            return work;
        }
    } while ((work = work->fNext));
    return NULL;
}

// OPTIMIZE ? If at_most_end_pts_in_common detects that one quad is near linear,
// use line intersection to guess a better split than 0.5
// OPTIMIZE Once at_most_end_pts_in_common detects linear, mark span so all future splits are linear
template<typename TCurve>
bool SkTSpan<TCurve>::intersects(const SkTSpan* span, bool* check) {
    if (!fBounds.intersects(span->fBounds)) {
        *check = false;  // no need to check to see if the bounds have end points in common
        return false;
    }
    if (!fIsLinear && fPart.hullIntersects(span->fPart, check)) {
        if (!*check) {
            return true;
        }
        fIsLinear = true;
    }
    if (fIsLinear) {
        *check = false;
        return linearIntersects(span->fPart);
    }
    return *check; 
}

template<typename TCurve>
bool SkTSpan<TCurve>::linearIntersects(const TCurve& q2) const {
    // looks like q1 is near-linear
    int start = 0, end = TCurve::kPointCount - 1;  // the outside points are usually the extremes
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
    double sign;
    for (int n = 0; n < TCurve::kPointCount; ++n) {
        double test = (q2[n].fY - origY) * adj - (q2[n].fX - origX) * opp;
        if (precisely_zero(test)) {
            return true;
        }
        if (n == 0) {
            sign = test;
            continue;
        }
        if (test * sign < 0) {
            return true;
        }
    }
    return false;
}

template<typename TCurve>
bool SkTSpan<TCurve>::splitAt(SkTSpan* work, double t) {
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
    work->fNext = this;
    if (fNext) {
        fNext->fPrev = this;
    }
    fBounded = work->fBounded;
    int count = fBounded.count();
    for (int index = 0; index < count; ++index) {
        fBounded[index]->fBounded.push_back() = this;
    }
    return true;
}

template<typename TCurve>
bool SkTSpan<TCurve>::tightBoundsIntersects(const SkTSpan* span) const {
    // skew all to an axis
    SkDVector v2_0 = fPart[TCurve::kPointLast] - fPart[0];
    bool skewToXAxis = fabs(v2_0.fX) > fabs(v2_0.fY);
    double ratio = skewToXAxis ? v2_0.fY / v2_0.fX : v2_0.fX / v2_0.fY;
    TCurve r1 = fPart;
    if (skewToXAxis) {
        r1[1].fY -= (fPart[1].fX - r1[0].fX) * ratio;
        if (TCurve::IsCubic()) {
            r1[2].fY -= (fPart[2].fX - r1[0].fX) * ratio;
            r1[3].fY = r1[0].fY;
        } else {
            r1[2].fY = r1[0].fY;
        }
    } else {
        r1[1].fX -= (fPart[1].fY - r1[0].fY) * ratio;
        if (TCurve::IsCubic()) {
            r1[2].fX -= (fPart[2].fY - r1[0].fY) * ratio;
            r1[3].fX = r1[0].fX;
        } else {
            r1[2].fX = r1[0].fX;
        }
    }
    // compute the tight skewed bounds
    SkDRect bounds;
    bounds.setBounds(r1);
    // see if opposite ends are within range of tight skewed bounds
    TCurve r2 = span->fPart;
    for (int i = 0; i < TCurve::kPointCount; i += 2) {
        if (skewToXAxis) {
            r2[i].fY -= (r2[i].fX - r1[0].fX) * ratio;
            if (between(bounds.fTop, r2[i].fY, bounds.fBottom)) {
                return true;
            }
        } else {
            r2[i].fX -= (r2[i].fY - r1[0].fY) * ratio;
            if (between(bounds.fLeft, r2[i].fX, bounds.fRight)) {
                return true;
            }
        }
    }
    // see if opposite ends are on either side of tight skewed bounds
    if ((skewToXAxis ? (r2[0].fY - r1[0].fY) * (r2[TCurve::kPointLast].fY - r1[0].fY)
                        : (r2[0].fX - r1[0].fX) * (r2[TCurve::kPointLast].fX - r1[0].fX)) < 0) {
        return true;
    }
    // compute opposite tight skewed bounds
    if (skewToXAxis) {
        r2[1].fY -= (r2[1].fX - r1[0].fX) * ratio;
        if (TCurve::IsCubic()) {
            r2[2].fY -= (r2[2].fX - r1[0].fX) * ratio;
        }
    } else {
        r2[1].fX -= (r2[1].fY - r1[0].fY) * ratio;
        if (TCurve::IsCubic()) {
            r2[2].fX -= (r2[2].fY - r1[0].fY) * ratio;
        }
    }
    SkDRect sBounds;
    sBounds.setBounds(r2);
    // see if tight bounds overlap
    if (skewToXAxis) {
        return bounds.fTop <= sBounds.fBottom && sBounds.fTop <= bounds.fBottom;  
    } else {
        return bounds.fLeft <= sBounds.fRight && sBounds.fLeft <= bounds.fRight;  
    }
}

#if DEBUG_T_SECT
template<typename TCurve>
void SkTSpan<TCurve>::validate() const {
    SkASSERT(fNext == NULL || fNext != fPrev);
    SkASSERT(fNext == NULL || this == fNext->fPrev);
    SkASSERT(fBounds.width() || fBounds.height());
    SkASSERT(fBoundsMax == SkTMax(fBounds.width(), fBounds.height()));
    SkASSERT(0 <= fStartT);
    SkASSERT(fEndT <= 1);
    SkASSERT(fStartT < fEndT);
    SkASSERT(fBounded.count() > 0);
    for (int index = 0; index < fBounded.count(); ++index) {
        const SkTSpan* overlap = fBounded[index];
        SkASSERT(((fDebugID ^ overlap->fDebugID) & 1) == 1);
        SkASSERT(overlap->contains(this));
    }
}
#endif

template<typename TCurve>
SkTSect<TCurve>::SkTSect(const TCurve& c PATH_OPS_DEBUG_PARAMS(int id))
    : fCurve(c)
    , fHeap(sizeof(SkTSpan<TCurve>) * 4)
    , fDeleted(NULL)
    , fActiveCount(0)
    PATH_OPS_DEBUG_PARAMS(fDebugID(id))
    PATH_OPS_DEBUG_PARAMS(fDebugCount(0))
    PATH_OPS_DEBUG_PARAMS(fDebugAllocatedCount(0))
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
#if DEBUG_T_SECT
        ++fDebugAllocatedCount;
#endif
    }
    ++fActiveCount; 
#if DEBUG_T_SECT
    result->fDebugID = fDebugCount++ * 2 + fDebugID;
#endif
    return result;
}

template<typename TCurve>
bool SkTSect<TCurve>::binarySearchCoin(const SkTSect& sect2, double tStart, double tStep,
        double* resultT, double* oppT) {
    SkTSpan<TCurve> work;
    double result = work.fStartT = work.fEndT = tStart;
    SkDPoint last = fCurve.ptAtT(tStart);
    SkDPoint oppPt;
    bool flip = false;
    SkDEBUGCODE(bool down = tStep < 0);
    const TCurve& opp = sect2.fCurve;
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
            double oppTTest = work.fCoinStart.perpT();
            if (sect2.fHead->contains(oppTTest)) {
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
    bool largestCoin = largest->fCoinStart.isCoincident() && largest->fCoinEnd.isCoincident();
    while ((test = test->fNext)) {
        bool testCoin = test->fCoinStart.isCoincident() || test->fCoinEnd.isCoincident();
        if ((largestCoin && !testCoin) || (largestCoin == testCoin
                && (largest->fBoundsMax < test->fBoundsMax
                || (largest->fCollapsed && !test->fCollapsed)))) {
            largest = test;
            largestCoin = testCoin;
        }
    }
    return largestCoin ? NULL : largest;
}

template<typename TCurve>
void SkTSect<TCurve>::coincidentCheck(SkTSect* sect2) {
    SkTSpan<TCurve>* first = fHead;
    SkTSpan<TCurve>* next;
    do {
        int consecutive = 1;
        SkTSpan<TCurve>* last = first;
        do {
            next = last->fNext;
            if (!next) {
                break;
            }
            if (next->fStartT > last->fEndT) {
                break;
            }
            ++consecutive;
            last = next;
        } while (true);
        if (consecutive < COINCIDENT_SPAN_COUNT) {
            continue;
        }
        setPerp(sect2->fCurve, first, last);
        // check to see if a range of points are on the curve
        onCurveCheck(sect2, first, last);
        SkTSpan<TCurve>* removalCandidate = NULL;
        if (!first->fCoinStart.isCoincident()) {
            SkTSpan<TCurve>* firstCoin = first->fNext;
            removalCandidate = first;
            first = firstCoin;
        }
        if (!first->fCoinStart.isCoincident()) {
            continue;
        }
        if (removalCandidate) {
            removeSpans(removalCandidate, sect2);
        }
        if (!last->fCoinStart.isCoincident()) {
            continue;
        }
        if (!last->fCoinEnd.isCoincident()) {
            if (--consecutive < COINCIDENT_SPAN_COUNT) {
                continue;
            }
            last = last->fPrev;
            SkASSERT(last->fCoinStart.isCoincident());
            SkASSERT(last->fCoinEnd.isCoincident());
        }
        SkASSERT(between(0, first->fCoinStart.perpT(), 1) || first->fCoinStart.perpT() == -1);
        if (first->fCoinStart.perpT() < 0) {
            first->fCoinStart.setPerp(fCurve, first->fStartT, first->fPart[0], sect2->fCurve);
        }
        SkASSERT(between(0, last->fCoinEnd.perpT(), 1) || last->fCoinEnd.perpT() == -1);
        if (last->fCoinEnd.perpT() < 0) {
            last->fCoinEnd.setPerp(fCurve, last->fEndT, last->fPart[TCurve::kPointLast],
                    sect2->fCurve);
        }
        SkTSpan<TCurve>* removeMe = first->fNext;
        while (removeMe != last) {
            SkTSpan<TCurve>* removeNext = removeMe->fNext;
            removeSpans(removeMe, sect2);
            removeMe = removeNext;
        }
    } while ((first = next));
}

template<typename TCurve>
bool SkTSect<TCurve>::intersects(SkTSpan<TCurve>* span, const SkTSect* opp,
        const SkTSpan<TCurve>* oppSpan) const {
    bool check;  // we ignore whether the end points are in common or not
    if (!span->intersects(oppSpan, &check)) {
        return false;
    }
    if (fActiveCount < COINCIDENT_SPAN_COUNT || opp->fActiveCount < COINCIDENT_SPAN_COUNT) {
        return true;
    }
    return span->tightBoundsIntersects(oppSpan);
}

template<typename TCurve>
void SkTSect<TCurve>::onCurveCheck(SkTSect* sect2, SkTSpan<TCurve>* first, SkTSpan<TCurve>* last) {
    SkTSpan<TCurve>* work = first;
    first = NULL;
    do {
        if (work->fCoinStart.isCoincident()) {
            if (!first) {
                first = work;
            }
        } else if (first) {
            break;
        }
        if (work == last) {
            break;
        }
        work = work->fNext;
        SkASSERT(work);
    } while (true);
    if (!first) {
        return;
    }
    // march outwards to find limit of coincidence from here to previous and next spans
    double startT = first->fStartT;
    double oppT;
    SkTSpan<TCurve>* prev = first->fPrev;
    if (prev) {
        double coinStart;
        if (binarySearchCoin(*sect2, startT, prev->fStartT - startT, &coinStart, &oppT)) {
            if (coinStart < startT) {
                SkASSERT(prev->fStartT < coinStart && coinStart < prev->fEndT);
                SkTSpan<TCurve>* oppStart = sect2->fHead->find(oppT);
                if (oppStart->fStartT < oppT && oppT < oppStart->fEndT) {
                    // split prev at coinStart if needed
                    SkTSpan<TCurve>* half2 = addOne();
                    half2->splitAt(prev, coinStart);
                    half2->initBounds(fCurve);
                    prev->initBounds(fCurve);
                    prev->fCoinEnd.markCoincident();
                    half2->fCoinStart.markCoincident();
                    half2->fCoinEnd.markCoincident();
                    // find span containing opposite t, and split that too
                    SkTSpan<TCurve>* oppHalf = sect2->addOne();
                    oppHalf->splitAt(oppStart, oppT);
                    oppHalf->initBounds(sect2->fCurve);
                    oppStart->initBounds(sect2->fCurve);
                } else {
                    SkASSERT(oppStart->fStartT == oppT || oppT == oppStart->fEndT);
                    first->fStartT = coinStart;
                    prev->fEndT = coinStart;
                    first->initBounds(fCurve);
                    prev->initBounds(fCurve);
                    first->fCoinStart.markCoincident();
                    first->fCoinEnd.markCoincident();
                }
            }
        }
    }
    if (!work->fCoinEnd.isCoincident()) {
        if (work->fEndT == 1) {
            SkDebugf("!");
        }
//        SkASSERT(work->fEndT < 1);
        startT = work->fStartT;
        double coinEnd;
        if (binarySearchCoin(*sect2, startT, work->fEndT - startT, &coinEnd, &oppT)) {
            if (coinEnd > startT) {
                SkTSpan<TCurve>* oppStart = sect2->fHead->find(oppT);
                if (oppStart->fStartT < oppT && oppT < oppStart->fEndT) {
                    SkASSERT(coinEnd < work->fEndT);
                    // split prev at coinEnd if needed
                    SkTSpan<TCurve>* half2 = addOne();
                    half2->splitAt(work, coinEnd);
                    half2->initBounds(fCurve);
                    work->initBounds(fCurve);
                    work->fCoinStart.markCoincident();
                    work->fCoinEnd.markCoincident();
                    half2->fCoinStart.markCoincident();
                    SkTSpan<TCurve>* oppHalf = sect2->addOne();
                    oppHalf->splitAt(oppStart, oppT);
                    oppHalf->initBounds(sect2->fCurve);
                    oppStart->initBounds(sect2->fCurve);
                } else {
                    SkASSERT(oppStart->fStartT == oppT || oppT == oppStart->fEndT);
                    SkTSpan<TCurve>* next = work->fNext;
                    bool hasNext = next && work->fEndT == next->fStartT;
                    work->fEndT = coinEnd;
                    work->initBounds(fCurve);
                    work->fCoinStart.markCoincident();
                    work->fCoinEnd.markCoincident();
                    if (hasNext) { 
                        next->fStartT = coinEnd;
                        next->initBounds(fCurve);
                    }
                }
            }
        }
    }
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
void SkTSect<TCurve>::removeSpan(SkTSpan<TCurve>* span) {
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
    --fActiveCount;
    span->fNext = fDeleted;
    fDeleted = span;
#if DEBUG_T_SECT
    SkASSERT(!span->fDebugDeleted);
    span->fDebugDeleted = true;
#endif
}

template<typename TCurve>
void SkTSect<TCurve>::removeOne(const SkTSpan<TCurve>* test, SkTSpan<TCurve>* span) {
    int last = span->fBounded.count() - 1;
    for (int index = 0; index <= last; ++index) {
        if (span->fBounded[index] == test) {
            span->fBounded.removeShuffle(index);
            if (!last) {
                removeSpan(span);
            }
            return;
        }
    }
}

template<typename TCurve>
void SkTSect<TCurve>::removeSpans(SkTSpan<TCurve>* span, SkTSect<TCurve>* opp) {
    int count = span->fBounded.count();
    for (int index = 0; index < count; ++index) {
        SkTSpan<TCurve>* bounded = span->fBounded[0];
        removeOne(bounded, span);  // shuffles last into position 0
        opp->removeOne(span, bounded);
    }
}

template<typename TCurve>
void SkTSect<TCurve>::setPerp(const TCurve& opp, SkTSpan<TCurve>* first, SkTSpan<TCurve>* last) {
    SkTSpan<TCurve>* work = first;
    if (!work->fHasPerp) {
        work->fCoinStart.setPerp(fCurve, work->fStartT, work->fPart[0], opp);
    }
    do {
        if (!work->fHasPerp) {
            work->fCoinEnd.setPerp(fCurve, work->fEndT, work->fPart[TCurve::kPointLast], opp);
            work->fHasPerp = true;
        }
        if (work == last) {
            break;
        }
        SkTSpan<TCurve>* last = work;
        work = work->fNext;
        SkASSERT(work);
        if (!work->fHasPerp) {
            work->fCoinStart = last->fCoinEnd;
        }
    } while (true);
}

template<typename TCurve>
const SkTSpan<TCurve>* SkTSect<TCurve>::tail() const {
    const SkTSpan<TCurve>* result = fHead;
    const SkTSpan<TCurve>* next = fHead;
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
    int count = span->fBounded.count();
    for (int index = 0; index < count; ) {
        SkTSpan<TCurve>* test = span->fBounded[index];
        bool sects = intersects(span, opp, test);
        if (sects) {
            ++index;
        } else {
            removeOne(test, span);
            opp->removeOne(span, test);
            --count;
        }
    }
}

#if DEBUG_T_SECT
template<typename TCurve>
void SkTSect<TCurve>::validate() const {
    int count = 0;
    if (fHead) {
        const SkTSpan<TCurve>* span = fHead;
        SkASSERT(!span->fPrev);
        double last = 0;
        do {
            span->validate();
            SkASSERT(span->fStartT >= last);
            last = span->fEndT;
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
    SkASSERT(fActiveCount + deletedCount == fDebugAllocatedCount);
}
#endif

template<typename TCurve>
int SkTSect<TCurve>::EndsEqual(const SkTSect* sect1, const SkTSect* sect2,
        SkIntersections* intersections) {
    int zeroOneSet = 0;
    // check for zero
    if (sect1->fCurve[0].approximatelyEqual(sect2->fCurve[0])) {
        zeroOneSet |= kZeroS1Set | kZeroS2Set;
        if (sect1->fCurve[0] != sect2->fCurve[0]) {
            intersections->insertNear(0, 0, sect1->fCurve[0], sect2->fCurve[0]);
        } else {
            intersections->insert(0, 0, sect1->fCurve[0]);
        }
    } 
    if (sect1->fCurve[0].approximatelyEqual(sect2->fCurve[TCurve::kPointLast])) {
        zeroOneSet |= kZeroS1Set | kOneS2Set;
        if (sect1->fCurve[0] != sect2->fCurve[TCurve::kPointLast]) {
            intersections->insertNear(0, 1, sect1->fCurve[0], sect2->fCurve[TCurve::kPointLast]);
        } else {
            intersections->insert(0, 1, sect1->fCurve[0]);
        }
    } 
    // check for one
    if (sect1->fCurve[TCurve::kPointLast].approximatelyEqual(sect2->fCurve[0])) {
        zeroOneSet |= kOneS1Set | kZeroS2Set;
        if (sect1->fCurve[TCurve::kPointLast] != sect2->fCurve[0]) {
            intersections->insertNear(1, 0, sect1->fCurve[TCurve::kPointLast], sect2->fCurve[0]);
        } else {
            intersections->insert(1, 0, sect1->fCurve[TCurve::kPointLast]);
        }
    } 
    if (sect1->fCurve[TCurve::kPointLast].approximatelyEqual(sect2->fCurve[TCurve::kPointLast])) {
        zeroOneSet |= kOneS1Set | kOneS2Set;
        if (sect1->fCurve[TCurve::kPointLast] != sect2->fCurve[TCurve::kPointLast]) {
            intersections->insertNear(1, 1, sect1->fCurve[TCurve::kPointLast],
                    sect2->fCurve[TCurve::kPointLast]);
        } else {
            intersections->insert(1, 1, sect1->fCurve[TCurve::kPointLast]);
        }
    }
    return zeroOneSet;
}

template<typename TCurve>
struct SkClosestRecord {
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

    void find(const SkTSpan<TCurve>* span1, const SkTSpan<TCurve>* span2) {
        SkClosestRecord<TCurve>* record = &fClosest[fUsed];
        record->findEnd(span1, span2, 0, 0);
        record->findEnd(span1, span2, 0, TCurve::kPointLast);
        record->findEnd(span1, span2, TCurve::kPointLast, 0);
        record->findEnd(span1, span2, TCurve::kPointLast, TCurve::kPointLast);
        if (record->fClosest == FLT_MAX) {
            return;
        }
        for (int index = 0; index < fUsed; ++index) {
            SkClosestRecord<TCurve>* test = &fClosest[index];
            if (test->matesWith(*record)) {
                if (test->fClosest > record->fClosest) {
                    test->merge(*record);
                }
                test->update(*record);
                record->reset();
                return;
            }
        }
        ++fUsed;
        fClosest.push_back().reset();
    }

    void finish(SkIntersections* intersections) const {
        for (int index = 0; index < fUsed; ++index) {
            const SkClosestRecord<TCurve>& test = fClosest[index];
            test.addIntersection(intersections);
        }
    }

    // this is oversized by one so that an extra record can merge into final one
    SkSTArray<TCurve::kMaxIntersections + 1, SkClosestRecord<TCurve>, true> fClosest;
    int fUsed;
};

// returns true if the rect is too small to consider
template<typename TCurve>
void SkTSect<TCurve>::BinarySearch(SkTSect* sect1, SkTSect* sect2, SkIntersections* intersections) {
    intersections->reset();
    intersections->setMax(TCurve::kMaxIntersections);
    SkTSpan<TCurve>* span1 = sect1->fHead;
    SkTSpan<TCurve>* span2 = sect2->fHead;
    bool check;
    if (!span1->intersects(span2, &check)) {
        return;
    }
    if (check) {
        (void) EndsEqual(sect1, sect2, intersections);
        return;
    }
    span1->fBounded.push_back() = span2;
    span2->fBounded.push_back() = span1;
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
        SkTSect* splitSect = split1 ? sect1 : sect2;
        SkTSpan<TCurve>* half1 = split1 ? largest1 : largest2;
        SkASSERT(half1);
        if (half1->fCollapsed) {
            break;
        }
        // trim parts that don't intersect the opposite
        SkTSpan<TCurve>* half2 = splitSect->addOne();
        SkTSect* unsplitSect = split1 ? sect2 : sect1;
        if (!half2->split(half1)) {
            break;
        }
        splitSect->trim(half1, unsplitSect);
        splitSect->trim(half2, unsplitSect);
        // if there are 9 or more continuous spans on both sects, suspect coincidence
        if (sect1->fActiveCount >= COINCIDENT_SPAN_COUNT
                && sect2->fActiveCount >= COINCIDENT_SPAN_COUNT) {
            sect1->coincidentCheck(sect2);
        }
#if DEBUG_T_SECT
        sect1->validate();
        sect2->validate();
#endif
#if DEBUG_T_SECT_DUMP > 1
        sect1->dumpBoth(*sect2);
#endif
        if (!sect1->fHead || !sect2->fHead) {
            return;
        }
    } while (true);
    if (sect1->fActiveCount >= 2 && sect2->fActiveCount >= 2) {
        // check for coincidence
        SkTSpan<TCurve>* first = sect1->fHead;
        do {
            if (!first->fCoinStart.isCoincident()) {
                continue;
            }
            int spanCount = 1;
            SkTSpan<TCurve>* last = first;
            while (last->fCoinEnd.isCoincident()) {
                SkTSpan<TCurve>* next = last->fNext;
                if (!next || !next->fCoinEnd.isCoincident()) {
                    break;
                }
                last = next;
                ++spanCount;
            }
            if (spanCount < 2) {
                first = last;
                continue;
            }
            int index = intersections->insertCoincident(first->fStartT, first->fCoinStart.perpT(),
                    first->fPart[0]);
            if (intersections->insertCoincident(last->fEndT, last->fCoinEnd.perpT(),
                    last->fPart[TCurve::kPointLast]) < 0) {
                intersections->clearCoincidence(index);
            }
        } while ((first = first->fNext));
    }
    int zeroOneSet = EndsEqual(sect1, sect2, intersections);
    sect1->recoverCollapsed();
    sect2->recoverCollapsed();
    SkTSpan<TCurve>* result1 = sect1->fHead;
    // check heads and tails for zero and ones and insert them if we haven't already done so
    const SkTSpan<TCurve>* head1 = result1;
    if (!(zeroOneSet & kZeroS1Set) && approximately_less_than_zero(head1->fStartT)) {
        const SkDPoint& start1 = sect1->fCurve[0];
        double t = head1->closestBoundedT(start1);
        if (sect2->fCurve.ptAtT(t).approximatelyEqual(start1)) {
            intersections->insert(0, t, start1);
        }
    }
    const SkTSpan<TCurve>* head2 = sect2->fHead;
    if (!(zeroOneSet & kZeroS2Set) && approximately_less_than_zero(head2->fStartT)) {
        const SkDPoint& start2 = sect2->fCurve[0];
        double t = head2->closestBoundedT(start2);
        if (sect1->fCurve.ptAtT(t).approximatelyEqual(start2)) {
            intersections->insert(t, 0, start2);
        }
    }
    const SkTSpan<TCurve>* tail1 = sect1->tail();
    if (!(zeroOneSet & kOneS1Set) && approximately_greater_than_one(tail1->fEndT)) {
        const SkDPoint& end1 = sect1->fCurve[TCurve::kPointLast];
        double t = tail1->closestBoundedT(end1);
        if (sect2->fCurve.ptAtT(t).approximatelyEqual(end1)) {
            intersections->insert(1, t, end1);
        }
    }
    const SkTSpan<TCurve>* tail2 = sect2->tail();
    if (!(zeroOneSet & kOneS2Set) && approximately_greater_than_one(tail2->fEndT)) {
        const SkDPoint& end2 = sect2->fCurve[TCurve::kPointLast];
        double t = tail2->closestBoundedT(end2);
        if (sect1->fCurve.ptAtT(t).approximatelyEqual(end2)) {
            intersections->insert(t, 1, end2);
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
        while (result2) {
            closest.find(result1, result2);
            result2 = result2->fNext;
        }
        
    } while ((result1 = result1->fNext));
    closest.finish(intersections);
}
