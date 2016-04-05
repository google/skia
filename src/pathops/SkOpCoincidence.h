/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpCoincidence_DEFINED
#define SkOpCoincidence_DEFINED

#include "SkOpTAllocator.h"
#include "SkOpSpan.h"
#include "SkPathOpsTypes.h"

class SkOpPtT;

struct SkCoincidentSpans {
    SkCoincidentSpans* fNext;
    SkOpPtT* fCoinPtTStart;
    SkOpPtT* fCoinPtTEnd;
    SkOpPtT* fOppPtTStart;
    SkOpPtT* fOppPtTEnd;
    bool fFlipped;
    SkDEBUGCODE(int fID);

    int debugID() const {
        return SkDEBUGRELEASE(fID, -1);
    }

    void dump() const;
};

class SkOpCoincidence {
public:
    SkOpCoincidence()
        : fHead(nullptr)
        , fTop(nullptr)
        SkDEBUGPARAMS(fDebugState(nullptr))
        {
    }

    void add(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
             SkOpPtT* oppPtTEnd, SkChunkAlloc* allocator);
    bool addExpanded(SkChunkAlloc* allocator  PATH_OPS_DEBUG_VALIDATE_PARAMS(SkOpGlobalState* ));
    bool addMissing(SkChunkAlloc* allocator);
    bool apply();
    bool contains(const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
                  const SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd, bool flipped) const;

    void debugAddExpanded(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugAddMissing(const char* id, SkPathOpsDebug::GlitchLog* ) const;

    const SkOpAngle* debugAngle(int id) const {
        return SkDEBUGRELEASE(fDebugState->debugAngle(id), nullptr);
    }

    SkOpContour* debugContour(int id) {
        return SkDEBUGRELEASE(fDebugState->debugContour(id), nullptr);
    }

    bool debugExpand(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugMark(const char* id, SkPathOpsDebug::GlitchLog* ) const;

    const SkOpPtT* debugPtT(int id) const {
        return SkDEBUGRELEASE(fDebugState->debugPtT(id), nullptr);
    }

    const SkOpSegment* debugSegment(int id) const {
        return SkDEBUGRELEASE(fDebugState->debugSegment(id), nullptr);
    }

    void debugSetGlobalState(SkOpGlobalState* debugState) {
        SkDEBUGCODE(fDebugState = debugState);
    }

    void debugFixAligned(const char* id, SkPathOpsDebug::GlitchLog* ) const;
    void debugShowCoincidence() const;

    const SkOpSpanBase* debugSpan(int id) const {
        return SkDEBUGRELEASE(fDebugState->debugSpan(id), nullptr);
    }

    void release(SkCoincidentSpans* );
    void dump() const;
    bool expand();
    bool extend(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
        SkOpPtT* oppPtTEnd);
    void findOverlaps(SkOpCoincidence* , SkChunkAlloc* allocator) const;
    void fixAligned();
    void fixUp(SkOpPtT* deleted, SkOpPtT* kept);

    bool isEmpty() const {
        return !fHead;
    }

    bool mark();

private:
    bool addIfMissing(const SkCoincidentSpans* outer, SkOpPtT* over1s, SkOpPtT* over1e,
                      SkChunkAlloc* );
    bool addIfMissing(const SkOpPtT* over1s, const SkOpPtT* over1e,
                      const SkOpPtT* over2s, const SkOpPtT* over2e,
                      double tStart, double tEnd,
                      SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
                      SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd,
                      SkChunkAlloc* );
    void addOverlap(SkOpSegment* seg1, SkOpSegment* seg1o, SkOpSegment* seg2, SkOpSegment* seg2o,
                    SkOpPtT* overS, SkOpPtT* overE, SkChunkAlloc* );
    bool debugAddIfMissing(const SkCoincidentSpans* outer, const SkOpPtT* over1s, 
                           const SkOpPtT* over1e) const;
    bool debugAddIfMissing(const SkOpPtT* over1s, const SkOpPtT* over1e,
                           const SkOpPtT* over2s, const SkOpPtT* over2e,
                           double tStart, double tEnd,
                           SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
                           SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd) const;
    bool overlap(const SkOpPtT* coinStart1, const SkOpPtT* coinEnd1,
                 const SkOpPtT* coinStart2, const SkOpPtT* coinEnd2,
                 double* overS, double* overE) const;

    bool testForCoincidence(const SkCoincidentSpans* outer, const SkOpPtT* testS,
                            const SkOpPtT* testE) const;
    SkCoincidentSpans* fHead;
    SkCoincidentSpans* fTop;
    SkDEBUGCODE_(SkOpGlobalState* fDebugState);
};

#endif
