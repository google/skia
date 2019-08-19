/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathIter_DEFINED
#define SkPathIter_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"

class SkPath;

class SkPathIter {
public:
    SkPathIter(const SkPoint pts[], const uint8_t verbs[], const SkScalar conicW[], int count)
        : fCurrPts(pts)
        , fNextPts(pts + 1)
        , fConicW(conicW)
        , fVerbs(verbs)
        , fStopVerbs(verbs - count)
        , fPrevPtsPerVerb(0)
    {
        if (count > 0) {
            SkASSERT(kMove_SkPathVerb == verbs[-1]);
        }
    }

    SkPathIter(const SkPath&);

    bool            done() const { return fStopVerbs == fVerbs; }
    SkPathVerb      currVerb() const { return (SkPathVerb)fVerbs[0]; }
    const SkPoint*  currPts() const { return fCurrPts; }
    SkScalar        currConicW() const { return *fConicW; }

#if 0
    bool next() {
        if (fStopVerbs == fVerbs) {
            return false;
        }

        static const uint8_t gPtsPerVerb[] = {
            1,  // Move
            1,  // Line
            2,  // Quad
            2,  // Conic
            3,  // Cubic
            0,  // Close
        };

        fCurrPts += fPrevPtsPerVerb;
        SkPathVerb verb = (SkPathVerb)(*--fVerbs);
        fNextPts += (fPrevPtsPerVerb = gPtsPerVerb[verb]);
        fConicW += (kConic_SkPathVerb == verb);
        return true;
    }
#else
    bool next();
#endif

private:
    const SkPoint*  fCurrPts;
    const SkPoint*  fNextPts;
    const SkScalar* fConicW;
    const uint8_t*  fVerbs;
    const uint8_t*  fStopVerbs;
    int             fPrevPtsPerVerb;
};

#endif
