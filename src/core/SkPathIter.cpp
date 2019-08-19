/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathIter.h"

bool SkPathIter::next() {
    if (fStopVerbs == fVerbs) {
        return false;
    }

#if 0
    const uint8_t gPtsPerVerb[] = {
        1,  // Move
        1,  // Line
        2,  // Quad
        2,  // Conic
        3,  // Cubic
        0,  // Close
    };
#endif

    fCurrPts += fPrevPtsPerVerb;
    SkPathVerb verb = (SkPathVerb)(*--fVerbs);
#if 0
    fPrevPtsPerVerb = gPtsPerVerb[verb];
#elif 0
    fPrevPtsPerVerb = ((verb >> 1) + 1) & -(kClose_SkPathVerb == verb);
#else
    fPrevPtsPerVerb = (verb >> 4) & 7;
#endif
    fNextPts += fPrevPtsPerVerb;
    fConicW += (verb >> 7);//(kConic_SkPathVerb == verb);
    return true;
}

