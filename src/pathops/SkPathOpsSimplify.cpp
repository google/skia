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

static bool bridgeWinding(SkTDArray<SkOpContour* >& contourList, SkPathWriter* simple,
        SkChunkAlloc* allocator) {
    bool firstContour = true;
    bool unsortable = false;
    bool topUnsortable = false;
    bool firstPass = true;
    SkPoint lastTopLeft;
    SkPoint topLeft = {SK_ScalarMin, SK_ScalarMin};
    do {
        SkOpSpanBase* start;
        SkOpSpanBase* end;
        bool topDone;
        bool onlyVertical = false;
        lastTopLeft = topLeft;
        SkOpSegment* current = FindSortableTop(contourList, firstPass, SkOpAngle::kUnaryWinding,
                &firstContour, &start, &end, &topLeft, &topUnsortable, &topDone, &onlyVertical,
                allocator);
        if (!current) {
            if ((!topUnsortable || firstPass) && !topDone) {
                SkASSERT(topLeft.fX != SK_ScalarMin && topLeft.fY != SK_ScalarMin);
                if (lastTopLeft.fX == SK_ScalarMin && lastTopLeft.fY == SK_ScalarMin) {
                    if (firstPass) {
                        firstPass = false;
                    } else {
                        break;
                    }
                }
                topLeft.fX = topLeft.fY = SK_ScalarMin;
                continue;
            }
            break;
        } else if (onlyVertical) {
            break;
        }
        firstPass = !topUnsortable || lastTopLeft != topLeft;
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
                            current->addCurveTo(start, end, simple, true);
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
                    current->addCurveTo(start, end, simple, true);
                    current = next;
                    start = nextStart;
                    end = nextEnd;
                } while (!simple->isClosed() && (!unsortable || !start->starter(end)->done()));
                if (current->activeWinding(start, end) && !simple->isClosed()) {
                    SkOpSpan* spanStart = start->starter(end);
                    if (!spanStart->done()) {
                        current->addCurveTo(start, end, simple, true);
                        current->markDone(spanStart);
                    }
                }
                simple->close();
            } else {
                SkOpSpanBase* last = current->markAndChaseDone(start, end);
                if (last && !last->chased()) {
                    last->setChased(true);
                    SkASSERT(!SkPathOpsDebug::ChaseContains(chase, last));
                    // assert that last isn't already in array
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
    return simple->someAssemblyRequired();
}

// returns true if all edges were processed
static bool bridgeXor(SkTDArray<SkOpContour* >& contourList, SkPathWriter* simple,
        SkChunkAlloc* allocator) {
    SkOpSegment* current;
    SkOpSpanBase* start;
    SkOpSpanBase* end;
    bool unsortable = false;
    bool closable = true;
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
                    current->addCurveTo(start, end, simple, true);
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
            current->addCurveTo(start, end, simple, true);
            current = next;
            start = nextStart;
            end = nextEnd;
        } while (!simple->isClosed() && (!unsortable || !start->starter(end)->done()));
        if (!simple->isClosed()) {
            SkASSERT(unsortable);
            SkOpSpan* spanStart = start->starter(end);
            if (!spanStart->done()) {
                current->addCurveTo(start, end, simple, true);
                current->markDone(spanStart);
            }
            closable = false;
        }
        simple->close();
    #if DEBUG_ACTIVE_SPANS
        DebugShowActiveSpans(contourList);
    #endif
    }
    return closable;
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
    SkOpGlobalState globalState(&coincidence  PATH_OPS_DEBUG_PARAMS(&contour));
#if DEBUG_SORT || DEBUG_SWAP_TOP
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    SkOpEdgeBuilder builder(path, &contour, &allocator, &globalState);
    if (!builder.finish(&allocator)) {
        return false;
    }
#if !FORCE_RELEASE
    contour.dumpSegments((SkPathOp) -1);
#endif
    result->reset();
    result->setFillType(fillType);
    SkTDArray<SkOpContour* > contourList;
    MakeContourList(&contour, contourList, false, false);
    SkOpContour** currentPtr = contourList.begin();
    if (!currentPtr) {
        return true;
    }
    if ((*currentPtr)->count() == 0) {
        SkASSERT((*currentPtr)->next() == NULL);
        return true;
    }
    SkOpContour** listEnd2 = contourList.end();
    // find all intersections between segments
    do {
        SkOpContour** nextPtr = currentPtr;
        SkOpContour* current = *currentPtr++;
        SkOpContour* next;
        do {
            next = *nextPtr++;
        } while (AddIntersectTs(current, next, &coincidence, &allocator) && nextPtr != listEnd2);
    } while (currentPtr != listEnd2);
#if DEBUG_VALIDATE
    globalState.setPhase(SkOpGlobalState::kWalking);
#endif
    if (!HandleCoincidence(&contourList, &coincidence, &allocator, &globalState)) {
        return false;
    }
    // construct closed contours
    SkPathWriter wrapper(*result);
    if (builder.xorMask() == kWinding_PathOpsMask ? bridgeWinding(contourList, &wrapper, &allocator)
                : !bridgeXor(contourList, &wrapper, &allocator))
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
