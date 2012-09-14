/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Simplify.h"

namespace Op {

#include "Simplify.cpp"

static bool windingIsActive(int winding, int spanWinding, int oppWinding,
        const ShapeOp op) {
    return winding * spanWinding <= 0 && abs(winding) <= abs(spanWinding)
            && (!winding || !spanWinding || winding == -spanWinding);
}

static void bridgeOp(SkTDArray<Contour*>& contourList, const ShapeOp op, 
        const int aXorMask, const int bXorMask, SkPath& simple) {
    bool firstContour = true;
    do {
        Segment* topStart = findTopContour(contourList);
        if (!topStart) {
            break;
        }
        // Start at the top. Above the top is outside, below is inside.
        // follow edges to intersection by changing the index by direction.
        int index, endIndex;
        Segment* current = topStart->findTop(index, endIndex);
        int contourWinding;
        if (firstContour) {
            contourWinding = 0;
            firstContour = false;
        } else {
            int sumWinding = current->windSum(SkMin32(index, endIndex));
            // FIXME: don't I have to adjust windSum to get contourWinding?
            if (sumWinding == SK_MinS32) {
                sumWinding = current->computeSum(index, endIndex);
            }
            if (sumWinding == SK_MinS32) {
                contourWinding = innerContourCheck(contourList, current,
                        index, endIndex);
            } else {
                contourWinding = sumWinding;
                int spanWinding = current->spanSign(index, endIndex);
                bool inner = useInnerWinding(sumWinding - spanWinding, sumWinding);
                if (inner) {
                    contourWinding -= spanWinding;
                }
#if DEBUG_WINDING
                SkDebugf("%s sumWinding=%d spanWinding=%d sign=%d inner=%d result=%d\n", __FUNCTION__,
                        sumWinding, spanWinding, SkSign32(index - endIndex),
                        inner, contourWinding);
#endif
            }
#if DEBUG_WINDING
         //   SkASSERT(current->debugVerifyWinding(index, endIndex, contourWinding));
            SkDebugf("%s contourWinding=%d\n", __FUNCTION__, contourWinding);
#endif
        }
        SkPoint lastPt;
        int winding = contourWinding;
        int spanWinding = current->spanSign(index, endIndex);
        int oppWinding = current->oppSign(index, endIndex);
        bool active = windingIsActive(winding, spanWinding, oppWinding, op);
        SkTDArray<Span*> chaseArray;
        do {
        #if DEBUG_WINDING
            SkDebugf("%s active=%s winding=%d spanWinding=%d\n",
                    __FUNCTION__, active ? "true" : "false",
                    winding, spanWinding);
        #endif
            const SkPoint* firstPt = NULL;
            do {
                SkASSERT(!current->done());
                int nextStart = index;
                int nextEnd = endIndex;
                Segment* next = current->findNextOp(chaseArray, active,
                        nextStart, nextEnd, winding, spanWinding, op,
                        aXorMask, bXorMask);
                if (!next) {
                    if (active && firstPt && current->verb() != SkPath::kLine_Verb && *firstPt != lastPt) {
                        lastPt = current->addCurveTo(index, endIndex, simple, true);
                        SkASSERT(*firstPt == lastPt);
                    }
                    break;
                }
                if (!firstPt) {
                    firstPt = &current->addMoveTo(index, simple, active);
                }
                lastPt = current->addCurveTo(index, endIndex, simple, active);
                current = next;
                index = nextStart;
                endIndex = nextEnd;
            } while (*firstPt != lastPt && (active || !current->done()));
            if (firstPt && active) {
        #if DEBUG_PATH_CONSTRUCTION
                SkDebugf("%s close\n", __FUNCTION__);
        #endif
                simple.close();
            }
            current = findChase(chaseArray, index, endIndex, contourWinding);
        #if DEBUG_ACTIVE_SPANS
            debugShowActiveSpans(contourList);
        #endif
            if (!current) {
                break;
            }
            int lesser = SkMin32(index, endIndex);
            spanWinding = current->spanSign(index, endIndex);
            winding = current->windSum(lesser);
            bool inner = useInnerWinding(winding - spanWinding, winding);
        #if DEBUG_WINDING
            SkDebugf("%s id=%d t=%1.9g spanWinding=%d winding=%d sign=%d"
                    " inner=%d result=%d\n",
                    __FUNCTION__, current->debugID(), current->t(lesser),
                    spanWinding, winding, SkSign32(index - endIndex),
                    useInnerWinding(winding - spanWinding, winding),
                    inner ? winding - spanWinding : winding);
        #endif
            if (inner) {
                winding -= spanWinding;
            }
            int oppWinding = current->oppSign(index, endIndex);
            active = windingIsActive(winding, spanWinding, oppWinding, op);
        } while (true);
    } while (true);
}

} // end of Op namespace


void operate(const SkPath& one, const SkPath& two, ShapeOp op, SkPath& result) {
    result.reset();
    result.setFillType(SkPath::kEvenOdd_FillType);
    // turn path into list of segments
    SkTArray<Op::Contour> contours;
    // FIXME: add self-intersecting cubics' T values to segment
    Op::EdgeBuilder builder(one, contours);
    const int aXorMask = builder.xorMask();
    builder.addOperand(two);
    const int bXorMask = builder.xorMask();
    builder.finish();
    SkTDArray<Op::Contour*> contourList;
    makeContourList(contours, contourList);
    Op::Contour** currentPtr = contourList.begin();
    if (!currentPtr) {
        return;
    }
    Op::Contour** listEnd = contourList.end();
    // find all intersections between segments
    do {
        Op::Contour** nextPtr = currentPtr;
        Op::Contour* current = *currentPtr++;
        Op::Contour* next;
        do {
            next = *nextPtr++;
        } while (addIntersectTs(current, next) && nextPtr != listEnd);
    } while (currentPtr != listEnd);
    // eat through coincident edges
    coincidenceCheck(contourList);
    fixOtherTIndex(contourList);
    // construct closed contours
    bridgeOp(contourList, op, aXorMask, bXorMask, result);
}
