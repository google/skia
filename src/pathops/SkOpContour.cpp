/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "SkOpContour.h"
#include "SkOpTAllocator.h"
#include "SkPathWriter.h"
#include "SkReduceOrder.h"
#include "SkTSort.h"

void SkOpContour::addCurve(SkPath::Verb verb, const SkPoint pts[4], SkChunkAlloc* allocator) {
    switch (verb) {
        case SkPath::kLine_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 2);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 2);
            appendSegment(allocator).addLine(ptStorage, this);
        } break;
        case SkPath::kQuad_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 3);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 3);
            appendSegment(allocator).addQuad(ptStorage, this);
        } break;
        case SkPath::kCubic_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 4);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 4);
            appendSegment(allocator).addCubic(ptStorage, this);
        } break;
        default:
            SkASSERT(0);
    }
}

SkOpSegment* SkOpContour::nonVerticalSegment(SkOpSpanBase** start, SkOpSpanBase** end) {
    int segmentCount = fSortedSegments.count();
    SkASSERT(segmentCount > 0);
    for (int sortedIndex = fFirstSorted; sortedIndex < segmentCount; ++sortedIndex) {
        SkOpSegment* testSegment = fSortedSegments[sortedIndex];
        if (testSegment->done()) {
            continue;
        }
        SkOpSpanBase* span = testSegment->head();
        SkOpSpanBase* testS, * testE;
        while (SkOpSegment::NextCandidate(span, &testS, &testE)) {
            if (!testSegment->isVertical(testS, testE)) {
                *start = testS;
                *end = testE;
                return testSegment;
            }
            span = span->upCast()->next();
        }
    }
    return NULL;
}

void SkOpContour::toPath(SkPathWriter* path) const {
    const SkPoint& pt = fHead.pts()[0];
    path->deferredMove(pt);
    const SkOpSegment* segment = &fHead;
    do {
        segment->addCurveTo(segment->head(), segment->tail(), path, true);
    } while ((segment = segment->next()));
    path->close();
}

void SkOpContour::topSortableSegment(const SkPoint& topLeft, SkPoint* bestXY,
        SkOpSegment** topStart) {
    int segmentCount = fSortedSegments.count();
    SkASSERT(segmentCount > 0);
    int sortedIndex = fFirstSorted;
    fDone = true;  // may be cleared below
    for ( ; sortedIndex < segmentCount; ++sortedIndex) {
        SkOpSegment* testSegment = fSortedSegments[sortedIndex];
        if (testSegment->done()) {
            if (sortedIndex == fFirstSorted) {
                ++fFirstSorted;
            }
            continue;
        }
        fDone = false;
        SkPoint testXY = testSegment->activeLeftTop(NULL);
        if (*topStart) {
            if (testXY.fY < topLeft.fY) {
                continue;
            }
            if (testXY.fY == topLeft.fY && testXY.fX < topLeft.fX) {
                continue;
            }
            if (bestXY->fY < testXY.fY) {
                continue;
            }
            if (bestXY->fY == testXY.fY && bestXY->fX < testXY.fX) {
                continue;
            }
        }
        *topStart = testSegment;
        *bestXY = testXY;
    }
}

SkOpSegment* SkOpContour::undoneSegment(SkOpSpanBase** startPtr, SkOpSpanBase** endPtr) {
    SkOpSegment* segment = &fHead;
    do {
        if (segment->done()) {
            continue;
        }
        segment->undoneSpan(startPtr, endPtr);
        return segment;
    } while ((segment = segment->next()));
    return NULL;
}
