/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsTSectDebug_DEFINED
#define PathOpsTSectDebug_DEFINED

#include "SkPathOpsTSect.h"

char SkTCoincident::dumpIsCoincidentStr() const {
    if (!!fMatch != fMatch) {
        return '?';
    }
    return fMatch ? '*' : 0;
}

void SkTCoincident::dump() const {
    SkDebugf("t=%1.9g pt=(%1.9g,%1.9g)%s\n", fPerpT, fPerpPt.fX, fPerpPt.fY,
            fMatch ? " match" : "");
}

const SkTSpan* SkTSect::debugSpan(int id) const {
    const SkTSpan* test = fHead;
    do {
        if (test->debugID() == id) {
            return test;
        }
    } while ((test = test->next()));
    return nullptr;
}

const SkTSpan* SkTSect::debugT(double t) const {
    const SkTSpan* test = fHead;
    const SkTSpan* closest = nullptr;
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

void SkTSect::dump() const {
    dumpCommon(fHead);
}

extern int gDumpTSectNum;

void SkTSect::dumpBoth(SkTSect* opp) const {
#if DEBUG_T_SECT_DUMP <= 2
#if DEBUG_T_SECT_DUMP == 2
    SkDebugf("%d ", ++gDumpTSectNum);
#endif
    this->dump();
    SkDebugf("\n");
    opp->dump();
    SkDebugf("\n");
#elif DEBUG_T_SECT_DUMP == 3
    SkDebugf("<div id=\"sect%d\">\n", ++gDumpTSectNum);
    if (this->fHead) {
        this->dumpCurves();
    }
    if (opp->fHead) {
        opp->dumpCurves();
    }
    SkDebugf("</div>\n\n");
#endif
}

void SkTSect::dumpBounded(int id) const {
    const SkTSpan* bounded = debugSpan(id);
    if (!bounded) {
        SkDebugf("no span matches %d\n", id);
        return;
    }
    const SkTSpan* test = bounded->debugOpp()->fHead;
    do {
        if (test->findOppSpan(bounded)) {
            test->dump();
            SkDebugf(" ");
        }
    } while ((test = test->next()));
    SkDebugf("\n");
}

void SkTSect::dumpBounds() const {
    const SkTSpan* test = fHead;
    do {
        test->dumpBounds();
    } while ((test = test->next()));
}

void SkTSect::dumpCoin() const {
    dumpCommon(fCoincident);
}

void SkTSect::dumpCoinCurves() const {
    dumpCommonCurves(fCoincident);
}

void SkTSect::dumpCommon(const SkTSpan* test) const {
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

void SkTSect::dumpCommonCurves(const SkTSpan* test) const {
    do {
        test->fPart->dumpID(test->debugID());
    } while ((test = test->next()));
}

void SkTSect::dumpCurves() const {
    dumpCommonCurves(fHead);
}

const SkTSpan* SkTSpan::debugSpan(int id) const {
    return SkDEBUGRELEASE(fDebugSect->debugSpan(id), nullptr);
}

const SkTSpan* SkTSpan::debugT(double t) const {
    return SkDEBUGRELEASE(fDebugSect->debugT(t), nullptr);
}

void SkTSpan::dumpAll() const {
    dumpID();
    SkDebugf("=(%g,%g) [", fStartT, fEndT);
    const SkTSpanBounded* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* span = testBounded->fBounded;
        const SkTSpanBounded* next = testBounded->fNext;
        span->dumpID();
        SkDebugf("=(%g,%g)", span->fStartT, span->fEndT);
        if (next) {
            SkDebugf(" ");
        }
        testBounded = next;
    }
    SkDebugf("]\n");
}

void SkTSpan::dump() const {
    dumpID();
    SkDebugf("=(%g,%g) [", fStartT, fEndT);
    const SkTSpanBounded* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* span = testBounded->fBounded;
        const SkTSpanBounded* next = testBounded->fNext;
        span->dumpID();
        if (next) {
            SkDebugf(",");
        }
        testBounded = next;
    }
    SkDebugf("]");
}

void SkTSpan::dumpBounded(int id) const {
    SkDEBUGCODE(fDebugSect->dumpBounded(id));
}

void SkTSpan::dumpBounds() const {
    dumpID();
    SkDebugf(" bounds=(%1.9g,%1.9g, %1.9g,%1.9g) boundsMax=%1.9g%s\n",
            fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom, fBoundsMax,
            fCollapsed ? " collapsed" : "");
}

void SkTSpan::dumpCoin() const {
    dumpID();
    SkDebugf(" coinStart ");
    fCoinStart.dump();
    SkDebugf(" coinEnd ");
    fCoinEnd.dump();
}

void SkTSpan::dumpID() const {
    char cS = fCoinStart.dumpIsCoincidentStr();
    if (cS) {
        SkDebugf("%c", cS);
    }
    SkDebugf("%d", debugID());
    char cE = fCoinEnd.dumpIsCoincidentStr();
    if (cE) {
        SkDebugf("%c", cE);
    }
}
#endif  // PathOpsTSectDebug_DEFINED
