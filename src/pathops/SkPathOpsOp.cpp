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
        bool sortable = true;
        bool done = true;
        *endPtr = NULL;
        if (SkOpAngle* last = segment->activeAngle(*startPtr, startPtr, endPtr, &done,
                &sortable)) {
            if (last->unorderable()) {
                continue;
            }
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
        if (!sortable) {
            continue;
        }
        // find first angle, initialize winding to computed fWindSum
        const SkOpAngle* angle = segment->spanToAngle(*startPtr, *endPtr);
        if (!angle) {
            continue;
        }
        const SkOpAngle* firstAngle = angle;
        bool loop = false;
        int winding = SK_MinS32;
        do {
            angle = angle->next();
            if (angle == firstAngle && loop) {
                break;    // if we get here, there's no winding, loop is unorderable
            }
            loop |= angle == firstAngle;
            segment = angle->segment();
            winding = segment->windSum(angle);
        } while (winding == SK_MinS32);
        if (winding == SK_MinS32) {
            continue;
        }
        int sumMiWinding = segment->updateWindingReverse(angle);
        int sumSuWinding = segment->updateOppWindingReverse(angle);
        if (segment->operand()) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
        SkOpSegment* first = NULL;
        firstAngle = angle;
        while ((angle = angle->next()) != firstAngle) {
            segment = angle->segment();
            SkOpSpanBase* start = angle->start();
            SkOpSpanBase* end = angle->end();
            int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
            segment->setUpWindings(start, end, &sumMiWinding, &sumSuWinding,
                    &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
            if (!segment->done(angle)) {
                if (!first) {
                    first = segment;
                    *startPtr = start;
                    *endPtr = end;
                }
                // OPTIMIZATION: should this also add to the chase?
                (void) segment->markAngle(maxWinding, sumWinding, oppMaxWinding,
                    oppSumWinding, angle);
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
    return NULL;
}

static bool bridgeOp(SkTDArray<SkOpContour* >& contourList, const SkPathOp op,
        const int xorMask, const int xorOpMask, SkPathWriter* simple, SkChunkAlloc* allocator) {
    bool firstContour = true;
    bool unsortable = false;
    bool topUnsortable = false;
    bool firstPass = true;
    SkDPoint lastTopLeft;
    SkDPoint topLeft = {SK_ScalarMin, SK_ScalarMin};
    do {
        SkOpSpanBase* start = NULL;
        SkOpSpanBase* end = NULL;
        bool topDone;
        bool onlyVertical = false;
        lastTopLeft = topLeft;
        SkOpSegment* current = FindSortableTop(contourList, firstPass, SkOpAngle::kBinarySingle,
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

#define DEBUGGING_PATHOPS_FROM_HOST 0  // enable to debug svg in chrome -- note path hardcoded below
#if DEBUGGING_PATHOPS_FROM_HOST
#include "SkData.h"
#include "SkStream.h"

static void dump_path(FILE* file, const SkPath& path, bool force, bool dumpAsHex) {
    SkDynamicMemoryWStream wStream;
    path.dump(&wStream, force, dumpAsHex);
    SkAutoDataUnref data(wStream.copyToData());
    fprintf(file, "%.*s\n", (int) data->size(), data->data());
}

static int dumpID = 0;

static void dump_op(const SkPath& one, const SkPath& two, SkPathOp op) {
#if SK_BUILD_FOR_MAC
    FILE* file = fopen("/Users/caryclark/Documents/svgop.txt", "w");
#else
    FILE* file = fopen("/usr/local/google/home/caryclark/Documents/svgop.txt", "w");
#endif
    fprintf(file,
            "\nstatic void fuzz763_%d(skiatest::Reporter* reporter, const char* filename) {\n",
            ++dumpID);
    fprintf(file, "    SkPath path;\n");
    fprintf(file, "    path.setFillType((SkPath::FillType) %d);\n", one.getFillType());
    dump_path(file, one, false, true);
    fprintf(file, "    SkPath path1(path);\n");
    fprintf(file, "    path.reset();\n");
    fprintf(file, "    path.setFillType((SkPath::FillType) %d);\n", two.getFillType());
    dump_path(file, two, false, true);
    fprintf(file, "    SkPath path2(path);\n");
    fprintf(file, "    testPathOp(reporter, path1, path2, (SkPathOp) %d, filename);\n", op);
    fprintf(file, "}\n");    
    fclose(file);
}
#endif

bool Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result) {
    SkChunkAlloc allocator(4096);  // FIXME: add a constant expression here, tune
    SkOpContour contour;
    SkOpCoincidence coincidence;
    SkOpGlobalState globalState(&coincidence  SkDEBUGPARAMS(&contour));
#if DEBUGGING_PATHOPS_FROM_HOST
    dump_op(one, two, op);
#endif    
#if 0 && DEBUG_SHOW_TEST_NAME
    char* debugName = DEBUG_FILENAME_STRING;
    if (debugName && debugName[0]) {
        SkPathOpsDebug::BumpTestName(debugName);
        SkPathOpsDebug::ShowPath(one, two, op, debugName);
    }
#endif
    op = gOpInverse[op][one.isInverseFillType()][two.isInverseFillType()];
    SkPath::FillType fillType = gOutInverse[op][one.isInverseFillType()][two.isInverseFillType()]
            ? SkPath::kInverseEvenOdd_FillType : SkPath::kEvenOdd_FillType;
    const SkPath* minuend = &one;
    const SkPath* subtrahend = &two;
    if (op == kReverseDifference_SkPathOp) {
        minuend = &two;
        subtrahend = &one;
        op = kDifference_SkPathOp;
    }
#if DEBUG_SORT || DEBUG_SWAP_TOP
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    // turn path into list of segments
    SkOpEdgeBuilder builder(*minuend, &contour, &allocator, &globalState);
    if (builder.unparseable()) {
        return false;
    }
    const int xorMask = builder.xorMask();
    builder.addOperand(*subtrahend);
    if (!builder.finish(&allocator)) {
        return false;
    }
#if DEBUG_DUMP_SEGMENTS
    contour.dumpSegments(op);
#endif

    const int xorOpMask = builder.xorMask();
    SkTDArray<SkOpContour* > contourList;
    MakeContourList(&contour, contourList, xorMask == kEvenOdd_PathOpsMask,
            xorOpMask == kEvenOdd_PathOpsMask);
    SkOpContour** currentPtr = contourList.begin();
    if (!currentPtr) {
        result->reset();
        result->setFillType(fillType);
        return true;
    }
    if ((*currentPtr)->count() == 0) {
        SkASSERT((*currentPtr)->next() == NULL);
        result->reset();
        result->setFillType(fillType);
        return true;
    }
    SkOpContour** listEnd = contourList.end();
    // find all intersections between segments
    do {
        SkOpContour** nextPtr = currentPtr;
        SkOpContour* current = *currentPtr++;
        SkOpContour* next;
        do {
            next = *nextPtr++;
        } while (AddIntersectTs(current, next, &coincidence, &allocator) && nextPtr != listEnd);
    } while (currentPtr != listEnd);
#if DEBUG_VALIDATE
    globalState.setPhase(SkOpGlobalState::kWalking);
#endif
    // eat through coincident edges
    if (!HandleCoincidence(&contourList, &coincidence, &allocator, &globalState)) {
        return false;
    }
    // construct closed contours
    result->reset();
    result->setFillType(fillType);
    SkPathWriter wrapper(*result);
    bridgeOp(contourList, op, xorMask, xorOpMask, &wrapper, &allocator);
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
