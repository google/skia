/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsCommon_DEFINED
#define SkPathOpsCommon_DEFINED

#include "SkOpAngle.h"
#include "SkTDArray.h"

class SkOpCoincidence;
class SkOpContour;
class SkPathWriter;

void Assemble(const SkPathWriter& path, SkPathWriter* simple);
SkOpSegment* FindChase(SkTDArray<SkOpSpanBase*>* chase, SkOpSpanBase** startPtr,
                       SkOpSpanBase** endPtr);
SkOpSegment* FindSortableTop(const SkTDArray<SkOpContour*>& , bool firstPass,
                              SkOpAngle::IncludeType , bool* firstContour, SkOpSpanBase** index,
                              SkOpSpanBase** endIndex, SkDPoint* topLeft, bool* unsortable,
                              bool* done, bool* onlyVertical, SkChunkAlloc* );
SkOpSegment* FindUndone(SkTDArray<SkOpContour*>& contourList, SkOpSpanBase** startPtr,
                         SkOpSpanBase** endPtr);
void MakeContourList(SkOpContour* , SkTDArray<SkOpContour*>& list,
                     bool evenOdd, bool oppEvenOdd);
bool HandleCoincidence(SkTDArray<SkOpContour*>* , SkOpCoincidence* , SkChunkAlloc* ,
                       SkOpGlobalState* );

#if DEBUG_ACTIVE_SPANS
void DebugShowActiveSpans(SkTDArray<SkOpContour*>& contourList);
#endif

#endif
