/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkAddIntersections.h"
#include "SkOpCoincidence.h"
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"
#include "SkPathWriter.h"

static bool bridgeWinding(SkOpContourHead* contourList, SkPathWriter* simple) {
    bool unsortable = false;
    do {
        SkOpSpan* span = FindSortableTop(contourList);
        if (!span) {
            break;
        }
        SkOpSegment* current = span->segment();
        SkOpSpanBase* start = span->next();
        SkOpSpanBase* end = span;
        SkTDArray<SkOpSpanBase*> chase;
        do {
            if (current->activeWinding(start, end)) {
                do {
                    if (!unsortable && current->done()) {
                          break;
                    }
                    SkASSERT(unsortable || !current->done());
                    SkOpSpanBase* nextStart = start;
                    SkOpSpanBase* nextEnd = end;
                    SkOpSegment* next = current->findNextWinding(&chase, &nextStart, &nextEnd,
                            &unsortable);
                    if (!next) {
                        break;
                    }
        #if DEBUG_FLOW
            SkDebugf("%s current id=%d from=(%1.9g,%1.9g) to=(%1.9g,%1.9g)\n", __FUNCTION__,
                    current->debugID(), start->pt().fX, start->pt().fY,
                    end->pt().fX, end->pt().fY);
        #endif
                    if (!current->addCurveTo(start, end, simple)) {
                        return false;
                    }
                    current = next;
                    start = nextStart;
                    end = nextEnd;
                } while (!simple->isClosed() && (!unsortable || !start->starter(end)->done()));
                if (current->activeWinding(start, end) && !simple->isClosed()) {
                    SkOpSpan* spanStart = start->starter(end);
                    if (!spanStart->done()) {
                        if (!current->addCurveTo(start, end, simple)) {
                            return false;
                        }
                        current->markDone(spanStart);
                    }
                }
                simple->finishContour();
            } else {
                SkOpSpanBase* last = current->markAndChaseDone(start, end);
                if (last && !last->chased()) {
                    last->setChased(true);
                    SkASSERT(!SkPathOpsDebug::ChaseContains(chase, last));
                    *chase.append() = last;
#if DEBUG_WINDING
                    SkDebugf("%s chase.append id=%d", __FUNCTION__, last->segment()->debugID());
                    if (!last->final()) {
                         SkDebugf(" windSum=%d", last->upCast()->windSum());
                    }
                    SkDebugf("\n");
#endif
                }
            }
            current = FindChase(&chase, &start, &end);
            SkPathOpsDebug::ShowActiveSpans(contourList);
            if (!current) {
                break;
            }
        } while (true);
    } while (true);
    return true;
}

// returns true if all edges were processed
static bool bridgeXor(SkOpContourHead* contourList, SkPathWriter* simple) {
    bool unsortable = false;
    do {
        SkOpSpan* span = FindUndone(contourList);
        if (!span) {
            break;
        }
        SkOpSegment* current = span->segment();
        SkOpSpanBase* start = span->next();
        SkOpSpanBase* end = span;
        do {
            if (!unsortable && current->done()) {
                break;
            }
            SkASSERT(unsortable || !current->done());
            SkOpSpanBase* nextStart = start;
            SkOpSpanBase* nextEnd = end;
            SkOpSegment* next = current->findNextXor(&nextStart, &nextEnd,
                    &unsortable);
            if (!next) {
                break;
            }
        #if DEBUG_FLOW
            SkDebugf("%s current id=%d from=(%1.9g,%1.9g) to=(%1.9g,%1.9g)\n", __FUNCTION__,
                    current->debugID(), start->pt().fX, start->pt().fY,
                    end->pt().fX, end->pt().fY);
        #endif
            if (!current->addCurveTo(start, end, simple)) {
                return false;
            }
            current = next;
            start = nextStart;
            end = nextEnd;
        } while (!simple->isClosed() && (!unsortable || !start->starter(end)->done()));
        if (!simple->isClosed()) {
            SkOpSpan* spanStart = start->starter(end);
            if (!spanStart->done()) {
                if (!current->addCurveTo(start, end, simple)) {
                    return false;
                }
                current->markDone(spanStart);
            }
        }
        simple->finishContour();
        SkPathOpsDebug::ShowActiveSpans(contourList);
    } while (true);
    return true;
}

// FIXME : add this as a member of SkPath
bool SimplifyDebug(const SkPath& path, SkPath* result
        SkDEBUGPARAMS(bool skipAssert) SkDEBUGPARAMS(const char* testName)) {
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    SkPath::FillType fillType = path.isInverseFillType() ? SkPath::kInverseEvenOdd_FillType
            : SkPath::kEvenOdd_FillType;
    if (path.isConvex()) {
        if (result != &path) {
            *result = path;
        }
        result->setFillType(fillType);
        return true;
    }
    // turn path into list of segments
    char storage[4096];
    SkArenaAlloc allocator(storage);  // FIXME: constant-ize, tune
    SkOpContour contour;
    SkOpContourHead* contourList = static_cast<SkOpContourHead*>(&contour);
    SkOpGlobalState globalState(contourList, &allocator
            SkDEBUGPARAMS(skipAssert) SkDEBUGPARAMS(testName));
    SkOpCoincidence coincidence(&globalState);
#if DEBUG_DUMP_VERIFY
#ifndef SK_DEBUG
    const char* testName = "release";
#endif
    if (SkPathOpsDebug::gDumpOp) {
        SkPathOpsDebug::DumpSimplify(path, testName);
    }
#endif
    SkScalar scaleFactor = ScaleFactor(path);
    SkPath scaledPath;
    const SkPath* workingPath;
    if (scaleFactor > SK_Scalar1) {
        ScalePath(path, 1.f / scaleFactor, &scaledPath);
        workingPath = &scaledPath;
    } else {
        workingPath = &path;
    }
#if DEBUG_SORT
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    SkOpEdgeBuilder builder(*workingPath, contourList, &globalState);
    if (!builder.finish()) {
        return false;
    }
#if DEBUG_DUMP_SEGMENTS
    contour.dumpSegments();
#endif
    if (!SortContourList(&contourList, false, false)) {
        result->reset();
        result->setFillType(fillType);
        return true;
    }
    // find all intersections between segments
    SkOpContour* current = contourList;
    do {
        SkOpContour* next = current;
        while (AddIntersectTs(current, next, &coincidence)
                && (next = next->next()));
    } while ((current = current->next()));
#if DEBUG_VALIDATE
    globalState.setPhase(SkOpPhase::kWalking);
#endif
    bool success = HandleCoincidence(contourList, &coincidence);
#if DEBUG_COIN
    globalState.debugAddToGlobalCoinDicts();
#endif
    if (!success) {
        return false;
    }
#if DEBUG_DUMP_ALIGNMENT
    contour.dumpSegments("aligned");
#endif
    // construct closed contours
    result->reset();
    result->setFillType(fillType);
    SkPathWriter wrapper(*result);
    if (builder.xorMask() == kWinding_PathOpsMask ? !bridgeWinding(contourList, &wrapper)
            : !bridgeXor(contourList, &wrapper)) {
        return false;
    }
    wrapper.assemble();  // if some edges could not be resolved, assemble remaining
    if (scaleFactor > 1) {
        ScalePath(*result, scaleFactor, result);
    }
    return true;
}

bool Simplify(const SkPath& path, SkPath* result) {
#if DEBUG_DUMP_VERIFY
    if (SkPathOpsDebug::gVerifyOp) {
        if (!SimplifyDebug(path, result  SkDEBUGPARAMS(false) SkDEBUGPARAMS(nullptr))) {
            SkPathOpsDebug::ReportSimplifyFail(path);
            return false;
        }
        SkPathOpsDebug::VerifySimplify(path, *result);
        return true;
    }
#endif
    return SimplifyDebug(path, result  SkDEBUGPARAMS(true) SkDEBUGPARAMS(nullptr));
}
