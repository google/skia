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

const SkOpAngle* AngleWinding(SkOpSpanBase* start, SkOpSpanBase* end, int* windingPtr,
        bool* sortablePtr) {
    // find first angle, initialize winding to computed fWindSum
    SkOpSegment* segment = start->segment();
    const SkOpAngle* angle = segment->spanToAngle(start, end);
    if (!angle) {
        *windingPtr = SK_MinS32;
        return NULL;
    }
    bool computeWinding = false;
    const SkOpAngle* firstAngle = angle;
    bool loop = false;
    bool unorderable = false;
    int winding = SK_MinS32;
    do {
        angle = angle->next();
        unorderable |= angle->unorderable();
        if ((computeWinding = unorderable || (angle == firstAngle && loop))) {
            break;    // if we get here, there's no winding, loop is unorderable
        }
        loop |= angle == firstAngle;
        segment = angle->segment();
        winding = segment->windSum(angle);
    } while (winding == SK_MinS32);
    // if the angle loop contains an unorderable span, the angle order may be useless
    // directly compute the winding in this case for each span
    if (computeWinding) {
        firstAngle = angle;
        winding = SK_MinS32;
        do {
            SkOpSpanBase* startSpan = angle->start();
            SkOpSpanBase* endSpan = angle->end();
            SkOpSpan* lesser = startSpan->starter(endSpan);
            int testWinding = lesser->windSum();
            if (testWinding == SK_MinS32) {
                testWinding = lesser->computeWindSum();
            }
            if (testWinding != SK_MinS32) {
                segment = angle->segment();
                winding = testWinding;
            }
            angle = angle->next();
        } while (angle != firstAngle);
    }
    *sortablePtr = !unorderable;
    *windingPtr = winding;
    return angle;
}

SkOpSegment* FindUndone(SkOpContourHead* contourList, SkOpSpanBase** startPtr,
         SkOpSpanBase** endPtr) {
    SkOpSegment* result;
    SkOpContour* contour = contourList;
    do {
        result = contour->undoneSegment(startPtr, endPtr);
        if (result) {
            return result;
        }
    } while ((contour = contour->next()));
    return NULL;
}

SkOpSegment* FindChase(SkTDArray<SkOpSpanBase*>* chase, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr) {
    while (chase->count()) {
        SkOpSpanBase* span;
        chase->pop(&span);
        SkOpSegment* segment = span->segment();
        *startPtr = span->ptT()->next()->span();
        bool done = true;
        *endPtr = NULL;
        if (SkOpAngle* last = segment->activeAngle(*startPtr, startPtr, endPtr, &done)) {
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
        // find first angle, initialize winding to computed wind sum
        int winding;
        bool sortable;
        const SkOpAngle* angle = AngleWinding(*startPtr, *endPtr, &winding, &sortable);
        if (winding == SK_MinS32) {
            continue;
        }
        int sumWinding SK_INIT_TO_AVOID_WARNING;
        if (sortable) {
            segment = angle->segment();
            sumWinding = segment->updateWindingReverse(angle);
        }
        SkOpSegment* first = NULL;
        const SkOpAngle* firstAngle = angle;
        while ((angle = angle->next()) != firstAngle) {
            segment = angle->segment();
            SkOpSpanBase* start = angle->start();
            SkOpSpanBase* end = angle->end();
            int maxWinding;
            if (sortable) {
                segment->setUpWinding(start, end, &maxWinding, &sumWinding);
            }
            if (!segment->done(angle)) {
                if (!first && (sortable || start->starter(end)->windSum() != SK_MinS32)) {
                    first = segment;
                    *startPtr = start;
                    *endPtr = end;
                }
                // OPTIMIZATION: should this also add to the chase?
                if (sortable) {
                    (void) segment->markAngle(maxWinding, sumWinding, angle);
                }
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
void DebugShowActiveSpans(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->debugShowActiveSpans();
    } while ((contour = contour->next()));
}
#endif

bool SortContourList(SkOpContourHead** contourList, bool evenOdd, bool oppEvenOdd) {
    SkTDArray<SkOpContour* > list;
    SkOpContour* contour = *contourList;
    do {
        if (contour->count()) {
            contour->setOppXor(contour->operand() ? evenOdd : oppEvenOdd);
            *list.append() = contour;
        }
    } while ((contour = contour->next()));
    int count = list.count();
    if (!count) {
        return false;
    }
    if (count > 1) {
        SkTQSort<SkOpContour>(list.begin(), list.end() - 1);
    }
    contour = list[0];
    SkOpContourHead* contourHead = static_cast<SkOpContourHead*>(contour);
    contour->globalState()->setContourHead(contourHead);
    *contourList = contourHead;
    for (int index = 1; index < count; ++index) {
        SkOpContour* next = list[index];
        contour->setNext(next);
        contour = next;
    }
    contour->setNext(NULL);
    return true;
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
    SkChunkAlloc allocator(4096);  // FIXME: constant-ize, tune
    SkOpContourHead contour;
    SkOpGlobalState globalState(NULL, &contour  SkDEBUGPARAMS(NULL));
#if DEBUG_SHOW_TEST_NAME
    SkDebugf("</div>\n");
#endif
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("%s\n", __FUNCTION__);
#endif
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

static void align(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->align();
    } while ((contour = contour->next()));
}

static void addAlignIntersections(SkOpContourHead* contourList, SkChunkAlloc* allocator) {
    SkOpContour* contour = contourList;
    do {
        contour->addAlignIntersections(contourList, allocator);
    } while ((contour = contour->next()));
}

static void calcAngles(SkOpContourHead* contourList, SkChunkAlloc* allocator) {
    SkOpContour* contour = contourList;
    do {
        contour->calcAngles(allocator);
    } while ((contour = contour->next()));
}

static void findCollapsed(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->findCollapsed();
    } while ((contour = contour->next()));
}

static bool missingCoincidence(SkOpContourHead* contourList,
        SkOpCoincidence* coincidence, SkChunkAlloc* allocator) {
    SkOpContour* contour = contourList;
    bool result = false;
    do {
        result |= contour->missingCoincidence(coincidence, allocator);
    } while ((contour = contour->next()));
    return result;
}

static void moveMultiples(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->moveMultiples();
    } while ((contour = contour->next()));
}

static void moveNearby(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->moveNearby();
    } while ((contour = contour->next()));
}

static void sortAngles(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->sortAngles();
    } while ((contour = contour->next()));
}

bool HandleCoincidence(SkOpContourHead* contourList, SkOpCoincidence* coincidence,
        SkChunkAlloc* allocator) {
    SkOpGlobalState* globalState = contourList->globalState();
    // combine t values when multiple intersections occur on some segments but not others
    moveMultiples(contourList);
    findCollapsed(contourList);
    // move t values and points together to eliminate small/tiny gaps
    moveNearby(contourList);
    align(contourList);  // give all span members common values
    coincidence->fixAligned();  // aligning may have marked a coincidence pt-t deleted
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kIntersecting);
#endif
    // look for intersections on line segments formed by moving end points
    addAlignIntersections(contourList, allocator);
    coincidence->addMissing(allocator);
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kWalking);
#endif
    // check to see if, loosely, coincident ranges may be expanded
    if (coincidence->expand()) {
        coincidence->addExpanded(allocator  PATH_OPS_DEBUG_VALIDATE_PARAMS(globalState));
    }
    // the expanded ranges may not align -- add the missing spans
    coincidence->mark();  // mark spans of coincident segments as coincident
    // look for coincidence missed earlier
    if (missingCoincidence(contourList, coincidence, allocator)) {
        (void) coincidence->expand();
        coincidence->addExpanded(allocator  PATH_OPS_DEBUG_VALIDATE_PARAMS(globalState));
        coincidence->mark();
    }
    SkOpCoincidence overlaps;
    do {
        SkOpCoincidence* pairs = overlaps.isEmpty() ? coincidence : &overlaps;
        if (!pairs->apply()) {  // adjust the winding value to account for coincident edges
            return false;
        }
        // For each coincident pair that overlaps another, when the receivers (the 1st of the pair)
        // are different, construct a new pair to resolve their mutual span
        pairs->findOverlaps(&overlaps, allocator);
    } while (!overlaps.isEmpty());
    calcAngles(contourList, allocator);
    sortAngles(contourList);
    if (globalState->angleCoincidence()) {
        (void) missingCoincidence(contourList, coincidence, allocator);
        if (!coincidence->apply()) {
            return false;
        }
    }
#if DEBUG_ACTIVE_SPANS
    coincidence->debugShowCoincidence();
    DebugShowActiveSpans(contourList);
#endif
    return true;
}
