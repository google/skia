/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkTypes.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/pathops/SkAddIntersections.h"
#include "src/pathops/SkOpCoincidence.h"
#include "src/pathops/SkOpContour.h"
#include "src/pathops/SkOpEdgeBuilder.h"
#include "src/pathops/SkOpSegment.h"
#include "src/pathops/SkOpSpan.h"
#include "src/pathops/SkPathOpsCommon.h"
#include "src/pathops/SkPathOpsTypes.h"
#include "src/pathops/SkPathWriter.h"

static bool bridgeWinding(SkOpContourHead* contourList, SkPathWriter* writer) {
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
                    if (!current->addCurveTo(start, end, writer)) {
                        return false;
                    }
                    current = next;
                    start = nextStart;
                    end = nextEnd;
                } while (!writer->isClosed() && (!unsortable || !start->starter(end)->done()));
                if (current->activeWinding(start, end) && !writer->isClosed()) {
                    SkOpSpan* spanStart = start->starter(end);
                    if (!spanStart->done()) {
                        if (!current->addCurveTo(start, end, writer)) {
                            return false;
                        }
                        current->markDone(spanStart);
                    }
                }
                writer->finishContour();
            } else {
                SkOpSpanBase* last;
                 if (!current->markAndChaseDone(start, end, &last)) {
                    return false;
                }
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
static bool bridgeXor(SkOpContourHead* contourList, SkPathWriter* writer) {
    bool unsortable = false;
    int safetyNet = 1000000;
    do {
        SkOpSpan* span = FindUndone(contourList);
        if (!span) {
            break;
        }
        SkOpSegment* current = span->segment();
        SkOpSpanBase* start = span->next();
        SkOpSpanBase* end = span;
        do {
            if (--safetyNet < 0) {
                return false;
            }
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
            if (!current->addCurveTo(start, end, writer)) {
                return false;
            }
            current = next;
            start = nextStart;
            end = nextEnd;
        } while (!writer->isClosed() && (!unsortable || !start->starter(end)->done()));
        if (!writer->isClosed()) {
            SkOpSpan* spanStart = start->starter(end);
            if (!spanStart->done()) {
                return false;
            }
        }
        writer->finishContour();
        SkPathOpsDebug::ShowActiveSpans(contourList);
    } while (true);
    return true;
}

static bool path_is_trivial(const SkPath& path) {
    SkPath::Iter iter(path, true);

    SkPath::Verb verb;
    SkPoint points[4];

    class Trivializer {
        SkPoint prevPt{0,0};
        SkVector prevVec{0,0};
    public:
        void moveTo(const SkPoint& currPt) {
            prevPt = currPt;
            prevVec = {0, 0};
        }
        bool addTrivialContourPoint(const SkPoint& currPt) {
            if (currPt == prevPt) {
                return true;
            }
            // There are more numericaly stable ways of determining if many points are co-linear.
            // However, this mirrors SkPath's Convexicator for consistency.
            SkVector currVec = currPt - prevPt;
            if (SkPoint::CrossProduct(prevVec, currVec) != 0) {
                return false;
            }
            prevVec = currVec;
            prevPt = currPt;
            return true;
        }
    } triv;

    while ((verb = iter.next(points)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                triv.moveTo(points[0]);
                break;
            case SkPath::kCubic_Verb:
                if (!triv.addTrivialContourPoint(points[3])) { return false; }
                [[fallthrough]];
            case SkPath::kConic_Verb:
            case SkPath::kQuad_Verb:
                if (!triv.addTrivialContourPoint(points[2])) { return false; }
                [[fallthrough]];
            case SkPath::kLine_Verb:
                if (!triv.addTrivialContourPoint(points[1])) { return false; }
                if (!triv.addTrivialContourPoint(points[0])) { return false; }
                break;
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }
    return true;
}

// FIXME : add this as a member of SkPath
bool SimplifyDebug(const SkPath& path, SkPath* result
        SkDEBUGPARAMS(bool skipAssert) SkDEBUGPARAMS(const char* testName)) {
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    SkPathFillType fillType = path.isInverseFillType() ? SkPathFillType::kInverseEvenOdd
            : SkPathFillType::kEvenOdd;

    if (path.isConvex()) {
        // If the path is trivially convex, simplify to empty.
        if (path_is_trivial(path)) {
            result->reset();
        } else if (result != &path) {
            *result = path;
        }
        result->setFillType(fillType);
        return true;
    }
    // turn path into list of segments
    SkSTArenaAlloc<4096> allocator;  // FIXME: constant-ize, tune
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
        DumpSimplify(path, testName);
    }
#endif
#if DEBUG_SORT
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    SkOpEdgeBuilder builder(path, contourList, &globalState);
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
    return true;
}

bool Simplify(const SkPath& path, SkPath* result) {
#if DEBUG_DUMP_VERIFY
    if (SkPathOpsDebug::gVerifyOp) {
        if (!SimplifyDebug(path, result  SkDEBUGPARAMS(false) SkDEBUGPARAMS(nullptr))) {
            ReportSimplifyFail(path);
            return false;
        }
        VerifySimplify(path, *result);
        return true;
    }
#endif
    return SimplifyDebug(path, result  SkDEBUGPARAMS(true) SkDEBUGPARAMS(nullptr));
}
