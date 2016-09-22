/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpCoincidence_DEFINED
#define SkOpCoincidence_DEFINED

#include "SkTDArray.h"
#include "SkOpTAllocator.h"
#include "SkOpSpan.h"
#include "SkPathOpsTypes.h"

class SkOpPtT;
class SkOpSpanBase;

class SkCoincidentSpans {
public:
    const SkOpPtT* coinPtTEnd() const { return fCoinPtTEnd; }
    const SkOpPtT* coinPtTStart() const { return fCoinPtTStart; }

    // These return non-const pointers so that, as copies, they can be added
    // to a new span pair
    SkOpPtT* coinPtTEndWritable() const { return const_cast<SkOpPtT*>(fCoinPtTEnd); }
    SkOpPtT* coinPtTStartWritable() const { return const_cast<SkOpPtT*>(fCoinPtTStart); }

    bool collapsed(const SkOpPtT* ) const;
    bool contains(const SkOpPtT* s, const SkOpPtT* e) const;
    void correctEnds();
    void correctOneEnd(const SkOpPtT* (SkCoincidentSpans::* getEnd)() const,
                       void (SkCoincidentSpans::* setEnd)(const SkOpPtT* ptT) );

#if DEBUG_COINCIDENCE_VERBOSE
    bool debugExpand(const char* id, SkPathOpsDebug::GlitchLog* log) const;
#endif

    int debugID() const {
        return SkDEBUGRELEASE(fID, -1);
    }

    void debugShow() const;
#ifdef SK_DEBUG
    void debugStartCheck(const SkOpSpanBase* outer, const SkOpSpanBase* over,
            const SkOpGlobalState* debugState) const;
#endif
    void dump() const;
    bool expand();
    bool extend(const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
                const SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd);
    bool flipped() const { return fOppPtTStart->fT > fOppPtTEnd->fT; }
    SkDEBUGCODE(SkOpGlobalState* globalState() { return fGlobalState; })

    void init(SkDEBUGCODE(SkOpGlobalState* globalState)) {
        sk_bzero(this, sizeof(*this)); 
        SkDEBUGCODE(fGlobalState = globalState);
    }

    SkCoincidentSpans* next() { return fNext; }
    const SkCoincidentSpans* next() const { return fNext; }
    SkCoincidentSpans** nextPtr() { return &fNext; }
    const SkOpPtT* oppPtTStart() const { return fOppPtTStart; }
    const SkOpPtT* oppPtTEnd() const { return fOppPtTEnd; }
    // These return non-const pointers so that, as copies, they can be added
    // to a new span pair
    SkOpPtT* oppPtTStartWritable() const { return const_cast<SkOpPtT*>(fOppPtTStart); }
    SkOpPtT* oppPtTEndWritable() const { return const_cast<SkOpPtT*>(fOppPtTEnd); }
    bool ordered() const;
    int spanCount() const;

    void set(SkCoincidentSpans* next, const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
        const SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd
        SkDEBUGPARAMS(int id));

    void setCoinPtTEnd(const SkOpPtT* ptT) {
        SkOPASSERT(ptT == ptT->span()->ptT());
        SkASSERT(!fCoinPtTStart || ptT->fT != fCoinPtTStart->fT);
        SkASSERT(!fCoinPtTStart || fCoinPtTStart->segment() == ptT->segment());
        fCoinPtTEnd = ptT;
        ptT->setCoincident();
    }

    void setCoinPtTStart(const SkOpPtT* ptT) {
        SkASSERT(ptT == ptT->span()->ptT());
        SkOPASSERT(!fCoinPtTEnd || ptT->fT != fCoinPtTEnd->fT);
        SkASSERT(!fCoinPtTEnd || fCoinPtTEnd->segment() == ptT->segment());
        fCoinPtTStart = ptT;
        ptT->setCoincident();
    }

    void setEnds(const SkOpPtT* coinPtTEnd, const SkOpPtT* oppPtTEnd) {
        this->setCoinPtTEnd(coinPtTEnd);
        this->setOppPtTEnd(oppPtTEnd);
    }

    void setOppPtTEnd(const SkOpPtT* ptT) {
        SkOPASSERT(ptT == ptT->span()->ptT());
        SkOPASSERT(!fOppPtTStart || ptT->fT != fOppPtTStart->fT);
        SkASSERT(!fOppPtTStart || fOppPtTStart->segment() == ptT->segment());
        fOppPtTEnd = ptT;
        ptT->setCoincident();
    }

    void setOppPtTStart(const SkOpPtT* ptT) {
        SkASSERT(ptT == ptT->span()->ptT());
        SkASSERT(!fOppPtTEnd || ptT->fT != fOppPtTEnd->fT);
        SkASSERT(!fOppPtTEnd || fOppPtTEnd->segment() == ptT->segment());
        fOppPtTStart = ptT;
        ptT->setCoincident();
    }

    void setStarts(const SkOpPtT* coinPtTStart, const SkOpPtT* oppPtTStart) {
        this->setCoinPtTStart(coinPtTStart);
        this->setOppPtTStart(oppPtTStart);
    }

    void setNext(SkCoincidentSpans* next) { fNext = next; }

    bool startEquals(const SkOpSpanBase* outer, const SkOpSpanBase* over) const {
        return fCoinPtTStart->span() == over && fOppPtTStart->span() == outer;
    }

private:
    SkCoincidentSpans* fNext;
    const SkOpPtT* fCoinPtTStart;
    const SkOpPtT* fCoinPtTEnd;
    const SkOpPtT* fOppPtTStart;
    const SkOpPtT* fOppPtTEnd;
    SkDEBUGCODE(SkOpGlobalState* fGlobalState);
    SkDEBUGCODE(int fID);
};

class SkOpCoincidence {
public:
    SkOpCoincidence(SkOpGlobalState* globalState)
        : fHead(nullptr)
        , fTop(nullptr)
        , fGlobalState(globalState)
        , fContinue(false)
        , fSpanDeleted(false)
        , fPtAllocated(false)
        , fCoinExtended(false)
        , fSpanMerged(false) {
        globalState->setCoincidence(this);
    }

    void add(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
             SkOpPtT* oppPtTEnd);
    bool addEndMovedSpans();
    bool addExpanded();
    bool addMissing(bool* added);
    bool addUncommon();
    bool apply();
    bool contains(const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
                  const SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd) const;
    void correctEnds();

#if DEBUG_COINCIDENCE_VERBOSE
    void debugAddExpanded(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugAddMissing(const char* id, SkPathOpsDebug::GlitchLog* , bool* added) const;
    void debugAddOrOverlap(const char* id, SkPathOpsDebug::GlitchLog* log,
                           const SkOpSegment* coinSeg, const SkOpSegment* oppSeg,
                           double coinTs, double coinTe, double oppTs, double oppTe,
                           bool* added) const;
#endif

    const SkOpAngle* debugAngle(int id) const {
        return SkDEBUGRELEASE(fGlobalState->debugAngle(id), nullptr);
    }

#if DEBUG_COINCIDENCE_VERBOSE
    void debugCheckValid(const char* id, SkPathOpsDebug::GlitchLog* log) const;
#endif

    SkOpContour* debugContour(int id) const {
        return SkDEBUGRELEASE(fGlobalState->debugContour(id), nullptr);
    }

#if DEBUG_COINCIDENCE_VERBOSE
    bool debugExpand(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugMark(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugMarkCollapsed(const char* id, SkPathOpsDebug::GlitchLog* ,
                            const SkCoincidentSpans* coin, const SkOpPtT* test) const;
    void debugMarkCollapsed(const char* id, SkPathOpsDebug::GlitchLog* , const SkOpPtT* test) const;
#endif

    const SkOpPtT* debugPtT(int id) const {
        return SkDEBUGRELEASE(fGlobalState->debugPtT(id), nullptr);
    }

    const SkOpSegment* debugSegment(int id) const {
        return SkDEBUGRELEASE(fGlobalState->debugSegment(id), nullptr);
    }

#if DEBUG_COINCIDENCE_VERBOSE
    void debugRemoveCollapsed(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugReorder(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugRelease(const char* id, SkPathOpsDebug::GlitchLog* , const SkCoincidentSpans* ,
                      const SkCoincidentSpans* ) const;
    void debugRelease(const char* id, SkPathOpsDebug::GlitchLog* , const SkOpSegment* ) const;
#endif
    void debugShowCoincidence() const;

    const SkOpSpanBase* debugSpan(int id) const {
        return SkDEBUGRELEASE(fGlobalState->debugSpan(id), nullptr);
    }

    void debugValidate() const;
    void dump() const;
    bool edge(const SkOpPtT* , bool* start) const;
    bool expand();
    bool extend(const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd, const SkOpPtT* oppPtTStart,
                const SkOpPtT* oppPtTEnd);
    bool findOverlaps(SkOpCoincidence* ) const;
    void fixUp(SkOpPtT* deleted, const SkOpPtT* kept);

    SkOpGlobalState* globalState() {
        return fGlobalState;
    }

    const SkOpGlobalState* globalState() const {
        return fGlobalState;
    }

    bool isEmpty() const {
        return !fHead && !fTop;
    }

    bool mark();
    void markCollapsed(SkOpPtT* );

    static bool Ordered(const SkOpPtT* coinPtTStart, const SkOpPtT* oppPtTStart) {
      return Ordered(coinPtTStart->segment(), oppPtTStart->segment());
    }

    static bool Ordered(const SkOpSegment* coin, const SkOpSegment* opp);
    void release(const SkOpSegment* );
    void releaseDeleted();
    bool removeCollapsed();
    bool reorder();

private:
    void add(const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd, const SkOpPtT* oppPtTStart,
             const SkOpPtT* oppPtTEnd) {
        this->add(const_cast<SkOpPtT*>(coinPtTStart), const_cast<SkOpPtT*>(coinPtTEnd),
            const_cast<SkOpPtT*>(oppPtTStart), const_cast<SkOpPtT*>(oppPtTEnd));
    }

    bool addEndMovedSpans(const SkOpSpan* base, const SkOpSpanBase* testSpan);
    bool addEndMovedSpans(const SkOpPtT* ptT);

    bool addIfMissing(const SkCoincidentSpans* outer, SkOpPtT* over1s, SkOpPtT* over1e);

    bool addIfMissing(const SkCoincidentSpans* outer, const SkOpPtT* over1s,
                      const SkOpPtT* over1e) {
        return addIfMissing(outer, const_cast<SkOpPtT*>(over1s), const_cast<SkOpPtT*>(over1e));
    }

    bool addIfMissing(const SkOpPtT* over1s, const SkOpPtT* over2s,
                      double tStart, double tEnd, SkOpSegment* coinSeg, SkOpSegment* oppSeg,
                      bool* added
                      SkDEBUGPARAMS(const SkOpPtT* over1e) SkDEBUGPARAMS(const SkOpPtT* over2e));
    bool addOrOverlap(SkOpSegment* coinSeg, SkOpSegment* oppSeg,
                      double coinTs, double coinTe, double oppTs, double oppTe, bool* added);
    bool addOverlap(const SkOpSegment* seg1, const SkOpSegment* seg1o,
                    const SkOpSegment* seg2, const SkOpSegment* seg2o,
                    const SkOpPtT* overS, const SkOpPtT* overE);
    bool alreadyAdded(const SkCoincidentSpans* check, const SkCoincidentSpans* outer,
                      const SkOpPtT* over1s, const SkOpPtT* over1e) const;
    bool checkOverlap(SkCoincidentSpans* check,
                      const SkOpSegment* coinSeg, const SkOpSegment* oppSeg,
                      double coinTs, double coinTe, double oppTs, double oppTe,
                      SkTDArray<SkCoincidentSpans*>* overlaps) const;
    bool contains(const SkOpSegment* seg, const SkOpSegment* opp, double oppT) const;
    bool contains(const SkCoincidentSpans* coin, const SkOpSegment* seg,
                  const SkOpSegment* opp, double oppT) const;
#if DEBUG_COINCIDENCE_VERBOSE
    void debugAddIfMissing(const char* id, SkPathOpsDebug::GlitchLog* ,
                           const SkCoincidentSpans* outer, const SkOpPtT* over1s,
                           const SkOpPtT* over1e) const;
    void debugAddIfMissing(const char* id, SkPathOpsDebug::GlitchLog* ,
                           const SkOpPtT* over1s, const SkOpPtT* over2s,
                           double tStart, double tEnd,
                           const SkOpSegment* coinSeg, const SkOpSegment* oppSeg, bool* added,
                           const SkOpPtT* over1e, const SkOpPtT* over2e) const;
#endif
    void fixUp(SkCoincidentSpans* coin, SkOpPtT* deleted, const SkOpPtT* kept);
    void markCollapsed(SkCoincidentSpans* head, SkOpPtT* test);
    bool overlap(const SkOpPtT* coinStart1, const SkOpPtT* coinEnd1,
                 const SkOpPtT* coinStart2, const SkOpPtT* coinEnd2,
                 double* overS, double* overE) const;
    bool release(SkCoincidentSpans* coin, SkCoincidentSpans* );
    void releaseDeleted(SkCoincidentSpans* );
    void restoreHead();
    bool testForCoincidence(const SkCoincidentSpans* outer, const SkOpPtT* testS,
                            const SkOpPtT* testE) const;
    // return coinPtT->segment()->t mapped from overS->fT <= t <= overE->fT
    static double TRange(const SkOpPtT* overS, double t, const SkOpSegment* coinPtT
                         SkDEBUGPARAMS(const SkOpPtT* overE));

    SkCoincidentSpans* fHead;
    SkCoincidentSpans* fTop;
    SkOpGlobalState* fGlobalState;
    bool fContinue;
    bool fSpanDeleted;
    bool fPtAllocated;
    bool fCoinExtended;
    bool fSpanMerged;
};

#endif
