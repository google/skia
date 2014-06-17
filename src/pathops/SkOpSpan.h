/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpSpan_DEFINED
#define SkOpSpan_DEFINED

#include "SkPoint.h"

class SkOpAngle;
class SkOpSegment;

struct SkOpSpan {
    SkPoint fPt;  // computed when the curves are intersected
    double fT;
    double fOtherT;  // value at fOther[fOtherIndex].fT
    SkOpSegment* fOther;
    SkOpAngle* fFromAngle;  // (if t > 0) index into segment's angle array going negative in t
    SkOpAngle* fToAngle;  // (if t < 1) index into segment's angle array going positive in t
    int fOtherIndex;  // can't be used during intersection
    int fWindSum;  // accumulated from contours surrounding this one.
    int fOppSum;  // for binary operators: the opposite winding sum
    int fWindValue;  // 0 == canceled; 1 == normal; >1 == coincident
    int fOppValue;  // normally 0 -- when binary coincident edges combine, opp value goes here
    bool fChased;  // set after span has been added to chase array
    bool fCoincident;  // set if span is bumped -- if set additional points aren't inserted
    bool fDone;  // if set, this span to next higher T has been processed
    bool fLoop;  // set when a cubic loops back to this point
    bool fMultiple;  // set if this is one of mutiple spans with identical t and pt values
    bool fNear;  // set if opposite end point is near but not equal to this one
    bool fSmall;   // if set, consecutive points are almost equal
    bool fTiny;  // if set, consecutive points are equal but consecutive ts are not precisely equal

    // available to testing only
    const SkOpSegment* debugToSegment(ptrdiff_t* ) const;
    void dump() const;
    void dumpOne() const;
};

#endif
