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

static SkOpSegment* findChaseOp(SkTDArray<SkOpSpanBase*>& chase, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr) {
    while (chase.count()) {
        SkOpSpanBase* span;
        chase.pop(&span);
        // OPTIMIZE: prev makes this compatible with old code -- but is it necessary?
        *startPtr = span->ptT()->prev()->span();
        SkOpSegment* segment = (*startPtr)->segment();
        bool done = true;
        *endPtr = nullptr;
        if (SkOpAngle* last = segment->activeAngle(*startPtr, startPtr, endPtr, &done)) {
            *startPtr = last->start();
            *endPtr = last->end();
   #if TRY_ROTATE
            *chase.insert(0) = span;
   #else
            *chase.append() = span;
   #endif
            return last->segment();
        }
        if (done) {
            continue;
        }
        int winding;
        bool sortable;
        const SkOpAngle* angle = AngleWinding(*startPtr, *endPtr, &winding, &sortable);
        if (!angle) {
            return nullptr;
        }
        if (winding == SK_MinS32) {
            continue;
        }
        int sumMiWinding, sumSuWinding;
        if (sortable) {
            segment = angle->segment();
            sumMiWinding = segment->updateWindingReverse(angle);
            if (sumMiWinding == SK_MinS32) {
                SkASSERT(segment->globalState()->debugSkipAssert());
                return nullptr;
            }
            sumSuWinding = segment->updateOppWindingReverse(angle);
            if (sumSuWinding == SK_MinS32) {
                SkASSERT(segment->globalState()->debugSkipAssert());
                return nullptr;
            }
            if (segment->operand()) {
                SkTSwap<int>(sumMiWinding, sumSuWinding);
            }
        }
        SkOpSegment* first = nullptr;
        const SkOpAngle* firstAngle = angle;
        while ((angle = angle->next()) != firstAngle) {
            segment = angle->segment();
            SkOpSpanBase* start = angle->start();
            SkOpSpanBase* end = angle->end();
            int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
            if (sortable) {
                segment->setUpWindings(start, end, &sumMiWinding, &sumSuWinding,
                        &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
            }
            if (!segment->done(angle)) {
                if (!first && (sortable || start->starter(end)->windSum() != SK_MinS32)) {
                    first = segment;
                    *startPtr = start;
                    *endPtr = end;
                }
                // OPTIMIZATION: should this also add to the chase?
                if (sortable) {
                    (void) segment->markAngle(maxWinding, sumWinding, oppMaxWinding,
                        oppSumWinding, angle);
                }
            }
        }
        if (first) {
       #if TRY_ROTATE
            *chase.insert(0) = span;
       #else
            *chase.append() = span;
       #endif
            return first;
        }
    }
    return nullptr;
}

static bool bridgeOp(SkOpContourHead* contourList, const SkPathOp op,
        const int xorMask, const int xorOpMask, SkPathWriter* simple) {
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
            if (current->activeOp(start, end, xorMask, xorOpMask, op)) {
                do {
                    if (!unsortable && current->done()) {
                        break;
                    }
                    SkASSERT(unsortable || !current->done());
                    SkOpSpanBase* nextStart = start;
                    SkOpSpanBase* nextEnd = end;
                    SkOpSegment* next = current->findNextOp(&chase, &nextStart, &nextEnd,
                            &unsortable, op, xorMask, xorOpMask);
                    if (!next) {
                        if (!unsortable && simple->hasMove()
                                && current->verb() != SkPath::kLine_Verb
                                && !simple->isClosed()) {
                            if (!current->addCurveTo(start, end, simple)) {
                                return false;
                            }
                            if (!simple->isClosed()) {
                                SkPathOpsDebug::ShowActiveSpans(contourList);
                            }
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
            current = findChaseOp(chase, &start, &end);
            SkPathOpsDebug::ShowActiveSpans(contourList);
            if (!current) {
                break;
            }
        } while (true);
    } while (true);
    return true;
}

// pretty picture:
// https://docs.google.com/a/google.com/drawings/d/1sPV8rPfpEFXymBp3iSbDRWAycp1b-7vD9JP2V-kn9Ss/edit?usp=sharing
static const SkPathOp gOpInverse[kReverseDifference_SkPathOp + 1][2][2] = {
//                  inside minuend                               outside minuend
//     inside subtrahend     outside subtrahend      inside subtrahend     outside subtrahend
{{ kDifference_SkPathOp,   kIntersect_SkPathOp }, { kUnion_SkPathOp, kReverseDifference_SkPathOp }},
{{ kIntersect_SkPathOp,   kDifference_SkPathOp }, { kReverseDifference_SkPathOp, kUnion_SkPathOp }},
{{ kUnion_SkPathOp, kReverseDifference_SkPathOp }, { kDifference_SkPathOp,   kIntersect_SkPathOp }},
{{ kXOR_SkPathOp,                 kXOR_SkPathOp }, { kXOR_SkPathOp,                kXOR_SkPathOp }},
{{ kReverseDifference_SkPathOp, kUnion_SkPathOp }, { kIntersect_SkPathOp,   kDifference_SkPathOp }},
};

static const bool gOutInverse[kReverseDifference_SkPathOp + 1][2][2] = {
    {{ false, false }, { true, false }},  // diff
    {{ false, false }, { false, true }},  // sect
    {{ false, true }, { true, true }},    // union
    {{ false, true }, { true, false }},   // xor
    {{ false, true }, { false, false }},  // rev diff
};

#if DEBUG_T_SECT_LOOP_COUNT

#include "SkMutex.h"

SK_DECLARE_STATIC_MUTEX(debugWorstLoop);

SkOpGlobalState debugWorstState(nullptr, nullptr  SkDEBUGPARAMS(false) SkDEBUGPARAMS(nullptr)
        SkDEBUGPARAMS(nullptr));

void ReportPathOpsDebugging() {
    debugWorstState.debugLoopReport();
}

extern void (*gVerboseFinalize)();

#endif

bool OpDebug(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result
        SkDEBUGPARAMS(bool skipAssert) SkDEBUGPARAMS(const char* testName)) {
    char storage[4096];
    SkArenaAlloc allocator(storage);  // FIXME: add a constant expression here, tune
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
        SkPathOpsDebug::DumpOp(one, two, op, testName);
    }
#endif
    op = gOpInverse[op][one.isInverseFillType()][two.isInverseFillType()];
    SkPath::FillType fillType = gOutInverse[op][one.isInverseFillType()][two.isInverseFillType()]
            ? SkPath::kInverseEvenOdd_FillType : SkPath::kEvenOdd_FillType;
    SkScalar scaleFactor = SkTMax(ScaleFactor(one), ScaleFactor(two));
    SkPath scaledOne, scaledTwo;
    const SkPath* minuend, * subtrahend;
    if (scaleFactor > SK_Scalar1) {
        ScalePath(one, 1.f / scaleFactor, &scaledOne);
        minuend = &scaledOne;
        ScalePath(two, 1.f / scaleFactor, &scaledTwo);
        subtrahend = &scaledTwo;
    } else {
        minuend = &one;
        subtrahend = &two;
    }
    if (op == kReverseDifference_SkPathOp) {
        SkTSwap(minuend, subtrahend);
        op = kDifference_SkPathOp;
    }
#if DEBUG_SORT
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    // turn path into list of segments
    SkOpEdgeBuilder builder(*minuend, contourList, &globalState);
    if (builder.unparseable()) {
        return false;
    }
    const int xorMask = builder.xorMask();
    builder.addOperand(*subtrahend);
    if (!builder.finish()) {
        return false;
    }
#if DEBUG_DUMP_SEGMENTS
    contourList->dumpSegments("seg", op);
#endif

    const int xorOpMask = builder.xorMask();
    if (!SortContourList(&contourList, xorMask == kEvenOdd_PathOpsMask,
            xorOpMask == kEvenOdd_PathOpsMask)) {
        result->reset();
        result->setFillType(fillType);
        return true;
    }
    // find all intersections between segments
    SkOpContour* current = contourList;
    do {
        SkOpContour* next = current;
        while (AddIntersectTs(current, next, &coincidence)
                && (next = next->next()))
            ;
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
#if DEBUG_ALIGNMENT
    contourList->dumpSegments("aligned");
#endif
    // construct closed contours
    result->reset();
    result->setFillType(fillType);
    SkPathWriter wrapper(*result);
    if (!bridgeOp(contourList, op, xorMask, xorOpMask, &wrapper)) {
        return false;
    }
    wrapper.assemble();  // if some edges could not be resolved, assemble remaining
#if DEBUG_T_SECT_LOOP_COUNT
    {
        SkAutoMutexAcquire autoM(debugWorstLoop);
        if (!gVerboseFinalize) {
            gVerboseFinalize = &ReportPathOpsDebugging;
        }
        debugWorstState.debugDoYourWorst(&globalState);
    }
#endif
    if (scaleFactor > 1) {
        ScalePath(*result, scaleFactor, result);
    }
    return true;
}

bool Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result) {
#if DEBUG_DUMP_VERIFY
    if (SkPathOpsDebug::gVerifyOp) {
        if (!OpDebug(one, two, op, result  SkDEBUGPARAMS(false) SkDEBUGPARAMS(nullptr))) {
            SkPathOpsDebug::ReportOpFail(one, two, op);
            return false;
        }
        SkPathOpsDebug::VerifyOp(one, two, op, *result);
        return true;
    }
#endif
    return OpDebug(one, two, op, result  SkDEBUGPARAMS(true) SkDEBUGPARAMS(nullptr));
}
