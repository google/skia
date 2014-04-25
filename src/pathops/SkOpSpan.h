/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpSpan_DEFINED
#define SkOpSpan_DEFINED

#include "SkPoint.h"

class SkOpSegment;

struct SkOpSpan {
    SkOpSegment* fOther;
    SkPoint fPt;  // computed when the curves are intersected
    double fT;
    double fOtherT;  // value at fOther[fOtherIndex].fT
    int fOtherIndex;  // can't be used during intersection
    int fFromAngleIndex;  // (if t > 0) index into segment's angle array going negative in t
    int fToAngleIndex;  // (if t < 1) index into segment's angle array going positive in t
    int fWindSum;  // accumulated from contours surrounding this one.
    int fOppSum;  // for binary operators: the opposite winding sum
    int fWindValue;  // 0 == canceled; 1 == normal; >1 == coincident
    int fOppValue;  // normally 0 -- when binary coincident edges combine, opp value goes here
    bool fChased;  // set after span has been added to chase array
    bool fDone;  // if set, this span to next higher T has been processed
    bool fLoop;  // set when a cubic loops back to this point
    bool fSmall;   // if set, consecutive points are almost equal
    bool fTiny;  // if set, consecutive points are equal but consecutive ts are not precisely equal

    // available to testing only
    const SkOpSegment* debugToSegment(ptrdiff_t* ) const;
    void dump() const;
    void dumpOne() const;
};

#endif
