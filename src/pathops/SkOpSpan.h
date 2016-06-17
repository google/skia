/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpSpan_DEFINED
#define SkOpSpan_DEFINED

#include "SkPathOpsDebug.h"
#include "SkPathOpsTypes.h"
#include "SkPoint.h"

class SkChunkAlloc;
struct SkOpAngle;
class SkOpContour;
class SkOpGlobalState;
class SkOpSegment;
class SkOpSpanBase;
class SkOpSpan;

// subset of op span used by terminal span (when t is equal to one)
class SkOpPtT {
public:
    enum {
        kIsAlias = 1,
        kIsDuplicate = 1
    };

    void addOpp(SkOpPtT* opp) {
        // find the fOpp ptr to opp
        SkOpPtT* oppPrev = opp->fNext;
        if (oppPrev == this) {
            return;
        }
        while (oppPrev->fNext != opp) {
            oppPrev = oppPrev->fNext;
             if (oppPrev == this) {
                 return;
             }
        }
        
        SkOpPtT* oldNext = this->fNext;
        SkASSERT(this != opp);
        this->fNext = opp;
        SkASSERT(oppPrev != oldNext);
        oppPrev->fNext = oldNext;
    }

    bool alias() const;
    bool collapsed(const SkOpPtT* ) const;
    bool contains(const SkOpPtT* ) const;
    SkOpPtT* contains(const SkOpSegment* );
    SkOpContour* contour() const;

    int debugID() const {
        return SkDEBUGRELEASE(fID, -1);
    }

    const SkOpAngle* debugAngle(int id) const;
    bool debugContains(const SkOpPtT* ) const;
    const SkOpPtT* debugContains(const SkOpSegment* check) const;
    SkOpContour* debugContour(int id);
    int debugLoopLimit(bool report) const;
    bool debugMatchID(int id) const;
    const SkOpPtT* debugPtT(int id) const;
    const SkOpSegment* debugSegment(int id) const;
    const SkOpSpanBase* debugSpan(int id) const;
    void debugValidate() const;

    bool deleted() const {
        return fDeleted;
    }

    SkOpPtT* doppelganger();

    bool duplicate() const {
        return fDuplicatePt;
    }

    void dump() const;  // available to testing only
    void dumpAll() const;
    void dumpBase() const;

    SkOpPtT* find(SkOpSegment* );
    SkOpGlobalState* globalState() const;
    void init(SkOpSpanBase* , double t, const SkPoint& , bool dup);

    void insert(SkOpPtT* span) {
        SkASSERT(span != this);
        span->fNext = fNext;
        fNext = span;
    }

    const SkOpPtT* next() const {
        return fNext;
    }

    SkOpPtT* next() {
        return fNext;
    }

    bool onEnd() const;

    static bool Overlaps(SkOpPtT* s1, SkOpPtT* e1, SkOpPtT* s2, SkOpPtT* e2,
            SkOpPtT** sOut, SkOpPtT** eOut) {
        SkOpPtT* start1 = s1->fT < e1->fT ? s1 : e1;
        SkOpPtT* start2 = s2->fT < e2->fT ? s2 : e2;
        *sOut = between(s1->fT, start2->fT, e1->fT) ? start2
                : between(s2->fT, start1->fT, e2->fT) ? start1 : nullptr;
        SkOpPtT* end1 = s1->fT < e1->fT ? e1 : s1;
        SkOpPtT* end2 = s2->fT < e2->fT ? e2 : s2;
        *eOut = between(s1->fT, end2->fT, e1->fT) ? end2
                : between(s2->fT, end1->fT, e2->fT) ? end1 : nullptr;
        if (*sOut == *eOut) {
            SkASSERT(start1->fT >= end2->fT || start2->fT >= end1->fT);
            return false;
        }
        SkASSERT(!*sOut || *sOut != *eOut);
        return *sOut && *eOut;
    }

    SkOpPtT* prev();
    SkOpPtT* remove();
    void removeNext(SkOpPtT* kept);

    const SkOpSegment* segment() const;
    SkOpSegment* segment();

    void setDeleted() {
        SkASSERT(!fDeleted);
        fDeleted = true;
    }

    const SkOpSpanBase* span() const {
        return fSpan;
    }

    SkOpSpanBase* span() {
        return fSpan;
    }

    const SkOpPtT* starter(const SkOpPtT* end) const {
        return fT < end->fT ? this : end;
    }

    double fT; 
    SkPoint fPt;   // cache of point value at this t
protected:
    SkOpSpanBase* fSpan;  // contains winding data
    SkOpPtT* fNext;  // intersection on opposite curve or alias on this curve
    bool fDeleted;  // set if removed from span list 
    bool fDuplicatePt;  // set if identical pt is somewhere in the next loop
    SkDEBUGCODE(int fID);
};

class SkOpSpanBase {
public:
    void align();

    bool aligned() const {
        return fAligned;
    }

    void alignEnd(double t, const SkPoint& pt);

    void bumpSpanAdds() {
        ++fSpanAdds;
    }

    bool chased() const {
        return fChased;
    }

    void clearCoinEnd() {
        SkASSERT(fCoinEnd != this);
        fCoinEnd = this;
    }

    const SkOpSpanBase* coinEnd() const {
        return fCoinEnd;
    }

    bool contains(const SkOpSpanBase* ) const;
    SkOpPtT* contains(const SkOpSegment* );

    bool containsCoinEnd(const SkOpSpanBase* coin) const {
        SkASSERT(this != coin);
        const SkOpSpanBase* next = this;
        while ((next = next->fCoinEnd) != this) {
            if (next == coin) {
                return true;
            }
        }
        return false;
    }

    bool containsCoinEnd(const SkOpSegment* ) const;
    SkOpContour* contour() const;

    int debugBumpCount() {
        return SkDEBUGRELEASE(++fCount, -1);
    }

    int debugID() const {
        return SkDEBUGRELEASE(fID, -1);
    }

    bool debugAlignedEnd(double t, const SkPoint& pt) const;
    bool debugAlignedInner() const;
    const SkOpAngle* debugAngle(int id) const;
    bool debugCoinEndLoopCheck() const;
    bool debugContains(const SkOpSegment* ) const;
    SkOpContour* debugContour(int id);
    const SkOpPtT* debugPtT(int id) const;
    const SkOpSegment* debugSegment(int id) const;
    const SkOpSpanBase* debugSpan(int id) const;
    const SkOpSpan* debugStarter(SkOpSpanBase const** endPtr) const;
    SkOpGlobalState* globalState() const;
    void debugValidate() const;

    bool deleted() const {
        return fPtT.deleted();
    }

    void dump() const;  // available to testing only
    void dumpCoin() const;
    void dumpAll() const;
    void dumpBase() const;

    bool final() const {
        return fPtT.fT == 1;
    }

    SkOpAngle* fromAngle() const {
        return fFromAngle;
    }

    void initBase(SkOpSegment* parent, SkOpSpan* prev, double t, const SkPoint& pt);

    void insertCoinEnd(SkOpSpanBase* coin) {
        if (containsCoinEnd(coin)) {
            SkASSERT(coin->containsCoinEnd(this));
            return;
        }
        debugValidate();
        SkASSERT(this != coin);
        SkOpSpanBase* coinNext = coin->fCoinEnd;
        coin->fCoinEnd = this->fCoinEnd;
        this->fCoinEnd = coinNext;
        debugValidate();
    }

    void merge(SkOpSpan* span);

    SkOpSpan* prev() const {
        return fPrev;
    }

    const SkPoint& pt() const {
        return fPtT.fPt;
    }

    const SkOpPtT* ptT() const {
        return &fPtT;
    }

    SkOpPtT* ptT() {
        return &fPtT;
    }

    SkOpSegment* segment() const {
        return fSegment;
    }

    void setAligned() {
        fAligned = true;
    }

    void setChased(bool chased) {
        fChased = chased;
    }

    SkOpPtT* setCoinEnd(SkOpSpanBase* oldCoinEnd, SkOpSegment* oppSegment);

    void setFromAngle(SkOpAngle* angle) {
        fFromAngle = angle;
    }

    void setPrev(SkOpSpan* prev) {
        fPrev = prev;
    }

    bool simple() const {
        fPtT.debugValidate();
        return fPtT.next()->next() == &fPtT; 
    }

    int spanAddsCount() const {
        return fSpanAdds;
    }

    const SkOpSpan* starter(const SkOpSpanBase* end) const {
        const SkOpSpanBase* result = t() < end->t() ? this : end;
        return result->upCast();
    }

    SkOpSpan* starter(SkOpSpanBase* end) {
        SkASSERT(this->segment() == end->segment());
        SkOpSpanBase* result = t() < end->t() ? this : end;
        return result->upCast();
    }

    SkOpSpan* starter(SkOpSpanBase** endPtr) {
        SkOpSpanBase* end = *endPtr;
        SkASSERT(this->segment() == end->segment());
        SkOpSpanBase* result;
        if (t() < end->t()) {
            result = this;
        } else {
            result = end;
            *endPtr = this;
        }
        return result->upCast();
    }

    int step(const SkOpSpanBase* end) const {
        return t() < end->t() ? 1 : -1;
    }

    double t() const {
        return fPtT.fT;
    }

    void unaligned() {
        fAligned = false;
    }

    SkOpSpan* upCast() {
        SkASSERT(!final());
        return (SkOpSpan*) this;
    }

    const SkOpSpan* upCast() const {
        SkASSERT(!final());
        return (const SkOpSpan*) this;
    }

    SkOpSpan* upCastable() {
        return final() ? nullptr : upCast();
    }

    const SkOpSpan* upCastable() const {
        return final() ? nullptr : upCast();
    }

private:
    void alignInner();

protected:  // no direct access to internals to avoid treating a span base as a span
    SkOpPtT fPtT;  // list of points and t values associated with the start of this span
    SkOpSegment* fSegment;  // segment that contains this span
    SkOpSpanBase* fCoinEnd;  // linked list of coincident spans that end here (may point to itself)
    SkOpAngle* fFromAngle;  // points to next angle from span start to end
    SkOpSpan* fPrev;  // previous intersection point
    int fSpanAdds;  // number of times intersections have been added to span
    bool fAligned;
    bool fChased;  // set after span has been added to chase array
    SkDEBUGCODE(int fCount);  // number of pt/t pairs added
    SkDEBUGCODE(int fID);
};

class SkOpSpan : public SkOpSpanBase {
public:
    bool alreadyAdded() const {
        if (fAlreadyAdded) {
            return true;
        }
        fAlreadyAdded = true;
        return false;
    }

    bool clearCoincident() {
        SkASSERT(!final());
        if (fCoincident == this) {
            return false;
        }
        fCoincident = this;
        return true;
    }

    int computeWindSum();
    bool containsCoincidence(const SkOpSegment* ) const;

    bool containsCoincidence(const SkOpSpan* coin) const {
        SkASSERT(this != coin);
        const SkOpSpan* next = this;
        while ((next = next->fCoincident) != this) {
            if (next == coin) {
                return true;
            }
        }
        return false;
    }

    bool debugCoinLoopCheck() const;
    void release(SkOpPtT* );

    bool done() const {
        SkASSERT(!final());
        return fDone;
    }

    void dumpCoin() const;
    bool dumpSpan() const;
    void init(SkOpSegment* parent, SkOpSpan* prev, double t, const SkPoint& pt);

    void insertCoincidence(SkOpSpan* coin) {
        if (containsCoincidence(coin)) {
            SkASSERT(coin->containsCoincidence(this));
            return;
        }
        debugValidate();
        SkASSERT(this != coin);
        SkOpSpan* coinNext = coin->fCoincident;
        coin->fCoincident = this->fCoincident;
        this->fCoincident = coinNext;
        debugValidate();
    }

    bool isCanceled() const {
        SkASSERT(!final());
        return fWindValue == 0 && fOppValue == 0;
    }

    bool isCoincident() const {
        SkASSERT(!final());
        return fCoincident != this;
    }

    SkOpSpanBase* next() const {
        SkASSERT(!final());
        return fNext;
    }

    int oppSum() const {
        SkASSERT(!final());
        return fOppSum;
    }

    int oppValue() const {
        SkASSERT(!final());
        return fOppValue;
    }

    SkOpPtT* setCoinStart(SkOpSpan* oldCoinStart, SkOpSegment* oppSegment);

    void setDone(bool done) {
        SkASSERT(!final());
        fDone = done;
    }

    void setNext(SkOpSpanBase* nextT) {
        SkASSERT(!final());
        fNext = nextT;
    }

    void setOppSum(int oppSum);

    void setOppValue(int oppValue) {
        SkASSERT(!final());
        SkASSERT(fOppSum == SK_MinS32);
        fOppValue = oppValue;
    }

    void setToAngle(SkOpAngle* angle) {
        SkASSERT(!final());
        fToAngle = angle;
    }

    void setWindSum(int windSum);

    void setWindValue(int windValue) {
        SkASSERT(!final());
        SkASSERT(windValue >= 0);
        SkASSERT(fWindSum == SK_MinS32);
        fWindValue = windValue;
    }

    bool sortableTop(SkOpContour* );

    SkOpAngle* toAngle() const {
        SkASSERT(!final());
        return fToAngle;
    }

    int windSum() const {
        SkASSERT(!final());
        return fWindSum;
    }

    int windValue() const {
        SkASSERT(!final());
        return fWindValue;
    }

private:  // no direct access to internals to avoid treating a span base as a span
    SkOpSpan* fCoincident;  // linked list of spans coincident with this one (may point to itself)
    SkOpAngle* fToAngle;  // points to next angle from span start to end
    SkOpSpanBase* fNext;  // next intersection point
    int fWindSum;  // accumulated from contours surrounding this one.
    int fOppSum;  // for binary operators: the opposite winding sum
    int fWindValue;  // 0 == canceled; 1 == normal; >1 == coincident
    int fOppValue;  // normally 0 -- when binary coincident edges combine, opp value goes here
    int fTopTTry; // specifies direction and t value to try next
    bool fDone;  // if set, this span to next higher T has been processed
    mutable bool fAlreadyAdded;
};

#endif
