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

SkScalar ScaleFactor(const SkPath& path) {
    static const SkScalar twoTo10 = 1024.f;
    SkScalar largest = 0;
    const SkScalar* oneBounds = &path.getBounds().fLeft;
    for (int index = 0; index < 4; ++index) {
        largest = SkTMax(largest, SkScalarAbs(oneBounds[index]));
    }
    SkScalar scale = twoTo10;
    SkScalar next;
    while ((next = scale * twoTo10) < largest) {
        scale = next;
    }
    return scale == twoTo10 ? SK_Scalar1 : scale;
}

void ScalePath(const SkPath& path, SkScalar scale, SkPath* scaled) {
    SkMatrix matrix;
    matrix.setScale(scale, scale);
    *scaled = path;
    scaled->transform(matrix);
}

const SkOpAngle* AngleWinding(SkOpSpanBase* start, SkOpSpanBase* end, int* windingPtr,
        bool* sortablePtr) {
    // find first angle, initialize winding to computed fWindSum
    SkOpSegment* segment = start->segment();
    const SkOpAngle* angle = segment->spanToAngle(start, end);
    if (!angle) {
        *windingPtr = SK_MinS32;
        return nullptr;
    }
    bool computeWinding = false;
    const SkOpAngle* firstAngle = angle;
    bool loop = false;
    bool unorderable = false;
    int winding = SK_MinS32;
    do {
        angle = angle->next();
        if (!angle) {
            return nullptr;
        }
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
    return nullptr;
}

SkOpSegment* FindChase(SkTDArray<SkOpSpanBase*>* chase, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr) {
    while (chase->count()) {
        SkOpSpanBase* span;
        chase->pop(&span);
        SkOpSegment* segment = span->segment();
        *startPtr = span->ptT()->next()->span();
        bool done = true;
        *endPtr = nullptr;
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
        if (!angle) {
            return nullptr;
        }
        if (winding == SK_MinS32) {
            continue;
        }
        int sumWinding SK_INIT_TO_AVOID_WARNING;
        if (sortable) {
            segment = angle->segment();
            sumWinding = segment->updateWindingReverse(angle);
        }
        SkOpSegment* first = nullptr;
        const SkOpAngle* firstAngle = angle;
        while ((angle = angle->next()) != firstAngle) {
            segment = angle->segment();
            SkOpSpanBase* start = angle->start();
            SkOpSpanBase* end = angle->end();
            int maxWinding SK_INIT_TO_AVOID_WARNING;
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
    return nullptr;
}

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
    contour->setNext(nullptr);
    return true;
}

static void calcAngles(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        contour->calcAngles();
    } while ((contour = contour->next()));
}

static bool missingCoincidence(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    bool result = false;
    do {
        result |= contour->missingCoincidence();
    } while ((contour = contour->next()));
    return result;
}

static bool moveMultiples(SkOpContourHead* contourList) {
    SkOpContour* contour = contourList;
    do {
        if (!contour->moveMultiples()) {
            return false;
        }
    } while ((contour = contour->next()));
    return true;
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

bool HandleCoincidence(SkOpContourHead* contourList, SkOpCoincidence* coincidence) {
    SkOpGlobalState* globalState = contourList->globalState();
    DEBUG_COINCIDENCE_HEALTH(contourList, "start");
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kIntersecting);
#endif

    // match up points within the coincident runs
    if (!coincidence->addExpanded()) {
        return false;
    }
    DEBUG_COINCIDENCE_HEALTH(contourList, "addExpanded");
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kWalking);
#endif
    // combine t values when multiple intersections occur on some segments but not others
    if (!moveMultiples(contourList)) {
        return false;
    }
    DEBUG_COINCIDENCE_HEALTH(contourList, "moveMultiples");
    // move t values and points together to eliminate small/tiny gaps
    (void) moveNearby(contourList);
    DEBUG_COINCIDENCE_HEALTH(contourList, "moveNearby");
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kIntersecting);
#endif
    // add coincidence formed by pairing on curve points and endpoints
    coincidence->correctEnds();
    if (!coincidence->addEndMovedSpans()) {
        return false;
    }
    DEBUG_COINCIDENCE_HEALTH(contourList, "addEndMovedSpans");

    const int SAFETY_COUNT = 100;  // FIXME: tune
    int safetyHatch = SAFETY_COUNT;
    // look for coincidence present in A-B and A-C but missing in B-C
    do {
        bool added;
        if (!coincidence->addMissing(&added)) {
            return false;
        }
        if (!added) {
            break;
        }
        if (!--safetyHatch) {
            SkASSERT(globalState->debugSkipAssert());
            return false;
        }
        DEBUG_COINCIDENCE_HEALTH(contourList, "addMissing");
        moveNearby(contourList);
        DEBUG_COINCIDENCE_HEALTH(contourList, "moveNearby");
    } while (true);
    DEBUG_COINCIDENCE_HEALTH(contourList, "addMissing2");
    // FIXME: only call this if addMissing modified something when returning false
    moveNearby(contourList);
    DEBUG_COINCIDENCE_HEALTH(contourList, "moveNearby2");
    // check to see if, loosely, coincident ranges may be expanded
    if (coincidence->expand()) {
        DEBUG_COINCIDENCE_HEALTH(contourList, "expand1");
        bool added;
        if (!coincidence->addMissing(&added)) {
            return false;
        }
        DEBUG_COINCIDENCE_HEALTH(contourList, "addMissing2");
        if (!coincidence->addExpanded()) {
            return false;
        }
        DEBUG_COINCIDENCE_HEALTH(contourList, "addExpanded2");
        if (!moveMultiples(contourList)) {
            return false;
        }
        DEBUG_COINCIDENCE_HEALTH(contourList, "moveMultiples2");
        moveNearby(contourList);
    }
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kWalking);
#endif
    DEBUG_COINCIDENCE_HEALTH(contourList, "expand2");
    // the expanded ranges may not align -- add the missing spans
    if (!coincidence->addExpanded()) {
        return false;
    }
    DEBUG_COINCIDENCE_HEALTH(contourList, "addExpanded3");
    coincidence->correctEnds();
    if (!coincidence->mark()) {  // mark spans of coincident segments as coincident
        return false;
    }
    DEBUG_COINCIDENCE_HEALTH(contourList, "mark1");
    // look for coincidence lines and curves undetected by intersection
    if (missingCoincidence(contourList)) {
#if DEBUG_VALIDATE
        globalState->setPhase(SkOpGlobalState::kIntersecting);
#endif
        DEBUG_COINCIDENCE_HEALTH(contourList, "missingCoincidence1");
        (void) coincidence->expand();
        DEBUG_COINCIDENCE_HEALTH(contourList, "expand3");
        if (!coincidence->addExpanded()) {
            return false;
        }
#if DEBUG_VALIDATE
        globalState->setPhase(SkOpGlobalState::kWalking);
#endif
        DEBUG_COINCIDENCE_HEALTH(contourList, "addExpanded3");
        if (!coincidence->mark()) {
            return false;
        }
    } else {
        DEBUG_COINCIDENCE_HEALTH(contourList, "missingCoincidence2");
        (void) coincidence->expand();
    }
    DEBUG_COINCIDENCE_HEALTH(contourList, "missingCoincidence3");

    (void) coincidence->expand();

#if 0  // under development
    // coincident runs may cross two or more spans, but the opposite spans may be out of order
    if (!coincidence->reorder()) {
      return false;
    }
#endif
    DEBUG_COINCIDENCE_HEALTH(contourList, "coincidence.reorder");
    SkOpCoincidence overlaps(globalState);
    do {
        SkOpCoincidence* pairs = overlaps.isEmpty() ? coincidence : &overlaps;
        if (!pairs->apply()) {  // adjust the winding value to account for coincident edges
            return false;
        }
        DEBUG_COINCIDENCE_HEALTH(contourList, "pairs->apply");
        // For each coincident pair that overlaps another, when the receivers (the 1st of the pair)
        // are different, construct a new pair to resolve their mutual span
        if (!pairs->findOverlaps(&overlaps)) {
            return false;
        }
        DEBUG_COINCIDENCE_HEALTH(contourList, "pairs->findOverlaps");
    } while (!overlaps.isEmpty());
    calcAngles(contourList);
    sortAngles(contourList);
    if (globalState->angleCoincidence()) {
        (void) missingCoincidence(contourList);
        if (!coincidence->apply()) {
            return false;
        }
    }
#if DEBUG_COINCIDENCE_VERBOSE
    coincidence->debugShowCoincidence();
#endif
#if DEBUG_COINCIDENCE
    coincidence->debugValidate();
#endif
    SkPathOpsDebug::ShowActiveSpans(contourList);
    return true;
}
