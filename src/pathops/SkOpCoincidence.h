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
};

class SkOpCoincidence {
public:
    SkOpCoincidence()
        : fHead(NULL) {
    }

    void add(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
             SkOpPtT* oppPtTEnd, bool flipped, SkChunkAlloc* allocator);
    void apply();
    bool contains(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
                  SkOpPtT* oppPtTEnd, bool flipped);
    void dump() const;
    void mark();

    SkCoincidentSpans* fHead;
};

#endif
