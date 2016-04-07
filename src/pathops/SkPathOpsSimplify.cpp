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

static bool bridgeWinding(SkOpContourHead* contourList, SkPathWriter* simple,
        SkChunkAlloc* allocator, bool* closable) {
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
                        if (!unsortable && simple->hasMove()
                                && current->verb() != SkPath::kLine_Verb
                                && !simple->isClosed()) {
                            if (!current->addCurveTo(start, end, simple)) {
                                return false;
                            }
                    #if DEBUG_ACTIVE_SPANS
                            if (!simple->isClosed()) {
                                DebugShowActiveSpans(contourList);
                            }
                    #endif
                        }
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
                simple->close();
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
        #if DEBUG_ACTIVE_SPANS
            DebugShowActiveSpans(contourList);
        #endif
            if (!current) {
                break;
            }
        } while (true);
    } while (true);
    *closable = !simple->someAssemblyRequired();
    return true;
}

// returns true if all edges were processed
static bool bridgeXor(SkOpContourHead* contourList, SkPathWriter* simple,
        SkChunkAlloc* allocator, bool* closable) {
    SkOpSegment* current;
    SkOpSpanBase* start;
    SkOpSpanBase* end;
    bool unsortable = false;
    *closable = true;
    while ((current = FindUndone(contourList, &start, &end))) {
        do {
    #if DEBUG_ACTIVE_SPANS
            if (!unsortable && current->done()) {
                DebugShowActiveSpans(contourList);
            }
    #endif
            SkASSERT(unsortable || !current->done());
            SkOpSpanBase* nextStart = start;
            SkOpSpanBase* nextEnd = end;
            SkOpSegment* next = current->findNextXor(&nextStart, &nextEnd, &unsortable);
            if (!next) {
                if (!unsortable && simple->hasMove()
                        && current->verb() != SkPath::kLine_Verb
                        && !simple->isClosed()) {
                    if (!current->addCurveTo(start, end, simple)) {
                        return false;
                    }
            #if DEBUG_ACTIVE_SPANS
                    if (!simple->isClosed()) {
                        DebugShowActiveSpans(contourList);
                    }
            #endif
                }
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
            SkASSERT(unsortable);
            SkOpSpan* spanStart = start->starter(end);
            if (!spanStart->done()) {
                if (!current->addCurveTo(start, end, simple)) {
                    return false;
                }
                current->markDone(spanStart);
            }
            *closable = false;
        }
        simple->close();
    #if DEBUG_ACTIVE_SPANS
        DebugShowActiveSpans(contourList);
    #endif
    }
    return true;
}

// FIXME : add this as a member of SkPath
bool Simplify(const SkPath& path, SkPath* result) {
    SkChunkAlloc allocator(4096);  // FIXME: constant-ize, tune
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
    SkOpCoincidence coincidence;
    SkOpContour contour;
    SkOpContourHead* contourList = static_cast<SkOpContourHead*>(&contour);
    SkOpGlobalState globalState(&coincidence, contourList  SkDEBUGPARAMS(nullptr));
#if DEBUG_SORT
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    SkOpEdgeBuilder builder(path, &contour, &allocator, &globalState);
    if (!builder.finish(&allocator)) {
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
        while (AddIntersectTs(current, next, &coincidence, &allocator)
                && (next = next->next()));
    } while ((current = current->next()));
#if DEBUG_VALIDATE
    globalState.setPhase(SkOpGlobalState::kWalking);
#endif
    if (!HandleCoincidence(contourList, &coincidence, &allocator)) {
        return false;
    }
#if DEBUG_DUMP_ALIGNMENT
    contour.dumpSegments("aligned");
#endif
    // construct closed contours
    result->reset();
    result->setFillType(fillType);
    SkPathWriter wrapper(*result);
    bool closable SK_INIT_TO_AVOID_WARNING;
    if (builder.xorMask() == kWinding_PathOpsMask
            ? !bridgeWinding(contourList, &wrapper, &allocator, &closable)
            : !bridgeXor(contourList, &wrapper, &allocator, &closable)) {
        return false;
    }
    if (!closable)
    {  // if some edges could not be resolved, assemble remaining fragments
        SkPath temp;
        temp.setFillType(fillType);
        SkPathWriter assembled(temp);
        Assemble(wrapper, &assembled);
        *result = *assembled.nativePath();
        result->setFillType(fillType);
    }
    return true;
}

