/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsTSect.h"

template<typename TCurve>
const SkTSpan<TCurve>* SkTSect<TCurve>::debugSpan(int id) const {
    const SkTSpan<TCurve>* test = fHead;
    do {
        if (test->debugID() == id) {
            return test;
        }
    } while ((test = test->next()));
#ifndef SK_RELEASE
    test = fOppSect->fHead;
    do {
        if (test->debugID() == id) {
            return test;
        }
    } while ((test = test->next()));
#endif
    return NULL;
}

template<typename TCurve>
const SkTSpan<TCurve>* SkTSect<TCurve>::debugT(double t) const {
    const SkTSpan<TCurve>* test = fHead;
    const SkTSpan<TCurve>* closest = NULL;
    double bestDist = DBL_MAX;
    do {
        if (between(test->fStartT, t, test->fEndT)) {
            return test;
        }
        double testDist = SkTMin(fabs(test->fStartT - t), fabs(test->fEndT - t));
        if (bestDist > testDist) {
            bestDist = testDist;
            closest = test;
        }
    } while ((test = test->next()));
    SkASSERT(closest);
    return closest;
}

template<typename TCurve>
void SkTSect<TCurve>::dump() const {
    dumpCommon(fHead);
}

extern int gDumpTSectNum;

template<typename TCurve>
void SkTSect<TCurve>::dumpBoth(SkTSect* opp) const {
#if DEBUG_T_SECT_DUMP <= 2
#if DEBUG_T_SECT_DUMP == 2
    SkDebugf("%d ", ++gDumpTSectNum);
#endif
    this->dump();
    SkDebugf(" ");
    opp->dump();
    SkDebugf("\n");
#elif DEBUG_T_SECT_DUMP == 3
    SkDebugf("<div id=\"sect%d\">\n", ++gDumpTSectNum);
    if (this->fHead) {
        this->dumpCurves();
    }
    if (opp->fHead) {
        PATH_OPS_DEBUG_CODE(opp->dumpCurves());
    }
    SkDebugf("</div>\n\n");
#endif
}

template<typename TCurve>
void SkTSect<TCurve>::dumpBounds(int id) const {
    const SkTSpan<TCurve>* bounded = debugSpan(id);
    if (!bounded) {
        SkDebugf("no span matches %d\n", id);
        return;
    }
    const SkTSpan<TCurve>* test = bounded->debugOpp()->fHead;
    do {
        if (test->findOppSpan(bounded)) {
            test->dump();
        }
    } while ((test = test->next()));
}

template<typename TCurve>
void SkTSect<TCurve>::dumpCoin() const {
    dumpCommon(fCoincident);
}

template<typename TCurve>
void SkTSect<TCurve>::dumpCoinCurves() const {
    dumpCommonCurves(fCoincident);
}

template<typename TCurve>
void SkTSect<TCurve>::dumpCommon(const SkTSpan<TCurve>* test) const {
    SkDebugf("id=%d", debugID());
    if (!test) {
        SkDebugf(" (empty)");
        return;
    }
    do {
        SkDebugf(" ");
        test->dump();
    } while ((test = test->next()));
}

template<typename TCurve>
void SkTSect<TCurve>::dumpCommonCurves(const SkTSpan<TCurve>* test) const {
    do {
        test->fPart.dumpID(test->debugID());
    } while ((test = test->next()));
}

template<typename TCurve>
void SkTSect<TCurve>::dumpCurves() const {
    dumpCommonCurves(fHead);
}

template<typename TCurve>
const SkTSpan<TCurve>* SkTSpan<TCurve>::debugSpan(int id) const {
    return PATH_OPS_DEBUG_RELEASE(fDebugSect->debugSpan(id), NULL);
}

template<typename TCurve>
const SkTSpan<TCurve>* SkTSpan<TCurve>::debugT(double t) const {
    return PATH_OPS_DEBUG_RELEASE(fDebugSect->debugT(t), NULL);
}

template<typename TCurve>
void SkTSpan<TCurve>::dump() const {
    dumpID();
    SkDebugf("=(%g,%g) [", fStartT, fEndT);
    const SkTSpanBounded<TCurve>* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* span = testBounded->fBounded;
        const SkTSpanBounded<TCurve>* next = testBounded->fNext;
        span->dumpID();
        if (next) {
            SkDebugf(",");
        }
        testBounded = next;
    }
    SkDebugf("]");
}

template<typename TCurve>
void SkTSpan<TCurve>::dumpBounds(int id) const {
    PATH_OPS_DEBUG_CODE(fDebugSect->dumpBounds(id));
}

template<typename TCurve>
void SkTSpan<TCurve>::dumpID() const {
    if (fCoinStart.isCoincident()) {
        SkDebugf("%c", '*');
    }
    SkDebugf("%d", debugID());
    if (fCoinEnd.isCoincident()) {
        SkDebugf("%c", '*');
    }
}
