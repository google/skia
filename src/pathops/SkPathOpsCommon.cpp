/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"
#include "SkPathWriter.h"
#include "SkTSort.h"

static int contourRangeCheckY(const SkTArray<SkOpContour*, true>& contourList, SkOpSegment** currentPtr,
                              int* indexPtr, int* endIndexPtr, double* bestHit, SkScalar* bestDx,
                              bool* tryAgain, double* midPtr, bool opp) {
    const int index = *indexPtr;
    const int endIndex = *endIndexPtr;
    const double mid = *midPtr;
    const SkOpSegment* current = *currentPtr;
    double tAtMid = current->tAtMid(index, endIndex, mid);
    SkPoint basePt = current->ptAtT(tAtMid);
    int contourCount = contourList.count();
    SkScalar bestY = SK_ScalarMin;
    SkOpSegment* bestSeg = NULL;
    int bestTIndex = 0;
    bool bestOpp;
    bool hitSomething = false;
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = contourList[cTest];
        bool testOpp = contour->operand() ^ current->operand() ^ opp;
        if (basePt.fY < contour->bounds().fTop) {
            continue;
        }
        if (bestY > contour->bounds().fBottom) {
            continue;
        }
        int segmentCount = contour->segments().count();
        for (int test = 0; test < segmentCount; ++test) {
            SkOpSegment* testSeg = &contour->segments()[test];
            SkScalar testY = bestY;
            double testHit;
            int testTIndex = testSeg->crossedSpanY(basePt, &testY, &testHit, &hitSomething, tAtMid,
                    testOpp, testSeg == current);
            if (testTIndex < 0) {
                if (testTIndex == SK_MinS32) {
                    hitSomething = true;
                    bestSeg = NULL;
                    goto abortContours;  // vertical encountered, return and try different point
                }
                continue;
            }
            if (testSeg == current && current->betweenTs(index, testHit, endIndex)) {
                double baseT = current->t(index);
                double endT = current->t(endIndex);
                double newMid = (testHit - baseT) / (endT - baseT);
#if DEBUG_WINDING
                double midT = current->tAtMid(index, endIndex, mid);
                SkPoint midXY = current->xyAtT(midT);
                double newMidT = current->tAtMid(index, endIndex, newMid);
                SkPoint newXY = current->xyAtT(newMidT);
                SkDebugf("%s [%d] mid=%1.9g->%1.9g s=%1.9g (%1.9g,%1.9g) m=%1.9g (%1.9g,%1.9g)"
                        " n=%1.9g (%1.9g,%1.9g) e=%1.9g (%1.9g,%1.9g)\n", __FUNCTION__,
                        current->debugID(), mid, newMid,
                        baseT, current->xAtT(index), current->yAtT(index),
                        baseT + mid * (endT - baseT), midXY.fX, midXY.fY,
                        baseT + newMid * (endT - baseT), newXY.fX, newXY.fY,
                        endT, current->xAtT(endIndex), current->yAtT(endIndex));
#endif
                *midPtr = newMid * 2;  // calling loop with divide by 2 before continuing
                return SK_MinS32;
            }
            bestSeg = testSeg;
            *bestHit = testHit;
            bestOpp = testOpp;
            bestTIndex = testTIndex;
            bestY = testY;
        }
    }
abortContours:
    int result;
    if (!bestSeg) {
        result = hitSomething ? SK_MinS32 : 0;
    } else {
        if (bestSeg->windSum(bestTIndex) == SK_MinS32) {
            *currentPtr = bestSeg;
            *indexPtr = bestTIndex;
            *endIndexPtr = bestSeg->nextSpan(bestTIndex, 1);
            SkASSERT(*indexPtr != *endIndexPtr && *indexPtr >= 0 && *endIndexPtr >= 0);
            *tryAgain = true;
            return 0;
        }
        result = bestSeg->windingAtT(*bestHit, bestTIndex, bestOpp, bestDx);
        SkASSERT(result == SK_MinS32 || *bestDx);
    }
    double baseT = current->t(index);
    double endT = current->t(endIndex);
    *bestHit = baseT + mid * (endT - baseT);
    return result;
}

SkOpSegment* FindUndone(SkTArray<SkOpContour*, true>& contourList, int* start, int* end) {
    int contourCount = contourList.count();
    SkOpSegment* result;
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = contourList[cIndex];
        result = contour->undoneSegment(start, end);
        if (result) {
            return result;
        }
    }
    return NULL;
}

SkOpSegment* FindChase(SkTDArray<SkOpSpan*>& chase, int& tIndex, int& endIndex) {
    while (chase.count()) {
        SkOpSpan* span;
        chase.pop(&span);
        const SkOpSpan& backPtr = span->fOther->span(span->fOtherIndex);
        SkOpSegment* segment = backPtr.fOther;
        tIndex = backPtr.fOtherIndex;
        SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
        int done = 0;
        if (segment->activeAngle(tIndex, &done, &angles)) {
            SkOpAngle* last = angles.end() - 1;
            tIndex = last->start();
            endIndex = last->end();
   #if TRY_ROTATE
            *chase.insert(0) = span;
   #else
            *chase.append() = span;
   #endif
            return last->segment();
        }
        if (done == angles.count()) {
            continue;
        }
        SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
        bool sortable = SkOpSegment::SortAngles(angles, &sorted,
                SkOpSegment::kMayBeUnordered_SortAngleKind);
        int angleCount = sorted.count();
#if DEBUG_SORT
        sorted[0]->segment()->debugShowSort(__FUNCTION__, sorted, 0, 0, 0, sortable);
#endif
        if (!sortable) {
            continue;
        }
        // find first angle, initialize winding to computed fWindSum
        int firstIndex = -1;
        const SkOpAngle* angle;
        int winding;
        do {
            angle = sorted[++firstIndex];
            segment = angle->segment();
            winding = segment->windSum(angle);
        } while (winding == SK_MinS32);
        int spanWinding = segment->spanSign(angle->start(), angle->end());
    #if DEBUG_WINDING
        SkDebugf("%s winding=%d spanWinding=%d\n",
                __FUNCTION__, winding, spanWinding);
    #endif
        // turn span winding into contour winding
        if (spanWinding * winding < 0) {
            winding += spanWinding;
        }
    #if DEBUG_SORT
        segment->debugShowSort(__FUNCTION__, sorted, firstIndex, winding, 0, sortable);
    #endif
        // we care about first sign and whether wind sum indicates this
        // edge is inside or outside. Maybe need to pass span winding
        // or first winding or something into this function?
        // advance to first undone angle, then return it and winding
        // (to set whether edges are active or not)
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        angle = sorted[firstIndex];
        winding -= angle->segment()->spanSign(angle);
        do {
            SkASSERT(nextIndex != firstIndex);
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            angle = sorted[nextIndex];
            segment = angle->segment();
            int maxWinding = winding;
            winding -= segment->spanSign(angle);
    #if DEBUG_SORT
            SkDebugf("%s id=%d maxWinding=%d winding=%d sign=%d\n", __FUNCTION__,
                    segment->debugID(), maxWinding, winding, angle->sign());
    #endif
            tIndex = angle->start();
            endIndex = angle->end();
            int lesser = SkMin32(tIndex, endIndex);
            const SkOpSpan& nextSpan = segment->span(lesser);
            if (!nextSpan.fDone) {
            // FIXME: this be wrong? assign startWinding if edge is in
            // same direction. If the direction is opposite, winding to
            // assign is flipped sign or +/- 1?
                if (SkOpSegment::UseInnerWinding(maxWinding, winding)) {
                    maxWinding = winding;
                }
                segment->markAndChaseWinding(angle, maxWinding, 0);
                break;
            }
        } while (++nextIndex != lastIndex);
        *chase.insert(0) = span;
        return segment;
    }
    return NULL;
}

#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
void DebugShowActiveSpans(SkTArray<SkOpContour*, true>& contourList) {
    int index;
    for (index = 0; index < contourList.count(); ++ index) {
        contourList[index]->debugShowActiveSpans();
    }
}
#endif

static SkOpSegment* findSortableTop(const SkTArray<SkOpContour*, true>& contourList,
                                    int* index, int* endIndex, SkPoint* topLeft, bool* unsortable,
                                    bool* done, bool onlySortable) {
    SkOpSegment* result;
    do {
        SkPoint bestXY = {SK_ScalarMax, SK_ScalarMax};
        int contourCount = contourList.count();
        SkOpSegment* topStart = NULL;
        *done = true;
        for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
            SkOpContour* contour = contourList[cIndex];
            if (contour->done()) {
                continue;
            }
            const SkPathOpsBounds& bounds = contour->bounds();
            if (bounds.fBottom < topLeft->fY) {
                *done = false;
                continue;
            }
            if (bounds.fBottom == topLeft->fY && bounds.fRight < topLeft->fX) {
                *done = false;
                continue;
            }
            contour->topSortableSegment(*topLeft, &bestXY, &topStart);
            if (!contour->done()) {
                *done = false;
            }
        }
        if (!topStart) {
            return NULL;
        }
        *topLeft = bestXY;
        result = topStart->findTop(index, endIndex, unsortable, onlySortable);
    } while (!result);
    if (result) {
        *unsortable = false;
    }
    return result;
}

static int rightAngleWinding(const SkTArray<SkOpContour*, true>& contourList,
                             SkOpSegment** current, int* index, int* endIndex, double* tHit,
                             SkScalar* hitDx, bool* tryAgain, bool opp) {
    double test = 0.9;
    int contourWinding;
    do {
        contourWinding = contourRangeCheckY(contourList, current, index, endIndex, tHit, hitDx,
                tryAgain, &test, opp);
        if (contourWinding != SK_MinS32 || *tryAgain) {
            return contourWinding;
        }
        test /= 2;
    } while (!approximately_negative(test));
    SkASSERT(0);  // should be OK to comment out, but interested when this hits
    return contourWinding;
}

static void skipVertical(const SkTArray<SkOpContour*, true>& contourList,
        SkOpSegment** current, int* index, int* endIndex) {
    if (!(*current)->isVertical(*index, *endIndex)) {
        return;
    }
    int contourCount = contourList.count();
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = contourList[cIndex];
        if (contour->done()) {
            continue;
        }
        *current = contour->nonVerticalSegment(index, endIndex);
        if (*current) {
            return;
        }
    }
}

SkOpSegment* FindSortableTop(const SkTArray<SkOpContour*, true>& contourList,
        SkOpAngle::IncludeType angleIncludeType, bool* firstContour, int* indexPtr,
        int* endIndexPtr, SkPoint* topLeft, bool* unsortable, bool* done) {
    SkOpSegment* current = findSortableTop(contourList, indexPtr, endIndexPtr, topLeft, unsortable,
            done, true);
    if (!current) {
        return NULL;
    }
    const int index = *indexPtr;
    const int endIndex = *endIndexPtr;
    if (*firstContour) {
        current->initWinding(index, endIndex);
        *firstContour = false;
        return current;
    }
    int minIndex = SkMin32(index, endIndex);
    int sumWinding = current->windSum(minIndex);
    if (sumWinding != SK_MinS32) {
        return current;
    }
    SkASSERT(current->windSum(SkMin32(index, endIndex)) == SK_MinS32);
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
    sumWinding = current->computeSum(index, endIndex, angleIncludeType, &angles, &sorted);
    if (sumWinding != SK_MinS32 && sumWinding != SK_NaN32) {
        return current;
    }
    int contourWinding;
    int oppContourWinding = 0;
    // the simple upward projection of the unresolved points hit unsortable angles
    // shoot rays at right angles to the segment to find its winding, ignoring angle cases
    bool tryAgain;
    double tHit;
    SkScalar hitDx = 0;
    SkScalar hitOppDx = 0;
    do {
        // if current is vertical, find another candidate which is not
        // if only remaining candidates are vertical, then they can be marked done
        SkASSERT(*indexPtr != *endIndexPtr && *indexPtr >= 0 && *endIndexPtr >= 0);
        skipVertical(contourList, &current, indexPtr, endIndexPtr);

        SkASSERT(*indexPtr != *endIndexPtr && *indexPtr >= 0 && *endIndexPtr >= 0);
        tryAgain = false;
        contourWinding = rightAngleWinding(contourList, &current, indexPtr, endIndexPtr, &tHit,
                &hitDx, &tryAgain, false);
        if (tryAgain) {
            continue;
        }
        if (angleIncludeType < SkOpAngle::kBinarySingle) {
            break;
        }
        oppContourWinding = rightAngleWinding(contourList, &current, indexPtr, endIndexPtr, &tHit,
                &hitOppDx, &tryAgain, true);
    } while (tryAgain);
    current->initWinding(*indexPtr, *endIndexPtr, tHit, contourWinding, hitDx, oppContourWinding,
            hitOppDx);
    return current;
}

void CheckEnds(SkTArray<SkOpContour*, true>* contourList) {
    // it's hard to determine if the end of a cubic or conic nearly intersects another curve.
    // instead, look to see if the connecting curve intersected at that same end.
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->checkEnds();
    }
}

// A tiny interval may indicate an undiscovered coincidence. Find and fix.
void CheckTiny(SkTArray<SkOpContour*, true>* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->checkTiny();
    }
}

void FixOtherTIndex(SkTArray<SkOpContour*, true>* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->fixOtherTIndex();
    }
}

void SortSegments(SkTArray<SkOpContour*, true>* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->sortSegments();
    }
}

void MakeContourList(SkTArray<SkOpContour>& contours, SkTArray<SkOpContour*, true>& list,
                     bool evenOdd, bool oppEvenOdd) {
    int count = contours.count();
    if (count == 0) {
        return;
    }
    for (int index = 0; index < count; ++index) {
        SkOpContour& contour = contours[index];
        contour.setOppXor(contour.operand() ? evenOdd : oppEvenOdd);
        list.push_back(&contour);
    }
    SkTQSort<SkOpContour>(list.begin(), list.end() - 1);
}

class DistanceLessThan {
public:
    DistanceLessThan(double* distances) : fDistances(distances) { }
    double* fDistances;
    bool operator()(const int one, const int two) {
        return fDistances[one] < fDistances[two];
    }
};

    /*
        check start and end of each contour
        if not the same, record them
        match them up
        connect closest
        reassemble contour pieces into new path
    */
void Assemble(const SkPathWriter& path, SkPathWriter* simple) {
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("%s\n", __FUNCTION__);
#endif
    SkTArray<SkOpContour> contours;
    SkOpEdgeBuilder builder(path, contours);
    builder.finish();
    int count = contours.count();
    int outer;
    SkTArray<int, true> runs(count);  // indices of partial contours
    for (outer = 0; outer < count; ++outer) {
        const SkOpContour& eContour = contours[outer];
        const SkPoint& eStart = eContour.start();
        const SkPoint& eEnd = eContour.end();
#if DEBUG_ASSEMBLE
        SkDebugf("%s contour", __FUNCTION__);
        if (!SkDPoint::ApproximatelyEqual(eStart, eEnd)) {
            SkDebugf("[%d]", runs.count());
        } else {
            SkDebugf("   ");
        }
        SkDebugf(" start=(%1.9g,%1.9g) end=(%1.9g,%1.9g)\n",
                eStart.fX, eStart.fY, eEnd.fX, eEnd.fY);
#endif
        if (SkDPoint::ApproximatelyEqual(eStart, eEnd)) {
            eContour.toPath(simple);
            continue;
        }
        runs.push_back(outer);
    }
    count = runs.count();
    if (count == 0) {
        return;
    }
    SkTArray<int, true> sLink, eLink;
    sLink.push_back_n(count);
    eLink.push_back_n(count);
    int rIndex, iIndex;
    for (rIndex = 0; rIndex < count; ++rIndex) {
        sLink[rIndex] = eLink[rIndex] = SK_MaxS32;
    }
    const int ends = count * 2;  // all starts and ends
    const int entries = (ends - 1) * count;  // folded triangle : n * (n - 1) / 2
    SkTArray<double, true> distances;
    distances.push_back_n(entries);
    for (rIndex = 0; rIndex < ends - 1; ++rIndex) {
        outer = runs[rIndex >> 1];
        const SkOpContour& oContour = contours[outer];
        const SkPoint& oPt = rIndex & 1 ? oContour.end() : oContour.start();
        const int row = rIndex < count - 1 ? rIndex * ends : (ends - rIndex - 2)
                * ends - rIndex - 1;
        for (iIndex = rIndex + 1; iIndex < ends; ++iIndex) {
            int inner = runs[iIndex >> 1];
            const SkOpContour& iContour = contours[inner];
            const SkPoint& iPt = iIndex & 1 ? iContour.end() : iContour.start();
            double dx = iPt.fX - oPt.fX;
            double dy = iPt.fY - oPt.fY;
            double dist = dx * dx + dy * dy;
            distances[row + iIndex] = dist;  // oStart distance from iStart
        }
    }
    SkTArray<int, true> sortedDist;
    sortedDist.push_back_n(entries);
    for (rIndex = 0; rIndex < entries; ++rIndex) {
        sortedDist[rIndex] = rIndex;
    }
    SkTQSort<int>(sortedDist.begin(), sortedDist.end() - 1, DistanceLessThan(distances.begin()));
    int remaining = count;  // number of start/end pairs
    for (rIndex = 0; rIndex < entries; ++rIndex) {
        int pair = sortedDist[rIndex];
        int row = pair / ends;
        int col = pair - row * ends;
        int thingOne = row < col ? row : ends - row - 2;
        int ndxOne = thingOne >> 1;
        bool endOne = thingOne & 1;
        int* linkOne = endOne ? eLink.begin() : sLink.begin();
        if (linkOne[ndxOne] != SK_MaxS32) {
            continue;
        }
        int thingTwo = row < col ? col : ends - row + col - 1;
        int ndxTwo = thingTwo >> 1;
        bool endTwo = thingTwo & 1;
        int* linkTwo = endTwo ? eLink.begin() : sLink.begin();
        if (linkTwo[ndxTwo] != SK_MaxS32) {
            continue;
        }
        SkASSERT(&linkOne[ndxOne] != &linkTwo[ndxTwo]);
        bool flip = endOne == endTwo;
        linkOne[ndxOne] = flip ? ~ndxTwo : ndxTwo;
        linkTwo[ndxTwo] = flip ? ~ndxOne : ndxOne;
        if (!--remaining) {
            break;
        }
    }
    SkASSERT(!remaining);
#if DEBUG_ASSEMBLE
    for (rIndex = 0; rIndex < count; ++rIndex) {
        int s = sLink[rIndex];
        int e = eLink[rIndex];
        SkDebugf("%s %c%d <- s%d - e%d -> %c%d\n", __FUNCTION__, s < 0 ? 's' : 'e',
                s < 0 ? ~s : s, rIndex, rIndex, e < 0 ? 'e' : 's', e < 0 ? ~e : e);
    }
#endif
    rIndex = 0;
    do {
        bool forward = true;
        bool first = true;
        int sIndex = sLink[rIndex];
        SkASSERT(sIndex != SK_MaxS32);
        sLink[rIndex] = SK_MaxS32;
        int eIndex;
        if (sIndex < 0) {
            eIndex = sLink[~sIndex];
            sLink[~sIndex] = SK_MaxS32;
        } else {
            eIndex = eLink[sIndex];
            eLink[sIndex] = SK_MaxS32;
        }
        SkASSERT(eIndex != SK_MaxS32);
#if DEBUG_ASSEMBLE
        SkDebugf("%s sIndex=%c%d eIndex=%c%d\n", __FUNCTION__, sIndex < 0 ? 's' : 'e',
                    sIndex < 0 ? ~sIndex : sIndex, eIndex < 0 ? 's' : 'e',
                    eIndex < 0 ? ~eIndex : eIndex);
#endif
        do {
            outer = runs[rIndex];
            const SkOpContour& contour = contours[outer];
            if (first) {
                first = false;
                const SkPoint* startPtr = &contour.start();
                simple->deferredMove(startPtr[0]);
            }
            if (forward) {
                contour.toPartialForward(simple);
            } else {
                contour.toPartialBackward(simple);
            }
#if DEBUG_ASSEMBLE
            SkDebugf("%s rIndex=%d eIndex=%s%d close=%d\n", __FUNCTION__, rIndex,
                eIndex < 0 ? "~" : "", eIndex < 0 ? ~eIndex : eIndex,
                sIndex == ((rIndex != eIndex) ^ forward ? eIndex : ~eIndex));
#endif
            if (sIndex == ((rIndex != eIndex) ^ forward ? eIndex : ~eIndex)) {
                simple->close();
                break;
            }
            if (forward) {
                eIndex = eLink[rIndex];
                SkASSERT(eIndex != SK_MaxS32);
                eLink[rIndex] = SK_MaxS32;
                if (eIndex >= 0) {
                    SkASSERT(sLink[eIndex] == rIndex);
                    sLink[eIndex] = SK_MaxS32;
                } else {
                    SkASSERT(eLink[~eIndex] == ~rIndex);
                    eLink[~eIndex] = SK_MaxS32;
                }
            } else {
                eIndex = sLink[rIndex];
                SkASSERT(eIndex != SK_MaxS32);
                sLink[rIndex] = SK_MaxS32;
                if (eIndex >= 0) {
                    SkASSERT(eLink[eIndex] == rIndex);
                    eLink[eIndex] = SK_MaxS32;
                } else {
                    SkASSERT(sLink[~eIndex] == ~rIndex);
                    sLink[~eIndex] = SK_MaxS32;
                }
            }
            rIndex = eIndex;
            if (rIndex < 0) {
                forward ^= 1;
                rIndex = ~rIndex;
            }
        } while (true);
        for (rIndex = 0; rIndex < count; ++rIndex) {
            if (sLink[rIndex] != SK_MaxS32) {
                break;
            }
        }
    } while (rIndex < count);
#if DEBUG_ASSEMBLE
    for (rIndex = 0; rIndex < count; ++rIndex) {
       SkASSERT(sLink[rIndex] == SK_MaxS32);
       SkASSERT(eLink[rIndex] == SK_MaxS32);
    }
#endif
}
