/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "SkIntersections.h"
#include "SkOpContour.h"
#include "SkPathWriter.h"
#include "SkTSort.h"

bool SkOpContour::addCoincident(int index, SkOpContour* other, int otherIndex,
        const SkIntersections& ts, bool swap) {
    SkPoint pt0 = ts.pt(0).asSkPoint();
    SkPoint pt1 = ts.pt(1).asSkPoint();
    if (pt0 == pt1) {
        // FIXME: one could imagine a case where it would be incorrect to ignore this
        // suppose two self-intersecting cubics overlap to be coincident --
        // this needs to check that by some measure the t values are far enough apart
        // or needs to check to see if the self-intersection bit was set on the cubic segment
        return false;
    }
    SkCoincidence& coincidence = fCoincidences.push_back();
    coincidence.fOther = other;
    coincidence.fSegments[0] = index;
    coincidence.fSegments[1] = otherIndex;
    coincidence.fTs[swap][0] = ts[0][0];
    coincidence.fTs[swap][1] = ts[0][1];
    coincidence.fTs[!swap][0] = ts[1][0];
    coincidence.fTs[!swap][1] = ts[1][1];
    coincidence.fPts[0] = pt0;
    coincidence.fPts[1] = pt1;
    return true;
}

SkOpSegment* SkOpContour::nonVerticalSegment(int* start, int* end) {
    int segmentCount = fSortedSegments.count();
    SkASSERT(segmentCount > 0);
    for (int sortedIndex = fFirstSorted; sortedIndex < segmentCount; ++sortedIndex) {
        SkOpSegment* testSegment = fSortedSegments[sortedIndex];
        if (testSegment->done()) {
            continue;
        }
        *start = *end = 0;
        while (testSegment->nextCandidate(start, end)) {
            if (!testSegment->isVertical(*start, *end)) {
                return testSegment;
            }
        }
    }
    return NULL;
}

// first pass, add missing T values
// second pass, determine winding values of overlaps
void SkOpContour::addCoincidentPoints() {
    int count = fCoincidences.count();
    for (int index = 0; index < count; ++index) {
        SkCoincidence& coincidence = fCoincidences[index];
        int thisIndex = coincidence.fSegments[0];
        SkOpSegment& thisOne = fSegments[thisIndex];
        SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        SkOpSegment& other = otherContour->fSegments[otherIndex];
        if ((thisOne.done() || other.done()) && thisOne.complete() && other.complete()) {
            // OPTIMIZATION: remove from array
            continue;
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs("-");
        other.debugShowTs("o");
    #endif
        double startT = coincidence.fTs[0][0];
        double endT = coincidence.fTs[0][1];
        bool startSwapped, oStartSwapped, cancelers;
        if ((cancelers = startSwapped = startT > endT)) {
            SkTSwap(startT, endT);
        }
        if (startT == endT) { // if one is very large the smaller may have collapsed to nothing
            if (endT <= 1 - FLT_EPSILON) {
                endT += FLT_EPSILON;
                SkASSERT(endT <= 1);
            } else {
                startT -= FLT_EPSILON;
                SkASSERT(startT >= 0);
            }
        }
        SkASSERT(!approximately_negative(endT - startT));
        double oStartT = coincidence.fTs[1][0];
        double oEndT = coincidence.fTs[1][1];
        if ((oStartSwapped = oStartT > oEndT)) {
            SkTSwap(oStartT, oEndT);
            cancelers ^= true;
        }
        SkASSERT(!approximately_negative(oEndT - oStartT));
        if (cancelers) {
            // make sure startT and endT have t entries
            const SkPoint& startPt = coincidence.fPts[startSwapped];
            if (startT > 0 || oEndT < 1
                    || thisOne.isMissing(startT, startPt) || other.isMissing(oEndT, startPt)) {
                thisOne.addTPair(startT, &other, oEndT, true, startPt);
            }
            const SkPoint& oStartPt = coincidence.fPts[oStartSwapped];
            if (oStartT > 0 || endT < 1
                    || thisOne.isMissing(endT, oStartPt) || other.isMissing(oStartT, oStartPt)) {
                other.addTPair(oStartT, &thisOne, endT, true, oStartPt);
            }
        } else {
            const SkPoint& startPt = coincidence.fPts[startSwapped];
            if (startT > 0 || oStartT > 0
                    || thisOne.isMissing(startT, startPt) || other.isMissing(oStartT, startPt)) {
                thisOne.addTPair(startT, &other, oStartT, true, startPt);
            }
            const SkPoint& oEndPt = coincidence.fPts[!oStartSwapped];
            if (endT < 1 || oEndT < 1
                    || thisOne.isMissing(endT, oEndPt) || other.isMissing(oEndT, oEndPt)) {
                other.addTPair(oEndT, &thisOne, endT, true, oEndPt);
            }
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs("+");
        other.debugShowTs("o");
    #endif
    }
}

bool SkOpContour::addPartialCoincident(int index, SkOpContour* other, int otherIndex,
        const SkIntersections& ts, int ptIndex, bool swap) {
    SkPoint pt0 = ts.pt(ptIndex).asSkPoint();
    SkPoint pt1 = ts.pt(ptIndex + 1).asSkPoint();
    if (SkDPoint::ApproximatelyEqual(pt0, pt1)) {
        // FIXME: one could imagine a case where it would be incorrect to ignore this
        // suppose two self-intersecting cubics overlap to form a partial coincidence --
        // although it isn't clear why the regular coincidence could wouldn't pick this up
        // this is exceptional enough to ignore for now
        return false;
    }
    SkCoincidence& coincidence = fPartialCoincidences.push_back();
    coincidence.fOther = other;
    coincidence.fSegments[0] = index;
    coincidence.fSegments[1] = otherIndex;
    coincidence.fTs[swap][0] = ts[0][ptIndex];
    coincidence.fTs[swap][1] = ts[0][ptIndex + 1];
    coincidence.fTs[!swap][0] = ts[1][ptIndex];
    coincidence.fTs[!swap][1] = ts[1][ptIndex + 1];
    coincidence.fPts[0] = pt0;
    coincidence.fPts[1] = pt1;
    return true;
}

void SkOpContour::calcCoincidentWinding() {
    int count = fCoincidences.count();
#if DEBUG_CONCIDENT
    if (count > 0) {
        SkDebugf("%s count=%d\n", __FUNCTION__, count);
    }
#endif
    for (int index = 0; index < count; ++index) {
        SkCoincidence& coincidence = fCoincidences[index];
         calcCommonCoincidentWinding(coincidence);
    }
}

void SkOpContour::calcPartialCoincidentWinding() {
    int count = fPartialCoincidences.count();
#if DEBUG_CONCIDENT
    if (count > 0) {
        SkDebugf("%s count=%d\n", __FUNCTION__, count);
    }
#endif
    for (int index = 0; index < count; ++index) {
        SkCoincidence& coincidence = fPartialCoincidences[index];
         calcCommonCoincidentWinding(coincidence);
    }
}

void SkOpContour::joinCoincidence(const SkTArray<SkCoincidence, true>& coincidences, bool partial) {
    int count = coincidences.count();
#if DEBUG_CONCIDENT
    if (count > 0) {
        SkDebugf("%s count=%d\n", __FUNCTION__, count);
    }
#endif
    // look for a lineup where the partial implies another adjoining coincidence
    for (int index = 0; index < count; ++index) {
        const SkCoincidence& coincidence = coincidences[index];
        int thisIndex = coincidence.fSegments[0];
        SkOpSegment& thisOne = fSegments[thisIndex];
        SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        SkOpSegment& other = otherContour->fSegments[otherIndex];
        double startT = coincidence.fTs[0][0];
        double endT = coincidence.fTs[0][1];
        if (startT == endT) {  // this can happen in very large compares
            continue;
        }
        double oStartT = coincidence.fTs[1][0];
        double oEndT = coincidence.fTs[1][1];
        if (oStartT == oEndT) {
            continue;
        }
        bool swapStart = startT > endT;
        bool swapOther = oStartT > oEndT;
        if (swapStart) {
            SkTSwap<double>(startT, endT);
            SkTSwap<double>(oStartT, oEndT);
        }
        bool cancel = swapOther != swapStart;
        int step = swapStart ? -1 : 1;
        int oStep = swapOther ? -1 : 1;
        double oMatchStart = cancel ? oEndT : oStartT;
        if (partial ? startT != 0 || oMatchStart != 0 : (startT == 0) != (oMatchStart == 0)) {
            bool added = false;
            if (oMatchStart != 0) {
                added = thisOne.joinCoincidence(&other, oMatchStart, oStep, cancel);
            }
            if (!cancel && startT != 0 && !added) {
                (void) other.joinCoincidence(&thisOne, startT, step, cancel);
            }
        }
        double oMatchEnd = cancel ? oStartT : oEndT;
        if (partial ? endT != 1 || oMatchEnd != 1 : (endT == 1) != (oMatchEnd == 1)) {
            bool added = false;
            if (cancel && endT != 1 && !added) {
                (void) other.joinCoincidence(&thisOne, endT, -step, cancel);
            }
        }
    }
}

void SkOpContour::calcCommonCoincidentWinding(const SkCoincidence& coincidence) {
    int thisIndex = coincidence.fSegments[0];
    SkOpSegment& thisOne = fSegments[thisIndex];
    if (thisOne.done()) {
        return;
    }
    SkOpContour* otherContour = coincidence.fOther;
    int otherIndex = coincidence.fSegments[1];
    SkOpSegment& other = otherContour->fSegments[otherIndex];
    if (other.done()) {
        return;
    }
    double startT = coincidence.fTs[0][0];
    double endT = coincidence.fTs[0][1];
    const SkPoint* startPt = &coincidence.fPts[0];
    const SkPoint* endPt = &coincidence.fPts[1];
    bool cancelers;
    if ((cancelers = startT > endT)) {
        SkTSwap<double>(startT, endT);
        SkTSwap<const SkPoint*>(startPt, endPt);
    }
    if (startT == endT) { // if span is very large, the smaller may have collapsed to nothing
        if (endT <= 1 - FLT_EPSILON) {
            endT += FLT_EPSILON;
            SkASSERT(endT <= 1);
        } else {
            startT -= FLT_EPSILON;
            SkASSERT(startT >= 0);
        }
    }
    SkASSERT(!approximately_negative(endT - startT));
    double oStartT = coincidence.fTs[1][0];
    double oEndT = coincidence.fTs[1][1];
    if (oStartT > oEndT) {
        SkTSwap<double>(oStartT, oEndT);
        cancelers ^= true;
    }
    SkASSERT(!approximately_negative(oEndT - oStartT));
    if (cancelers) {
        thisOne.addTCancel(*startPt, *endPt, &other);
    } else {
        thisOne.addTCoincident(*startPt, *endPt, endT, &other);
    }
#if DEBUG_CONCIDENT
    thisOne.debugShowTs("p");
    other.debugShowTs("o");
#endif
}

void SkOpContour::sortSegments() {
    int segmentCount = fSegments.count();
    fSortedSegments.push_back_n(segmentCount);
    for (int test = 0; test < segmentCount; ++test) {
        fSortedSegments[test] = &fSegments[test];
    }
    SkTQSort<SkOpSegment>(fSortedSegments.begin(), fSortedSegments.end() - 1);
    fFirstSorted = 0;
}

void SkOpContour::toPath(SkPathWriter* path) const {
    int segmentCount = fSegments.count();
    const SkPoint& pt = fSegments.front().pts()[0];
    path->deferredMove(pt);
    for (int test = 0; test < segmentCount; ++test) {
        fSegments[test].addCurveTo(0, 1, path, true);
    }
    path->close();
}

void SkOpContour::topSortableSegment(const SkPoint& topLeft, SkPoint* bestXY,
        SkOpSegment** topStart) {
    int segmentCount = fSortedSegments.count();
    SkASSERT(segmentCount > 0);
    int sortedIndex = fFirstSorted;
    fDone = true;  // may be cleared below
    for ( ; sortedIndex < segmentCount; ++sortedIndex) {
        SkOpSegment* testSegment = fSortedSegments[sortedIndex];
        if (testSegment->done()) {
            if (sortedIndex == fFirstSorted) {
                ++fFirstSorted;
            }
            continue;
        }
        fDone = false;
        SkPoint testXY = testSegment->activeLeftTop(true, NULL);
        if (*topStart) {
            if (testXY.fY < topLeft.fY) {
                continue;
            }
            if (testXY.fY == topLeft.fY && testXY.fX < topLeft.fX) {
                continue;
            }
            if (bestXY->fY < testXY.fY) {
                continue;
            }
            if (bestXY->fY == testXY.fY && bestXY->fX < testXY.fX) {
                continue;
            }
        }
        *topStart = testSegment;
        *bestXY = testXY;
    }
}

SkOpSegment* SkOpContour::undoneSegment(int* start, int* end) {
    int segmentCount = fSegments.count();
    for (int test = 0; test < segmentCount; ++test) {
        SkOpSegment* testSegment = &fSegments[test];
        if (testSegment->done()) {
            continue;
        }
        testSegment->undoneSpan(start, end);
        return testSegment;
    }
    return NULL;
}

#if DEBUG_SHOW_WINDING
int SkOpContour::debugShowWindingValues(int totalSegments, int ofInterest) {
    int count = fSegments.count();
    int sum = 0;
    for (int index = 0; index < count; ++index) {
        sum += fSegments[index].debugShowWindingValues(totalSegments, ofInterest);
    }
//      SkDebugf("%s sum=%d\n", __FUNCTION__, sum);
    return sum;
}

void SkOpContour::debugShowWindingValues(const SkTArray<SkOpContour*, true>& contourList) {
//     int ofInterest = 1 << 1 | 1 << 5 | 1 << 9 | 1 << 13;
//    int ofInterest = 1 << 4 | 1 << 8 | 1 << 12 | 1 << 16;
    int ofInterest = 1 << 5 | 1 << 8;
    int total = 0;
    int index;
    for (index = 0; index < contourList.count(); ++index) {
        total += contourList[index]->segments().count();
    }
    int sum = 0;
    for (index = 0; index < contourList.count(); ++index) {
        sum += contourList[index]->debugShowWindingValues(total, ofInterest);
    }
//       SkDebugf("%s total=%d\n", __FUNCTION__, sum);
}
#endif

void SkOpContour::setBounds() {
    int count = fSegments.count();
    if (count == 0) {
        SkDebugf("%s empty contour\n", __FUNCTION__);
        SkASSERT(0);
        // FIXME: delete empty contour?
        return;
    }
    fBounds = fSegments.front().bounds();
    for (int index = 1; index < count; ++index) {
        fBounds.add(fSegments[index].bounds());
    }
}
