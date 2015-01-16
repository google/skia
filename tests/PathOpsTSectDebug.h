/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsTSect.h"

template<typename TCurve>
void SkTSect<TCurve>::dump() const {
    SkDebugf("id=%d", debugID());
    const SkTSpan<TCurve>* test = fHead;
    if (!test) {
        SkDebugf(" (empty)");
        return;
    }
    do {
        SkDebugf(" ");
        test->dump(this);
    } while ((test = test->next()));
}

template<typename TCurve>
void SkTSect<TCurve>::dumpBoth(const SkTSect& opp) const {
    dump();
    SkDebugf(" ");
    opp.dump();
    SkDebugf("\n");
}

template<typename TCurve>
void SkTSect<TCurve>::dumpBoth(const SkTSect* opp) const {
    dumpBoth(*opp);
}

template<typename TCurve>
void SkTSect<TCurve>::dumpCurves() const {
    const SkTSpan<TCurve>* test = fHead;
    do {
        test->fPart.dump();
    } while ((test = test->next()));
}

#if !DEBUG_T_SECT
template<typename TCurve>
int SkTSpan<TCurve>::debugID(const SkTSect<TCurve>* sect) const {
    if (!sect) {
        return -1;
    }
    int id = 1;
    const SkTSpan* test = sect->fHead;
    while (test && test != this) {
        ++id;
        test = test->fNext;
    }
    return id;
}
#endif

template<typename TCurve>
void SkTSpan<TCurve>::dumpID(const SkTSect<TCurve>* sect) const {
    if (fCoinStart.isCoincident()) {
        SkDebugf("%c", '*');
    }
    SkDebugf("%d", debugID(sect));
    if (fCoinEnd.isCoincident()) {
        SkDebugf("%c", '*');
    }
}

template<typename TCurve>
void SkTSpan<TCurve>::dump(const SkTSect<TCurve>* sect) const {
    dumpID(sect);
    SkDebugf("=(%g,%g) [", fStartT, fEndT);
    for (int index = 0; index < fBounded.count(); ++index) {
        SkTSpan* span = fBounded[index];
        span->dumpID(sect);
        if (index < fBounded.count() - 1) {
            SkDebugf(",");
        }
    }
    SkDebugf("]");
}
