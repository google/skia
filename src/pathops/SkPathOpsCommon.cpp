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
#include "SkTSort.h"

static int contourRangeCheckY(const SkTDArray<SkOpContour* >& contourList,
        SkOpSegment** currentPtr, SkOpSpanBase** startPtr, SkOpSpanBase** endPtr,
        double* bestHit, SkScalar* bestDx, bool* tryAgain, double* midPtr, bool opp) {
    SkOpSpanBase* start = *startPtr;
    SkOpSpanBase* end = *endPtr;
    const double mid = *midPtr;
    const SkOpSegment* current = *currentPtr;
    double tAtMid = SkOpSegment::TAtMid(start, end, mid);
    SkPoint basePt = current->ptAtT(tAtMid);
    int contourCount = contourList.count();
    SkScalar bestY = SK_ScalarMin;
    SkOpSegment* bestSeg = NULL;
    SkOpSpan* bestTSpan = NULL;
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
        SkOpSegment* testSeg = contour->first();
        SkASSERT(testSeg);
        do {
            SkScalar testY = bestY;
            double testHit;
            bool vertical;
            SkOpSpan* testTSpan = testSeg->crossedSpanY(basePt, tAtMid, testOpp,
                    testSeg == current, &testY, &testHit, &hitSomething, &vertical);
            if (!testTSpan) {
                if (vertical) {
                    hitSomething = true;
                    bestSeg = NULL;
                    goto abortContours;  // vertical encountered, return and try different point
                }
                continue;
            }
            if (testSeg == current && SkOpSegment::BetweenTs(start, testHit, end)) {
                double baseT = start->t();
                double endT = end->t();
                double newMid = (testHit - baseT) / (endT - baseT);
#if DEBUG_WINDING
                double midT = SkOpSegment::TAtMid(start, end, mid);
                SkPoint midXY = current->ptAtT(midT);
                double newMidT = SkOpSegment::TAtMid(start, end, newMid);
                SkPoint newXY = current->ptAtT(newMidT);
                SkDebugf("%s [%d] mid=%1.9g->%1.9g s=%1.9g (%1.9g,%1.9g) m=%1.9g (%1.9g,%1.9g)"
                        " n=%1.9g (%1.9g,%1.9g) e=%1.9g (%1.9g,%1.9g)\n", __FUNCTION__,
                        current->debugID(), mid, newMid,
                        baseT, start->pt().fX, start->pt().fY,
                        baseT + mid * (endT - baseT), midXY.fX, midXY.fY,
                        baseT + newMid * (endT - baseT), newXY.fX, newXY.fY,
                        endT, end->pt().fX, end->pt().fY);
#endif
                *midPtr = newMid * 2;  // calling loop with divide by 2 before continuing
                return SK_MinS32;
            }
            bestSeg = testSeg;
            *bestHit = testHit;
            bestOpp = testOpp;
            bestTSpan = testTSpan;
            bestY = testY;
        } while ((testSeg = testSeg->next()));
    }
abortContours:
    int result;
    if (!bestSeg) {
        result = hitSomething ? SK_MinS32 : 0;
    } else {
        if (bestTSpan->windSum() == SK_MinS32) {
            *currentPtr = bestSeg;
            *startPtr = bestTSpan;
            *endPtr = bestTSpan->next();
            SkASSERT(*startPtr != *endPtr && *startPtr && *endPtr);
            *tryAgain = true;
            return 0;
        }
        result = bestSeg->windingAtT(*bestHit, bestTSpan, bestOpp, bestDx);
        SkASSERT(result == SK_MinS32 || *bestDx);
    }
    double baseT = (*startPtr)->t();
    double endT = (*endPtr)->t();
    *bestHit = baseT + mid * (endT - baseT);
    return result;
}

SkOpSegment* FindUndone(SkTDArray<SkOpContour* >& contourList, SkOpSpanBase** startPtr,
         SkOpSpanBase** endPtr) {
    int contourCount = contourList.count();
    SkOpSegment* result;
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = contourList[cIndex];
        result = contour->undoneSegment(startPtr, endPtr);
        if (result) {
            return result;
        }
    }
    return NULL;
}

SkOpSegment* FindChase(SkTDArray<SkOpSpanBase*>* chase, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr) {
    while (chase->count()) {
        SkOpSpanBase* span;
        chase->pop(&span);
        SkOpSegment* segment = span->segment();
        *startPtr = span->ptT()->next()->span();
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
            *chase->insert(0) = span;
    #else
            *chase->append() = span;
    #endif
            return last->segment();
        }
        if (done) {
            continue;
        }
        if (!sortable) {
            continue;
        }
        // find first angle, initialize winding to computed wind sum
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
        int sumWinding = segment->updateWindingReverse(angle);
        SkOpSegment* first = NULL;
        firstAngle = angle;
        while ((angle = angle->next()) != firstAngle) {
            segment = angle->segment();
            SkOpSpanBase* start = angle->start();
            SkOpSpanBase* end = angle->end();
            int maxWinding;
            segment->setUpWinding(start, end, &maxWinding, &sumWinding);
            if (!segment->done(angle)) {
                if (!first) {
                    first = segment;
                    *startPtr = start;
                    *endPtr = end;
                }
                // OPTIMIZATION: should this also add to the chase?
                (void) segment->markAngle(maxWinding, sumWinding, angle);
            }
        }
        if (first) {
       #if TRY_ROTATE
            *chase->insert(0) = span;
       #else
            *chase->append() = span;
       #endif
            return first;
        }
    }
    return NULL;
}

#if DEBUG_ACTIVE_SPANS
void DebugShowActiveSpans(SkTDArray<SkOpContour* >& contourList) {
    int index;
    for (index = 0; index < contourList.count(); ++ index) {
        contourList[index]->debugShowActiveSpans();
    }
}
#endif

static SkOpSegment* findTopSegment(const SkTDArray<SkOpContour* >& contourList,
        bool firstPass, SkOpSpanBase** start, SkOpSpanBase** end, SkPoint* topLeft,
        bool* unsortable, bool* done, SkChunkAlloc* allocator) {
    SkOpSegment* result;
    const SkOpSegment* lastTopStart = NULL;
    SkOpSpanBase* lastStart = NULL, * lastEnd = NULL;
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
        result = topStart->findTop(firstPass, start, end, unsortable, allocator);
        if (!result) {
            if (lastTopStart == topStart && lastStart == *start && lastEnd == *end) {
                *done = true;
                return NULL;
            }
            lastTopStart = topStart;
            lastStart = *start;
            lastEnd = *end;
        }
    } while (!result);
    return result;
}

static int rightAngleWinding(const SkTDArray<SkOpContour* >& contourList,
        SkOpSegment** currentPtr, SkOpSpanBase** start, SkOpSpanBase** end, double* tHit,
        SkScalar* hitDx, bool* tryAgain, bool* onlyVertical, bool opp) {
    double test = 0.9;
    int contourWinding;
    do {
        contourWinding = contourRangeCheckY(contourList, currentPtr, start, end,
                tHit, hitDx, tryAgain, &test, opp);
        if (contourWinding != SK_MinS32 || *tryAgain) {
            return contourWinding;
        }
        if (*currentPtr && (*currentPtr)->isVertical()) {
            *onlyVertical = true;
            return contourWinding;
        }
        test /= 2;
    } while (!approximately_negative(test));
    SkASSERT(0);  // FIXME: incomplete functionality
    return contourWinding;
}

static void skipVertical(const SkTDArray<SkOpContour* >& contourList,
        SkOpSegment** current, SkOpSpanBase** start, SkOpSpanBase** end) {
    if (!(*current)->isVertical(*start, *end)) {
        return;
    }
    int contourCount = contourList.count();
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = contourList[cIndex];
        if (contour->done()) {
            continue;
        }
        SkOpSegment* nonVertical = contour->nonVerticalSegment(start, end);
        if (nonVertical) {
            *current = nonVertical;
            return;
        }
    }
    return;
}

struct SortableTop2 {  // error if local in pre-C++11
    SkOpSpanBase* fStart;
    SkOpSpanBase* fEnd;
};

SkOpSegment* FindSortableTop(const SkTDArray<SkOpContour* >& contourList, bool firstPass,
        SkOpAngle::IncludeType angleIncludeType, bool* firstContour, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr, SkPoint* topLeft, bool* unsortable, bool* done, bool* onlyVertical,
        SkChunkAlloc* allocator) {
    SkOpSegment* current = findTopSegment(contourList, firstPass, startPtr, endPtr, topLeft,
            unsortable, done, allocator);
    if (!current) {
        return NULL;
    }
    SkOpSpanBase* start = *startPtr;
    SkOpSpanBase* end = *endPtr;
    SkASSERT(current == start->segment());
    if (*firstContour) {
        current->initWinding(start, end, angleIncludeType);
        *firstContour = false;
        return current;
    }
    SkOpSpan* minSpan = start->starter(end);
    int sumWinding = minSpan->windSum();
    if (sumWinding == SK_MinS32) {
        SkOpSpanBase* iSpan = end;
        SkOpSpanBase* oSpan = start;
        do {
            bool checkFrom = oSpan->t() < iSpan->t();
            if ((checkFrom ? iSpan->fromAngle() : iSpan->upCast()->toAngle()) == NULL) {
                iSpan->addSimpleAngle(checkFrom, allocator);
            }
            sumWinding = current->computeSum(oSpan, iSpan, angleIncludeType);
            SkTSwap(iSpan, oSpan);
        } while (sumWinding == SK_MinS32 && iSpan == start);
    }
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
    // keep track of subsequent returns to detect infinite loops
    SkTDArray<SortableTop2> sortableTops;
    do {
        // if current is vertical, find another candidate which is not
        // if only remaining candidates are vertical, then they can be marked done
        SkASSERT(*startPtr != *endPtr && *startPtr && *endPtr);
        SkASSERT(current == (*startPtr)->segment());
        skipVertical(contourList, &current, startPtr, endPtr);
        SkASSERT(current);  // FIXME: if null, all remaining are vertical
        SkASSERT(*startPtr != *endPtr && *startPtr && *endPtr);
        SkASSERT(current == (*startPtr)->segment());
        tryAgain = false;
        contourWinding = rightAngleWinding(contourList, &current, startPtr, endPtr, &tHit,
                &hitDx, &tryAgain, onlyVertical, false);
        SkASSERT(current == (*startPtr)->segment());
        if (tryAgain) {
            bool giveUp = false;
            int count = sortableTops.count();
            for (int index = 0; index < count; ++index) {
                const SortableTop2& prev = sortableTops[index];
                if (giveUp) {
                    prev.fStart->segment()->markDone(prev.fStart->starter(prev.fEnd));
                } else if (prev.fStart == *startPtr || prev.fEnd == *endPtr) {
                    // remaining edges are non-vertical and cannot have their winding computed
                    // mark them as done and return, and hope that assembly can fill the holes
                    giveUp = true;
                    index = -1;
                }
            }
            if (giveUp) {
                *done = true;
                return NULL;
            }
        }
        SortableTop2* sortableTop = sortableTops.append();
        sortableTop->fStart = *startPtr;
        sortableTop->fEnd = *endPtr;
#if DEBUG_SORT
        SkDebugf("%s current=%d index=%d endIndex=%d tHit=%1.9g hitDx=%1.9g try=%d vert=%d\n",
                __FUNCTION__, current->debugID(), (*startPtr)->debugID(), (*endPtr)->debugID(),
                tHit, hitDx, tryAgain, *onlyVertical);
#endif
        if (*onlyVertical) {
            return current;
        }
        if (tryAgain) {
            continue;
        }
        if (angleIncludeType < SkOpAngle::kBinarySingle) {
            break;
        }
        oppContourWinding = rightAngleWinding(contourList, &current, startPtr, endPtr, &tHit,
                &hitOppDx, &tryAgain, NULL, true);
        SkASSERT(current == (*startPtr)->segment());
    } while (tryAgain);
    bool success = current->initWinding(*startPtr, *endPtr, tHit, contourWinding, hitDx,
            oppContourWinding, hitOppDx);
    if (current->done()) {
        return NULL;
    } else if (!success) {  // check if the span has a valid winding
        SkOpSpan* minSpan = (*startPtr)->t() < (*endPtr)->t() ? (*startPtr)->upCast()
            : (*endPtr)->upCast();
        if (minSpan->windSum() == SK_MinS32) {
            return NULL;
        }
    }
    return current;
}

void MakeContourList(SkOpContour* contour, SkTDArray<SkOpContour* >& list,
                     bool evenOdd, bool oppEvenOdd) {
    do {
        if (contour->count()) {
            contour->setOppXor(contour->operand() ? evenOdd : oppEvenOdd);
            *list.append() = contour;
        }
    } while ((contour = contour->next()));
    if (list.count() < 2) {
        return;
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
    SkOpContour contour;
    SkOpGlobalState globalState(NULL  PATH_OPS_DEBUG_PARAMS(&contour));
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("%s\n", __FUNCTION__);
#endif
    SkChunkAlloc allocator(4096);  // FIXME: constant-ize, tune
    SkOpEdgeBuilder builder(path, &contour, &allocator, &globalState);
    builder.finish(&allocator);
    SkTDArray<const SkOpContour* > runs;  // indices of partial contours
    const SkOpContour* eContour = builder.head();
    do {
        if (!eContour->count()) {
            continue;
        }
        const SkPoint& eStart = eContour->start();
        const SkPoint& eEnd = eContour->end();
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
            eContour->toPath(simple);
            continue;
        }
        *runs.append() = eContour;
    } while ((eContour = eContour->next()));
    int count = runs.count();
    if (count == 0) {
        return;
    }
    SkTDArray<int> sLink, eLink;
    sLink.append(count);
    eLink.append(count);
    int rIndex, iIndex;
    for (rIndex = 0; rIndex < count; ++rIndex) {
        sLink[rIndex] = eLink[rIndex] = SK_MaxS32;
    }
    const int ends = count * 2;  // all starts and ends
    const int entries = (ends - 1) * count;  // folded triangle : n * (n - 1) / 2
    SkTDArray<double> distances;
    distances.append(entries);
    for (rIndex = 0; rIndex < ends - 1; ++rIndex) {
        const SkOpContour* oContour = runs[rIndex >> 1];
        const SkPoint& oPt = rIndex & 1 ? oContour->end() : oContour->start();
        const int row = rIndex < count - 1 ? rIndex * ends : (ends - rIndex - 2)
                * ends - rIndex - 1;
        for (iIndex = rIndex + 1; iIndex < ends; ++iIndex) {
            const SkOpContour* iContour = runs[iIndex >> 1];
            const SkPoint& iPt = iIndex & 1 ? iContour->end() : iContour->start();
            double dx = iPt.fX - oPt.fX;
            double dy = iPt.fY - oPt.fY;
            double dist = dx * dx + dy * dy;
            distances[row + iIndex] = dist;  // oStart distance from iStart
        }
    }
    SkTDArray<int> sortedDist;
    sortedDist.append(entries);
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
            const SkOpContour* contour = runs[rIndex];
            if (first) {
                first = false;
                const SkPoint* startPtr = &contour->start();
                simple->deferredMove(startPtr[0]);
            }
            if (forward) {
                contour->toPartialForward(simple);
            } else {
                contour->toPartialBackward(simple);
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

static void align(SkTDArray<SkOpContour* >* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->align();
    }
}

static void calcAngles(SkTDArray<SkOpContour* >* contourList, SkChunkAlloc* allocator) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->calcAngles(allocator);
    }
}

static void missingCoincidence(SkTDArray<SkOpContour* >* contourList,
        SkOpCoincidence* coincidence, SkChunkAlloc* allocator) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->missingCoincidence(coincidence, allocator);
    }
}

static bool moveNearby(SkTDArray<SkOpContour* >* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        if (!contour->moveNearby()) {
            return false;
        }
    }
    return true;
}

static void sortAngles(SkTDArray<SkOpContour* >* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->sortAngles();
    }
}

static void sortSegments(SkTDArray<SkOpContour* >* contourList) {
    int contourCount = (*contourList).count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        SkOpContour* contour = (*contourList)[cTest];
        contour->sortSegments();
    }
}

bool HandleCoincidence(SkTDArray<SkOpContour* >* contourList, SkOpCoincidence* coincidence,
        SkChunkAlloc* allocator, SkOpGlobalState* globalState) {
    // move t values and points together to eliminate small/tiny gaps
    if (!moveNearby(contourList)) {
        return false;
    }
    align(contourList);  // give all span members common values
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kIntersecting);
#endif
    coincidence->addMissing(allocator);
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kWalking);
#endif
    coincidence->expand();  // check to see if, loosely, coincident ranges may be expanded
    coincidence->mark();  // mark spans of coincident segments as coincident
    missingCoincidence(contourList, coincidence, allocator);  // look for coincidence missed earlier
    if (!coincidence->apply()) {  // adjust the winding value to account for coincident edges
        return false;
    }
    sortSegments(contourList);
    calcAngles(contourList, allocator);
    sortAngles(contourList);
    if (globalState->angleCoincidence()) {
        missingCoincidence(contourList, coincidence, allocator);
        if (!coincidence->apply()) {
            return false;
        }
    }
#if DEBUG_ACTIVE_SPANS
    DebugShowActiveSpans(*contourList);
#endif
    return true;
}
