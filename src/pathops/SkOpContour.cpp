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

SkOpSegment* SkOpContour::addCurve(SkPath::Verb verb, const SkPoint pts[4], SkScalar weight) {
    SkChunkAlloc* allocator = this->globalState()->allocator();
    switch (verb) {
        case SkPath::kLine_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 2);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 2);
            return appendSegment().addLine(ptStorage, this);
        } break;
        case SkPath::kQuad_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 3);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 3);
            return appendSegment().addQuad(ptStorage, this);
        } break;
        case SkPath::kConic_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 3);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 3);
            return appendSegment().addConic(ptStorage, weight, this);
        } break;
        case SkPath::kCubic_Verb: {
            SkPoint* ptStorage = SkOpTAllocator<SkPoint>::AllocateArray(allocator, 4);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 4);
            return appendSegment().addCubic(ptStorage, this);
        } break;
        default:
            SkASSERT(0);
    }
    return nullptr;
}

void SkOpContour::toPath(SkPathWriter* path) const {
    const SkOpSegment* segment = &fHead;
    do {
        SkAssertResult(segment->addCurveTo(segment->head(), segment->tail(), path));
    } while ((segment = segment->next()));
    path->finishContour();
    path->assemble();
}

void SkOpContour::toReversePath(SkPathWriter* path) const {
    const SkOpSegment* segment = fTail;
    do {
        SkAssertResult(segment->addCurveTo(segment->tail(), segment->head(), path));
    } while ((segment = segment->prev()));
    path->finishContour();
    path->assemble();
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
    return nullptr;
}
