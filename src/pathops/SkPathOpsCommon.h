/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsCommon_DEFINED
#define SkPathOpsCommon_DEFINED

#include "include/pathops/SkPathOps.h"
#include "src/pathops/SkPathOpsTypes.h"

class SkOpAngle;
class SkOpCoincidence;
class SkOpContourHead;
class SkOpSegment;
class SkOpSpan;
class SkOpSpanBase;
class SkPath;
struct SkRect;

template <typename T> class SkTDArray;

const SkOpAngle* AngleWinding(SkOpSpanBase* start, SkOpSpanBase* end, int* windingPtr,
                              bool* sortable);
SkOpSegment* FindChase(SkTDArray<SkOpSpanBase*>* chase, SkOpSpanBase** startPtr,
                       SkOpSpanBase** endPtr);
SkOpSpan* FindSortableTop(SkOpContourHead* );
SkOpSpan* FindUndone(SkOpContourHead* );
bool FixWinding(SkPath* path);
bool SortContourList(SkOpContourHead** , bool evenOdd, bool oppEvenOdd);
bool HandleCoincidence(SkOpContourHead* , SkOpCoincidence* );
bool OpDebug(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result
             SkDEBUGPARAMS(bool skipAssert)
             SkDEBUGPARAMS(const char* testName));

bool ComputeTightBounds(const SkPath&, SkRect*);

#endif
