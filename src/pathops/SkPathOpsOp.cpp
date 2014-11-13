/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkAddIntersections.h"
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"
#include "SkPathWriter.h"

static SkOpSegment* findChaseOp(SkTDArray<SkOpSpan*>& chase, int* tIndex, int* endIndex) {
    while (chase.count()) {
        SkOpSpan* span;
        chase.pop(&span);
        const SkOpSpan& backPtr = span->fOther->span(span->fOtherIndex);
        SkOpSegment* segment = backPtr.fOther;
        *tIndex = backPtr.fOtherIndex;
        bool sortable = true;
        bool done = true;
        *endIndex = -1;
        if (const SkOpAngle* last = segment->activeAngle(*tIndex, tIndex, endIndex, &done,
                &sortable)) {
            if (last->unorderable()) {
                continue;
            }
            *tIndex = last->start();
            *endIndex = last->end();
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
        const SkOpAngle* angle = segment->spanToAngle(*tIndex, *endIndex);
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
        bool badData = false;
        while ((angle = angle->next()) != firstAngle && !badData) {
            segment = angle->segment();
            int start = angle->start();
            int end = angle->end();
            int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
            segment->setUpWindings(start, end, &sumMiWinding, &sumSuWinding,
                    &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
            if (!segment->done(angle)) {
                if (!first) {
                    first = segment;
                    *tIndex = start;
                    *endIndex = end;
                }
                if (segment->inconsistentAngle(maxWinding, sumWinding, oppMaxWinding,
                        oppSumWinding, angle)) {
                    badData = true;
                    break;
                }
                // OPTIMIZATION: should this also add to the chase?
                (void) segment->markAngle(maxWinding, sumWinding, oppMaxWinding,
                    oppSumWinding, angle);
            }
        }
        if (badData) {
            continue;
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

/*
static bool windingIsActive(int winding, int oppWinding, int spanWinding, int oppSpanWinding,
        bool windingIsOp, PathOp op) {
    bool active = windingIsActive(winding, spanWinding);
    if (!active) {
        return false;
    }
    if (oppSpanWinding && windingIsActive(oppWinding, oppSpanWinding)) {
        switch (op) {
            case kIntersect_Op:
            case kUnion_Op:
                return true;
            case kDifference_Op: {
                int absSpan = abs(spanWinding);
                int absOpp = abs(oppSpanWinding);
                return windingIsOp ? absSpan < absOpp : absSpan > absOpp;
            }
            case kXor_Op:
                return spanWinding != oppSpanWinding;
            default:
                SkASSERT(0);
        }
    }
    bool opActive = oppWinding != 0;
    return gOpLookup[op][opActive][windingIsOp];
}
*/

static bool bridgeOp(SkTArray<SkOpContour*, true>& contourList, const SkPathOp op,
        const int xorMask, const int xorOpMask, SkPathWriter* simple) {
    bool firstContour = true;
    bool unsortable = false;
    bool topUnsortable = false;
    bool firstPass = true;
    SkPoint lastTopLeft;
    SkPoint topLeft = {SK_ScalarMin, SK_ScalarMin};
    do {
        int index, endIndex;
        bool topDone;
        bool onlyVertical = false;
        lastTopLeft = topLeft;
        SkOpSegment* current = FindSortableTop(contourList, SkOpAngle::kBinarySingle, &firstContour,
                &index, &endIndex, &topLeft, &topUnsortable, &topDone, &onlyVertical, firstPass);
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
        SkTDArray<SkOpSpan*> chase;
        do {
            if (current->activeOp(index, endIndex, xorMask, xorOpMask, op)) {
                do {
                    if (!unsortable && current->done()) {
                        break;
                    }
                    SkASSERT(unsortable || !current->done());
                    int nextStart = index;
                    int nextEnd = endIndex;
                    SkOpSegment* next = current->findNextOp(&chase, &nextStart, &nextEnd,
                            &unsortable, op, xorMask, xorOpMask);
                    if (!next) {
                        if (!unsortable && simple->hasMove()
                                && current->verb() != SkPath::kLine_Verb
                                && !simple->isClosed()) {
                            current->addCurveTo(index, endIndex, simple, true);
                    #if DEBUG_ACTIVE_SPANS
                            if (!simple->isClosed()) {
                                DebugShowActiveSpans(contourList);
                            }
                    #endif
//                            SkASSERT(simple->isClosed());
                        }
                        break;
                    }
        #if DEBUG_FLOW
            SkDebugf("%s current id=%d from=(%1.9g,%1.9g) to=(%1.9g,%1.9g)\n", __FUNCTION__,
                    current->debugID(), current->xyAtT(index).fX, current->xyAtT(index).fY,
                    current->xyAtT(endIndex).fX, current->xyAtT(endIndex).fY);
        #endif
                    current->addCurveTo(index, endIndex, simple, true);
                    current = next;
                    index = nextStart;
                    endIndex = nextEnd;
                } while (!simple->isClosed() && (!unsortable
                        || !current->done(SkMin32(index, endIndex))));
                if (current->activeWinding(index, endIndex) && !simple->isClosed()) {
                    // FIXME : add to simplify, xor cpaths
                    int min = SkMin32(index, endIndex);
                    if (!unsortable && !simple->isEmpty()) {
                        unsortable = current->checkSmall(min);
                    }
                    if (!current->done(min)) {
                        current->addCurveTo(index, endIndex, simple, true);
                        current->markDoneBinary(min);
                    }
                }
                simple->close();
            } else {
                SkOpSpan* last = current->markAndChaseDoneBinary(index, endIndex);
                if (last && !last->fChased && !last->fLoop) {
                    last->fChased = true;
                    SkASSERT(!SkPathOpsDebug::ChaseContains(chase, last));
                    *chase.append() = last;
#if DEBUG_WINDING
                    SkDebugf("%s chase.append id=%d windSum=%d small=%d\n", __FUNCTION__,
                            last->fOther->span(last->fOtherIndex).fOther->debugID(), last->fWindSum,
                            last->fSmall);
#endif
                }
            }
            current = findChaseOp(chase, &index, &endIndex);
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
static const SkPathOp gOpInverse[kReverseDifference_PathOp + 1][2][2] = {
//                  inside minuend                               outside minuend
//     inside subtrahend     outside subtrahend      inside subtrahend     outside subtrahend
    {{ kDifference_PathOp,    kIntersect_PathOp }, { kUnion_PathOp, kReverseDifference_PathOp }},
    {{ kIntersect_PathOp,    kDifference_PathOp }, { kReverseDifference_PathOp, kUnion_PathOp }},
    {{ kUnion_PathOp, kReverseDifference_PathOp }, { kDifference_PathOp,    kIntersect_PathOp }},
    {{ kXOR_PathOp,                 kXOR_PathOp }, { kXOR_PathOp,                 kXOR_PathOp }},
    {{ kReverseDifference_PathOp, kUnion_PathOp }, { kIntersect_PathOp,    kDifference_PathOp }},
};

static const bool gOutInverse[kReverseDifference_PathOp + 1][2][2] = {
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
#if DEBUGGING_PATHOPS_FROM_HOST
    dump_op(one, two, op);
#endif	
#if DEBUG_SHOW_TEST_NAME
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
    if (op == kReverseDifference_PathOp) {
        minuend = &two;
        subtrahend = &one;
        op = kDifference_PathOp;
    }
#if DEBUG_SORT || DEBUG_SWAP_TOP
    SkPathOpsDebug::gSortCount = SkPathOpsDebug::gSortCountDefault;
#endif
    // turn path into list of segments
    SkTArray<SkOpContour> contours;
    // FIXME: add self-intersecting cubics' T values to segment
    SkOpEdgeBuilder builder(*minuend, contours);
    if (builder.unparseable()) {
        return false;
    }
    const int xorMask = builder.xorMask();
    builder.addOperand(*subtrahend);
    if (!builder.finish()) {
        return false;
    }
    result->reset();
    result->setFillType(fillType);
    const int xorOpMask = builder.xorMask();
    SkTArray<SkOpContour*, true> contourList;
    MakeContourList(contours, contourList, xorMask == kEvenOdd_PathOpsMask,
            xorOpMask == kEvenOdd_PathOpsMask);
    SkOpContour** currentPtr = contourList.begin();
    if (!currentPtr) {
        return true;
    }
    SkOpContour** listEnd = contourList.end();
    // find all intersections between segments
    do {
        SkOpContour** nextPtr = currentPtr;
        SkOpContour* current = *currentPtr++;
        if (current->containsCubics()) {
            AddSelfIntersectTs(current);
        }
        SkOpContour* next;
        do {
            next = *nextPtr++;
        } while (AddIntersectTs(current, next) && nextPtr != listEnd);
    } while (currentPtr != listEnd);
    // eat through coincident edges

    int total = 0;
    int index;
    for (index = 0; index < contourList.count(); ++index) {
        total += contourList[index]->segments().count();
    }
    if (!HandleCoincidence(&contourList, total)) {
        return false;
    }
    // construct closed contours
    SkPathWriter wrapper(*result);
    bridgeOp(contourList, op, xorMask, xorOpMask, &wrapper);
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
