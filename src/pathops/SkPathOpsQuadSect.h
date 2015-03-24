/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkQuadSpan_DEFINE
#define SkQuadSpan_DEFINE

#include "SkChunkAlloc.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsQuad.h"
#include "SkTArray.h"

class SkIntersections;

class SkQuadCoincident {
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

    void setPerp(const SkDQuad& quad1, double t, const SkDPoint& qPt, const SkDQuad& quad2);

private:
    SkDPoint fPerpPt;
    double fPerpT;  // perpendicular intersection on opposite quad
    bool fCoincident;
};

class SkQuadSect;  // used only by debug id

class SkQuadSpan {
public:
    void init(const SkDQuad& quad);
    void initBounds(const SkDQuad& quad);

    bool contains(double t) const {
        return !! const_cast<SkQuadSpan*>(this)->innerFind(t);
    }

    bool contains(const SkQuadSpan* span) const;

    SkQuadSpan* find(double t) {
        SkQuadSpan* result = innerFind(t);
        SkASSERT(result);
        return result;
    }

    bool intersects(const SkQuadSpan* span) const;

    const SkQuadSpan* next() const {
        return fNext;
    }

    void reset() {
        fBounded.reset();
    }

    bool split(SkQuadSpan* work) {
        return splitAt(work, (work->fStartT + work->fEndT) * 0.5);
    }

    bool splitAt(SkQuadSpan* work, double t);
    bool tightBoundsIntersects(const SkQuadSpan* span) const;

    // implementation is for testing only
    void dump() const;

private:
    bool hullIntersects(const SkDQuad& q2) const;
    SkQuadSpan* innerFind(double t);
    bool linearIntersects(const SkDQuad& q2) const;

    // implementation is for testing only
#if DEBUG_BINARY_QUAD
    int debugID(const SkQuadSect* ) const { return fDebugID; }
#else
    int debugID(const SkQuadSect* ) const;
#endif
    void dump(const SkQuadSect* ) const;
    void dumpID(const SkQuadSect* ) const;

#if DEBUG_BINARY_QUAD
    void validate() const;
#endif

    SkDQuad fPart;
    SkQuadCoincident fCoinStart;
    SkQuadCoincident fCoinEnd;
    SkSTArray<4, SkQuadSpan*, true> fBounded;
    SkQuadSpan* fPrev;
    SkQuadSpan* fNext;
    SkDRect fBounds;
    double fStartT;
    double fEndT;
    double fBoundsMax;
    bool fCollapsed;
    bool fHasPerp;
    mutable bool fIsLinear;
#if DEBUG_BINARY_QUAD
    int fDebugID;
    bool fDebugDeleted;
#endif
    friend class SkQuadSect;
};

class SkQuadSect {
public:
    SkQuadSect(const SkDQuad& quad PATH_OPS_DEBUG_PARAMS(int id));
    static void BinarySearch(SkQuadSect* sect1, SkQuadSect* sect2, SkIntersections* intersections);

    // for testing only
    void dumpQuads() const;
private:
    SkQuadSpan* addOne();
    bool binarySearchCoin(const SkQuadSect& , double tStart, double tStep, double* t, double* oppT);
    SkQuadSpan* boundsMax() const;
    void coincidentCheck(SkQuadSect* sect2);
    bool intersects(const SkQuadSpan* span, const SkQuadSect* opp, const SkQuadSpan* oppSpan) const;
    void onCurveCheck(SkQuadSect* sect2, SkQuadSpan* first, SkQuadSpan* last);
    void recoverCollapsed();
    void removeSpan(SkQuadSpan* span);
    void removeOne(const SkQuadSpan* test, SkQuadSpan* span);
    void removeSpans(SkQuadSpan* span, SkQuadSect* opp);
    void setPerp(const SkDQuad& opp, SkQuadSpan* first, SkQuadSpan* last);
    const SkQuadSpan* tail() const;
    void trim(SkQuadSpan* span, SkQuadSect* opp);

    // for testing only
    void dump() const;
    void dumpBoth(const SkQuadSect& opp) const;
    void dumpBoth(const SkQuadSect* opp) const;

#if DEBUG_BINARY_QUAD
    int debugID() const { return fDebugID; }
    void validate() const;
#else
    int debugID() const { return 0; }
#endif
    const SkDQuad& fQuad;
    SkChunkAlloc fHeap;
    SkQuadSpan* fHead;
    SkQuadSpan* fDeleted;
    int fActiveCount;
#if DEBUG_BINARY_QUAD
    int fDebugID;
    int fDebugCount;
    int fDebugAllocatedCount;
#endif
    friend class SkQuadSpan;  // only used by debug id
};

#endif
