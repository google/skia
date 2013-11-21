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
    int fWindSum;  // accumulated from contours surrounding this one.
    int fOppSum;  // for binary operators: the opposite winding sum
    int fWindValue;  // 0 == canceled; 1 == normal; >1 == coincident
    int fOppValue;  // normally 0 -- when binary coincident edges combine, opp value goes here
    bool fDone;  // if set, this span to next higher T has been processed
    bool fUnsortableStart;  // set when start is part of an unsortable pair
    bool fUnsortableEnd;  // set when end is part of an unsortable pair
    bool fSmall;   // if set, consecutive points are almost equal
    bool fTiny;  // if set, span may still be considered once for edge following
    bool fLoop;  // set when a cubic loops back to this point

#ifdef SK_DEBUG
    void dump() const;
#endif
};

#endif
