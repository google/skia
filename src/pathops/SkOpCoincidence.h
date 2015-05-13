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

class SkOpPtT;

struct SkCoincidentSpans {
    SkCoincidentSpans* fNext;
    SkOpPtT* fCoinPtTStart;
    SkOpPtT* fCoinPtTEnd;
    SkOpPtT* fOppPtTStart;
    SkOpPtT* fOppPtTEnd;
    bool fFlipped;

    void dump() const;
};

class SkOpCoincidence {
public:
    SkOpCoincidence()
        : fHead(NULL) {
    }

    void add(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
             SkOpPtT* oppPtTEnd, SkChunkAlloc* allocator);
    bool addMissing(SkChunkAlloc* allocator);
    void addMissing(SkCoincidentSpans* check, SkChunkAlloc* allocator);
    bool apply();
    bool contains(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
                  SkOpPtT* oppPtTEnd, bool flipped);
    void debugShowCoincidence() const;
    void detach(SkCoincidentSpans* );
    void dump() const;
    void expand();
    bool extend(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
        SkOpPtT* oppPtTEnd);
    void fixUp(SkOpPtT* deleted, SkOpPtT* kept);
    void mark();

private:
    bool addIfMissing(const SkOpPtT* over1s, const SkOpPtT* over1e,
                      const SkOpPtT* over2s, const SkOpPtT* over2e, double tStart, double tEnd,
                      SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
                      SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd,
                      SkChunkAlloc* allocator);
    bool overlap(const SkOpPtT* coinStart1, const SkOpPtT* coinEnd1,
                 const SkOpPtT* coinStart2, const SkOpPtT* coinEnd2,
                 double* overS, double* overE) const;

    SkCoincidentSpans* fHead;
};

#endif
