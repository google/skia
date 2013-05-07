/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsCommon_DEFINED
#define SkPathOpsCommon_DEFINED

#include "SkOpContour.h"

class SkPathWriter;

void Assemble(const SkPathWriter& path, SkPathWriter* simple);
SkOpSegment* FindChase(SkTDArray<SkOpSpan*>& chase, int& tIndex, int& endIndex);
SkOpSegment* FindSortableTop(const SkTDArray<SkOpContour*>& contourList, bool* firstContour,
                             int* index, int* endIndex, SkPoint* topLeft, bool* unsortable,
                             bool* done, bool binary);
SkOpSegment* FindUndone(SkTDArray<SkOpContour*>& contourList, int* start, int* end);
void FixOtherTIndex(SkTDArray<SkOpContour*>* contourList);
void MakeContourList(SkTArray<SkOpContour>& contours, SkTDArray<SkOpContour*>& list,
                     bool evenOdd, bool oppEvenOdd);
void SortSegments(SkTDArray<SkOpContour*>* contourList);

#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
void DebugShowActiveSpans(SkTDArray<SkOpContour*>& contourList);
#endif

#endif
