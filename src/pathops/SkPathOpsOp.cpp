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
        if (winding == SK_MinS32) {
            continue;
        }
        int sumMiWinding, sumSuWinding;
        if (sortable) {
            segment = angle->segment();
            sumMiWinding = segment->updateWindingReverse(angle);
            sumSuWinding = segment->updateOppWindingReverse(angle);
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
        const int xorMask, const int xorOpMask, SkPathWriter* simple, SkChunkAlloc* allocator) {
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


#if DEBUG_T_SECT_LOOP_COUNT

#include "SkMutex.h"

SK_DECLARE_STATIC_MUTEX(debugWorstLoop);

SkOpGlobalState debugWorstState(nullptr, nullptr  SkDEBUGPARAMS(nullptr));

void ReportPathOpsDebugging() {
    debugWorstState.debugLoopReport();
}

extern void (*gVerboseFinalize)();

#endif

bool OpDebug(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result,
        bool expectSuccess  SkDEBUGPARAMS(const char* testName)) {
    SkChunkAlloc allocator(4096);  // FIXME: add a constant expression here, tune
    SkOpContour contour;
    SkOpContourHead* contourList = static_cast<SkOpContourHead*>(&contour);
    SkOpCoincidence coincidence;
    SkOpGlobalState globalState(&coincidence, contourList  SkDEBUGPARAMS(testName));
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
#if DEBUG_SORT
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
        while (AddIntersectTs(current, next, &coincidence, &allocator)
                && (next = next->next()))
            ;
    } while ((current = current->next()));
#if DEBUG_VALIDATE
    globalState.setPhase(SkOpGlobalState::kWalking);
#endif
    if (!HandleCoincidence(contourList, &coincidence, &allocator)) {
        return false;
    }
#if DEBUG_ALIGNMENT
    contourList->dumpSegments("aligned");
#endif
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
#if DEBUG_T_SECT_LOOP_COUNT
    {
        SkAutoMutexAcquire autoM(debugWorstLoop);
        if (!gVerboseFinalize) {
            gVerboseFinalize = &ReportPathOpsDebugging;
        }
        debugWorstState.debugDoYourWorst(&globalState);
    }
#endif
    return true;
}

#define DEBUG_VERIFY 0

#if DEBUG_VERIFY
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"

const int bitWidth = 64;
const int bitHeight = 64;

static void debug_scale_matrix(const SkPath& one, const SkPath& two, SkMatrix& scale) {
    SkRect larger = one.getBounds();
    larger.join(two.getBounds());
    SkScalar largerWidth = larger.width();
    if (largerWidth < 4) {
        largerWidth = 4;
    }
    SkScalar largerHeight = larger.height();
    if (largerHeight < 4) {
        largerHeight = 4;
    }
    SkScalar hScale = (bitWidth - 2) / largerWidth;
    SkScalar vScale = (bitHeight - 2) / largerHeight;
    scale.reset();
    scale.preScale(hScale, vScale);
    larger.fLeft *= hScale;
    larger.fRight *= hScale;
    larger.fTop *= vScale;
    larger.fBottom *= vScale;
    SkScalar dx = -16000 > larger.fLeft ? -16000 - larger.fLeft
            : 16000 < larger.fRight ? 16000 - larger.fRight : 0;
    SkScalar dy = -16000 > larger.fTop ? -16000 - larger.fTop
            : 16000 < larger.fBottom ? 16000 - larger.fBottom : 0;
    scale.preTranslate(dx, dy);
}

static int debug_paths_draw_the_same(const SkPath& one, const SkPath& two, SkBitmap& bits) {
    if (bits.width() == 0) {
        bits.allocN32Pixels(bitWidth * 2, bitHeight);
    }
    SkCanvas canvas(bits);
    canvas.drawColor(SK_ColorWHITE);
    SkPaint paint;
    canvas.save();
    const SkRect& bounds1 = one.getBounds();
    canvas.translate(-bounds1.fLeft + 1, -bounds1.fTop + 1);
    canvas.drawPath(one, paint);
    canvas.restore();
    canvas.save();
    canvas.translate(-bounds1.fLeft + 1 + bitWidth, -bounds1.fTop + 1);
    canvas.drawPath(two, paint);
    canvas.restore();
    int errors = 0;
    for (int y = 0; y < bitHeight - 1; ++y) {
        uint32_t* addr1 = bits.getAddr32(0, y);
        uint32_t* addr2 = bits.getAddr32(0, y + 1);
        uint32_t* addr3 = bits.getAddr32(bitWidth, y);
        uint32_t* addr4 = bits.getAddr32(bitWidth, y + 1);
        for (int x = 0; x < bitWidth - 1; ++x) {
            // count 2x2 blocks
            bool err = addr1[x] != addr3[x];
            if (err) {
                errors += addr1[x + 1] != addr3[x + 1]
                        && addr2[x] != addr4[x] && addr2[x + 1] != addr4[x + 1];
            }
        }
    }
    return errors;
}

#endif

bool Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result) {
#if DEBUG_VERIFY
    if (!OpDebug(one, two, op, result, true  SkDEBUGPARAMS(nullptr))) {
        SkDebugf("%s did not expect failure\none: fill=%d\n", __FUNCTION__, one.getFillType());
        one.dumpHex();
        SkDebugf("two: fill=%d\n", two.getFillType());
        two.dumpHex();
        SkASSERT(0);
        return false;
    }
    SkPath pathOut, scaledPathOut;
    SkRegion rgnA, rgnB, openClip, rgnOut;
    openClip.setRect(-16000, -16000, 16000, 16000);
    rgnA.setPath(one, openClip);
    rgnB.setPath(two, openClip);
    rgnOut.op(rgnA, rgnB, (SkRegion::Op) op);
    rgnOut.getBoundaryPath(&pathOut);
    SkMatrix scale;
    debug_scale_matrix(one, two, scale);
    SkRegion scaledRgnA, scaledRgnB, scaledRgnOut;
    SkPath scaledA, scaledB;
    scaledA.addPath(one, scale);
    scaledA.setFillType(one.getFillType());
    scaledB.addPath(two, scale);
    scaledB.setFillType(two.getFillType());
    scaledRgnA.setPath(scaledA, openClip);
    scaledRgnB.setPath(scaledB, openClip);
    scaledRgnOut.op(scaledRgnA, scaledRgnB, (SkRegion::Op) op);
    scaledRgnOut.getBoundaryPath(&scaledPathOut);
    SkBitmap bitmap;
    SkPath scaledOut;
    scaledOut.addPath(*result, scale);
    scaledOut.setFillType(result->getFillType());
    int errors = debug_paths_draw_the_same(scaledPathOut, scaledOut, bitmap);
    const int MAX_ERRORS = 9;
    if (errors > MAX_ERRORS) {
        SkDebugf("%s did not expect failure\none: fill=%d\n", __FUNCTION__, one.getFillType());
        one.dumpHex();
        SkDebugf("two: fill=%d\n", two.getFillType());
        two.dumpHex();
        SkASSERT(0);
    }
    return true;
#else
    return OpDebug(one, two, op, result, true  SkDEBUGPARAMS(nullptr));
#endif
}
