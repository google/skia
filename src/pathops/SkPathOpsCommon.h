/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsCommon_DEFINED
#define SkPathOpsCommon_DEFINED

#include "SkOpAngle.h"
#include "SkOpContour.h"
#include "SkTDArray.h"

class SkPathWriter;

void Assemble(const SkPathWriter& path, SkPathWriter* simple);
// FIXME: find chase uses insert, so it can't be converted to SkTArray yet
SkOpSegment* FindChase(SkTDArray<SkOpSpan*>& chase, int& tIndex, int& endIndex);
SkOpSegment* FindSortableTop(const SkTArray<SkOpContour*, true>& , SkOpAngle::IncludeType ,
                             bool* firstContour, int* index, int* endIndex, SkPoint* topLeft,
                             bool* unsortable, bool* done);
SkOpSegment* FindUndone(SkTArray<SkOpContour*, true>& contourList, int* start, int* end);
void MakeContourList(SkTArray<SkOpContour>& contours, SkTArray<SkOpContour*, true>& list,
                     bool evenOdd, bool oppEvenOdd);
void HandleCoincidence(SkTArray<SkOpContour*, true>* , int );

#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
void DebugShowActiveSpans(SkTArray<SkOpContour*, true>& contourList);
#endif

#endif
