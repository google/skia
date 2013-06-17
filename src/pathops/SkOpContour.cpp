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

void SkOpContour::addCoincident(int index, SkOpContour* other, int otherIndex,
        const SkIntersections& ts, bool swap) {
    SkCoincidence& coincidence = fCoincidences.push_back();
    coincidence.fContours[0] = this;  // FIXME: no need to store
    coincidence.fContours[1] = other;
    coincidence.fSegments[0] = index;
    coincidence.fSegments[1] = otherIndex;
    coincidence.fTs[swap][0] = ts[0][0];
    coincidence.fTs[swap][1] = ts[0][1];
    coincidence.fTs[!swap][0] = ts[1][0];
    coincidence.fTs[!swap][1] = ts[1][1];
    coincidence.fPts[0] = ts.pt(0).asSkPoint();
    coincidence.fPts[1] = ts.pt(1).asSkPoint();
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
        SkASSERT(coincidence.fContours[0] == this);
        int thisIndex = coincidence.fSegments[0];
        SkOpSegment& thisOne = fSegments[thisIndex];
        SkOpContour* otherContour = coincidence.fContours[1];
        int otherIndex = coincidence.fSegments[1];
        SkOpSegment& other = otherContour->fSegments[otherIndex];
        if ((thisOne.done() || other.done()) && thisOne.complete() && other.complete()) {
            // OPTIMIZATION: remove from array
            continue;
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs();
        other.debugShowTs();
    #endif
        double startT = coincidence.fTs[0][0];
        double endT = coincidence.fTs[0][1];
        bool startSwapped, oStartSwapped, cancelers;
        if ((cancelers = startSwapped = startT > endT)) {
            SkTSwap(startT, endT);
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
            if (startT > 0 || oEndT < 1
                    || thisOne.isMissing(startT) || other.isMissing(oEndT)) {
                thisOne.addTPair(startT, &other, oEndT, true, coincidence.fPts[startSwapped]);
            }
            if (oStartT > 0 || endT < 1
                    || thisOne.isMissing(endT) || other.isMissing(oStartT)) {
                other.addTPair(oStartT, &thisOne, endT, true, coincidence.fPts[oStartSwapped]);
            }
        } else {
            if (startT > 0 || oStartT > 0
                    || thisOne.isMissing(startT) || other.isMissing(oStartT)) {
                thisOne.addTPair(startT, &other, oStartT, true, coincidence.fPts[startSwapped]);
            }
            if (endT < 1 || oEndT < 1
                    || thisOne.isMissing(endT) || other.isMissing(oEndT)) {
                other.addTPair(oEndT, &thisOne, endT, true, coincidence.fPts[!oStartSwapped]);
            }
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs();
        other.debugShowTs();
    #endif
    }
}

void SkOpContour::calcCoincidentWinding() {
    int count = fCoincidences.count();
    for (int index = 0; index < count; ++index) {
        SkCoincidence& coincidence = fCoincidences[index];
        SkASSERT(coincidence.fContours[0] == this);
        int thisIndex = coincidence.fSegments[0];
        SkOpSegment& thisOne = fSegments[thisIndex];
        if (thisOne.done()) {
            continue;
        }
        SkOpContour* otherContour = coincidence.fContours[1];
        int otherIndex = coincidence.fSegments[1];
        SkOpSegment& other = otherContour->fSegments[otherIndex];
        if (other.done()) {
            continue;
        }
        double startT = coincidence.fTs[0][0];
        double endT = coincidence.fTs[0][1];
        bool cancelers;
        if ((cancelers = startT > endT)) {
            SkTSwap<double>(startT, endT);
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
            // make sure startT and endT have t entries
            if (!thisOne.done() && !other.done()) {
                thisOne.addTCancel(startT, endT, &other, oStartT, oEndT);
            }
        } else {
            if (!thisOne.done() && !other.done()) {
                thisOne.addTCoincident(startT, endT, &other, oStartT, oEndT);
            }
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs();
        other.debugShowTs();
    #endif
    }
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

static void SkOpContour::debugShowWindingValues(const SkTArray<SkOpContour*, true>& contourList) {
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
