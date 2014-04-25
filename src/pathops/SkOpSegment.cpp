/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkOpSegment.h"
#include "SkPathWriter.h"
#include "SkTSort.h"

#define F (false)      // discard the edge
#define T (true)       // keep the edge

static const bool gUnaryActiveEdge[2][2] = {
//  from=0  from=1
//  to=0,1  to=0,1
    {F, T}, {T, F},
};

static const bool gActiveEdge[kXOR_PathOp + 1][2][2][2][2] = {
//                 miFrom=0                              miFrom=1
//         miTo=0             miTo=1             miTo=0             miTo=1
//     suFrom=0    1      suFrom=0    1      suFrom=0    1      suFrom=0    1
//   suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1
    {{{{F, F}, {F, F}}, {{T, F}, {T, F}}}, {{{T, T}, {F, F}}, {{F, T}, {T, F}}}},  // mi - su
    {{{{F, F}, {F, F}}, {{F, T}, {F, T}}}, {{{F, F}, {T, T}}, {{F, T}, {T, F}}}},  // mi & su
    {{{{F, T}, {T, F}}, {{T, T}, {F, F}}}, {{{T, F}, {T, F}}, {{F, F}, {F, F}}}},  // mi | su
    {{{{F, T}, {T, F}}, {{T, F}, {F, T}}}, {{{T, F}, {F, T}}, {{F, T}, {T, F}}}},  // mi ^ su
};

#undef F
#undef T

enum {
    kOutsideTrackedTCount = 16,  // FIXME: determine what this should be
    kMissingSpanCount = 4,  // FIXME: determine what this should be
};

const SkOpAngle* SkOpSegment::activeAngle(int index, int* start, int* end, bool* done,
        bool* sortable) const {
    if (const SkOpAngle* result = activeAngleInner(index, start, end, done, sortable)) {
        return result;
    }
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0
            && (precisely_negative(referenceT - fTs[lesser].fT) || fTs[lesser].fTiny)) {
        if (const SkOpAngle* result = activeAngleOther(lesser, start, end, done, sortable)) {
            return result;
        }
    }
    do {
        if (const SkOpAngle* result = activeAngleOther(index, start, end, done, sortable)) {
            return result;
        }
        if (++index == fTs.count()) {
            break;
        }
        if (fTs[index - 1].fTiny) {
            referenceT = fTs[index].fT;
            continue;
        }
    } while (precisely_negative(fTs[index].fT - referenceT));
    return NULL;
}

const SkOpAngle* SkOpSegment::activeAngleInner(int index, int* start, int* end, bool* done,
        bool* sortable) const {
    int next = nextExactSpan(index, 1);
    if (next > 0) {
        const SkOpSpan& upSpan = fTs[index];
        if (upSpan.fWindValue || upSpan.fOppValue) {
            if (*end < 0) {
                *start = index;
                *end = next;
            }
            if (!upSpan.fDone) {
                if (upSpan.fWindSum != SK_MinS32) {
                    return spanToAngle(index, next);
                }
                *done = false;
            }
        } else {
            SkASSERT(upSpan.fDone);
        }
    }
    int prev = nextExactSpan(index, -1);
    // edge leading into junction
    if (prev >= 0) {
        const SkOpSpan& downSpan = fTs[prev];
        if (downSpan.fWindValue || downSpan.fOppValue) {
            if (*end < 0) {
                *start = index;
                *end = prev;
            }
            if (!downSpan.fDone) {
                if (downSpan.fWindSum != SK_MinS32) {
                    return spanToAngle(index, prev);
                }
                *done = false;
            }
        } else {
            SkASSERT(downSpan.fDone);
        }
    }
    return NULL;
}

const SkOpAngle* SkOpSegment::activeAngleOther(int index, int* start, int* end, bool* done,
        bool* sortable) const {
    const SkOpSpan* span = &fTs[index];
    SkOpSegment* other = span->fOther;
    int oIndex = span->fOtherIndex;
    return other->activeAngleInner(oIndex, start, end, done, sortable);
}

SkPoint SkOpSegment::activeLeftTop(int* firstT) const {
    SkASSERT(!done());
    SkPoint topPt = {SK_ScalarMax, SK_ScalarMax};
    int count = fTs.count();
    // see if either end is not done since we want smaller Y of the pair
    bool lastDone = true;
    double lastT = -1;
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        if (span.fDone && lastDone) {
            goto next;
        }
        if (approximately_negative(span.fT - lastT)) {
            goto next;
        }
        {
            const SkPoint& xy = xyAtT(&span);
            if (topPt.fY > xy.fY || (topPt.fY == xy.fY && topPt.fX > xy.fX)) {
                topPt = xy;
                if (firstT) {
                    *firstT = index;
                }
            }
            if (fVerb != SkPath::kLine_Verb && !lastDone) {
                SkPoint curveTop = (*CurveTop[SkPathOpsVerbToPoints(fVerb)])(fPts, lastT, span.fT);
                if (topPt.fY > curveTop.fY || (topPt.fY == curveTop.fY
                        && topPt.fX > curveTop.fX)) {
                    topPt = curveTop;
                    if (firstT) {
                        *firstT = index;
                    }
                }
            }
            lastT = span.fT;
        }
next:
        lastDone = span.fDone;
    }
    return topPt;
}

bool SkOpSegment::activeOp(int index, int endIndex, int xorMiMask, int xorSuMask, SkPathOp op) {
    int sumMiWinding = updateWinding(endIndex, index);
    int sumSuWinding = updateOppWinding(endIndex, index);
    if (fOperand) {
        SkTSwap<int>(sumMiWinding, sumSuWinding);
    }
    return activeOp(xorMiMask, xorSuMask, index, endIndex, op, &sumMiWinding, &sumSuWinding);
}

bool SkOpSegment::activeOp(int xorMiMask, int xorSuMask, int index, int endIndex, SkPathOp op,
        int* sumMiWinding, int* sumSuWinding) {
    int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
    setUpWindings(index, endIndex, sumMiWinding, sumSuWinding,
            &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
    bool miFrom;
    bool miTo;
    bool suFrom;
    bool suTo;
    if (operand()) {
        miFrom = (oppMaxWinding & xorMiMask) != 0;
        miTo = (oppSumWinding & xorMiMask) != 0;
        suFrom = (maxWinding & xorSuMask) != 0;
        suTo = (sumWinding & xorSuMask) != 0;
    } else {
        miFrom = (maxWinding & xorMiMask) != 0;
        miTo = (sumWinding & xorMiMask) != 0;
        suFrom = (oppMaxWinding & xorSuMask) != 0;
        suTo = (oppSumWinding & xorSuMask) != 0;
    }
    bool result = gActiveEdge[op][miFrom][miTo][suFrom][suTo];
#if DEBUG_ACTIVE_OP
    SkDebugf("%s op=%s miFrom=%d miTo=%d suFrom=%d suTo=%d result=%d\n", __FUNCTION__,
            SkPathOpsDebug::kPathOpStr[op], miFrom, miTo, suFrom, suTo, result);
#endif
    return result;
}

bool SkOpSegment::activeWinding(int index, int endIndex) {
    int sumWinding = updateWinding(endIndex, index);
    return activeWinding(index, endIndex, &sumWinding);
}

bool SkOpSegment::activeWinding(int index, int endIndex, int* sumWinding) {
    int maxWinding;
    setUpWinding(index, endIndex, &maxWinding, sumWinding);
    bool from = maxWinding != 0;
    bool to = *sumWinding  != 0;
    bool result = gUnaryActiveEdge[from][to];
    return result;
}

void SkOpSegment::addCancelOutsides(const SkPoint& startPt, const SkPoint& endPt,
        SkOpSegment* other) {
    int tIndex = -1;
    int tCount = fTs.count();
    int oIndex = -1;
    int oCount = other->fTs.count();
    do {
        ++tIndex;
    } while (startPt != fTs[tIndex].fPt && tIndex < tCount);
    int tIndexStart = tIndex;
    do {
        ++oIndex;
    } while (endPt != other->fTs[oIndex].fPt && oIndex < oCount);
    int oIndexStart = oIndex;
    const SkPoint* nextPt;
    do {
        nextPt = &fTs[++tIndex].fPt;
        SkASSERT(fTs[tIndex].fT < 1 || startPt != *nextPt);
    } while (startPt == *nextPt);
    double nextT = fTs[tIndex].fT;
    const SkPoint* oNextPt;
    do {
        oNextPt = &other->fTs[++oIndex].fPt;
        SkASSERT(other->fTs[oIndex].fT < 1 || endPt != *oNextPt);
    } while (endPt == *oNextPt);
    double oNextT = other->fTs[oIndex].fT;
    // at this point, spans before and after are at:
    //  fTs[tIndexStart - 1], fTs[tIndexStart], fTs[tIndex]
    // if tIndexStart == 0, no prior span
    // if nextT == 1, no following span

    // advance the span with zero winding
    // if the following span exists (not past the end, non-zero winding)
    // connect the two edges
    if (!fTs[tIndexStart].fWindValue) {
        if (tIndexStart > 0 && fTs[tIndexStart - 1].fWindValue) {
#if DEBUG_CONCIDENT
            SkDebugf("%s 1 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, fID, other->fID, tIndexStart - 1,
                    fTs[tIndexStart].fT, xyAtT(tIndexStart).fX,
                    xyAtT(tIndexStart).fY);
#endif
            addTPair(fTs[tIndexStart].fT, other, other->fTs[oIndex].fT, false,
                    fTs[tIndexStart].fPt);
        }
        if (nextT < 1 && fTs[tIndex].fWindValue) {
#if DEBUG_CONCIDENT
            SkDebugf("%s 2 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, fID, other->fID, tIndex,
                    fTs[tIndex].fT, xyAtT(tIndex).fX,
                    xyAtT(tIndex).fY);
#endif
            addTPair(fTs[tIndex].fT, other, other->fTs[oIndexStart].fT, false, fTs[tIndex].fPt);
        }
    } else {
        SkASSERT(!other->fTs[oIndexStart].fWindValue);
        if (oIndexStart > 0 && other->fTs[oIndexStart - 1].fWindValue) {
#if DEBUG_CONCIDENT
            SkDebugf("%s 3 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, fID, other->fID, oIndexStart - 1,
                    other->fTs[oIndexStart].fT, other->xyAtT(oIndexStart).fX,
                    other->xyAtT(oIndexStart).fY);
            other->debugAddTPair(other->fTs[oIndexStart].fT, *this, fTs[tIndex].fT);
#endif
        }
        if (oNextT < 1 && other->fTs[oIndex].fWindValue) {
#if DEBUG_CONCIDENT
            SkDebugf("%s 4 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, fID, other->fID, oIndex,
                    other->fTs[oIndex].fT, other->xyAtT(oIndex).fX,
                    other->xyAtT(oIndex).fY);
            other->debugAddTPair(other->fTs[oIndex].fT, *this, fTs[tIndexStart].fT);
#endif
        }
    }
}

void SkOpSegment::addCoinOutsides(const SkPoint& startPt, const SkPoint& endPt,
        SkOpSegment* other) {
    // walk this to startPt
    // walk other to startPt
    // if either is > 0, add a pointer to the other, copying adjacent winding
    int tIndex = -1;
    int oIndex = -1;
    do {
        ++tIndex;
    } while (startPt != fTs[tIndex].fPt);
    int ttIndex = tIndex;
    bool checkOtherTMatch = false;
    do {
        const SkOpSpan& span = fTs[ttIndex];
        if (startPt != span.fPt) {
            break;
        }
        if (span.fOther == other && span.fPt == startPt) {
            checkOtherTMatch = true;
            break;
        }
    } while (++ttIndex < count());
    do {
        ++oIndex;
    } while (startPt != other->fTs[oIndex].fPt);
    bool skipAdd = false;
    if (checkOtherTMatch) {
        int ooIndex = oIndex;
        do {
            const SkOpSpan& oSpan = other->fTs[ooIndex];
            if (startPt != oSpan.fPt) {
                break;
            }
            if (oSpan.fT == fTs[ttIndex].fOtherT) {
                skipAdd = true;
                break;
            }
        } while (++ooIndex < other->count());
    }
    if ((tIndex > 0 || oIndex > 0 || fOperand != other->fOperand) && !skipAdd) {
        addTPair(fTs[tIndex].fT, other, other->fTs[oIndex].fT, false, startPt);
    }
    SkPoint nextPt = startPt;
    do {
        const SkPoint* workPt;
        do {
            workPt = &fTs[++tIndex].fPt;
        } while (nextPt == *workPt);
        const SkPoint* oWorkPt;
        do {
            oWorkPt = &other->fTs[++oIndex].fPt;
        } while (nextPt == *oWorkPt);
        nextPt = *workPt;
        double tStart = fTs[tIndex].fT;
        double oStart = other->fTs[oIndex].fT;
        if (tStart == 1 && oStart == 1 && fOperand == other->fOperand) {
            break;
        }
        if (*workPt == *oWorkPt) {
            addTPair(tStart, other, oStart, false, nextPt);
        }
    } while (endPt != nextPt);
}

void SkOpSegment::addCubic(const SkPoint pts[4], bool operand, bool evenOdd) {
    init(pts, SkPath::kCubic_Verb, operand, evenOdd);
    fBounds.setCubicBounds(pts);
}

void SkOpSegment::addCurveTo(int start, int end, SkPathWriter* path, bool active) const {
    SkPoint edge[4];
    const SkPoint* ePtr;
    int lastT = fTs.count() - 1;
    if (lastT < 0 || (start == 0 && end == lastT) || (start == lastT && end == 0)) {
        ePtr = fPts;
    } else {
    // OPTIMIZE? if not active, skip remainder and return xyAtT(end)
        subDivide(start, end, edge);
        ePtr = edge;
    }
    if (active) {
        bool reverse = ePtr == fPts && start != 0;
        if (reverse) {
            path->deferredMoveLine(ePtr[SkPathOpsVerbToPoints(fVerb)]);
            switch (fVerb) {
                case SkPath::kLine_Verb:
                    path->deferredLine(ePtr[0]);
                    break;
                case SkPath::kQuad_Verb:
                    path->quadTo(ePtr[1], ePtr[0]);
                    break;
                case SkPath::kCubic_Verb:
                    path->cubicTo(ePtr[2], ePtr[1], ePtr[0]);
                    break;
                default:
                    SkASSERT(0);
            }
   //         return ePtr[0];
       } else {
            path->deferredMoveLine(ePtr[0]);
            switch (fVerb) {
                case SkPath::kLine_Verb:
                    path->deferredLine(ePtr[1]);
                    break;
                case SkPath::kQuad_Verb:
                    path->quadTo(ePtr[1], ePtr[2]);
                    break;
                case SkPath::kCubic_Verb:
                    path->cubicTo(ePtr[1], ePtr[2], ePtr[3]);
                    break;
                default:
                    SkASSERT(0);
            }
        }
    }
  //  return ePtr[SkPathOpsVerbToPoints(fVerb)];
}

void SkOpSegment::addEndSpan(int endIndex) {
    int spanCount = fTs.count();
    int angleIndex = fAngles.count();
    int startIndex = endIndex - 1;
    while (fTs[startIndex].fT == 1 || fTs[startIndex].fTiny) {
        ++startIndex;
        SkASSERT(startIndex < spanCount - 1);
        ++endIndex;
    }
    fAngles.push_back().set(this, spanCount - 1, startIndex);
#if DEBUG_ANGLE
    fAngles.back().setID(angleIndex);
    debugCheckPointsEqualish(endIndex, spanCount);
#endif
    setFromAngleIndex(endIndex, angleIndex);
}

void SkOpSegment::setFromAngleIndex(int endIndex, int angleIndex) {
    int spanCount = fTs.count();
    do {
        fTs[endIndex].fFromAngleIndex = angleIndex;
    } while (++endIndex < spanCount);
}

void SkOpSegment::addLine(const SkPoint pts[2], bool operand, bool evenOdd) {
    init(pts, SkPath::kLine_Verb, operand, evenOdd);
    fBounds.set(pts, 2);
}

// add 2 to edge or out of range values to get T extremes
void SkOpSegment::addOtherT(int index, double otherT, int otherIndex) {
    SkOpSpan& span = fTs[index];
    if (precisely_zero(otherT)) {
        otherT = 0;
    } else if (precisely_equal(otherT, 1)) {
        otherT = 1;
    }
    span.fOtherT = otherT;
    span.fOtherIndex = otherIndex;
}

void SkOpSegment::addQuad(const SkPoint pts[3], bool operand, bool evenOdd) {
    init(pts, SkPath::kQuad_Verb, operand, evenOdd);
    fBounds.setQuadBounds(pts);
}

int SkOpSegment::addSingletonAngleDown(int endIndex, SkOpSegment** otherPtr) {
    int startIndex = findEndSpan(endIndex);
    SkASSERT(startIndex > 0);
    int spanIndex = endIndex - 1;
    fSingletonAngles.push_back().set(this, spanIndex, startIndex - 1);
    setFromAngleIndex(startIndex, fAngles.count() + fSingletonAngles.count() - 1);
    SkOpSegment* other;
    do {
        other = fTs[spanIndex].fOther;
        if (other->fTs[0].fWindValue) {
            break;
        }
        --spanIndex;
        SkASSERT(fTs[spanIndex].fT == 1);
    } while (true);
    int oEndIndex = other->findStartSpan(0);
    SkASSERT(oEndIndex > 0);
    int otherIndex = other->fSingletonAngles.count();
    other->fSingletonAngles.push_back().set(other, 0, oEndIndex);
    other->setToAngleIndex(oEndIndex, other->fAngles.count() + otherIndex);
    *otherPtr = other;
    return otherIndex;
}

int SkOpSegment::addSingletonAngleUp(int start, SkOpSegment** otherPtr) {
    int endIndex = findStartSpan(start);
    SkASSERT(endIndex > 0);
    int thisIndex = fSingletonAngles.count();
    fSingletonAngles.push_back().set(this, start, endIndex);
    setToAngleIndex(endIndex, fAngles.count() + thisIndex);
    int spanIndex = start;
    SkOpSegment* other;
    int oCount, oStartIndex;
    do {
        other = fTs[spanIndex].fOther;
        oCount = other->count();
        oStartIndex = other->findEndSpan(oCount);
        SkASSERT(oStartIndex > 0);
        if (other->fTs[oStartIndex - 1].fWindValue) {
            break;
        }
        ++spanIndex;
        SkASSERT(fTs[spanIndex].fT == 0);
    } while (true);
    int otherIndex = other->fSingletonAngles.count();
    other->fSingletonAngles.push_back().set(other, oCount - 1, oStartIndex - 1);
    other->setFromAngleIndex(oStartIndex, other->fAngles.count() + otherIndex);
    *otherPtr = other;
    return otherIndex;
}

SkOpAngle* SkOpSegment::addSingletonAngles(int start, int step) {
    int thisIndex = fSingletonAngles.count();
    SkOpSegment* other;
    int otherIndex;
    if (step > 0) {
        otherIndex = addSingletonAngleUp(start, &other);
    } else {
        otherIndex = addSingletonAngleDown(start + 1, &other);
    }
    fSingletonAngles[thisIndex].insert(&other->fSingletonAngles[otherIndex]);
#if DEBUG_ANGLE
    fSingletonAngles[thisIndex].setID(fAngles.count() + thisIndex);
    other->fSingletonAngles[otherIndex].setID(other->fAngles.count() + otherIndex);
#endif
    return &fSingletonAngles[thisIndex];
}

void SkOpSegment::addStartSpan(int endIndex) {
    int angleIndex = fAngles.count();
    int index = 0;
    fAngles.push_back().set(this, index, endIndex);
#if DEBUG_ANGLE
    fAngles.back().setID(angleIndex);
    debugCheckPointsEqualish(index, endIndex);
#endif
    setToAngleIndex(endIndex, angleIndex);
}

void SkOpSegment::setToAngleIndex(int endIndex, int angleIndex) {
    int index = 0;
    do {
        fTs[index].fToAngleIndex = angleIndex;
    } while (++index < endIndex);
}

    // Defer all coincident edge processing until
    // after normal intersections have been computed

// no need to be tricky; insert in normal T order
// resolve overlapping ts when considering coincidence later

    // add non-coincident intersection. Resulting edges are sorted in T.
int SkOpSegment::addT(SkOpSegment* other, const SkPoint& pt, double newT) {
    SkASSERT(this != other || fVerb == SkPath::kCubic_Verb);
 #if 0  // this needs an even rougher association to be useful
    SkASSERT(SkDPoint::RoughlyEqual(ptAtT(newT), pt));
 #endif
    if (precisely_zero(newT)) {
        newT = 0;
    } else if (precisely_equal(newT, 1)) {
        newT = 1;
    }
    // FIXME: in the pathological case where there is a ton of intercepts,
    //  binary search?
    int insertedAt = -1;
    int tCount = fTs.count();
    const SkPoint& firstPt = fPts[0];
    const SkPoint& lastPt = fPts[SkPathOpsVerbToPoints(fVerb)];
    for (int index = 0; index < tCount; ++index) {
        // OPTIMIZATION: if there are three or more identical Ts, then
        // the fourth and following could be further insertion-sorted so
        // that all the edges are clockwise or counterclockwise.
        // This could later limit segment tests to the two adjacent
        // neighbors, although it doesn't help with determining which
        // circular direction to go in.
        const SkOpSpan& span = fTs[index];
        if (newT < span.fT) {
            insertedAt = index;
            break;
        }
        if (newT == span.fT) {
            if (pt == span.fPt) {
                insertedAt = index;
                break;
            }
            if ((pt == firstPt && newT == 0) || (span.fPt == lastPt && newT == 1)) {
                insertedAt = index;
                break;
            }
        }
    }
    SkOpSpan* span;
    if (insertedAt >= 0) {
        span = fTs.insert(insertedAt);
    } else {
        insertedAt = tCount;
        span = fTs.append();
    }
    span->fT = newT;
    span->fOther = other;
    span->fPt = pt;
#if 0
    // cubics, for instance, may not be exact enough to satisfy this check (e.g., cubicOp69d)
    SkASSERT(approximately_equal(xyAtT(newT).fX, pt.fX)
            && approximately_equal(xyAtT(newT).fY, pt.fY));
#endif
    span->fFromAngleIndex = -1;
    span->fToAngleIndex = -1;
    span->fWindSum = SK_MinS32;
    span->fOppSum = SK_MinS32;
    span->fWindValue = 1;
    span->fOppValue = 0;
    span->fChased = false;
    if ((span->fDone = newT == 1)) {
        ++fDoneSpans;
    }
    span->fLoop = false;
    span->fSmall = false;
    span->fTiny = false;
    int less = -1;
// find range of spans with nearly the same point as this one
    while (&span[less + 1] - fTs.begin() > 0 && AlmostEqualUlps(span[less].fPt, pt)) {
        if (fVerb == SkPath::kCubic_Verb) {
            double tInterval = newT - span[less].fT;
            double tMid = newT - tInterval / 2;
            SkDPoint midPt = dcubic_xy_at_t(fPts, tMid);
            if (!midPt.approximatelyEqual(xyAtT(span))) {
                break;
            }
        }
        --less;
    }
    int more = 1;
    while (fTs.end() - &span[more - 1] > 1 && AlmostEqualUlps(span[more].fPt, pt)) {
        if (fVerb == SkPath::kCubic_Verb) {
            double tEndInterval = span[more].fT - newT;
            double tMid = newT - tEndInterval / 2;
            SkDPoint midEndPt = dcubic_xy_at_t(fPts, tMid);
            if (!midEndPt.approximatelyEqual(xyAtT(span))) {
                break;
            }
        }
        ++more;
    }
    ++less;
    --more;
    while (more - 1 > less && span[more].fPt == span[more - 1].fPt
            && span[more].fT == span[more - 1].fT) {
        --more;
    }
    if (less == more) {
        return insertedAt;
    }
    if (precisely_negative(span[more].fT - span[less].fT)) {
        return insertedAt;
    }
// if the total range of t values is big enough, mark all tiny
    bool tiny = span[less].fPt == span[more].fPt;
    int index = less;
    do {
        fSmall = span[index].fSmall = true;
        fTiny |= span[index].fTiny = tiny;
        if (!span[index].fDone) {
            span[index].fDone = true;
            ++fDoneSpans;
        }
    } while (++index < more);
    return insertedAt;
}

// set spans from start to end to decrement by one
// note this walks other backwards
// FIXME: there's probably an edge case that can be constructed where
// two span in one segment are separated by float epsilon on one span but
// not the other, if one segment is very small. For this
// case the counts asserted below may or may not be enough to separate the
// spans. Even if the counts work out, what if the spans aren't correctly
// sorted? It feels better in such a case to match the span's other span
// pointer since both coincident segments must contain the same spans.
// FIXME? It seems that decrementing by one will fail for complex paths that
// have three or more coincident edges. Shouldn't this subtract the difference
// between the winding values?
/*                                      |-->                           |-->
this     0>>>>1>>>>2>>>>3>>>4      0>>>>1>>>>2>>>>3>>>4      0>>>>1>>>>2>>>>3>>>4
other         2<<<<1<<<<0               2<<<<1<<<<0               2<<<<1<<<<0
              ^         ^                 <--|                           <--|
           startPt    endPt        test/oTest first pos      test/oTest final pos
*/
void SkOpSegment::addTCancel(const SkPoint& startPt, const SkPoint& endPt, SkOpSegment* other) {
    bool binary = fOperand != other->fOperand;
    int index = 0;
    while (startPt != fTs[index].fPt) {
        SkASSERT(index < fTs.count());
        ++index;
    }
    while (index > 0 && precisely_equal(fTs[index].fT, fTs[index - 1].fT)) {
        --index;
    }
    int oIndex = other->fTs.count();
    while (startPt != other->fTs[--oIndex].fPt) {  // look for startPt match
        SkASSERT(oIndex > 0);
    }
    double oStartT = other->fTs[oIndex].fT;
    // look for first point beyond match
    while (startPt == other->fTs[--oIndex].fPt || precisely_equal(oStartT, other->fTs[oIndex].fT)) {
        SkASSERT(oIndex > 0);
    }
    SkOpSpan* test = &fTs[index];
    SkOpSpan* oTest = &other->fTs[oIndex];
    SkSTArray<kOutsideTrackedTCount, SkPoint, true> outsidePts;
    SkSTArray<kOutsideTrackedTCount, SkPoint, true> oOutsidePts;
    do {
        SkASSERT(test->fT < 1);
        SkASSERT(oTest->fT < 1);
        bool decrement = test->fWindValue && oTest->fWindValue;
        bool track = test->fWindValue || oTest->fWindValue;
        bool bigger = test->fWindValue >= oTest->fWindValue;
        const SkPoint& testPt = test->fPt;
        double testT = test->fT;
        const SkPoint& oTestPt = oTest->fPt;
        double oTestT = oTest->fT;
        do {
            if (decrement) {
                if (binary && bigger) {
                    test->fOppValue--;
                } else {
                    decrementSpan(test);
                }
            } else if (track) {
                TrackOutsidePair(&outsidePts, testPt, oTestPt);
            }
            SkASSERT(index < fTs.count() - 1);
            test = &fTs[++index];
        } while (testPt == test->fPt || precisely_equal(testT, test->fT));
        SkDEBUGCODE(int originalWindValue = oTest->fWindValue);
        do {
            SkASSERT(oTest->fT < 1);
            SkASSERT(originalWindValue == oTest->fWindValue);
            if (decrement) {
                if (binary && !bigger) {
                    oTest->fOppValue--;
                } else {
                    other->decrementSpan(oTest);
                }
            } else if (track) {
                TrackOutsidePair(&oOutsidePts, oTestPt, testPt);
            }
            if (!oIndex) {
                break;
            }
            oTest = &other->fTs[--oIndex];
        } while (oTestPt == oTest->fPt || precisely_equal(oTestT, oTest->fT));
    } while (endPt != test->fPt && test->fT < 1);
    // FIXME: determine if canceled edges need outside ts added
    int outCount = outsidePts.count();
    if (!done() && outCount) {
        addCancelOutsides(outsidePts[0], outsidePts[1], other);
        if (outCount > 2) {
            addCancelOutsides(outsidePts[outCount - 2], outsidePts[outCount - 1], other);
        }
    }
    if (!other->done() && oOutsidePts.count()) {
        other->addCancelOutsides(oOutsidePts[0], oOutsidePts[1], this);
    }
}

int SkOpSegment::addSelfT(const SkPoint& pt, double newT) {
    // if the tail nearly intersects itself but not quite, the caller records this separately
    int result = addT(this, pt, newT);
    SkOpSpan* span = &fTs[result];
    fLoop = span->fLoop = true;
    return result;
}

void SkOpSegment::addSimpleAngle(int index) {
    if (index == 0) {
        SkASSERT(!fTs[index].fTiny && fTs[index + 1].fT > 0);
        addStartSpan(1);
    } else {
        SkASSERT(!fTs[index - 1].fTiny && fTs[index - 1].fT < 1);
        addEndSpan(index);
    }
    SkOpSpan& span = fTs[index];
    SkOpSegment* other = span.fOther;
    SkOpSpan& oSpan = other->fTs[span.fOtherIndex];
    SkOpAngle* angle, * oAngle;
    if (index == 0) {
        SkASSERT(span.fOtherIndex - 1 >= 0);
        SkASSERT(span.fOtherT == 1);
        SkDEBUGCODE(SkOpSpan& oPrior = other->fTs[span.fOtherIndex - 1]);
        SkASSERT(!oPrior.fTiny && oPrior.fT < 1);
        other->addEndSpan(span.fOtherIndex);
        angle = &this->angle(span.fToAngleIndex);
        oAngle = &other->angle(oSpan.fFromAngleIndex);
    } else {
        SkASSERT(span.fOtherIndex + 1 < other->count());
        SkASSERT(span.fOtherT == 0);
        SkASSERT(!oSpan.fTiny && (other->fTs[span.fOtherIndex + 1].fT > 0
                || (other->fTs[span.fOtherIndex + 1].fFromAngleIndex < 0
                && other->fTs[span.fOtherIndex + 1].fToAngleIndex < 0)));
        int oIndex = 1;
        do {
            const SkOpSpan& osSpan = other->span(oIndex);
            if (osSpan.fFromAngleIndex >= 0 || osSpan.fT > 0) {
                break;
            }
            ++oIndex;
            SkASSERT(oIndex < other->count());
        } while (true);
        other->addStartSpan(oIndex);
        angle = &this->angle(span.fFromAngleIndex);
        oAngle = &other->angle(oSpan.fToAngleIndex);
    }
    angle->insert(oAngle);
}

bool SkOpSegment::alignSpan(int index, double thisT, const SkPoint& thisPt) {
    bool aligned = false;
    SkOpSpan* span = &fTs[index];
    SkOpSegment* other = span->fOther;
    int oIndex = span->fOtherIndex;
    SkOpSpan* oSpan = &other->fTs[oIndex];
    if (span->fT != thisT) {
        span->fT = thisT;
        oSpan->fOtherT = thisT;
        aligned = true;
    }
    if (span->fPt != thisPt) {
        span->fPt = thisPt;
        oSpan->fPt = thisPt;
        aligned = true;
    }
    double oT = oSpan->fT;
    if (oT == 0) {
        return aligned;
    }
    int oStart = other->nextSpan(oIndex, -1) + 1;
    oSpan = &other->fTs[oStart];
    int otherIndex = oStart;
    if (oT == 1) {
        if (aligned) {
            while (oSpan->fPt == thisPt && oSpan->fT != 1) {
                oSpan->fTiny = true;
                ++oSpan;
            }
        }
        return aligned;
    }
    oT = oSpan->fT;
    int oEnd = other->nextSpan(oIndex, 1);
    bool oAligned = false;
    if (oSpan->fPt != thisPt) {
        oAligned |= other->alignSpan(oStart, oT, thisPt);
    }
    while (++otherIndex < oEnd) {
        SkOpSpan* oNextSpan = &other->fTs[otherIndex];
        if (oNextSpan->fT != oT || oNextSpan->fPt != thisPt) {
            oAligned |= other->alignSpan(otherIndex, oT, thisPt);
        }
    }
    if (oAligned) {
        other->alignSpanState(oStart, oEnd);
    }
    return aligned;
}

void SkOpSegment::alignSpanState(int start, int end) {
    SkOpSpan* lastSpan = &fTs[--end];
    bool allSmall = lastSpan->fSmall;
    bool allTiny = lastSpan->fTiny;
    bool allDone = lastSpan->fDone;
    SkDEBUGCODE(int winding = lastSpan->fWindValue);
    SkDEBUGCODE(int oppWinding = lastSpan->fOppValue);
    int index = start;
    while (index < end) {
        SkOpSpan* span = &fTs[index];
        span->fSmall = allSmall;
        span->fTiny = allTiny;
        if (span->fDone != allDone) {
            span->fDone = allDone;
            fDoneSpans += allDone ? 1 : -1;
        }
        SkASSERT(span->fWindValue == winding);
        SkASSERT(span->fOppValue == oppWinding);
        ++index;
    }
}

void SkOpSegment::bumpCoincidentThis(const SkOpSpan& oTest, bool binary, int* indexPtr,
        SkTArray<SkPoint, true>* outsideTs) {
    int index = *indexPtr;
    int oWindValue = oTest.fWindValue;
    int oOppValue = oTest.fOppValue;
    if (binary) {
        SkTSwap<int>(oWindValue, oOppValue);
    }
    SkOpSpan* const test = &fTs[index];
    SkOpSpan* end = test;
    const SkPoint& oStartPt = oTest.fPt;
    do {
        if (bumpSpan(end, oWindValue, oOppValue)) {
            TrackOutside(outsideTs, oStartPt);
        }
        end = &fTs[++index];
    } while ((end->fPt == test->fPt || precisely_equal(end->fT, test->fT)) && end->fT < 1);
    *indexPtr = index;
}

// because of the order in which coincidences are resolved, this and other
// may not have the same intermediate points. Compute the corresponding
// intermediate T values (using this as the master, other as the follower)
// and walk other conditionally -- hoping that it catches up in the end
void SkOpSegment::bumpCoincidentOther(const SkOpSpan& test, int* oIndexPtr,
        SkTArray<SkPoint, true>* oOutsidePts) {
    int oIndex = *oIndexPtr;
    SkOpSpan* const oTest = &fTs[oIndex];
    SkOpSpan* oEnd = oTest;
    const SkPoint& oStartPt = oTest->fPt;
    double oStartT = oTest->fT;
#if 0  // FIXME : figure out what disabling this breaks
    const SkPoint& startPt = test.fPt;
    // this is always true since oEnd == oTest && oStartPt == oTest->fPt -- find proper condition
    if (oStartPt == oEnd->fPt || precisely_equal(oStartT, oEnd->fT)) {
        TrackOutside(oOutsidePts, startPt);
    }
#endif
    while (oStartPt == oEnd->fPt || precisely_equal(oStartT, oEnd->fT)) {
        zeroSpan(oEnd);
        oEnd = &fTs[++oIndex];
    }
    *oIndexPtr = oIndex;
}

// FIXME: need to test this case:
// contourA has two segments that are coincident
// contourB has two segments that are coincident in the same place
// each ends up with +2/0 pairs for winding count
// since logic below doesn't transfer count (only increments/decrements) can this be
// resolved to +4/0 ?

// set spans from start to end to increment the greater by one and decrement
// the lesser
void SkOpSegment::addTCoincident(const SkPoint& startPt, const SkPoint& endPt, double endT,
        SkOpSegment* other) {
    bool binary = fOperand != other->fOperand;
    int index = 0;
    while (startPt != fTs[index].fPt) {
        SkASSERT(index < fTs.count());
        ++index;
    }
    double startT = fTs[index].fT;
    while (index > 0 && precisely_equal(fTs[index - 1].fT, startT)) {
        --index;
    }
    int oIndex = 0;
    while (startPt != other->fTs[oIndex].fPt) {
        SkASSERT(oIndex < other->fTs.count());
        ++oIndex;
    }
    double oStartT = other->fTs[oIndex].fT;
    while (oIndex > 0 && precisely_equal(other->fTs[oIndex - 1].fT, oStartT)) {
        --oIndex;
    }
    SkSTArray<kOutsideTrackedTCount, SkPoint, true> outsidePts;
    SkSTArray<kOutsideTrackedTCount, SkPoint, true> oOutsidePts;
    SkOpSpan* test = &fTs[index];
    const SkPoint* testPt = &test->fPt;
    double testT = test->fT;
    SkOpSpan* oTest = &other->fTs[oIndex];
    const SkPoint* oTestPt = &oTest->fPt;
    SkASSERT(AlmostEqualUlps(*testPt, *oTestPt));
    do {
        SkASSERT(test->fT < 1);
        SkASSERT(oTest->fT < 1);

        // consolidate the winding count even if done
        if ((test->fWindValue == 0 && test->fOppValue == 0)
                || (oTest->fWindValue == 0 && oTest->fOppValue == 0)) {
            SkDEBUGCODE(int firstWind = test->fWindValue);
            SkDEBUGCODE(int firstOpp = test->fOppValue);
            do {
                SkASSERT(firstWind == fTs[index].fWindValue);
                SkASSERT(firstOpp == fTs[index].fOppValue);
                ++index;
                SkASSERT(index < fTs.count());
            } while (*testPt == fTs[index].fPt);
            SkDEBUGCODE(firstWind = oTest->fWindValue);
            SkDEBUGCODE(firstOpp = oTest->fOppValue);
            do {
                SkASSERT(firstWind == other->fTs[oIndex].fWindValue);
                SkASSERT(firstOpp == other->fTs[oIndex].fOppValue);
                ++oIndex;
                SkASSERT(oIndex < other->fTs.count());
            } while (*oTestPt == other->fTs[oIndex].fPt);
        } else {
            if (!binary || test->fWindValue + oTest->fOppValue >= 0) {
                bumpCoincidentThis(*oTest, binary, &index, &outsidePts);
                other->bumpCoincidentOther(*test, &oIndex, &oOutsidePts);
            } else {
                other->bumpCoincidentThis(*test, binary, &oIndex, &oOutsidePts);
                bumpCoincidentOther(*oTest, &index, &outsidePts);
            }
        }
        test = &fTs[index];
        testPt = &test->fPt;
        testT = test->fT;
        oTest = &other->fTs[oIndex];
        oTestPt = &oTest->fPt;
        if (endPt == *testPt || precisely_equal(endT, testT)) {
            break;
        }
//        SkASSERT(AlmostEqualUlps(*testPt, *oTestPt));
    } while (endPt != *oTestPt);
    // in rare cases, one may have ended before the other
    if (endPt != *testPt && !precisely_equal(endT, testT)) {
        int lastWind = test[-1].fWindValue;
        int lastOpp = test[-1].fOppValue;
        bool zero = lastWind == 0 && lastOpp == 0;
        do {
            if (test->fWindValue || test->fOppValue) {
                test->fWindValue = lastWind;
                test->fOppValue = lastOpp;
                if (zero) {
                    test->fDone = true;
                    ++fDoneSpans;
                }
            }
            test = &fTs[++index];
            testPt = &test->fPt;
        } while (endPt != *testPt);
    }
    if (endPt != *oTestPt) {
        // look ahead to see if zeroing more spans will allows us to catch up
        int oPeekIndex = oIndex;
        bool success = true;
        SkOpSpan* oPeek;
        do {
            oPeek = &other->fTs[oPeekIndex];
            if (oPeek->fT == 1) {
                success = false;
                break;
            }
            ++oPeekIndex;
        } while (endPt != oPeek->fPt);
        if (success) {
            // make sure the matching point completes the coincidence span
            success = false;
            do {
                if (oPeek->fOther == this) {
                    success = true;
                    break;
                }
                oPeek = &other->fTs[++oPeekIndex];
            } while (endPt == oPeek->fPt);
        }
        if (success) {
            do {
                if (!binary || test->fWindValue + oTest->fOppValue >= 0) {
                    other->bumpCoincidentOther(*test, &oIndex, &oOutsidePts);
                } else {
                    other->bumpCoincidentThis(*test, binary, &oIndex, &oOutsidePts);
                }
                oTest = &other->fTs[oIndex];
                oTestPt = &oTest->fPt;
            } while (endPt != *oTestPt);
        }
    }
    int outCount = outsidePts.count();
    if (!done() && outCount) {
        addCoinOutsides(outsidePts[0], endPt, other);
    }
    if (!other->done() && oOutsidePts.count()) {
        other->addCoinOutsides(oOutsidePts[0], endPt, this);
    }
}

// FIXME: this doesn't prevent the same span from being added twice
// fix in caller, SkASSERT here?
const SkOpSpan* SkOpSegment::addTPair(double t, SkOpSegment* other, double otherT, bool borrowWind,
                           const SkPoint& pt) {
    int tCount = fTs.count();
    for (int tIndex = 0; tIndex < tCount; ++tIndex) {
        const SkOpSpan& span = fTs[tIndex];
        if (!approximately_negative(span.fT - t)) {
            break;
        }
        if (approximately_negative(span.fT - t) && span.fOther == other
                && approximately_equal(span.fOtherT, otherT)) {
#if DEBUG_ADD_T_PAIR
            SkDebugf("%s addTPair duplicate this=%d %1.9g other=%d %1.9g\n",
                    __FUNCTION__, fID, t, other->fID, otherT);
#endif
            return NULL;
        }
    }
#if DEBUG_ADD_T_PAIR
    SkDebugf("%s addTPair this=%d %1.9g other=%d %1.9g\n",
            __FUNCTION__, fID, t, other->fID, otherT);
#endif
    int insertedAt = addT(other, pt, t);
    int otherInsertedAt = other->addT(this, pt, otherT);
    addOtherT(insertedAt, otherT, otherInsertedAt);
    other->addOtherT(otherInsertedAt, t, insertedAt);
    matchWindingValue(insertedAt, t, borrowWind);
    other->matchWindingValue(otherInsertedAt, otherT, borrowWind);
    return &span(insertedAt);
}

const SkOpAngle& SkOpSegment::angle(int index) const {
    SkASSERT(index >= 0);
    int count = fAngles.count();
    if (index < count) {
        return fAngles[index];
    }
    index -= count;
    count = fSingletonAngles.count();
    SkASSERT(index < count);
    return fSingletonAngles[index];
}

bool SkOpSegment::betweenPoints(double midT, const SkPoint& pt1, const SkPoint& pt2) const {
    const SkPoint midPt = ptAtT(midT);
    SkPathOpsBounds bounds;
    bounds.set(pt1.fX, pt1.fY, pt2.fX, pt2.fY);
    bounds.sort();
    return bounds.almostContains(midPt);
}

bool SkOpSegment::betweenTs(int lesser, double testT, int greater) const {
    if (lesser > greater) {
        SkTSwap<int>(lesser, greater);
    }
    return approximately_between(fTs[lesser].fT, testT, fTs[greater].fT);
}

// in extreme cases (like the buffer overflow test) return false to abort
// for now, if one t value represents two different points, then the values are too extreme
// to generate meaningful results
bool SkOpSegment::calcAngles() {
    int spanCount = fTs.count();
    if (spanCount <= 2) {
        return spanCount == 2;
    }
    int index = 1;
    const SkOpSpan* firstSpan = &fTs[index];
    int activePrior = checkSetAngle(0);
    const SkOpSpan* span = &fTs[0];
    if (firstSpan->fT == 0 || span->fTiny || span->fOtherT != 1 || span->fOther->multipleEnds()) {
        index = findStartSpan(0);  // curve start intersects
        if (index < 0) {
            return false;
        }
        if (activePrior >= 0) {
            addStartSpan(index);
        }
    }
    bool addEnd;
    int endIndex = spanCount - 1;
    span = &fTs[endIndex - 1];
    if ((addEnd = span->fT == 1 || span->fTiny)) {  // if curve end intersects
        endIndex = findEndSpan(endIndex);
        if (endIndex < 0) {
            return false;
        }
    } else {
        addEnd = fTs[endIndex].fOtherT != 0 || fTs[endIndex].fOther->multipleStarts();
    }
    SkASSERT(endIndex >= index);
    int prior = 0;
    while (index < endIndex) {
        const SkOpSpan& fromSpan = fTs[index];  // for each intermediate intersection
        const SkOpSpan* lastSpan;
        span = &fromSpan;
        int start = index;
        do {
            lastSpan = span;
            span = &fTs[++index];
            SkASSERT(index < spanCount);
            if (!precisely_negative(span->fT - lastSpan->fT) && !lastSpan->fTiny) {
                break;
            }
            if (!SkDPoint::ApproximatelyEqual(lastSpan->fPt, span->fPt)) {
                return false;
            }
        } while (true);
        int angleIndex = fAngles.count();
        int priorAngleIndex;
        if (activePrior >= 0) {
            int pActive = firstActive(prior);
            SkASSERT(pActive < start);
            fAngles.push_back().set(this, start, pActive);
            priorAngleIndex = angleIndex++;
    #if DEBUG_ANGLE
            fAngles.back().setID(priorAngleIndex);
    #endif
        }
        int active = checkSetAngle(start);
        if (active >= 0) {
            SkASSERT(active < index);
            fAngles.push_back().set(this, active, index);
    #if DEBUG_ANGLE
            fAngles.back().setID(angleIndex);
    #endif
        }
    #if DEBUG_ANGLE
        debugCheckPointsEqualish(start, index);
    #endif
        prior = start;
        do {
            const SkOpSpan* startSpan = &fTs[start - 1];
            if (!startSpan->fSmall || startSpan->fFromAngleIndex >= 0
                    || startSpan->fToAngleIndex >= 0) {
                break;
            }
            --start;
        } while (start > 0);
        do {
            if (activePrior >= 0) {
                SkASSERT(fTs[start].fFromAngleIndex == -1);
                fTs[start].fFromAngleIndex = priorAngleIndex;
            }
            if (active >= 0) {
                SkASSERT(fTs[start].fToAngleIndex == -1);
                fTs[start].fToAngleIndex = angleIndex;
            }
        } while (++start < index);
        activePrior = active;
    }
    if (addEnd && activePrior >= 0) {
        addEndSpan(endIndex);
    }
    return true;
}

int SkOpSegment::checkSetAngle(int tIndex) const {
    const SkOpSpan* span = &fTs[tIndex];
    while (span->fTiny /* || span->fSmall */) {
        span = &fTs[++tIndex];
    }
    return isCanceled(tIndex) ? -1 : tIndex;
}

// at this point, the span is already ordered, or unorderable, or unsortable
int SkOpSegment::computeSum(int startIndex, int endIndex, SkOpAngle::IncludeType includeType) {
    SkASSERT(includeType != SkOpAngle::kUnaryXor);
    SkOpAngle* firstAngle = spanToAngle(endIndex, startIndex);
    if (NULL == firstAngle) {
        return SK_NaN32;
    }
    // if all angles have a computed winding,
    //  or if no adjacent angles are orderable,
    //  or if adjacent orderable angles have no computed winding,
    //  there's nothing to do
    // if two orderable angles are adjacent, and one has winding computed, transfer to the other
    SkOpAngle* baseAngle = NULL;
    bool tryReverse = false;
    // look for counterclockwise transfers
    SkOpAngle* angle = firstAngle;
    do {
        int testWinding = angle->segment()->windSum(angle);
        if (SK_MinS32 != testWinding && !angle->unorderable()) {
            baseAngle = angle;
            continue;
        }
        if (angle->unorderable()) {
            baseAngle = NULL;
            tryReverse = true;
            continue;
        }
        if (baseAngle) {
            ComputeOneSum(baseAngle, angle, includeType);
            baseAngle = SK_MinS32 != angle->segment()->windSum(angle) ? angle : NULL;
            tryReverse |= !baseAngle;
        }
    } while ((angle = angle->next()) != firstAngle);
    if (baseAngle && SK_MinS32 == firstAngle->segment()->windSum(angle)) {
        firstAngle = baseAngle;
        tryReverse = true;
    }
    if (tryReverse) {
        baseAngle = NULL;
        angle = firstAngle;
        do {
            int testWinding = angle->segment()->windSum(angle);
            if (SK_MinS32 != testWinding) {
                baseAngle = angle;
                continue;
            }
            if (angle->unorderable()) {
                baseAngle = NULL;
                continue;
            }
            if (baseAngle) {
                ComputeOneSumReverse(baseAngle, angle, includeType);
                baseAngle = SK_MinS32 != angle->segment()->windSum(angle) ? angle : NULL;
            }
        } while ((angle = angle->previous()) != firstAngle);
    }
    int minIndex = SkMin32(startIndex, endIndex);
    return windSum(minIndex);
}

void SkOpSegment::ComputeOneSum(const SkOpAngle* baseAngle, SkOpAngle* nextAngle,
        SkOpAngle::IncludeType includeType) {
    const SkOpSegment* baseSegment = baseAngle->segment();
    int sumMiWinding = baseSegment->updateWindingReverse(baseAngle);
    int sumSuWinding;
    bool binary = includeType >= SkOpAngle::kBinarySingle;
    if (binary) {
        sumSuWinding = baseSegment->updateOppWindingReverse(baseAngle);
        if (baseSegment->operand()) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
    }
    SkOpSegment* nextSegment = nextAngle->segment();
    int maxWinding, sumWinding;
    SkOpSpan* last;
    if (binary) {
        int oppMaxWinding, oppSumWinding;
        nextSegment->setUpWindings(nextAngle->start(), nextAngle->end(), &sumMiWinding,
                &sumSuWinding, &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, oppMaxWinding, oppSumWinding,
                nextAngle);
    } else {
        nextSegment->setUpWindings(nextAngle->start(), nextAngle->end(), &sumMiWinding,
                &maxWinding, &sumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, nextAngle);
    }
    nextAngle->setLastMarked(last);
}

void SkOpSegment::ComputeOneSumReverse(const SkOpAngle* baseAngle, SkOpAngle* nextAngle,
        SkOpAngle::IncludeType includeType) {
    const SkOpSegment* baseSegment = baseAngle->segment();
    int sumMiWinding = baseSegment->updateWinding(baseAngle);
    int sumSuWinding;
    bool binary = includeType >= SkOpAngle::kBinarySingle;
    if (binary) {
        sumSuWinding = baseSegment->updateOppWinding(baseAngle);
        if (baseSegment->operand()) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
    }
    SkOpSegment* nextSegment = nextAngle->segment();
    int maxWinding, sumWinding;
    SkOpSpan* last;
    if (binary) {
        int oppMaxWinding, oppSumWinding;
        nextSegment->setUpWindings(nextAngle->end(), nextAngle->start(), &sumMiWinding,
                &sumSuWinding, &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, oppMaxWinding, oppSumWinding,
                nextAngle);
    } else {
        nextSegment->setUpWindings(nextAngle->end(), nextAngle->start(), &sumMiWinding,
                &maxWinding, &sumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, nextAngle);
    }
    nextAngle->setLastMarked(last);
}

bool SkOpSegment::containsPt(const SkPoint& pt, int index, int endIndex) const {
    int step = index < endIndex ? 1 : -1;
    do {
        const SkOpSpan& span = this->span(index);
        if (span.fPt == pt) {
            const SkOpSpan& endSpan = this->span(endIndex);
            return span.fT == endSpan.fT && pt != endSpan.fPt;
        }
        index += step;
    } while (index != endIndex);
    return false;
}

int SkOpSegment::crossedSpanY(const SkPoint& basePt, SkScalar* bestY, double* hitT,
                              bool* hitSomething, double mid, bool opp, bool current) const {
    SkScalar bottom = fBounds.fBottom;
    int bestTIndex = -1;
    if (bottom <= *bestY) {
        return bestTIndex;
    }
    SkScalar top = fBounds.fTop;
    if (top >= basePt.fY) {
        return bestTIndex;
    }
    if (fBounds.fLeft > basePt.fX) {
        return bestTIndex;
    }
    if (fBounds.fRight < basePt.fX) {
        return bestTIndex;
    }
    if (fBounds.fLeft == fBounds.fRight) {
        // if vertical, and directly above test point, wait for another one
        return AlmostEqualUlps(basePt.fX, fBounds.fLeft) ? SK_MinS32 : bestTIndex;
    }
    // intersect ray starting at basePt with edge
    SkIntersections intersections;
    // OPTIMIZE: use specialty function that intersects ray with curve,
    // returning t values only for curve (we don't care about t on ray)
    intersections.allowNear(false);
    int pts = (intersections.*CurveVertical[SkPathOpsVerbToPoints(fVerb)])
            (fPts, top, bottom, basePt.fX, false);
    if (pts == 0 || (current && pts == 1)) {
        return bestTIndex;
    }
    if (current) {
        SkASSERT(pts > 1);
        int closestIdx = 0;
        double closest = fabs(intersections[0][0] - mid);
        for (int idx = 1; idx < pts; ++idx) {
            double test = fabs(intersections[0][idx] - mid);
            if (closest > test) {
                closestIdx = idx;
                closest = test;
            }
        }
        intersections.quickRemoveOne(closestIdx, --pts);
    }
    double bestT = -1;
    for (int index = 0; index < pts; ++index) {
        double foundT = intersections[0][index];
        if (approximately_less_than_zero(foundT)
                    || approximately_greater_than_one(foundT)) {
            continue;
        }
        SkScalar testY = (*CurvePointAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, foundT).fY;
        if (approximately_negative(testY - *bestY)
                || approximately_negative(basePt.fY - testY)) {
            continue;
        }
        if (pts > 1 && fVerb == SkPath::kLine_Verb) {
            return SK_MinS32;  // if the intersection is edge on, wait for another one
        }
        if (fVerb > SkPath::kLine_Verb) {
            SkScalar dx = (*CurveSlopeAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, foundT).fX;
            if (approximately_zero(dx)) {
                return SK_MinS32;  // hit vertical, wait for another one
            }
        }
        *bestY = testY;
        bestT = foundT;
    }
    if (bestT < 0) {
        return bestTIndex;
    }
    SkASSERT(bestT >= 0);
    SkASSERT(bestT <= 1);
    int start;
    int end = 0;
    do {
        start = end;
        end = nextSpan(start, 1);
    } while (fTs[end].fT < bestT);
    // FIXME: see next candidate for a better pattern to find the next start/end pair
    while (start + 1 < end && fTs[start].fDone) {
        ++start;
    }
    if (!isCanceled(start)) {
        *hitT = bestT;
        bestTIndex = start;
        *hitSomething = true;
    }
    return bestTIndex;
}

bool SkOpSegment::decrementSpan(SkOpSpan* span) {
    SkASSERT(span->fWindValue > 0);
    if (--(span->fWindValue) == 0) {
        if (!span->fOppValue && !span->fDone) {
            span->fDone = true;
            ++fDoneSpans;
            return true;
        }
    }
    return false;
}

bool SkOpSegment::bumpSpan(SkOpSpan* span, int windDelta, int oppDelta) {
    SkASSERT(!span->fDone || span->fTiny || span->fSmall);
    span->fWindValue += windDelta;
    SkASSERT(span->fWindValue >= 0);
    span->fOppValue += oppDelta;
    SkASSERT(span->fOppValue >= 0);
    if (fXor) {
        span->fWindValue &= 1;
    }
    if (fOppXor) {
        span->fOppValue &= 1;
    }
    if (!span->fWindValue && !span->fOppValue) {
        span->fDone = true;
        ++fDoneSpans;
        return true;
    }
    return false;
}

const SkOpSpan& SkOpSegment::firstSpan(const SkOpSpan& thisSpan) const {
    const SkOpSpan* firstSpan = &thisSpan; // rewind to the start
    const SkOpSpan* beginSpan = fTs.begin();
    const SkPoint& testPt = thisSpan.fPt;
    while (firstSpan > beginSpan && firstSpan[-1].fPt == testPt) {
        --firstSpan;
    }
    return *firstSpan;
}

const SkOpSpan& SkOpSegment::lastSpan(const SkOpSpan& thisSpan) const {
    const SkOpSpan* endSpan = fTs.end() - 1;  // last can't be small
    const SkOpSpan* lastSpan = &thisSpan;  // find the end
    const SkPoint& testPt = thisSpan.fPt;
    while (lastSpan < endSpan && lastSpan[1].fPt == testPt) {
        ++lastSpan;
    }
    return *lastSpan;
}

// with a loop, the comparison is move involved
// scan backwards and forwards to count all matching points
// (verify that there are twp scans marked as loops)
// compare that against 2 matching scans for loop plus other results
bool SkOpSegment::calcLoopSpanCount(const SkOpSpan& thisSpan, int* smallCounts) {
    const SkOpSpan& firstSpan = this->firstSpan(thisSpan); // rewind to the start
    const SkOpSpan& lastSpan = this->lastSpan(thisSpan);  // find the end
    double firstLoopT = -1, lastLoopT = -1;
    const SkOpSpan* testSpan = &firstSpan - 1;
    while (++testSpan <= &lastSpan) {
        if (testSpan->fLoop) {
            firstLoopT = testSpan->fT;
            break;
        }
    }
    testSpan = &lastSpan + 1;
    while (--testSpan >= &firstSpan) {
        if (testSpan->fLoop) {
            lastLoopT = testSpan->fT;
            break;
        }
    }
    SkASSERT((firstLoopT == -1) == (lastLoopT == -1));
    if (firstLoopT == -1) {
        return false;
    }
    SkASSERT(firstLoopT < lastLoopT);
    testSpan = &firstSpan - 1;
    smallCounts[0] = smallCounts[1] = 0;
    while (++testSpan <= &lastSpan) {
        SkASSERT(approximately_equal(testSpan->fT, firstLoopT) +
                approximately_equal(testSpan->fT, lastLoopT) == 1);
        smallCounts[approximately_equal(testSpan->fT, lastLoopT)]++;
    }
    return true;
}

// see if spans with two or more intersections have the same number on the other end
void SkOpSegment::checkDuplicates() {
    debugValidate();
    SkSTArray<kMissingSpanCount, MissingSpan, true> missingSpans;
    int index;
    int endIndex = 0;
    bool endFound;
    do {
        index = endIndex;
        endIndex = nextExactSpan(index, 1);
        if ((endFound = endIndex < 0)) {
            endIndex = count();
        }
        int dupCount = endIndex - index;
        if (dupCount < 2) {
            continue;
        }
        do {
            const SkOpSpan* thisSpan = &fTs[index];
            SkOpSegment* other = thisSpan->fOther;
            int oIndex = thisSpan->fOtherIndex;
            int oStart = other->nextExactSpan(oIndex, -1) + 1;
            int oEnd = other->nextExactSpan(oIndex, 1);
            if (oEnd < 0) {
                oEnd = other->count();
            }
            int oCount = oEnd - oStart;
            // force the other to match its t and this pt if not on an end point
            if (oCount != dupCount) {
                MissingSpan& missing = missingSpans.push_back();
                missing.fOther = NULL;
                SkDEBUGCODE(sk_bzero(&missing, sizeof(missing)));
                missing.fPt = thisSpan->fPt;
                const SkOpSpan& oSpan = other->span(oIndex);
                if (oCount > dupCount) {
                    missing.fSegment = this;
                    missing.fT = thisSpan->fT;
                    other->checkLinks(&oSpan, &missingSpans);
                } else {
                    missing.fSegment = other;
                    missing.fT = oSpan.fT;
                    checkLinks(thisSpan, &missingSpans);
                }
                if (!missingSpans.back().fOther) {
                    missingSpans.pop_back();
                }
            }
        } while (++index < endIndex);
    } while (!endFound);
    int missingCount = missingSpans.count();
    if (missingCount == 0) {
        return;
    }
    SkSTArray<kMissingSpanCount, MissingSpan, true> missingCoincidence;
    for (index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        SkOpSegment* missingOther = missing.fOther;
        if (missing.fSegment == missing.fOther) {
            continue;
        }
        const SkOpSpan* added = missing.fSegment->addTPair(missing.fT, missingOther,
                missing.fOtherT, false, missing.fPt);
        if (added && added->fSmall) {
            missing.fSegment->checkSmallCoincidence(*added, &missingCoincidence);
        }
    }
    for (index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        missing.fSegment->fixOtherTIndex();
        missing.fOther->fixOtherTIndex();
    }
    for (index = 0; index < missingCoincidence.count(); ++index) {
        MissingSpan& missing = missingCoincidence[index];
        missing.fSegment->fixOtherTIndex();
    }
    debugValidate();
}

// look to see if the curve end intersects an intermediary that intersects the other
void SkOpSegment::checkEnds() {
    debugValidate();
    SkSTArray<kMissingSpanCount, MissingSpan, true> missingSpans;
    int count = fTs.count();
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        double otherT = span.fOtherT;
        if (otherT != 0 && otherT != 1) { // only check ends
            continue;
        }
        const SkOpSegment* other = span.fOther;
        // peek start/last describe the range of spans that match the other t of this span
        int peekStart = span.fOtherIndex;
        while (--peekStart >= 0 && other->fTs[peekStart].fT == otherT)
            ;
        int otherCount = other->fTs.count();
        int peekLast = span.fOtherIndex;
        while (++peekLast < otherCount && other->fTs[peekLast].fT == otherT)
            ;
        if (++peekStart == --peekLast) { // if there isn't a range, there's nothing to do
            continue;
        }
        // t start/last describe the range of spans that match the t of this span
        double t = span.fT;
        double tBottom = -1;
        int tStart = -1;
        int tLast = count;
        bool lastSmall = false;
        double afterT = t;
        for (int inner = 0; inner < count; ++inner) {
            double innerT = fTs[inner].fT;
            if (innerT <= t && innerT > tBottom) {
                if (innerT < t || !lastSmall) {
                    tStart = inner - 1;
                }
                tBottom = innerT;
            }
            if (innerT > afterT) {
                if (t == afterT && lastSmall) {
                    afterT = innerT;
                } else {
                    tLast = inner;
                    break;
                }
            }
            lastSmall = innerT <= t ? fTs[inner].fSmall : false;
        }
        for (int peekIndex = peekStart; peekIndex <= peekLast; ++peekIndex) {
            if (peekIndex == span.fOtherIndex) {  // skip the other span pointed to by this span
                continue;
            }
            const SkOpSpan& peekSpan = other->fTs[peekIndex];
            SkOpSegment* match = peekSpan.fOther;
            if (match->done()) {
                continue;  // if the edge has already been eaten (likely coincidence), ignore it
            }
            const double matchT = peekSpan.fOtherT;
            // see if any of the spans match the other spans
            for (int tIndex = tStart + 1; tIndex < tLast; ++tIndex) {
                const SkOpSpan& tSpan = fTs[tIndex];
                if (tSpan.fOther == match) {
                    if (tSpan.fOtherT == matchT) {
                        goto nextPeekIndex;
                    }
                    double midT = (tSpan.fOtherT + matchT) / 2;
                    if (match->betweenPoints(midT, tSpan.fPt, peekSpan.fPt)) {
                        goto nextPeekIndex;
                    }
                }
            }
            if (missingSpans.count() > 0) {
                const MissingSpan& lastMissing = missingSpans.back();
                if (lastMissing.fT == t
                        && lastMissing.fOther == match
                        && lastMissing.fOtherT == matchT) {
                    SkASSERT(lastMissing.fPt == peekSpan.fPt);
                    continue;
                }
            }
#if DEBUG_CHECK_ENDS
            SkDebugf("%s id=%d missing t=%1.9g other=%d otherT=%1.9g pt=(%1.9g,%1.9g)\n",
                    __FUNCTION__, fID, t, match->fID, matchT, peekSpan.fPt.fX, peekSpan.fPt.fY);
#endif
            // this segment is missing a entry that the other contains
            // remember so we can add the missing one and recompute the indices
            {
                MissingSpan& missing = missingSpans.push_back();
                SkDEBUGCODE(sk_bzero(&missing, sizeof(missing)));
                missing.fT = t;
                missing.fOther = match;
                missing.fOtherT = matchT;
                missing.fPt = peekSpan.fPt;
            }
            break;
nextPeekIndex:
            ;
        }
    }
    if (missingSpans.count() == 0) {
        debugValidate();
        return;
    }
    debugValidate();
    int missingCount = missingSpans.count();
    for (int index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        if (this != missing.fOther) {
            addTPair(missing.fT, missing.fOther, missing.fOtherT, false, missing.fPt);
        }
    }
    fixOtherTIndex();
    // OPTIMIZATION: this may fix indices more than once. Build an array of unique segments to
    // avoid this
    for (int index = 0; index < missingCount; ++index)  {
        missingSpans[index].fOther->fixOtherTIndex();
    }
    debugValidate();
}

void SkOpSegment::checkLinks(const SkOpSpan* base,
        SkTArray<MissingSpan, true>* missingSpans) const {
    const SkOpSpan* first = fTs.begin();
    const SkOpSpan* last = fTs.end() - 1;
    SkASSERT(base >= first && last >= base);
    const SkOpSegment* other = base->fOther;
    const SkOpSpan* oFirst = other->fTs.begin();
    const SkOpSpan* oLast = other->fTs.end() - 1;
    const SkOpSpan* oSpan = &other->fTs[base->fOtherIndex];
    const SkOpSpan* test = base;
    const SkOpSpan* missing = NULL;
    while (test > first && (--test)->fPt == base->fPt) {
        CheckOneLink(test, oSpan, oFirst, oLast, &missing, missingSpans);
    }
    test = base;
    while (test < last && (++test)->fPt == base->fPt) {
        CheckOneLink(test, oSpan, oFirst, oLast, &missing, missingSpans);
    }
}

// see if spans with two or more intersections all agree on common t and point values
void SkOpSegment::checkMultiples() {
    debugValidate();
    int index;
    int end = 0;
    while (fTs[++end].fT == 0)
        ;
    while (fTs[end].fT < 1) {
        int start = index = end;
        end = nextExactSpan(index, 1);
        if (end <= index) {
            return;  // buffer overflow example triggers this
        }
        if (index + 1 == end) {
            continue;
        }
        // force the duplicates to agree on t and pt if not on the end
        double thisT = fTs[index].fT;
        const SkPoint& thisPt = fTs[index].fPt;
        bool aligned = false;
        while (++index < end) {
            aligned |= alignSpan(index, thisT, thisPt);
        }
        if (aligned) {
            alignSpanState(start, end);
        }
    }
    debugValidate();
}

void SkOpSegment::CheckOneLink(const SkOpSpan* test, const SkOpSpan* oSpan,
        const SkOpSpan* oFirst, const SkOpSpan* oLast, const SkOpSpan** missingPtr,
        SkTArray<MissingSpan, true>* missingSpans) {
    SkASSERT(oSpan->fPt == test->fPt);
    const SkOpSpan* oTest = oSpan;
    while (oTest > oFirst && (--oTest)->fPt == test->fPt) {
        if (oTest->fOther == test->fOther && oTest->fOtherT == test->fOtherT) {
            return;
        }
    }
    oTest = oSpan;
    while (oTest < oLast && (++oTest)->fPt == test->fPt) {
        if (oTest->fOther == test->fOther && oTest->fOtherT == test->fOtherT) {
            return;
        }
    }
    if (*missingPtr) {
        missingSpans->push_back();
    }
    MissingSpan& lastMissing = missingSpans->back();
    if (*missingPtr) {
        lastMissing = missingSpans->end()[-2];
    }
    *missingPtr = test;
    lastMissing.fOther = test->fOther;
    lastMissing.fOtherT = test->fOtherT;
}

bool SkOpSegment::checkSmall(int index) const {
    if (fTs[index].fSmall) {
        return true;
    }
    double tBase = fTs[index].fT;
    while (index > 0 && precisely_negative(tBase - fTs[--index].fT))
        ;
    return fTs[index].fSmall;
}

// a pair of curves may turn into coincident lines -- small may be a hint that that happened
// if a cubic contains a loop, the counts must be adjusted
void SkOpSegment::checkSmall() {
    SkSTArray<kMissingSpanCount, MissingSpan, true> missingSpans;
    const SkOpSpan* beginSpan = fTs.begin();
    const SkOpSpan* thisSpan = beginSpan - 1;
    const SkOpSpan* endSpan = fTs.end() - 1;  // last can't be small
    while (++thisSpan < endSpan) {
        if (!thisSpan->fSmall) {
            continue;
        }
        if (!thisSpan->fWindValue) {
            continue;
        }
        const SkOpSpan& firstSpan = this->firstSpan(*thisSpan);
        const SkOpSpan& lastSpan = this->lastSpan(*thisSpan);
        ptrdiff_t smallCount = &lastSpan - &firstSpan + 1;
        SkASSERT(1 <= smallCount && smallCount < count());
        if (smallCount <= 1) {
            SkASSERT(1 == smallCount);
            checkSmallCoincidence(firstSpan, NULL);
            continue;
        }
        // at this point, check for missing computed intersections
        const SkPoint& testPt = firstSpan.fPt;
        thisSpan = &firstSpan - 1;
        SkOpSegment* other = NULL;
        while (++thisSpan <= &lastSpan) {
            other = thisSpan->fOther;
            if (other != this) {
                break;
            }
        }
        SkASSERT(other != this);
        int oIndex = thisSpan->fOtherIndex;
        const SkOpSpan& oSpan = other->span(oIndex);
        const SkOpSpan& oFirstSpan = other->firstSpan(oSpan);
        const SkOpSpan& oLastSpan = other->lastSpan(oSpan);
        ptrdiff_t oCount = &oLastSpan - &oFirstSpan + 1;
        if (fLoop) {
            int smallCounts[2];
            SkASSERT(!other->fLoop);  // FIXME: we need more complicated logic for pair of loops
            if (calcLoopSpanCount(*thisSpan, smallCounts)) {
                if (smallCounts[0] && oCount != smallCounts[0]) {
                    SkASSERT(0);  // FIXME: need a working test case to properly code & debug
                }
                if (smallCounts[1] && oCount != smallCounts[1]) {
                    SkASSERT(0);  // FIXME: need a working test case to properly code & debug
                }
                goto nextSmallCheck;
            }
        }
        if (other->fLoop) {
            int otherCounts[2];
            if (other->calcLoopSpanCount(other->span(oIndex), otherCounts)) {
                if (otherCounts[0] && otherCounts[0] != smallCount) {
                    SkASSERT(0);  // FIXME: need a working test case to properly code & debug
                }
                if (otherCounts[1] && otherCounts[1] != smallCount) {
                    SkASSERT(0);  // FIXME: need a working test case to properly code & debug
                }
                goto nextSmallCheck;
            }
        }
        if (oCount != smallCount) {  // check if number of pts in this match other
            MissingSpan& missing = missingSpans.push_back();
            missing.fOther = NULL;
            SkDEBUGCODE(sk_bzero(&missing, sizeof(missing)));
            missing.fPt = testPt;
            const SkOpSpan& oSpan = other->span(oIndex);
            if (oCount > smallCount) {
                missing.fSegment = this;
                missing.fT = thisSpan->fT;
                other->checkLinks(&oSpan, &missingSpans);
            } else {
                missing.fSegment = other;
                missing.fT = oSpan.fT;
                checkLinks(thisSpan, &missingSpans);
            }
            if (!missingSpans.back().fOther || missing.fSegment->done()) {
                missingSpans.pop_back();
            }
        }
nextSmallCheck:
        thisSpan = &lastSpan;
    }
    int missingCount = missingSpans.count();
    for (int index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        SkOpSegment* missingOther = missing.fOther;
        // note that add t pair may edit span arrays, so prior pointers to spans are no longer valid
        if (!missing.fSegment->addTPair(missing.fT, missingOther, missing.fOtherT, false,
                missing.fPt)) {
            continue;
        }
        int otherTIndex = missingOther->findT(missing.fOtherT, missing.fPt, missing.fSegment);
        const SkOpSpan& otherSpan = missingOther->span(otherTIndex);
        if (otherSpan.fSmall) {
            const SkOpSpan* nextSpan = &otherSpan;
            do {
                ++nextSpan;
            } while (nextSpan->fSmall);
            missing.fSegment->addTCoincident(missing.fPt, nextSpan->fPt, nextSpan->fT,
                    missingOther);
        } else if (otherSpan.fT > 0) {
            const SkOpSpan* priorSpan = &otherSpan;
            do {
                --priorSpan;
            } while (priorSpan->fT == otherSpan.fT);
            if (priorSpan->fSmall) {
                missing.fSegment->addTCancel(missing.fPt, priorSpan->fPt, missingOther);
            }
        }
    }
    // OPTIMIZATION: this may fix indices more than once. Build an array of unique segments to
    // avoid this
    for (int index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        missing.fSegment->fixOtherTIndex();
        missing.fOther->fixOtherTIndex();
    }
    debugValidate();
}

void SkOpSegment::checkSmallCoincidence(const SkOpSpan& span,
        SkTArray<MissingSpan, true>* checkMultiple) {
    SkASSERT(span.fSmall);
    if (0 && !span.fWindValue) {
        return;
    }
    SkASSERT(&span < fTs.end() - 1);
    const SkOpSpan* next = &span + 1;
    SkASSERT(!next->fSmall || checkMultiple);
    if (checkMultiple) {
        while (next->fSmall) {
            ++next;
            SkASSERT(next < fTs.end());
        }
    }
    SkOpSegment* other = span.fOther;
    while (other != next->fOther) {
        if (!checkMultiple) {
            return;
        }
        const SkOpSpan* test = next + 1;
        if (test == fTs.end()) {
            return;
        }
        if (test->fPt != next->fPt || !precisely_equal(test->fT, next->fT)) {
            return;
        }
        next = test;
    }
    SkASSERT(span.fT < next->fT);
    int oStartIndex = other->findExactT(span.fOtherT, this);
    int oEndIndex = other->findExactT(next->fOtherT, this);
    // FIXME: be overly conservative by limiting this to the caller that allows multiple smalls
    if (!checkMultiple || fVerb != SkPath::kLine_Verb || other->fVerb != SkPath::kLine_Verb) {
        SkPoint mid = ptAtT((span.fT + next->fT) / 2);
        const SkOpSpan& oSpanStart = other->fTs[oStartIndex];
        const SkOpSpan& oSpanEnd = other->fTs[oEndIndex];
        SkPoint oMid = other->ptAtT((oSpanStart.fT + oSpanEnd.fT) / 2);
        if (!SkDPoint::ApproximatelyEqual(mid, oMid)) {
            return;
        }
    }
    // FIXME: again, be overly conservative to avoid breaking existing tests
    const SkOpSpan& oSpan = oStartIndex < oEndIndex ? other->fTs[oStartIndex]
            : other->fTs[oEndIndex];
    if (checkMultiple && !oSpan.fSmall) {
        return;
    }
    SkASSERT(oSpan.fSmall);
    if (oStartIndex < oEndIndex) {
        addTCoincident(span.fPt, next->fPt, next->fT, other);
    } else {
        addTCancel(span.fPt, next->fPt, other);
    }
    if (!checkMultiple) {
        return;
    }
    // check to see if either segment is coincident with a third segment -- if it is, and if
    // the opposite segment is not already coincident with the third, make it so
    // OPTIMIZE: to make this check easier, add coincident and cancel could set a coincident bit
    if (span.fWindValue != 1 || span.fOppValue != 0) {
//        start here;
        // iterate through the spans, looking for the third coincident case
        // if we find one, we need to return state to the caller so that the indices can be fixed
        // this also suggests that all of this function is fragile since it relies on a valid index
    }
    // probably should make this a common function rather than copy/paste code
    if (oSpan.fWindValue != 1 || oSpan.fOppValue != 0) {
        const SkOpSpan* oTest = &oSpan;
        while (--oTest >= other->fTs.begin()) {
            if (oTest->fPt != oSpan.fPt || !precisely_equal(oTest->fT, oSpan.fT)) {
                break;
            }
            SkOpSegment* testOther = oTest->fOther;
            SkASSERT(testOther != this);
            // look in both directions to see if there is a coincident span
            const SkOpSpan* tTest = testOther->fTs.begin();
            for (int testIndex = 0; testIndex < testOther->count(); ++testIndex) {
                if (tTest->fPt != span.fPt) {
                    ++tTest;
                    continue;
                }
                if (testOther->verb() != SkPath::kLine_Verb
                        || other->verb() != SkPath::kLine_Verb) {
                    SkPoint mid = ptAtT((span.fT + next->fT) / 2);
                    SkPoint oMid = other->ptAtT((oTest->fOtherT + tTest->fT) / 2);
                    if (!SkDPoint::ApproximatelyEqual(mid, oMid)) {
                        continue;
                    }
                }
#if DEBUG_CONCIDENT
                SkDebugf("%s coincident found=%d %1.9g %1.9g\n", __FUNCTION__, testOther->fID,
                        oTest->fOtherT, tTest->fT);
#endif
                if (tTest->fT < oTest->fOtherT) {
                    addTCoincident(span.fPt, next->fPt, next->fT, testOther);
                } else {
                    addTCancel(span.fPt, next->fPt, testOther);
                }
                MissingSpan missing;
                missing.fSegment = testOther;
                checkMultiple->push_back(missing);
                break;
            }
        }
        oTest = &oSpan;
        while (++oTest < other->fTs.end()) {
            if (oTest->fPt != oSpan.fPt || !precisely_equal(oTest->fT, oSpan.fT)) {
                break;
            }

        }
    }
}

// if pair of spans on either side of tiny have the same end point and mid point, mark
// them as parallel
void SkOpSegment::checkTiny() {
    SkSTArray<kMissingSpanCount, MissingSpan, true> missingSpans;
    SkOpSpan* thisSpan = fTs.begin() - 1;
    const SkOpSpan* endSpan = fTs.end() - 1;  // last can't be tiny
    while (++thisSpan < endSpan) {
        if (!thisSpan->fTiny) {
            continue;
        }
        SkOpSpan* nextSpan = thisSpan + 1;
        double thisT = thisSpan->fT;
        double nextT = nextSpan->fT;
        if (thisT == nextT) {
            continue;
        }
        SkASSERT(thisT < nextT);
        SkASSERT(thisSpan->fPt == nextSpan->fPt);
        SkOpSegment* thisOther = thisSpan->fOther;
        SkOpSegment* nextOther = nextSpan->fOther;
        int oIndex = thisSpan->fOtherIndex;
        for (int oStep = -1; oStep <= 1; oStep += 2) {
            int oEnd = thisOther->nextExactSpan(oIndex, oStep);
            if (oEnd < 0) {
                continue;
            }
            const SkOpSpan& oSpan = thisOther->span(oEnd);
            int nIndex = nextSpan->fOtherIndex;
            for (int nStep = -1; nStep <= 1; nStep += 2) {
                int nEnd = nextOther->nextExactSpan(nIndex, nStep);
                if (nEnd < 0) {
                    continue;
                }
                const SkOpSpan& nSpan = nextOther->span(nEnd);
                if (oSpan.fPt != nSpan.fPt) {
                    continue;
                }
                double oMidT = (thisSpan->fOtherT + oSpan.fT) / 2;
                const SkPoint& oPt = thisOther->ptAtT(oMidT);
                double nMidT = (nextSpan->fOtherT + nSpan.fT) / 2;
                const SkPoint& nPt = nextOther->ptAtT(nMidT);
                if (!AlmostEqualUlps(oPt, nPt)) {
                    continue;
                }
#if DEBUG_CHECK_TINY
                SkDebugf("%s [%d] add coincidence [%d] [%d]\n", __FUNCTION__, fID,
                    thisOther->fID, nextOther->fID);
#endif
                // this segment is missing a entry that the other contains
                // remember so we can add the missing one and recompute the indices
                MissingSpan& missing = missingSpans.push_back();
                SkDEBUGCODE(sk_bzero(&missing, sizeof(missing)));
                missing.fSegment = thisOther;
                missing.fT = thisSpan->fOtherT;
                missing.fOther = nextOther;
                missing.fOtherT = nextSpan->fOtherT;
                missing.fPt = thisSpan->fPt;
            }
        }
    }
    int missingCount = missingSpans.count();
    if (!missingCount) {
        return;
    }
    for (int index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        if (missing.fSegment != missing.fOther) {
            missing.fSegment->addTPair(missing.fT, missing.fOther, missing.fOtherT, false,
                    missing.fPt);
        }
    }
    // OPTIMIZE: consolidate to avoid multiple calls to fix index
    for (int index = 0; index < missingCount; ++index)  {
        MissingSpan& missing = missingSpans[index];
        missing.fSegment->fixOtherTIndex();
        missing.fOther->fixOtherTIndex();
    }
}

bool SkOpSegment::findCoincidentMatch(const SkOpSpan* span, const SkOpSegment* other, int oStart,
        int oEnd, int step, SkPoint* startPt, SkPoint* endPt, double* endT) const {
    SkASSERT(span->fT == 0 || span->fT == 1);
    SkASSERT(span->fOtherT == 0 || span->fOtherT == 1);
    const SkOpSpan* otherSpan = &other->span(oEnd);
    double refT = otherSpan->fT;
    const SkPoint& refPt = otherSpan->fPt;
    const SkOpSpan* lastSpan = &other->span(step > 0 ? other->count() - 1 : 0);
    do {
        const SkOpSegment* match = span->fOther;
        if (match == otherSpan->fOther) {
            // find start of respective spans and see if both have winding
            int startIndex, endIndex;
            if (span->fOtherT == 1) {
                endIndex = span->fOtherIndex;
                startIndex = match->nextExactSpan(endIndex, -1);
            } else {
                startIndex = span->fOtherIndex;
                endIndex = match->nextExactSpan(startIndex, 1);
            }
            const SkOpSpan& startSpan = match->span(startIndex);
            if (startSpan.fWindValue != 0) {
                // draw ray from endSpan.fPt perpendicular to end tangent and measure distance
                // to other segment.
                const SkOpSpan& endSpan = match->span(endIndex);
                SkDLine ray;
                SkVector dxdy;
                if (span->fOtherT == 1) {
                    ray.fPts[0].set(startSpan.fPt);
                    dxdy = match->dxdy(startIndex);
                } else {
                    ray.fPts[0].set(endSpan.fPt);
                    dxdy = match->dxdy(endIndex);
                }
                ray.fPts[1].fX = ray.fPts[0].fX + dxdy.fY;
                ray.fPts[1].fY = ray.fPts[0].fY - dxdy.fX;
                SkIntersections i;
                int roots = (i.*CurveRay[SkPathOpsVerbToPoints(other->verb())])(other->pts(), ray);
                for (int index = 0; index < roots; ++index) {
                    if (ray.fPts[0].approximatelyEqual(i.pt(index))) {
                        double matchMidT = (match->span(startIndex).fT
                                + match->span(endIndex).fT) / 2;
                        SkPoint matchMidPt = match->ptAtT(matchMidT);
                        double otherMidT = (i[0][index] + other->span(oStart).fT) / 2;
                        SkPoint otherMidPt = other->ptAtT(otherMidT);
                        if (SkDPoint::ApproximatelyEqual(matchMidPt, otherMidPt)) {
                            *startPt = startSpan.fPt;
                            *endPt = endSpan.fPt;
                            *endT = endSpan.fT;
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        if (otherSpan == lastSpan) {
            break;
        }
        otherSpan += step;
    } while (otherSpan->fT == refT || otherSpan->fPt == refPt);
    return false;
}

int SkOpSegment::findEndSpan(int endIndex) const {
    const SkOpSpan* span = &fTs[--endIndex];
    const SkPoint& lastPt = span->fPt;
    double endT = span->fT;
    do {
        span = &fTs[--endIndex];
    } while (SkDPoint::ApproximatelyEqual(span->fPt, lastPt) && (span->fT == endT || span->fTiny));
    return endIndex + 1;
}

/*
 The M and S variable name parts stand for the operators.
   Mi stands for Minuend (see wiki subtraction, analogous to difference)
   Su stands for Subtrahend
 The Opp variable name part designates that the value is for the Opposite operator.
 Opposite values result from combining coincident spans.
 */
SkOpSegment* SkOpSegment::findNextOp(SkTDArray<SkOpSpan*>* chase, int* nextStart, int* nextEnd,
                                     bool* unsortable, SkPathOp op, const int xorMiMask,
                                     const int xorSuMask) {
    const int startIndex = *nextStart;
    const int endIndex = *nextEnd;
    SkASSERT(startIndex != endIndex);
    SkDEBUGCODE(const int count = fTs.count());
    SkASSERT(startIndex < endIndex ? startIndex < count - 1 : startIndex > 0);
    const int step = SkSign32(endIndex - startIndex);
    const int end = nextExactSpan(startIndex, step);
    SkASSERT(end >= 0);
    SkOpSpan* endSpan = &fTs[end];
    SkOpSegment* other;
    if (isSimple(end)) {
    // mark the smaller of startIndex, endIndex done, and all adjacent
    // spans with the same T value (but not 'other' spans)
#if DEBUG_WINDING
        SkDebugf("%s simple\n", __FUNCTION__);
#endif
        int min = SkMin32(startIndex, endIndex);
        if (fTs[min].fDone) {
            return NULL;
        }
        markDoneBinary(min);
        other = endSpan->fOther;
        *nextStart = endSpan->fOtherIndex;
        double startT = other->fTs[*nextStart].fT;
        *nextEnd = *nextStart;
        do {
            *nextEnd += step;
        } while (precisely_zero(startT - other->fTs[*nextEnd].fT));
        SkASSERT(step < 0 ? *nextEnd >= 0 : *nextEnd < other->fTs.count());
        if (other->isTiny(SkMin32(*nextStart, *nextEnd))) {
            *unsortable = true;
            return NULL;
        }
        return other;
    }
    SkASSERT(startIndex - endIndex != 0);
    SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
    // more than one viable candidate -- measure angles to find best

    int calcWinding = computeSum(startIndex, end, SkOpAngle::kBinaryOpp);
    bool sortable = calcWinding != SK_NaN32;
    if (!sortable) {
        *unsortable = true;
        markDoneBinary(SkMin32(startIndex, endIndex));
        return NULL;
    }
    SkOpAngle* angle = spanToAngle(end, startIndex);
    if (angle->unorderable()) {
        *unsortable = true;
        markDoneBinary(SkMin32(startIndex, endIndex));
        return NULL;
    }
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    angle->debugLoop();
#endif
    int sumMiWinding = updateWinding(endIndex, startIndex);
    if (sumMiWinding == SK_MinS32) {
        *unsortable = true;
        markDoneBinary(SkMin32(startIndex, endIndex));
        return NULL;
    }
    int sumSuWinding = updateOppWinding(endIndex, startIndex);
    if (operand()) {
        SkTSwap<int>(sumMiWinding, sumSuWinding);
    }
    SkOpAngle* nextAngle = angle->next();
    const SkOpAngle* foundAngle = NULL;
    bool foundDone = false;
    // iterate through the angle, and compute everyone's winding
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        nextSegment = nextAngle->segment();
        bool activeAngle = nextSegment->activeOp(xorMiMask, xorSuMask, nextAngle->start(),
                nextAngle->end(), op, &sumMiWinding, &sumSuWinding);
        if (activeAngle) {
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                if (nextSegment->isTiny(nextAngle)) {
                    *unsortable = true;
                    markDoneBinary(SkMin32(startIndex, endIndex));
                    return NULL;
                }
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle);
            }
        }
        if (nextSegment->done()) {
            continue;
        }
        if (nextSegment->isTiny(nextAngle)) {
            continue;
        }
        if (!activeAngle) {
            nextSegment->markAndChaseDoneBinary(nextAngle->start(), nextAngle->end());
        }
        SkOpSpan* last = nextAngle->lastMarked();
        if (last) {
            SkASSERT(!SkPathOpsDebug::ChaseContains(*chase, last));
            *chase->append() = last;
#if DEBUG_WINDING
            SkDebugf("%s chase.append id=%d windSum=%d small=%d\n", __FUNCTION__,
                    last->fOther->fTs[last->fOtherIndex].fOther->debugID(), last->fWindSum,
                    last->fSmall);
#endif
        }
    } while ((nextAngle = nextAngle->next()) != angle);
#if DEBUG_ANGLE
    if (foundAngle) {
        foundAngle->debugSameAs(foundAngle);
    }
#endif

    markDoneBinary(SkMin32(startIndex, endIndex));
    if (!foundAngle) {
        return NULL;
    }
    *nextStart = foundAngle->start();
    *nextEnd = foundAngle->end();
    nextSegment = foundAngle->segment();
#if DEBUG_WINDING
    SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
            __FUNCTION__, debugID(), nextSegment->debugID(), *nextStart, *nextEnd);
 #endif
    return nextSegment;
}

SkOpSegment* SkOpSegment::findNextWinding(SkTDArray<SkOpSpan*>* chase, int* nextStart,
                                          int* nextEnd, bool* unsortable) {
    const int startIndex = *nextStart;
    const int endIndex = *nextEnd;
    SkASSERT(startIndex != endIndex);
    SkDEBUGCODE(const int count = fTs.count());
    SkASSERT(startIndex < endIndex ? startIndex < count - 1 : startIndex > 0);
    const int step = SkSign32(endIndex - startIndex);
    const int end = nextExactSpan(startIndex, step);
    SkASSERT(end >= 0);
    SkOpSpan* endSpan = &fTs[end];
    SkOpSegment* other;
    if (isSimple(end)) {
    // mark the smaller of startIndex, endIndex done, and all adjacent
    // spans with the same T value (but not 'other' spans)
#if DEBUG_WINDING
        SkDebugf("%s simple\n", __FUNCTION__);
#endif
        int min = SkMin32(startIndex, endIndex);
        if (fTs[min].fDone) {
            return NULL;
        }
        markDoneUnary(min);
        other = endSpan->fOther;
        *nextStart = endSpan->fOtherIndex;
        double startT = other->fTs[*nextStart].fT;
        *nextEnd = *nextStart;
        do {
            *nextEnd += step;
        } while (precisely_zero(startT - other->fTs[*nextEnd].fT));
        SkASSERT(step < 0 ? *nextEnd >= 0 : *nextEnd < other->fTs.count());
        if (other->isTiny(SkMin32(*nextStart, *nextEnd))) {
            *unsortable = true;
            return NULL;
        }
        return other;
    }
    SkASSERT(startIndex - endIndex != 0);
    SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
    // more than one viable candidate -- measure angles to find best

    int calcWinding = computeSum(startIndex, end, SkOpAngle::kUnaryWinding);
    bool sortable = calcWinding != SK_NaN32;
    if (!sortable) {
        *unsortable = true;
        markDoneUnary(SkMin32(startIndex, endIndex));
        return NULL;
    }
    SkOpAngle* angle = spanToAngle(end, startIndex);
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    angle->debugLoop();
#endif
    int sumWinding = updateWinding(endIndex, startIndex);
    SkOpAngle* nextAngle = angle->next();
    const SkOpAngle* foundAngle = NULL;
    bool foundDone = false;
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        nextSegment = nextAngle->segment();
        bool activeAngle = nextSegment->activeWinding(nextAngle->start(), nextAngle->end(),
                &sumWinding);
        if (activeAngle) {
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                if (nextSegment->isTiny(nextAngle)) {
                    *unsortable = true;
                    markDoneUnary(SkMin32(startIndex, endIndex));
                    return NULL;
                }
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle);
            }
        }
        if (nextSegment->done()) {
            continue;
        }
        if (nextSegment->isTiny(nextAngle)) {
            continue;
        }
        if (!activeAngle) {
            nextSegment->markAndChaseDoneUnary(nextAngle->start(), nextAngle->end());
        }
        SkOpSpan* last = nextAngle->lastMarked();
        if (last) {
            SkASSERT(!SkPathOpsDebug::ChaseContains(*chase, last));
            *chase->append() = last;
#if DEBUG_WINDING
            SkDebugf("%s chase.append id=%d windSum=%d small=%d\n", __FUNCTION__,
                    last->fOther->fTs[last->fOtherIndex].fOther->debugID(), last->fWindSum,
                    last->fSmall);
#endif
        }
    } while ((nextAngle = nextAngle->next()) != angle);
    markDoneUnary(SkMin32(startIndex, endIndex));
    if (!foundAngle) {
        return NULL;
    }
    *nextStart = foundAngle->start();
    *nextEnd = foundAngle->end();
    nextSegment = foundAngle->segment();
#if DEBUG_WINDING
    SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
            __FUNCTION__, debugID(), nextSegment->debugID(), *nextStart, *nextEnd);
 #endif
    return nextSegment;
}

SkOpSegment* SkOpSegment::findNextXor(int* nextStart, int* nextEnd, bool* unsortable) {
    const int startIndex = *nextStart;
    const int endIndex = *nextEnd;
    SkASSERT(startIndex != endIndex);
    SkDEBUGCODE(int count = fTs.count());
    SkASSERT(startIndex < endIndex ? startIndex < count - 1 : startIndex > 0);
    int step = SkSign32(endIndex - startIndex);
    int end = nextExactSpan(startIndex, step);
    SkASSERT(end >= 0);
    SkOpSpan* endSpan = &fTs[end];
    SkOpSegment* other;
// Detect cases where all the ends canceled out (e.g.,
// there is no angle) and therefore there's only one valid connection
    if (isSimple(end) || !spanToAngle(end, startIndex)) {
#if DEBUG_WINDING
        SkDebugf("%s simple\n", __FUNCTION__);
#endif
        int min = SkMin32(startIndex, endIndex);
        if (fTs[min].fDone) {
            return NULL;
        }
        markDone(min, 1);
        other = endSpan->fOther;
        *nextStart = endSpan->fOtherIndex;
        double startT = other->fTs[*nextStart].fT;
        // FIXME: I don't know why the logic here is difference from the winding case
        SkDEBUGCODE(bool firstLoop = true;)
        if ((approximately_less_than_zero(startT) && step < 0)
                || (approximately_greater_than_one(startT) && step > 0)) {
            step = -step;
            SkDEBUGCODE(firstLoop = false;)
        }
        do {
            *nextEnd = *nextStart;
            do {
                *nextEnd += step;
            } while (precisely_zero(startT - other->fTs[*nextEnd].fT));
            if (other->fTs[SkMin32(*nextStart, *nextEnd)].fWindValue) {
                break;
            }
            SkASSERT(firstLoop);
            SkDEBUGCODE(firstLoop = false;)
            step = -step;
        } while (true);
        SkASSERT(step < 0 ? *nextEnd >= 0 : *nextEnd < other->fTs.count());
        return other;
    }
    SkASSERT(startIndex - endIndex != 0);
    SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
    // parallel block above with presorted version
    SkOpAngle* angle = spanToAngle(end, startIndex);
    SkASSERT(angle);
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    angle->debugLoop();
#endif
    SkOpAngle* nextAngle = angle->next();
    const SkOpAngle* foundAngle = NULL;
    bool foundDone = false;
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        nextSegment = nextAngle->segment();
        ++activeCount;
        if (!foundAngle || (foundDone && activeCount & 1)) {
            if (nextSegment->isTiny(nextAngle)) {
                *unsortable = true;
                return NULL;
            }
            foundAngle = nextAngle;
            if (!(foundDone = nextSegment->done(nextAngle))) {
                break;
            }
        }
        nextAngle = nextAngle->next();
    } while (nextAngle != angle);
    markDone(SkMin32(startIndex, endIndex), 1);
    if (!foundAngle) {
        return NULL;
    }
    *nextStart = foundAngle->start();
    *nextEnd = foundAngle->end();
    nextSegment = foundAngle->segment();
#if DEBUG_WINDING
    SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
            __FUNCTION__, debugID(), nextSegment->debugID(), *nextStart, *nextEnd);
 #endif
    return nextSegment;
}

int SkOpSegment::findStartSpan(int startIndex) const {
    int index = startIndex;
    const SkOpSpan* span = &fTs[index];
    const SkPoint& firstPt = span->fPt;
    double firstT = span->fT;
    do {
        if (index > 0) {
            if (span->fT != firstT) {
                break;
            }
            if (!SkDPoint::ApproximatelyEqual(firstPt, fTs[index].fPt)) {
                return -1;
            }
        }
        ++index;
        if (span->fTiny) {
            if (!SkDPoint::ApproximatelyEqual(firstPt, fTs[index].fPt)) {
                return -1;
            }
            firstT = fTs[index++].fT;
        }
        span = &fTs[index];
    } while (true);
    return index;
}

int SkOpSegment::findExactT(double t, const SkOpSegment* match) const {
    int count = this->count();
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        if (span.fT == t && span.fOther == match) {
            return index;
        }
    }
    SkASSERT(0);
    return -1;
}

int SkOpSegment::findT(double t, const SkPoint& pt, const SkOpSegment* match) const {
    int count = this->count();
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        if (approximately_equal_orderable(span.fT, t) && span.fOther == match) {
            return index;
        }
    }
    // Usually, the pair of ts are an exact match. It's possible that the t values have
    // been adjusted to make multiple intersections align. In this rare case, look for a
    // matching point / match pair instead.
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        if (span.fPt == pt && span.fOther == match) {
            return index;
        }
    }
    SkASSERT(0);
    return -1;
}

SkOpSegment* SkOpSegment::findTop(int* tIndexPtr, int* endIndexPtr, bool* unsortable,
        bool firstPass) {
    // iterate through T intersections and return topmost
    // topmost tangent from y-min to first pt is closer to horizontal
    SkASSERT(!done());
    int firstT = -1;
    /* SkPoint topPt = */ activeLeftTop(&firstT);
    if (firstT < 0) {
        *unsortable = !firstPass;
        firstT = 0;
        while (fTs[firstT].fDone) {
            SkASSERT(firstT < fTs.count());
            ++firstT;
        }
        *tIndexPtr = firstT;
        *endIndexPtr = nextExactSpan(firstT, 1);
        return this;
    }
    // sort the edges to find the leftmost
    int step = 1;
    int end;
    if (span(firstT).fDone || (end = nextSpan(firstT, step)) == -1) {
        step = -1;
        end = nextSpan(firstT, step);
        SkASSERT(end != -1);
    }
    // if the topmost T is not on end, or is three-way or more, find left
    // look for left-ness from tLeft to firstT (matching y of other)
    SkASSERT(firstT - end != 0);
    SkOpAngle* markAngle = spanToAngle(firstT, end);
    if (!markAngle) {
        markAngle = addSingletonAngles(firstT, step);
    }
    markAngle->markStops();
    const SkOpAngle* baseAngle = markAngle->findFirst();
    if (!baseAngle) {
        return NULL;  // nothing to do
    }
    SkScalar top = SK_ScalarMax;
    const SkOpAngle* firstAngle = NULL;
    const SkOpAngle* angle = baseAngle;
    do {
        if (!angle->unorderable()) {
            SkOpSegment* next = angle->segment();
            SkPathOpsBounds bounds;
            next->subDivideBounds(angle->end(), angle->start(), &bounds);
            if (approximately_greater(top, bounds.fTop)) {
                top = bounds.fTop;
                firstAngle = angle;
            }
        }
        angle = angle->next();
    } while (angle != baseAngle);
    SkASSERT(firstAngle);
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    firstAngle->debugLoop();
#endif
    // skip edges that have already been processed
    angle = firstAngle;
    SkOpSegment* leftSegment = NULL;
    bool looped = false;
    do {
        *unsortable = angle->unorderable();
        if (firstPass || !*unsortable) {
            leftSegment = angle->segment();
            *tIndexPtr = angle->end();
            *endIndexPtr = angle->start();
            if (!leftSegment->fTs[SkMin32(*tIndexPtr, *endIndexPtr)].fDone) {
                break;
            }
        }
        angle = angle->next();
        looped = true;
    } while (angle != firstAngle);
    if (angle == firstAngle && looped) {
        return NULL;
    }
    if (leftSegment->verb() >= SkPath::kQuad_Verb) {
        const int tIndex = *tIndexPtr;
        const int endIndex = *endIndexPtr;
        if (!leftSegment->clockwise(tIndex, endIndex)) {
            bool swap = !leftSegment->monotonicInY(tIndex, endIndex)
                    && !leftSegment->serpentine(tIndex, endIndex);
    #if DEBUG_SWAP_TOP
            SkDebugf("%s swap=%d inflections=%d serpentine=%d controlledbyends=%d monotonic=%d\n",
                    __FUNCTION__,
                    swap, leftSegment->debugInflections(tIndex, endIndex),
                    leftSegment->serpentine(tIndex, endIndex),
                    leftSegment->controlsContainedByEnds(tIndex, endIndex),
                    leftSegment->monotonicInY(tIndex, endIndex));
    #endif
            if (swap) {
    // FIXME: I doubt it makes sense to (necessarily) swap if the edge was not the first
    // sorted but merely the first not already processed (i.e., not done)
                SkTSwap(*tIndexPtr, *endIndexPtr);
            }
        }
    }
    SkASSERT(!leftSegment->fTs[SkMin32(*tIndexPtr, *endIndexPtr)].fTiny);
    return leftSegment;
}

int SkOpSegment::firstActive(int tIndex) const {
    while (fTs[tIndex].fTiny) {
        SkASSERT(!isCanceled(tIndex));
        ++tIndex;
    }
    return tIndex;
}

// FIXME: not crazy about this
// when the intersections are performed, the other index is into an
// incomplete array. As the array grows, the indices become incorrect
// while the following fixes the indices up again, it isn't smart about
// skipping segments whose indices are already correct
// assuming we leave the code that wrote the index in the first place
// FIXME: if called after remove, this needs to correct tiny
void SkOpSegment::fixOtherTIndex() {
    int iCount = fTs.count();
    for (int i = 0; i < iCount; ++i) {
        SkOpSpan& iSpan = fTs[i];
        double oT = iSpan.fOtherT;
        SkOpSegment* other = iSpan.fOther;
        int oCount = other->fTs.count();
        SkDEBUGCODE(iSpan.fOtherIndex = -1);
        for (int o = 0; o < oCount; ++o) {
            SkOpSpan& oSpan = other->fTs[o];
            if (oT == oSpan.fT && this == oSpan.fOther && oSpan.fOtherT == iSpan.fT) {
                iSpan.fOtherIndex = o;
                oSpan.fOtherIndex = i;
                break;
            }
        }
        SkASSERT(iSpan.fOtherIndex >= 0);
    }
}

void SkOpSegment::init(const SkPoint pts[], SkPath::Verb verb, bool operand, bool evenOdd) {
    fDoneSpans = 0;
    fOperand = operand;
    fXor = evenOdd;
    fPts = pts;
    fVerb = verb;
    fLoop = fSmall = fTiny = false;
}

void SkOpSegment::initWinding(int start, int end, SkOpAngle::IncludeType angleIncludeType) {
    int local = spanSign(start, end);
    if (angleIncludeType == SkOpAngle::kBinarySingle) {
        int oppLocal = oppSign(start, end);
        (void) markAndChaseWinding(start, end, local, oppLocal);
    // OPTIMIZATION: the reverse mark and chase could skip the first marking
        (void) markAndChaseWinding(end, start, local, oppLocal);
    } else {
        (void) markAndChaseWinding(start, end, local);
    // OPTIMIZATION: the reverse mark and chase could skip the first marking
        (void) markAndChaseWinding(end, start, local);
    }
}

/*
when we start with a vertical intersect, we try to use the dx to determine if the edge is to
the left or the right of vertical. This determines if we need to add the span's
sign or not. However, this isn't enough.
If the supplied sign (winding) is zero, then we didn't hit another vertical span, so dx is needed.
If there was a winding, then it may or may not need adjusting. If the span the winding was borrowed
from has the same x direction as this span, the winding should change. If the dx is opposite, then
the same winding is shared by both.
*/
void SkOpSegment::initWinding(int start, int end, double tHit, int winding, SkScalar hitDx,
                              int oppWind, SkScalar hitOppDx) {
    SkASSERT(hitDx || !winding);
    SkScalar dx = (*CurveSlopeAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, tHit).fX;
    SkASSERT(dx);
    int windVal = windValue(SkMin32(start, end));
#if DEBUG_WINDING_AT_T
    SkDebugf("%s oldWinding=%d hitDx=%c dx=%c windVal=%d", __FUNCTION__, winding,
            hitDx ? hitDx > 0 ? '+' : '-' : '0', dx > 0 ? '+' : '-', windVal);
#endif
    int sideWind = winding + (dx < 0 ? windVal : -windVal);
    if (abs(winding) < abs(sideWind)) {
        winding = sideWind;
    }
#if DEBUG_WINDING_AT_T
    SkDebugf(" winding=%d\n", winding);
#endif
    SkDEBUGCODE(int oppLocal = oppSign(start, end));
    SkASSERT(hitOppDx || !oppWind || !oppLocal);
    int oppWindVal = oppValue(SkMin32(start, end));
    if (!oppWind) {
        oppWind = dx < 0 ? oppWindVal : -oppWindVal;
    } else if (hitOppDx * dx >= 0) {
        int oppSideWind = oppWind + (dx < 0 ? oppWindVal : -oppWindVal);
        if (abs(oppWind) < abs(oppSideWind)) {
            oppWind = oppSideWind;
        }
    }
    (void) markAndChaseWinding(start, end, winding, oppWind);
    // OPTIMIZATION: the reverse mark and chase could skip the first marking
    (void) markAndChaseWinding(end, start, winding, oppWind);
}

bool SkOpSegment::inLoop(const SkOpAngle* baseAngle, int spanCount, int* indexPtr) const {
    if (!baseAngle->inLoop()) {
        return false;
    }
    int index = *indexPtr;
    int froIndex = fTs[index].fFromAngleIndex;
    int toIndex = fTs[index].fToAngleIndex;
    while (++index < spanCount) {
        int nextFro = fTs[index].fFromAngleIndex;
        int nextTo = fTs[index].fToAngleIndex;
        if (froIndex != nextFro || toIndex != nextTo) {
            break;
        }
    }
    *indexPtr = index;
    return true;
}

// OPTIMIZE: successive calls could start were the last leaves off
// or calls could specialize to walk forwards or backwards
bool SkOpSegment::isMissing(double startT, const SkPoint& pt) const {
    int tCount = fTs.count();
    for (int index = 0; index < tCount; ++index) {
        const SkOpSpan& span = fTs[index];
        if (approximately_zero(startT - span.fT) && pt == span.fPt) {
            return false;
        }
    }
    return true;
}

bool SkOpSegment::isSimple(int end) const {
#if 1
    int count = fTs.count();
    if (count == 2) {
        return true;
    }
    double t = fTs[end].fT;
    if (approximately_less_than_zero(t)) {
        return !approximately_less_than_zero(fTs[1].fT);
    }
    if (approximately_greater_than_one(t)) {
        return !approximately_greater_than_one(fTs[count - 2].fT);
    }
    return false;
#else
    // OPTIMIZE: code could be rejiggered to use this simpler test. To make this work, a little
    // more logic is required to ignore the dangling canceled out spans
    const SkOpSpan& span = fTs[end];
    return span.fFromAngleIndex < 0 && span.fToAngleIndex < 0;
#endif
}

bool SkOpSegment::isTiny(const SkOpAngle* angle) const {
    int start = angle->start();
    int end = angle->end();
    const SkOpSpan& mSpan = fTs[SkMin32(start, end)];
    return mSpan.fTiny;
}

bool SkOpSegment::isTiny(int index) const {
    return fTs[index].fTiny;
}

// look pair of active edges going away from coincident edge
// one of them should be the continuation of other
// if both are active, look to see if they both the connect to another coincident pair
// if at least one is a line, then make the pair coincident
// if neither is a line, test for coincidence
bool SkOpSegment::joinCoincidence(SkOpSegment* other, double otherT, const SkPoint& otherPt,
        int step, bool cancel) {
    int otherTIndex = other->findT(otherT, otherPt, this);
    int next = other->nextExactSpan(otherTIndex, step);
    int otherMin = SkMin32(otherTIndex, next);
    int otherWind = other->span(otherMin).fWindValue;
    if (otherWind == 0) {
        return false;
    }
    SkASSERT(next >= 0);
    int tIndex = 0;
    do {
        SkOpSpan* test = &fTs[tIndex];
        SkASSERT(test->fT == 0);
        if (test->fOther == other || test->fOtherT != 1) {
            continue;
        }
        SkPoint startPt, endPt;
        double endT;
        if (findCoincidentMatch(test, other, otherTIndex, next, step, &startPt, &endPt, &endT)) {
            SkOpSegment* match = test->fOther;
            if (cancel) {
                match->addTCancel(startPt, endPt, other);
            } else {
                match->addTCoincident(startPt, endPt, endT, other);
            }
            return true;
        }
    } while (fTs[++tIndex].fT == 0);
    return false;
}

// this span is excluded by the winding rule -- chase the ends
// as long as they are unambiguous to mark connections as done
// and give them the same winding value

SkOpSpan* SkOpSegment::markAndChaseDoneBinary(int index, int endIndex) {
    int step = SkSign32(endIndex - index);
    int min = SkMin32(index, endIndex);
    markDoneBinary(min);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        if (other->done()) {
            return NULL;
        }
        other->markDoneBinary(min);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAndChaseDoneUnary(int index, int endIndex) {
    int step = SkSign32(endIndex - index);
    int min = SkMin32(index, endIndex);
    markDoneUnary(min);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        if (other->done()) {
            return NULL;
        }
        other->markDoneUnary(min);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAndChaseWinding(const SkOpAngle* angle, int winding) {
    int index = angle->start();
    int endIndex = angle->end();
    int step = SkSign32(endIndex - index);
    int min = SkMin32(index, endIndex);
    markWinding(min, winding);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        if (other->fTs[min].fWindSum != SK_MinS32) {
            SkASSERT(other->fTs[min].fWindSum == winding);
            return NULL;
        }
        other->markWinding(min, winding);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAndChaseWinding(int index, int endIndex, int winding) {
    int min = SkMin32(index, endIndex);
    int step = SkSign32(endIndex - index);
    markWinding(min, winding);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        if (other->fTs[min].fWindSum != SK_MinS32) {
            SkASSERT(other->fTs[min].fWindSum == winding || other->fTs[min].fLoop);
            return NULL;
        }
        other->markWinding(min, winding);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAndChaseWinding(int index, int endIndex, int winding, int oppWinding) {
    int min = SkMin32(index, endIndex);
    int step = SkSign32(endIndex - index);
    markWinding(min, winding, oppWinding);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        if (other->fTs[min].fWindSum != SK_MinS32) {
            SkASSERT(other->fTs[min].fWindSum == winding || other->fTs[min].fLoop);
            return NULL;
        }
        other->markWinding(min, winding, oppWinding);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAndChaseWinding(const SkOpAngle* angle, int winding, int oppWinding) {
    int start = angle->start();
    int end = angle->end();
    return markAndChaseWinding(start, end, winding, oppWinding);
}

SkOpSpan* SkOpSegment::markAngle(int maxWinding, int sumWinding, const SkOpAngle* angle) {
    SkASSERT(angle->segment() == this);
    if (UseInnerWinding(maxWinding, sumWinding)) {
        maxWinding = sumWinding;
    }
    SkOpSpan* last = markAndChaseWinding(angle, maxWinding);
#if DEBUG_WINDING
    if (last) {
        SkDebugf("%s last id=%d windSum=", __FUNCTION__,
                last->fOther->fTs[last->fOtherIndex].fOther->debugID());
        SkPathOpsDebug::WindingPrintf(last->fWindSum);
        SkDebugf(" small=%d\n", last->fSmall);
    }
#endif
    return last;
}

SkOpSpan* SkOpSegment::markAngle(int maxWinding, int sumWinding, int oppMaxWinding,
                                 int oppSumWinding, const SkOpAngle* angle) {
    SkASSERT(angle->segment() == this);
    if (UseInnerWinding(maxWinding, sumWinding)) {
        maxWinding = sumWinding;
    }
    if (oppMaxWinding != oppSumWinding && UseInnerWinding(oppMaxWinding, oppSumWinding)) {
        oppMaxWinding = oppSumWinding;
    }
    SkOpSpan* last = markAndChaseWinding(angle, maxWinding, oppMaxWinding);
#if DEBUG_WINDING
    if (last) {
        SkDebugf("%s last id=%d windSum=", __FUNCTION__,
                last->fOther->fTs[last->fOtherIndex].fOther->debugID());
        SkPathOpsDebug::WindingPrintf(last->fWindSum);
        SkDebugf(" small=%d\n", last->fSmall);
    }
#endif
    return last;
}

// FIXME: this should also mark spans with equal (x,y)
// This may be called when the segment is already marked done. While this
// wastes time, it shouldn't do any more than spin through the T spans.
// OPTIMIZATION: abort on first done found (assuming that this code is
// always called to mark segments done).
void SkOpSegment::markDone(int index, int winding) {
  //  SkASSERT(!done());
    SkASSERT(winding);
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
        markOneDone(__FUNCTION__, lesser, winding);
    }
    do {
        markOneDone(__FUNCTION__, index, winding);
    } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    debugValidate();
}

void SkOpSegment::markDoneBinary(int index) {
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
        markOneDoneBinary(__FUNCTION__, lesser);
    }
    do {
        markOneDoneBinary(__FUNCTION__, index);
    } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    debugValidate();
}

void SkOpSegment::markDoneUnary(int index) {
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
        markOneDoneUnary(__FUNCTION__, lesser);
    }
    do {
        markOneDoneUnary(__FUNCTION__, index);
    } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    debugValidate();
}

void SkOpSegment::markOneDone(const char* funName, int tIndex, int winding) {
    SkOpSpan* span = markOneWinding(funName, tIndex, winding);
    if (!span || span->fDone) {
        return;
    }
    span->fDone = true;
    fDoneSpans++;
}

void SkOpSegment::markOneDoneBinary(const char* funName, int tIndex) {
    SkOpSpan* span = verifyOneWinding(funName, tIndex);
    if (!span) {
        return;
    }
    SkASSERT(!span->fDone);
    span->fDone = true;
    fDoneSpans++;
}

void SkOpSegment::markOneDoneUnary(const char* funName, int tIndex) {
    SkOpSpan* span = verifyOneWindingU(funName, tIndex);
    if (!span) {
        return;
    }
    if (span->fWindSum == SK_MinS32) {
        SkDebugf("%s uncomputed\n", __FUNCTION__);
    }
    SkASSERT(!span->fDone);
    span->fDone = true;
    fDoneSpans++;
}

SkOpSpan* SkOpSegment::markOneWinding(const char* funName, int tIndex, int winding) {
    SkOpSpan& span = fTs[tIndex];
    if (span.fDone && !span.fSmall) {
        return NULL;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(funName, span, winding);
#endif
    SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
#if DEBUG_LIMIT_WIND_SUM
    SkASSERT(abs(winding) <= DEBUG_LIMIT_WIND_SUM);
#endif
    span.fWindSum = winding;
    return &span;
}

SkOpSpan* SkOpSegment::markOneWinding(const char* funName, int tIndex, int winding,
                                      int oppWinding) {
    SkOpSpan& span = fTs[tIndex];
    if (span.fDone && !span.fSmall) {
        return NULL;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(funName, span, winding, oppWinding);
#endif
    SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
#if DEBUG_LIMIT_WIND_SUM
    SkASSERT(abs(winding) <= DEBUG_LIMIT_WIND_SUM);
#endif
    span.fWindSum = winding;
    SkASSERT(span.fOppSum == SK_MinS32 || span.fOppSum == oppWinding);
#if DEBUG_LIMIT_WIND_SUM
    SkASSERT(abs(oppWinding) <= DEBUG_LIMIT_WIND_SUM);
#endif
    span.fOppSum = oppWinding;
    debugValidate();
    return &span;
}

// from http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
bool SkOpSegment::clockwise(int tStart, int tEnd) const {
    SkASSERT(fVerb != SkPath::kLine_Verb);
    SkPoint edge[4];
    subDivide(tStart, tEnd, edge);
    int points = SkPathOpsVerbToPoints(fVerb);
    double sum = (edge[0].fX - edge[points].fX) * (edge[0].fY + edge[points].fY);
    if (fVerb == SkPath::kCubic_Verb) {
        SkScalar lesser = SkTMin<SkScalar>(edge[0].fY, edge[3].fY);
        if (edge[1].fY < lesser && edge[2].fY < lesser) {
            SkDLine tangent1 = {{ {edge[0].fX, edge[0].fY}, {edge[1].fX, edge[1].fY} }};
            SkDLine tangent2 = {{ {edge[2].fX, edge[2].fY}, {edge[3].fX, edge[3].fY} }};
            if (SkIntersections::Test(tangent1, tangent2)) {
                SkPoint topPt = cubic_top(fPts, fTs[tStart].fT, fTs[tEnd].fT);
                sum += (topPt.fX - edge[0].fX) * (topPt.fY + edge[0].fY);
                sum += (edge[3].fX - topPt.fX) * (edge[3].fY + topPt.fY);
                return sum <= 0;
            }
        }
    }
    for (int idx = 0; idx < points; ++idx){
        sum += (edge[idx + 1].fX - edge[idx].fX) * (edge[idx + 1].fY + edge[idx].fY);
    }
    return sum <= 0;
}

bool SkOpSegment::monotonicInY(int tStart, int tEnd) const {
    SkASSERT(fVerb != SkPath::kLine_Verb);
    if (fVerb == SkPath::kQuad_Verb) {
        SkDQuad dst = SkDQuad::SubDivide(fPts, fTs[tStart].fT, fTs[tEnd].fT);
        return dst.monotonicInY();
    }
    SkASSERT(fVerb == SkPath::kCubic_Verb);
    SkDCubic dst = SkDCubic::SubDivide(fPts, fTs[tStart].fT, fTs[tEnd].fT);
    return dst.monotonicInY();
}

bool SkOpSegment::serpentine(int tStart, int tEnd) const {
    if (fVerb != SkPath::kCubic_Verb) {
        return false;
    }
    SkDCubic dst = SkDCubic::SubDivide(fPts, fTs[tStart].fT, fTs[tEnd].fT);
    return dst.serpentine();
}

SkOpSpan* SkOpSegment::verifyOneWinding(const char* funName, int tIndex) {
    SkOpSpan& span = fTs[tIndex];
    if (span.fDone) {
        return NULL;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(funName, span, span.fWindSum, span.fOppSum);
#endif
// If the prior angle in the sort is unorderable, the winding sum may not be computable.
// To enable the assert, the 'prior is unorderable' state could be
// piped down to this test, but not sure it's worth it.
// (Once the sort order is stored in the span, this test may be feasible.)
//    SkASSERT(span.fWindSum != SK_MinS32);
//    SkASSERT(span.fOppSum != SK_MinS32);
    return &span;
}

SkOpSpan* SkOpSegment::verifyOneWindingU(const char* funName, int tIndex) {
    SkOpSpan& span = fTs[tIndex];
    if (span.fDone) {
        return NULL;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(funName, span, span.fWindSum);
#endif
// If the prior angle in the sort is unorderable, the winding sum may not be computable.
// To enable the assert, the 'prior is unorderable' state could be
// piped down to this test, but not sure it's worth it.
// (Once the sort order is stored in the span, this test may be feasible.)
//    SkASSERT(span.fWindSum != SK_MinS32);
    return &span;
}

void SkOpSegment::markWinding(int index, int winding) {
//    SkASSERT(!done());
    SkASSERT(winding);
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
        markOneWinding(__FUNCTION__, lesser, winding);
    }
    do {
        markOneWinding(__FUNCTION__, index, winding);
   } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    debugValidate();
}

void SkOpSegment::markWinding(int index, int winding, int oppWinding) {
//    SkASSERT(!done());
    SkASSERT(winding || oppWinding);
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
        markOneWinding(__FUNCTION__, lesser, winding, oppWinding);
    }
    do {
        markOneWinding(__FUNCTION__, index, winding, oppWinding);
   } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    debugValidate();
}

void SkOpSegment::matchWindingValue(int tIndex, double t, bool borrowWind) {
    int nextDoorWind = SK_MaxS32;
    int nextOppWind = SK_MaxS32;
    // prefer exact matches
    if (tIndex > 0) {
        const SkOpSpan& below = fTs[tIndex - 1];
        if (below.fT == t) {
            nextDoorWind = below.fWindValue;
            nextOppWind = below.fOppValue;
        }
    }
    if (nextDoorWind == SK_MaxS32 && tIndex + 1 < fTs.count()) {
        const SkOpSpan& above = fTs[tIndex + 1];
        if (above.fT == t) {
            nextDoorWind = above.fWindValue;
            nextOppWind = above.fOppValue;
        }
    }
    if (nextDoorWind == SK_MaxS32 && tIndex > 0) {
        const SkOpSpan& below = fTs[tIndex - 1];
        if (approximately_negative(t - below.fT)) {
            nextDoorWind = below.fWindValue;
            nextOppWind = below.fOppValue;
        }
    }
    if (nextDoorWind == SK_MaxS32 && tIndex + 1 < fTs.count()) {
        const SkOpSpan& above = fTs[tIndex + 1];
        if (approximately_negative(above.fT - t)) {
            nextDoorWind = above.fWindValue;
            nextOppWind = above.fOppValue;
        }
    }
    if (nextDoorWind == SK_MaxS32 && borrowWind && tIndex > 0 && t < 1) {
        const SkOpSpan& below = fTs[tIndex - 1];
        nextDoorWind = below.fWindValue;
        nextOppWind = below.fOppValue;
    }
    if (nextDoorWind != SK_MaxS32) {
        SkOpSpan& newSpan = fTs[tIndex];
        newSpan.fWindValue = nextDoorWind;
        newSpan.fOppValue = nextOppWind;
        if (!nextDoorWind && !nextOppWind && !newSpan.fDone) {
            newSpan.fDone = true;
            ++fDoneSpans;
        }
    }
}

// return span if when chasing, two or more radiating spans are not done
// OPTIMIZATION: ? multiple spans is detected when there is only one valid
// candidate and the remaining spans have windValue == 0 (canceled by
// coincidence). The coincident edges could either be removed altogether,
// or this code could be more complicated in detecting this case. Worth it?
bool SkOpSegment::multipleSpans(int end) const {
    return end > 0 && end < fTs.count() - 1;
}

bool SkOpSegment::nextCandidate(int* start, int* end) const {
    while (fTs[*end].fDone) {
        if (fTs[*end].fT == 1) {
            return false;
        }
        ++(*end);
    }
    *start = *end;
    *end = nextExactSpan(*start, 1);
    return true;
}

SkOpSegment* SkOpSegment::nextChase(int* index, const int step, int* min, SkOpSpan** last) {
    int end = nextExactSpan(*index, step);
    SkASSERT(end >= 0);
    if (fTs[end].fSmall) {
        *last = NULL;
        return NULL;
    }
    if (multipleSpans(end)) {
        *last = &fTs[end];
        return NULL;
    }
    const SkOpSpan& endSpan = fTs[end];
    SkOpSegment* other = endSpan.fOther;
    *index = endSpan.fOtherIndex;
    SkASSERT(*index >= 0);
    int otherEnd = other->nextExactSpan(*index, step);
    SkASSERT(otherEnd >= 0);
    *min = SkMin32(*index, otherEnd);
    if (other->fTs[*min].fSmall) {
        *last = NULL;
        return NULL;
    }
    return other;
}

// This has callers for two different situations: one establishes the end
// of the current span, and one establishes the beginning of the next span
// (thus the name). When this is looking for the end of the current span,
// coincidence is found when the beginning Ts contain -step and the end
// contains step. When it is looking for the beginning of the next, the
// first Ts found can be ignored and the last Ts should contain -step.
// OPTIMIZATION: probably should split into two functions
int SkOpSegment::nextSpan(int from, int step) const {
    const SkOpSpan& fromSpan = fTs[from];
    int count = fTs.count();
    int to = from;
    while (step > 0 ? ++to < count : --to >= 0) {
        const SkOpSpan& span = fTs[to];
        if (approximately_zero(span.fT - fromSpan.fT)) {
            continue;
        }
        return to;
    }
    return -1;
}

// FIXME
// this returns at any difference in T, vs. a preset minimum. It may be
// that all callers to nextSpan should use this instead.
int SkOpSegment::nextExactSpan(int from, int step) const {
    int to = from;
    if (step < 0) {
        const SkOpSpan& fromSpan = fTs[from];
        while (--to >= 0) {
            const SkOpSpan& span = fTs[to];
            if (precisely_negative(fromSpan.fT - span.fT) || span.fTiny) {
                continue;
            }
            return to;
        }
    } else {
        while (fTs[from].fTiny) {
            from++;
        }
        const SkOpSpan& fromSpan = fTs[from];
        int count = fTs.count();
        while (++to < count) {
            const SkOpSpan& span = fTs[to];
            if (precisely_negative(span.fT - fromSpan.fT)) {
                continue;
            }
            return to;
        }
    }
    return -1;
}

void SkOpSegment::setUpWindings(int index, int endIndex, int* sumMiWinding, int* sumSuWinding,
        int* maxWinding, int* sumWinding, int* oppMaxWinding, int* oppSumWinding) {
    int deltaSum = spanSign(index, endIndex);
    int oppDeltaSum = oppSign(index, endIndex);
    if (operand()) {
        *maxWinding = *sumSuWinding;
        *sumWinding = *sumSuWinding -= deltaSum;
        *oppMaxWinding = *sumMiWinding;
        *oppSumWinding = *sumMiWinding -= oppDeltaSum;
    } else {
        *maxWinding = *sumMiWinding;
        *sumWinding = *sumMiWinding -= deltaSum;
        *oppMaxWinding = *sumSuWinding;
        *oppSumWinding = *sumSuWinding -= oppDeltaSum;
    }
#if DEBUG_LIMIT_WIND_SUM
    SkASSERT(abs(*sumWinding) <= DEBUG_LIMIT_WIND_SUM);
    SkASSERT(abs(*oppSumWinding) <= DEBUG_LIMIT_WIND_SUM);
#endif
}

void SkOpSegment::setUpWindings(int index, int endIndex, int* sumMiWinding,
        int* maxWinding, int* sumWinding) {
    int deltaSum = spanSign(index, endIndex);
    *maxWinding = *sumMiWinding;
    *sumWinding = *sumMiWinding -= deltaSum;
#if DEBUG_LIMIT_WIND_SUM
    SkASSERT(abs(*sumWinding) <= DEBUG_LIMIT_WIND_SUM);
#endif
}

void SkOpSegment::sortAngles() {
    int spanCount = fTs.count();
    if (spanCount <= 2) {
        return;
    }
    int index = 0;
    do {
        int fromIndex = fTs[index].fFromAngleIndex;
        int toIndex = fTs[index].fToAngleIndex;
        if (fromIndex < 0 && toIndex < 0) {
            index += 1;
            continue;
        }
        SkOpAngle* baseAngle = NULL;
        if (fromIndex >= 0) {
            baseAngle = &this->angle(fromIndex);
            if (inLoop(baseAngle, spanCount, &index)) {
                continue;
            }
        }
#if DEBUG_ANGLE
        bool wroteAfterHeader = false;
#endif
        if (toIndex >= 0) {
            SkOpAngle* toAngle = &this->angle(toIndex);
            if (!baseAngle) {
                baseAngle = toAngle;
                if (inLoop(baseAngle, spanCount, &index)) {
                    continue;
                }
            } else {
                SkDEBUGCODE(int newIndex = index);
                SkASSERT(!inLoop(baseAngle, spanCount, &newIndex) && newIndex == index);
#if DEBUG_ANGLE
                SkDebugf("%s [%d] tStart=%1.9g [%d]\n", __FUNCTION__, debugID(), fTs[index].fT,
                        index);
                wroteAfterHeader = true;
#endif
                baseAngle->insert(toAngle);
            }
        }
        int nextFrom, nextTo;
        int firstIndex = index;
        do {
            SkOpSpan& span = fTs[index];
            SkOpSegment* other = span.fOther;
            SkOpSpan& oSpan = other->fTs[span.fOtherIndex];
            int otherAngleIndex = oSpan.fFromAngleIndex;
            if (otherAngleIndex >= 0) {
#if DEBUG_ANGLE
                if (!wroteAfterHeader) {
                    SkDebugf("%s [%d] tStart=%1.9g [%d]\n", __FUNCTION__, debugID(), fTs[index].fT,
                            index);
                    wroteAfterHeader = true;
                }
#endif
                SkOpAngle* oAngle = &other->angle(otherAngleIndex);
                if (!oAngle->loopContains(*baseAngle)) {
                    baseAngle->insert(oAngle);
                }
            }
            otherAngleIndex = oSpan.fToAngleIndex;
            if (otherAngleIndex >= 0) {
#if DEBUG_ANGLE
                if (!wroteAfterHeader) {
                    SkDebugf("%s [%d] tStart=%1.9g [%d]\n", __FUNCTION__, debugID(), fTs[index].fT,
                            index);
                    wroteAfterHeader = true;
                }
#endif
                SkOpAngle* oAngle = &other->angle(otherAngleIndex);
                if (!oAngle->loopContains(*baseAngle)) {
                    baseAngle->insert(oAngle);
                }
            }
            if (++index == spanCount) {
                break;
            }
            nextFrom = fTs[index].fFromAngleIndex;
            nextTo = fTs[index].fToAngleIndex;
        } while (fromIndex == nextFrom && toIndex == nextTo);
        if (baseAngle && baseAngle->loopCount() == 1) {
            index = firstIndex;
            do {
                SkOpSpan& span = fTs[index];
                span.fFromAngleIndex = span.fToAngleIndex = -1;
                if (++index == spanCount) {
                    break;
                }
                nextFrom = fTs[index].fFromAngleIndex;
                nextTo = fTs[index].fToAngleIndex;
            } while (fromIndex == nextFrom && toIndex == nextTo);
            baseAngle = NULL;
        }
#if DEBUG_SORT
        SkASSERT(!baseAngle || baseAngle->loopCount() > 1);
#endif
    } while (index < spanCount);
}

// return true if midpoints were computed
bool SkOpSegment::subDivide(int start, int end, SkPoint edge[4]) const {
    SkASSERT(start != end);
    edge[0] = fTs[start].fPt;
    int points = SkPathOpsVerbToPoints(fVerb);
    edge[points] = fTs[end].fPt;
    if (fVerb == SkPath::kLine_Verb) {
        return false;
    }
    double startT = fTs[start].fT;
    double endT = fTs[end].fT;
    if ((startT == 0 || endT == 0) && (startT == 1 || endT == 1)) {
        // don't compute midpoints if we already have them
        if (fVerb == SkPath::kQuad_Verb) {
            edge[1] = fPts[1];
            return false;
        }
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        if (start < end) {
            edge[1] = fPts[1];
            edge[2] = fPts[2];
            return false;
        }
        edge[1] = fPts[2];
        edge[2] = fPts[1];
        return false;
    }
    const SkDPoint sub[2] = {{ edge[0].fX, edge[0].fY}, {edge[points].fX, edge[points].fY }};
    if (fVerb == SkPath::kQuad_Verb) {
        edge[1] = SkDQuad::SubDivide(fPts, sub[0], sub[1], startT, endT).asSkPoint();
    } else {
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        SkDPoint ctrl[2];
        SkDCubic::SubDivide(fPts, sub[0], sub[1], startT, endT, ctrl);
        edge[1] = ctrl[0].asSkPoint();
        edge[2] = ctrl[1].asSkPoint();
    }
    return true;
}

// return true if midpoints were computed
bool SkOpSegment::subDivide(int start, int end, SkDCubic* result) const {
    SkASSERT(start != end);
    (*result)[0].set(fTs[start].fPt);
    int points = SkPathOpsVerbToPoints(fVerb);
    (*result)[points].set(fTs[end].fPt);
    if (fVerb == SkPath::kLine_Verb) {
        return false;
    }
    double startT = fTs[start].fT;
    double endT = fTs[end].fT;
    if ((startT == 0 || endT == 0) && (startT == 1 || endT == 1)) {
        // don't compute midpoints if we already have them
        if (fVerb == SkPath::kQuad_Verb) {
            (*result)[1].set(fPts[1]);
            return false;
        }
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        if (start < end) {
            (*result)[1].set(fPts[1]);
            (*result)[2].set(fPts[2]);
            return false;
        }
        (*result)[1].set(fPts[2]);
        (*result)[2].set(fPts[1]);
        return false;
    }
    if (fVerb == SkPath::kQuad_Verb) {
        (*result)[1] = SkDQuad::SubDivide(fPts, (*result)[0], (*result)[2], startT, endT);
    } else {
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        SkDCubic::SubDivide(fPts, (*result)[0], (*result)[3], startT, endT, &(*result)[1]);
    }
    return true;
}

void SkOpSegment::subDivideBounds(int start, int end, SkPathOpsBounds* bounds) const {
    SkPoint edge[4];
    subDivide(start, end, edge);
    (bounds->*SetCurveBounds[SkPathOpsVerbToPoints(fVerb)])(edge);
}

void SkOpSegment::TrackOutsidePair(SkTArray<SkPoint, true>* outsidePts, const SkPoint& endPt,
        const SkPoint& startPt) {
    int outCount = outsidePts->count();
    if (outCount == 0 || endPt != (*outsidePts)[outCount - 2]) {
        outsidePts->push_back(endPt);
        outsidePts->push_back(startPt);
    }
}

void SkOpSegment::TrackOutside(SkTArray<SkPoint, true>* outsidePts, const SkPoint& startPt) {
    int outCount = outsidePts->count();
    if (outCount == 0 || startPt != (*outsidePts)[outCount - 1]) {
        outsidePts->push_back(startPt);
    }
}

void SkOpSegment::undoneSpan(int* start, int* end) {
    int tCount = fTs.count();
    int index;
    for (index = 0; index < tCount; ++index) {
        if (!fTs[index].fDone) {
            break;
        }
    }
    SkASSERT(index < tCount - 1);
    *start = index;
    double startT = fTs[index].fT;
    while (approximately_negative(fTs[++index].fT - startT))
        SkASSERT(index < tCount);
    SkASSERT(index < tCount);
    *end = index;
}

int SkOpSegment::updateOppWinding(int index, int endIndex) const {
    int lesser = SkMin32(index, endIndex);
    int oppWinding = oppSum(lesser);
    int oppSpanWinding = oppSign(index, endIndex);
    if (oppSpanWinding && UseInnerWinding(oppWinding - oppSpanWinding, oppWinding)
            && oppWinding != SK_MaxS32) {
        oppWinding -= oppSpanWinding;
    }
    return oppWinding;
}

int SkOpSegment::updateOppWinding(const SkOpAngle* angle) const {
    int startIndex = angle->start();
    int endIndex = angle->end();
    return updateOppWinding(endIndex, startIndex);
}

int SkOpSegment::updateOppWindingReverse(const SkOpAngle* angle) const {
    int startIndex = angle->start();
    int endIndex = angle->end();
    return updateOppWinding(startIndex, endIndex);
}

int SkOpSegment::updateWinding(int index, int endIndex) const {
    int lesser = SkMin32(index, endIndex);
    int winding = windSum(lesser);
    if (winding == SK_MinS32) {
        return winding;
    }
    int spanWinding = spanSign(index, endIndex);
    if (winding && UseInnerWinding(winding - spanWinding, winding)
            && winding != SK_MaxS32) {
        winding -= spanWinding;
    }
    return winding;
}

int SkOpSegment::updateWinding(const SkOpAngle* angle) const {
    int startIndex = angle->start();
    int endIndex = angle->end();
    return updateWinding(endIndex, startIndex);
}

int SkOpSegment::updateWindingReverse(int index, int endIndex) const {
    int lesser = SkMin32(index, endIndex);
    int winding = windSum(lesser);
    int spanWinding = spanSign(endIndex, index);
    if (winding && UseInnerWindingReverse(winding - spanWinding, winding)
            && winding != SK_MaxS32) {
        winding -= spanWinding;
    }
    return winding;
}

int SkOpSegment::updateWindingReverse(const SkOpAngle* angle) const {
    int startIndex = angle->start();
    int endIndex = angle->end();
    return updateWindingReverse(endIndex, startIndex);
}

// OPTIMIZATION: does the following also work, and is it any faster?
// return outerWinding * innerWinding > 0
//      || ((outerWinding + innerWinding < 0) ^ ((outerWinding - innerWinding) < 0)))
bool SkOpSegment::UseInnerWinding(int outerWinding, int innerWinding) {
    SkASSERT(outerWinding != SK_MaxS32);
    SkASSERT(innerWinding != SK_MaxS32);
    int absOut = abs(outerWinding);
    int absIn = abs(innerWinding);
    bool result = absOut == absIn ? outerWinding < 0 : absOut < absIn;
    return result;
}

bool SkOpSegment::UseInnerWindingReverse(int outerWinding, int innerWinding) {
    SkASSERT(outerWinding != SK_MaxS32);
    SkASSERT(innerWinding != SK_MaxS32);
    int absOut = abs(outerWinding);
    int absIn = abs(innerWinding);
    bool result = absOut == absIn ? true : absOut < absIn;
    return result;
}

int SkOpSegment::windingAtT(double tHit, int tIndex, bool crossOpp, SkScalar* dx) const {
    if (approximately_zero(tHit - t(tIndex))) {  // if we hit the end of a span, disregard
        return SK_MinS32;
    }
    int winding = crossOpp ? oppSum(tIndex) : windSum(tIndex);
    SkASSERT(winding != SK_MinS32);
    int windVal = crossOpp ? oppValue(tIndex) : windValue(tIndex);
#if DEBUG_WINDING_AT_T
    SkDebugf("%s oldWinding=%d windValue=%d", __FUNCTION__, winding, windVal);
#endif
    // see if a + change in T results in a +/- change in X (compute x'(T))
    *dx = (*CurveSlopeAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, tHit).fX;
    if (fVerb > SkPath::kLine_Verb && approximately_zero(*dx)) {
        *dx = fPts[2].fX - fPts[1].fX - *dx;
    }
    if (*dx == 0) {
#if DEBUG_WINDING_AT_T
        SkDebugf(" dx=0 winding=SK_MinS32\n");
#endif
        return SK_MinS32;
    }
    if (windVal < 0) {  // reverse sign if opp contour traveled in reverse
            *dx = -*dx;
    }
    if (winding * *dx > 0) {  // if same signs, result is negative
        winding += *dx > 0 ? -windVal : windVal;
    }
#if DEBUG_WINDING_AT_T
    SkDebugf(" dx=%c winding=%d\n", *dx > 0 ? '+' : '-', winding);
#endif
    return winding;
}

int SkOpSegment::windSum(const SkOpAngle* angle) const {
    int start = angle->start();
    int end = angle->end();
    int index = SkMin32(start, end);
    return windSum(index);
}

void SkOpSegment::zeroSpan(SkOpSpan* span) {
    SkASSERT(span->fWindValue > 0 || span->fOppValue != 0);
    span->fWindValue = 0;
    span->fOppValue = 0;
    if (span->fTiny || span->fSmall) {
        return;
    }
    SkASSERT(!span->fDone);
    span->fDone = true;
    ++fDoneSpans;
}
