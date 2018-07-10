/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "SkOpContour.h"
#include "SkPathWriter.h"
#include "SkReduceOrder.h"
#include "SkTSort.h"

void SkOpContour::toPath(SkPathWriter* path) const {
    if (!this->count()) {
        return;
    }
    const SkOpSegment* segment = &fHead;
    do {
        SkAssertResult(segment->addCurveTo(segment->head(), segment->tail(), path));
    } while ((segment = segment->next()));
    path->finishContour();
    path->assemble(nullptr);
}

void SkOpContour::toReversePath(SkPathWriter* path) const {
    const SkOpSegment* segment = fTail;
    do {
        SkAssertResult(segment->addCurveTo(segment->tail(), segment->head(), path));
    } while ((segment = segment->prev()));
    path->finishContour();
    path->assemble(nullptr);
}

SkOpSpan* SkOpContour::undoneSpan() {
    SkOpSegment* testSegment = &fHead;
    do {
        if (testSegment->done()) {
            continue;
        }
        return testSegment->undoneSpan();
    } while ((testSegment = testSegment->next()));
    fDone = true;
    return nullptr;
}

void SkOpContourBuilder::addConic(SkPoint pts[3], SkScalar weight) {
    this->flush();
    fContour->addConic(pts, weight);
}

void SkOpContourBuilder::addCubic(SkPoint pts[4]) {
    this->flush();
    fContour->addCubic(pts);
}

void SkOpContourBuilder::addCurve(SkPath::Verb verb, const SkPoint pts[4], SkScalar weight) {
    if (SkPath::kLine_Verb == verb) {
        this->addLine(pts);
        return;
    }
    SkArenaAlloc* allocator = fContour->globalState()->allocator();
    switch (verb) {
        case SkPath::kQuad_Verb: {
            SkPoint* ptStorage = allocator->makeArrayDefault<SkPoint>(3);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 3);
            this->addQuad(ptStorage);
        } break;
        case SkPath::kConic_Verb: {
            SkPoint* ptStorage = allocator->makeArrayDefault<SkPoint>(3);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 3);
            this->addConic(ptStorage, weight);
        } break;
        case SkPath::kCubic_Verb: {
            SkPoint* ptStorage = allocator->makeArrayDefault<SkPoint>(4);
            memcpy(ptStorage, pts, sizeof(SkPoint) * 4);
            this->addCubic(ptStorage);
        } break;
        default:
            SkASSERT(0);
    }
}

void SkOpContourBuilder::addLine(const SkPoint pts[2]) {
    // if the previous line added is the exact opposite, eliminate both
    if (fLastIsLine) {
        if (fLastLine[0] == pts[1] && fLastLine[1] == pts[0]) {
            fLastIsLine = false;
            return;
        } else {
            flush();
        }
    }
    memcpy(fLastLine, pts, sizeof(fLastLine));
    fLastIsLine = true;
}

void SkOpContourBuilder::addQuad(SkPoint pts[3]) {
    this->flush();
    fContour->addQuad(pts);
}

void SkOpContourBuilder::flush() {
    if (!fLastIsLine)
        return;
    SkArenaAlloc* allocator = fContour->globalState()->allocator();
    SkPoint* ptStorage = allocator->makeArrayDefault<SkPoint>(2);
    memcpy(ptStorage, fLastLine, sizeof(fLastLine));
    (void) fContour->addLine(ptStorage);
    fLastIsLine = false;
}

float SkOpContourHead::distHops(const SkPoint& start, const SkPoint& end, int hops) const {
    const SkOpSpanBase* ptTStart = this->find(start);
    const SkOpSpanBase* ptTEnd = this->find(end);
    // find connection from start to end not exceeding hops
    // maybe can reuse fSpanAdds as follows
    // pass a unique negative into distHops
    // store it in fSpanAdds, so that same dist node is only visited once
    // track traversal on local stack
    // if start -> end is found, stack shows the way
    // return Euclidean distance from start to end (may be debugging only)
    return 0;
}

const SkOpSpanBase* SkOpContourHead::find(const SkPoint& pt) const {
    const SkOpContour* next = this;
    do {
        if (!next->count()) {
            continue;
        }
        const SkOpSpanBase* result = next->findPt(pt);
        if (result) {
            return result;
        }
    } while ((next = next->next()));
    SkASSERT(0);
    return nullptr;
}
