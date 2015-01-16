/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCubicSpan_DEFINE
#define SkCubicSpan_DEFINE

#include "SkChunkAlloc.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsCubic.h"
#include "SkTArray.h"

class SkIntersections;

class SkCubicCoincident {
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

    void setPerp(const SkDCubic& cubic1, double t, const SkDPoint& qPt, const SkDCubic& cubic2);

private:
    SkDPoint fPerpPt;
    double fPerpT;  // perpendicular intersection on opposite Cubic
    bool fCoincident;
};

class SkCubicSect;  // used only by debug id

class SkCubicSpan {
public:
    void init(const SkDCubic& Cubic);
    void initBounds(const SkDCubic& Cubic);

    bool contains(double t) const {
        return !! const_cast<SkCubicSpan*>(this)->innerFind(t);
    }

    bool contains(const SkCubicSpan* span) const;

    SkCubicSpan* find(double t) {
        SkCubicSpan* result = innerFind(t);
        SkASSERT(result);
        return result;
    }

    bool intersects(const SkCubicSpan* span) const;

    const SkCubicSpan* next() const {
        return fNext;
    }

    void reset() {
        fBounded.reset();
    }

    bool split(SkCubicSpan* work) {
        return splitAt(work, (work->fStartT + work->fEndT) * 0.5);
    }

    bool splitAt(SkCubicSpan* work, double t);
    bool tightBoundsIntersects(const SkCubicSpan* span) const;

    // implementation is for testing only
    void dump() const;

private:
    bool hullIntersects(const SkDCubic& ) const;
    SkCubicSpan* innerFind(double t);
    bool linearIntersects(const SkDCubic& ) const;

    // implementation is for testing only
#if DEBUG_BINARY_CUBIC
    int debugID(const SkCubicSect* ) const { return fDebugID; }
#else
    int debugID(const SkCubicSect* ) const;
#endif
    void dump(const SkCubicSect* ) const;
    void dumpID(const SkCubicSect* ) const;

#if DEBUG_BINARY_CUBIC
    void validate() const;
#endif

    SkDCubic fPart;
    SkCubicCoincident fCoinStart;
    SkCubicCoincident fCoinEnd;
    SkSTArray<4, SkCubicSpan*, true> fBounded;
    SkCubicSpan* fPrev;
    SkCubicSpan* fNext;
    SkDRect fBounds;
    double fStartT;
    double fEndT;
    double fBoundsMax;
    bool fCollapsed;
    bool fHasPerp;
    mutable bool fIsLinear;
#if DEBUG_BINARY_CUBIC
    int fDebugID;
    bool fDebugDeleted;
#endif
    friend class SkCubicSect;
};

class SkCubicSect {
public:
    SkCubicSect(const SkDCubic& Cubic PATH_OPS_DEBUG_PARAMS(int id));
    static void BinarySearch(SkCubicSect* sect1, SkCubicSect* sect2, SkIntersections* intersections);

    // for testing only
    void dumpCubics() const;
private:
    SkCubicSpan* addOne();
    bool binarySearchCoin(const SkCubicSect& , double tStart, double tStep, double* t,
            double* oppT);
    SkCubicSpan* boundsMax() const;
    void coincidentCheck(SkCubicSect* sect2);
    bool intersects(const SkCubicSpan* span, const SkCubicSect* opp, const SkCubicSpan* oppSpan) const;
    void onCurveCheck(SkCubicSect* sect2, SkCubicSpan* first, SkCubicSpan* last);
    void recoverCollapsed();
    void removeSpan(SkCubicSpan* span);
    void removeOne(const SkCubicSpan* test, SkCubicSpan* span);
    void removeSpans(SkCubicSpan* span, SkCubicSect* opp);
    void setPerp(const SkDCubic& opp, SkCubicSpan* first, SkCubicSpan* last);
    void trim(SkCubicSpan* span, SkCubicSect* opp);

    // for testing only
    void dump() const;
    void dumpBoth(const SkCubicSect& opp) const;
    void dumpBoth(const SkCubicSect* opp) const;

#if DEBUG_BINARY_CUBIC
    int debugID() const { return fDebugID; }
    void validate() const;
#else
    int debugID() const { return 0; }
#endif
    const SkDCubic& fCubic;
    SkChunkAlloc fHeap;
    SkCubicSpan* fHead;
    SkCubicSpan* fDeleted;
    int fActiveCount;
#if DEBUG_BINARY_CUBIC
    int fDebugID;
    int fDebugCount;
    int fDebugAllocatedCount;
#endif
    friend class SkCubicSpan;  // only used by debug id
};

#endif
