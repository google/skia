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
    coincidence.fPts[swap][0] = pt0;
    coincidence.fPts[swap][1] = pt1;
    bool nearStart = ts.nearlySame(0);
    bool nearEnd = ts.nearlySame(1);
    coincidence.fPts[!swap][0] = nearStart ? ts.pt2(0).asSkPoint() : pt0;
    coincidence.fPts[!swap][1] = nearEnd ? ts.pt2(1).asSkPoint() : pt1;
    coincidence.fNearly[0] = nearStart;
    coincidence.fNearly[1] = nearEnd;
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
        const SkPoint& startPt = coincidence.fPts[0][startSwapped];
        if (cancelers) {
            // make sure startT and endT have t entries
            if (startT > 0 || oEndT < 1
                    || thisOne.isMissing(startT, startPt) || other.isMissing(oEndT, startPt)) {
                thisOne.addTPair(startT, &other, oEndT, true, startPt,
                        coincidence.fPts[1][startSwapped]);
            }
            const SkPoint& oStartPt = coincidence.fPts[1][oStartSwapped];
            if (oStartT > 0 || endT < 1
                    || thisOne.isMissing(endT, oStartPt) || other.isMissing(oStartT, oStartPt)) {
                other.addTPair(oStartT, &thisOne, endT, true, oStartPt,
                        coincidence.fPts[0][oStartSwapped]);
            }
        } else {
            if (startT > 0 || oStartT > 0
                    || thisOne.isMissing(startT, startPt) || other.isMissing(oStartT, startPt)) {
                thisOne.addTPair(startT, &other, oStartT, true, startPt,
                        coincidence.fPts[1][startSwapped]);
            }
            const SkPoint& oEndPt = coincidence.fPts[1][!oStartSwapped];
            if (endT < 1 || oEndT < 1
                    || thisOne.isMissing(endT, oEndPt) || other.isMissing(oEndT, oEndPt)) {
                other.addTPair(oEndT, &thisOne, endT, true, oEndPt,
                        coincidence.fPts[0][!oStartSwapped]);
            }
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs("+");
        other.debugShowTs("o");
    #endif
    }
    // if there are multiple pairs of coincidence that share an edge, see if the opposite
    // are also coincident
    for (int index = 0; index < count - 1; ++index) {
        const SkCoincidence& coincidence = fCoincidences[index];
        int thisIndex = coincidence.fSegments[0];
        SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        for (int idx2 = 1; idx2 < count; ++idx2) {
            const SkCoincidence& innerCoin = fCoincidences[idx2];
            int innerThisIndex = innerCoin.fSegments[0];
            if (thisIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 1, innerCoin, 1, false);
            }
            if (this == otherContour && otherIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 0, innerCoin, 1, false);
            }
            SkOpContour* innerOtherContour = innerCoin.fOther;
            innerThisIndex = innerCoin.fSegments[1];
            if (this == innerOtherContour && thisIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 1, innerCoin, 0, false);
            }
            if (otherContour == innerOtherContour && otherIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 0, innerCoin, 0, false);
            }
        }
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
    coincidence.fPts[0][0] = coincidence.fPts[1][0] = pt0;
    coincidence.fPts[0][1] = coincidence.fPts[1][1] = pt1;
    coincidence.fNearly[0] = 0;
    coincidence.fNearly[1] = 0;
    return true;
}

void SkOpContour::align(const SkOpSegment::AlignedSpan& aligned, bool swap,
        SkCoincidence* coincidence) {
    for (int idx2 = 0; idx2 < 2; ++idx2) {
        if (coincidence->fPts[0][idx2] == aligned.fOldPt
                && coincidence->fTs[swap][idx2] == aligned.fOldT) {
            SkASSERT(SkDPoint::RoughlyEqual(coincidence->fPts[0][idx2], aligned.fPt));
            coincidence->fPts[0][idx2] = aligned.fPt;
            SkASSERT(way_roughly_equal(coincidence->fTs[swap][idx2], aligned.fT));
            coincidence->fTs[swap][idx2] = aligned.fT;
        }
    }
}

void SkOpContour::alignCoincidence(const SkOpSegment::AlignedSpan& aligned,
        SkTArray<SkCoincidence, true>* coincidences) {
    int count = coincidences->count();
    for (int index = 0; index < count; ++index) {
        SkCoincidence& coincidence = (*coincidences)[index];
        int thisIndex = coincidence.fSegments[0];
        const SkOpSegment* thisOne = &fSegments[thisIndex];
        const SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        const SkOpSegment* other = &otherContour->fSegments[otherIndex];
        if (thisOne == aligned.fOther1 && other == aligned.fOther2) {
            align(aligned, false, &coincidence);
        } else if (thisOne == aligned.fOther2 && other == aligned.fOther1) {
            align(aligned, true, &coincidence);
        }
    }
}

void SkOpContour::alignTPt(int segmentIndex, const SkOpContour* other, int otherIndex, 
        bool swap, int tIndex, SkIntersections* ts, SkPoint* point) const {
    int zeroPt;
    if ((zeroPt = alignT(swap, tIndex, ts)) >= 0) {
        alignPt(segmentIndex, point, zeroPt);
    }
    if ((zeroPt = other->alignT(!swap, tIndex, ts)) >= 0) {
        other->alignPt(otherIndex, point, zeroPt);
    }
}

void SkOpContour::alignPt(int index, SkPoint* point, int zeroPt) const {
    const SkOpSegment& segment = fSegments[index];
    if (0 == zeroPt) {     
        *point = segment.pts()[0];
    } else {
        *point = segment.pts()[SkPathOpsVerbToPoints(segment.verb())];
    }
}

int SkOpContour::alignT(bool swap, int tIndex, SkIntersections* ts) const {
    double tVal = (*ts)[swap][tIndex];
    if (tVal != 0 && precisely_zero(tVal)) {
        ts->set(swap, tIndex, 0);
        return 0;
    } 
     if (tVal != 1 && precisely_equal(tVal, 1)) {
        ts->set(swap, tIndex, 1);
        return 1;
    }
    return -1;
}

bool SkOpContour::calcAngles() {
    int segmentCount = fSegments.count();
    for (int test = 0; test < segmentCount; ++test) {
        if (!fSegments[test].calcAngles()) {
            return false;
        }
    }
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
    // if there are multiple pairs of partial coincidence that share an edge, see if the opposite
    // are also coincident
    for (int index = 0; index < count - 1; ++index) {
        const SkCoincidence& coincidence = fPartialCoincidences[index];
        int thisIndex = coincidence.fSegments[0];
        SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        for (int idx2 = 1; idx2 < count; ++idx2) {
            const SkCoincidence& innerCoin = fPartialCoincidences[idx2];
            int innerThisIndex = innerCoin.fSegments[0];
            if (thisIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 1, innerCoin, 1, true);
            }
            if (this == otherContour && otherIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 0, innerCoin, 1, true);
            }
            SkOpContour* innerOtherContour = innerCoin.fOther;
            innerThisIndex = innerCoin.fSegments[1];
            if (this == innerOtherContour && thisIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 1, innerCoin, 0, true);
            }
            if (otherContour == innerOtherContour && otherIndex == innerThisIndex) {
                checkCoincidentPair(coincidence, 0, innerCoin, 0, true);
            }
        }
    }
}

void SkOpContour::checkCoincidentPair(const SkCoincidence& oneCoin, int oneIdx,
        const SkCoincidence& twoCoin, int twoIdx, bool partial) {
    SkASSERT((oneIdx ? this : oneCoin.fOther) == (twoIdx ? this : twoCoin.fOther));
    SkASSERT(oneCoin.fSegments[!oneIdx] == twoCoin.fSegments[!twoIdx]);
    // look for common overlap
    double min = SK_ScalarMax;
    double max = SK_ScalarMin;
    double min1 = oneCoin.fTs[!oneIdx][0];
    double max1 = oneCoin.fTs[!oneIdx][1];
    double min2 = twoCoin.fTs[!twoIdx][0];
    double max2 = twoCoin.fTs[!twoIdx][1];
    bool cancelers = (min1 < max1) != (min2 < max2);
    if (min1 > max1) {
        SkTSwap(min1, max1);
    }
    if (min2 > max2) {
        SkTSwap(min2, max2);
    }
    if (between(min1, min2, max1)) {
        min = min2;
    }
    if (between(min1, max2, max1)) {
        max = max2;
    }
    if (between(min2, min1, max2)) {
        min = SkTMin(min, min1);
    }
    if (between(min2, max1, max2)) {
        max = SkTMax(max, max1);
    }
    if (min >= max) {
        return;  // no overlap
    }
    // look to see if opposite are different segments
    int seg1Index = oneCoin.fSegments[oneIdx];
    int seg2Index = twoCoin.fSegments[twoIdx];
    if (seg1Index == seg2Index) {
        return;
    }
    SkOpContour* contour1 = oneIdx ? oneCoin.fOther : this;
    SkOpContour* contour2 = twoIdx ? twoCoin.fOther : this;
    SkOpSegment* segment1 = &contour1->fSegments[seg1Index];
    SkOpSegment* segment2 = &contour2->fSegments[seg2Index];
    // find opposite t value ranges corresponding to reference min/max range
    const SkOpContour* refContour = oneIdx ? this : oneCoin.fOther;
    const int refSegIndex = oneCoin.fSegments[!oneIdx];
    const SkOpSegment* refSegment = &refContour->fSegments[refSegIndex];
    int seg1Start = segment1->findOtherT(min, refSegment);
    int seg1End = segment1->findOtherT(max, refSegment);
    int seg2Start = segment2->findOtherT(min, refSegment);
    int seg2End = segment2->findOtherT(max, refSegment);
    // if the opposite pairs already contain min/max, we're done
    if (seg1Start >= 0 && seg1End >= 0 && seg2Start >= 0 && seg2End >= 0) {
        return;
    }
    double loEnd = SkTMin(min1, min2);
    double hiEnd = SkTMax(max1, max2);
    // insert the missing coincident point(s)
    double missingT1 = -1;
    double otherT1 = -1;
    if (seg1Start < 0) {
        if (seg2Start < 0) {
            return;
        }
        missingT1 = segment1->calcMissingTStart(refSegment, loEnd, min, max, hiEnd,
                segment2, seg1End);
        if (missingT1 < 0) {
            return;
        }
        const SkOpSpan* missingSpan = &segment2->span(seg2Start);
        otherT1 = missingSpan->fT;
    } else if (seg2Start < 0) {
        SkASSERT(seg1Start >= 0);
        missingT1 = segment2->calcMissingTStart(refSegment, loEnd, min, max, hiEnd,
                segment1, seg2End);
        if (missingT1 < 0) {
            return;
        }
        const SkOpSpan* missingSpan = &segment1->span(seg1Start);
        otherT1 = missingSpan->fT;
    }
    SkPoint missingPt1;
    SkOpSegment* addTo1 = NULL;
    SkOpSegment* addOther1 = seg1Start < 0 ? segment2 : segment1;
    int minTIndex = refSegment->findExactT(min, addOther1);
    SkASSERT(minTIndex >= 0);
    if (missingT1 >= 0) {
        missingPt1 = refSegment->span(minTIndex).fPt;
        addTo1 = seg1Start < 0 ? segment1 : segment2;
    }
    double missingT2 = -1;
    double otherT2 = -1;
    if (seg1End < 0) {
        if (seg2End < 0) {
            return;
        }
        missingT2 = segment1->calcMissingTEnd(refSegment, loEnd, min, max, hiEnd,
                segment2, seg1Start);
        if (missingT2 < 0) {
            return;
        }
        const SkOpSpan* missingSpan = &segment2->span(seg2End);
        otherT2 = missingSpan->fT;
    } else if (seg2End < 0) {
        SkASSERT(seg1End >= 0);
        missingT2 = segment2->calcMissingTEnd(refSegment, loEnd, min, max, hiEnd,
                segment1, seg2Start);
        if (missingT2 < 0) {
            return;
        }
        const SkOpSpan* missingSpan = &segment1->span(seg1End);
        otherT2 = missingSpan->fT;
    }
    SkPoint missingPt2;
    SkOpSegment* addTo2 = NULL;
    SkOpSegment* addOther2 = seg1End < 0 ? segment2 : segment1;
    int maxTIndex = refSegment->findExactT(max, addOther2);
    SkASSERT(maxTIndex >= 0);
    if (missingT2 >= 0) {
        missingPt2 = refSegment->span(maxTIndex).fPt;
        addTo2 = seg1End < 0 ? segment1 : segment2;
    }
    if (missingT1 >= 0) {
        addTo1->pinT(missingPt1, &missingT1);
        addTo1->addTPair(missingT1, addOther1, otherT1, false, missingPt1);
    } else {
        SkASSERT(minTIndex >= 0);
        missingPt1 = refSegment->span(minTIndex).fPt;
    }
    if (missingT2 >= 0) {
        addTo2->pinT(missingPt2, &missingT2);
        addTo2->addTPair(missingT2, addOther2, otherT2, false, missingPt2);
    } else {
        SkASSERT(minTIndex >= 0);
        missingPt2 = refSegment->span(maxTIndex).fPt;
    }
    if (!partial) {
        return;
    }
    if (cancelers) {
        if (missingT1 >= 0) {
            addTo1->addTCancel(missingPt1, missingPt2, addOther1);
        } else {
            addTo2->addTCancel(missingPt1, missingPt2, addOther2);
        }
    } else if (missingT1 >= 0) {
        addTo1->addTCoincident(missingPt1, missingPt2, addTo1 == addTo2 ? missingT2 : otherT2,
                addOther1);
    } else {
        addTo2->addTCoincident(missingPt2, missingPt1, addTo2 == addTo1 ? missingT1 : otherT1,
                addOther2);
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
        if (thisOne.done()) {
            continue;
        }
        SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        SkOpSegment& other = otherContour->fSegments[otherIndex];
        if (other.done()) {
            continue;
        }
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
        const SkPoint* startPt = &coincidence.fPts[0][0];
        const SkPoint* endPt = &coincidence.fPts[0][1];
        if (swapStart) {
            SkTSwap(startT, endT);
            SkTSwap(oStartT, oEndT);
            SkTSwap(startPt, endPt);
        }
        bool cancel = swapOther != swapStart;
        int step = swapStart ? -1 : 1;
        int oStep = swapOther ? -1 : 1;
        double oMatchStart = cancel ? oEndT : oStartT;
        if (partial ? startT != 0 || oMatchStart != 0 : (startT == 0) != (oMatchStart == 0)) {
            bool added = false;
            if (oMatchStart != 0) {
                const SkPoint& oMatchStartPt = cancel ? *endPt : *startPt;
                added = thisOne.joinCoincidence(&other, oMatchStart, oMatchStartPt, oStep, cancel);
            }
            if (!cancel && startT != 0 && !added) {
                (void) other.joinCoincidence(&thisOne, startT, *startPt, step, cancel);
            }
        }
        double oMatchEnd = cancel ? oStartT : oEndT;
        if (partial ? endT != 1 || oMatchEnd != 1 : (endT == 1) != (oMatchEnd == 1)) {
            bool added = false;
            if (cancel && endT != 1 && !added) {
                (void) other.joinCoincidence(&thisOne, endT, *endPt, -step, cancel);
            }
        }
    }
}

void SkOpContour::calcCommonCoincidentWinding(const SkCoincidence& coincidence) {
    if (coincidence.fNearly[0] && coincidence.fNearly[1]) {
        return;
    }
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
    const SkPoint* startPt = &coincidence.fPts[0][0];
    const SkPoint* endPt = &coincidence.fPts[0][1];
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

void SkOpContour::resolveNearCoincidence() {
    int count = fCoincidences.count();
    for (int index = 0; index < count; ++index) {
        SkCoincidence& coincidence = fCoincidences[index];
        if (!coincidence.fNearly[0] || !coincidence.fNearly[1]) {
            continue;
        }
        int thisIndex = coincidence.fSegments[0];
        SkOpSegment& thisOne = fSegments[thisIndex];
        SkOpContour* otherContour = coincidence.fOther;
        int otherIndex = coincidence.fSegments[1];
        SkOpSegment& other = otherContour->fSegments[otherIndex];
        if ((thisOne.done() || other.done()) && thisOne.complete() && other.complete()) {
            // OPTIMIZATION: remove from coincidence array
            continue;
        }
    #if DEBUG_CONCIDENT
        thisOne.debugShowTs("-");
        other.debugShowTs("o");
    #endif
        double startT = coincidence.fTs[0][0];
        double endT = coincidence.fTs[0][1];
        bool cancelers;
        if ((cancelers = startT > endT)) {
            SkTSwap<double>(startT, endT);
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
            thisOne.blindCancel(coincidence, &other);
        } else {
            thisOne.blindCoincident(coincidence, &other);
        }
    }
}

void SkOpContour::sortAngles() {
    int segmentCount = fSegments.count();
    for (int test = 0; test < segmentCount; ++test) {
        fSegments[test].sortAngles();
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
        SkPoint testXY = testSegment->activeLeftTop(NULL);
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
