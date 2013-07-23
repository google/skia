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
//         miTo=0            miTo=1              miTo=0             miTo=1
//    suFrom=0    1     suFrom=0     1      suFrom=0    1      suFrom=0    1
//   suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1
    {{{{F, F}, {F, F}}, {{T, F}, {T, F}}}, {{{T, T}, {F, F}}, {{F, T}, {T, F}}}},  // mi - su
    {{{{F, F}, {F, F}}, {{F, T}, {F, T}}}, {{{F, F}, {T, T}}, {{F, T}, {T, F}}}},  // mi & su
    {{{{F, T}, {T, F}}, {{T, T}, {F, F}}}, {{{T, F}, {T, F}}, {{F, F}, {F, F}}}},  // mi | su
    {{{{F, T}, {T, F}}, {{T, F}, {F, T}}}, {{{T, F}, {F, T}}, {{F, T}, {T, F}}}},  // mi ^ su
};

#undef F
#undef T

enum { kOutsideTrackedTCount = 16 }; // FIXME: determine what this should be

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

bool SkOpSegment::activeAngle(int index, int* done, SkTArray<SkOpAngle, true>* angles) {
    if (activeAngleInner(index, done, angles)) {
        return true;
    }
    int lesser = index;
    while (--lesser >= 0 && equalPoints(index, lesser)) {
        if (activeAngleOther(lesser, done, angles)) {
            return true;
        }
    }
    lesser = index;
    do {
        if (activeAngleOther(index, done, angles)) {
            return true;
        }
    } while (++index < fTs.count() && equalPoints(index, lesser));
    return false;
}

bool SkOpSegment::activeAngleOther(int index, int* done, SkTArray<SkOpAngle, true>* angles) {
    SkOpSpan* span = &fTs[index];
    SkOpSegment* other = span->fOther;
    int oIndex = span->fOtherIndex;
    return other->activeAngleInner(oIndex, done, angles);
}

bool SkOpSegment::activeAngleInner(int index, int* done, SkTArray<SkOpAngle, true>* angles) {
    int next = nextExactSpan(index, 1);
    if (next > 0) {
        SkOpSpan& upSpan = fTs[index];
        if (upSpan.fWindValue || upSpan.fOppValue) {
            addAngle(angles, index, next);
            if (upSpan.fDone || upSpan.fUnsortableEnd) {
                (*done)++;
            } else if (upSpan.fWindSum != SK_MinS32) {
                return true;
            }
        } else if (!upSpan.fDone) {
            upSpan.fDone = true;
            fDoneSpans++;
        }
    }
    int prev = nextExactSpan(index, -1);
    // edge leading into junction
    if (prev >= 0) {
        SkOpSpan& downSpan = fTs[prev];
        if (downSpan.fWindValue || downSpan.fOppValue) {
            addAngle(angles, index, prev);
            if (downSpan.fDone) {
                (*done)++;
             } else if (downSpan.fWindSum != SK_MinS32) {
                return true;
            }
        } else if (!downSpan.fDone) {
            downSpan.fDone = true;
            fDoneSpans++;
        }
    }
    return false;
}

SkPoint SkOpSegment::activeLeftTop(bool onlySortable, int* firstT) const {
    SkASSERT(!done());
    SkPoint topPt = {SK_ScalarMax, SK_ScalarMax};
    int count = fTs.count();
    // see if either end is not done since we want smaller Y of the pair
    bool lastDone = true;
    bool lastUnsortable = false;
    double lastT = -1;
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        if (onlySortable && (span.fUnsortableStart || lastUnsortable)) {
            goto next;
        }
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
        lastUnsortable = span.fUnsortableEnd;
    }
    return topPt;
}

bool SkOpSegment::activeOp(int index, int endIndex, int xorMiMask, int xorSuMask, SkPathOp op) {
    int sumMiWinding = updateWinding(endIndex, index);
    int sumSuWinding = updateOppWinding(endIndex, index);
    if (fOperand) {
        SkTSwap<int>(sumMiWinding, sumSuWinding);
    }
    int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
    return activeOp(xorMiMask, xorSuMask, index, endIndex, op, &sumMiWinding, &sumSuWinding,
            &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
}

bool SkOpSegment::activeOp(int xorMiMask, int xorSuMask, int index, int endIndex, SkPathOp op,
        int* sumMiWinding, int* sumSuWinding,
        int* maxWinding, int* sumWinding, int* oppMaxWinding, int* oppSumWinding) {
    setUpWindings(index, endIndex, sumMiWinding, sumSuWinding,
            maxWinding, sumWinding, oppMaxWinding, oppSumWinding);
    bool miFrom;
    bool miTo;
    bool suFrom;
    bool suTo;
    if (operand()) {
        miFrom = (*oppMaxWinding & xorMiMask) != 0;
        miTo = (*oppSumWinding & xorMiMask) != 0;
        suFrom = (*maxWinding & xorSuMask) != 0;
        suTo = (*sumWinding & xorSuMask) != 0;
    } else {
        miFrom = (*maxWinding & xorMiMask) != 0;
        miTo = (*sumWinding & xorMiMask) != 0;
        suFrom = (*oppMaxWinding & xorSuMask) != 0;
        suTo = (*oppSumWinding & xorSuMask) != 0;
    }
    bool result = gActiveEdge[op][miFrom][miTo][suFrom][suTo];
#if DEBUG_ACTIVE_OP
    SkDebugf("%s op=%s miFrom=%d miTo=%d suFrom=%d suTo=%d result=%d\n", __FUNCTION__,
            kPathOpStr[op], miFrom, miTo, suFrom, suTo, result);
#endif
    return result;
}

bool SkOpSegment::activeWinding(int index, int endIndex) {
    int sumWinding = updateWinding(endIndex, index);
    int maxWinding;
    return activeWinding(index, endIndex, &maxWinding, &sumWinding);
}

bool SkOpSegment::activeWinding(int index, int endIndex, int* maxWinding, int* sumWinding) {
    setUpWinding(index, endIndex, maxWinding, sumWinding);
    bool from = *maxWinding != 0;
    bool to = *sumWinding  != 0;
    bool result = gUnaryActiveEdge[from][to];
    return result;
}

void SkOpSegment::addAngle(SkTArray<SkOpAngle, true>* anglesPtr, int start, int end) const {
    SkASSERT(start != end);
    SkOpAngle& angle = anglesPtr->push_back();
#if 0 && DEBUG_ANGLE // computed pt and actual pt may differ by more than approx eq
    SkTArray<SkOpAngle, true>& angles = *anglesPtr;
    if (angles.count() > 1) {
        const SkOpSegment* aSeg = angles[0].segment();
        int aStart = angles[0].start();
        if (!aSeg->fTs[aStart].fTiny) {
            const SkPoint& angle0Pt = aSeg->xyAtT(aStart);
            SkDPoint newPt = (*CurveDPointAtT[SkPathOpsVerbToPoints(aSeg->fVerb)])(aSeg->fPts,
                    aSeg->fTs[aStart].fT);
#if ONE_OFF_DEBUG
            SkDebugf("%s t1=%1.9g (%1.9g, %1.9g) (%1.9g, %1.9g)\n", __FUNCTION__,
                    aSeg->fTs[aStart].fT, newPt.fX, newPt.fY, angle0Pt.fX, angle0Pt.fY);
#endif
            SkASSERT(newPt.approximatelyEqual(angle0Pt));
        }
    }
#endif
    angle.set(this, start, end);
}

void SkOpSegment::addCancelOutsides(double tStart, double oStart, SkOpSegment* other, double oEnd) {
    int tIndex = -1;
    int tCount = fTs.count();
    int oIndex = -1;
    int oCount = other->fTs.count();
    do {
        ++tIndex;
    } while (!approximately_negative(tStart - fTs[tIndex].fT) && tIndex < tCount);
    int tIndexStart = tIndex;
    do {
        ++oIndex;
    } while (!approximately_negative(oStart - other->fTs[oIndex].fT) && oIndex < oCount);
    int oIndexStart = oIndex;
    double nextT;
    do {
        nextT = fTs[++tIndex].fT;
    } while (nextT < 1 && approximately_negative(nextT - tStart));
    double oNextT;
    do {
        oNextT = other->fTs[++oIndex].fT;
    } while (oNextT < 1 && approximately_negative(oNextT - oStart));
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

void SkOpSegment::addCoinOutsides(const SkTArray<double, true>& outsideTs, SkOpSegment* other,
                                  double oEnd) {
    // walk this to outsideTs[0]
    // walk other to outsideTs[1]
    // if either is > 0, add a pointer to the other, copying adjacent winding
    int tIndex = -1;
    int oIndex = -1;
    double tStart = outsideTs[0];
    double oStart = outsideTs[1];
    do {
        ++tIndex;
    } while (!approximately_negative(tStart - fTs[tIndex].fT));
    SkPoint ptStart = fTs[tIndex].fPt;
    do {
        ++oIndex;
    } while (!approximately_negative(oStart - other->fTs[oIndex].fT));
    if (tIndex > 0 || oIndex > 0 || fOperand != other->fOperand) {
        addTPair(tStart, other, oStart, false, ptStart);
    }
    tStart = fTs[tIndex].fT;
    oStart = other->fTs[oIndex].fT;
    do {
        double nextT;
        do {
            nextT = fTs[++tIndex].fT;
        } while (approximately_negative(nextT - tStart));
        tStart = nextT;
        ptStart = fTs[tIndex].fPt;
        do {
            nextT = other->fTs[++oIndex].fT;
        } while (approximately_negative(nextT - oStart));
        oStart = nextT;
        if (tStart == 1 && oStart == 1 && fOperand == other->fOperand) {
            break;
        }
        addTPair(tStart, other, oStart, false, ptStart);
    } while (tStart < 1 && oStart < 1 && !approximately_negative(oEnd - oStart));
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

    // Defer all coincident edge processing until
    // after normal intersections have been computed

// no need to be tricky; insert in normal T order
// resolve overlapping ts when considering coincidence later

    // add non-coincident intersection. Resulting edges are sorted in T.
int SkOpSegment::addT(SkOpSegment* other, const SkPoint& pt, double newT) {
    if (precisely_zero(newT)) {
        newT = 0;
    } else if (precisely_equal(newT, 1)) {
        newT = 1;
    }
    // FIXME: in the pathological case where there is a ton of intercepts,
    //  binary search?
    int insertedAt = -1;
    size_t tCount = fTs.count();
    for (size_t index = 0; index < tCount; ++index) {
        // OPTIMIZATION: if there are three or more identical Ts, then
        // the fourth and following could be further insertion-sorted so
        // that all the edges are clockwise or counterclockwise.
        // This could later limit segment tests to the two adjacent
        // neighbors, although it doesn't help with determining which
        // circular direction to go in.
        if (newT < fTs[index].fT) {
            insertedAt = index;
            break;
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
    span->fWindSum = SK_MinS32;
    span->fOppSum = SK_MinS32;
    span->fWindValue = 1;
    span->fOppValue = 0;
    span->fTiny = false;
    span->fLoop = false;
    if ((span->fDone = newT == 1)) {
        ++fDoneSpans;
    }
    span->fUnsortableStart = false;
    span->fUnsortableEnd = false;
    int less = -1;
    while (&span[less + 1] - fTs.begin() > 0 && xyAtT(&span[less]) == xyAtT(span)) {
        if (span[less].fDone) {
            break;
        }
        double tInterval = newT - span[less].fT;
        if (precisely_negative(tInterval)) {
            break;
        }
        if (fVerb == SkPath::kCubic_Verb) {
            double tMid = newT - tInterval / 2;
            SkDPoint midPt = dcubic_xy_at_t(fPts, tMid);
            if (!midPt.approximatelyEqual(xyAtT(span))) {
                break;
            }
        }
        span[less].fTiny = true;
        span[less].fDone = true;
        if (approximately_negative(newT - span[less].fT)) {
            if (approximately_greater_than_one(newT)) {
                SkASSERT(&span[less] - fTs.begin() < fTs.count());
                span[less].fUnsortableStart = true;
                if (&span[less - 1] - fTs.begin() >= 0) {
                    span[less - 1].fUnsortableEnd = true;
                }
            }
            if (approximately_less_than_zero(span[less].fT)) {
                SkASSERT(&span[less + 1] - fTs.begin() < fTs.count());
                SkASSERT(&span[less] - fTs.begin() >= 0);
                span[less + 1].fUnsortableStart = true;
                span[less].fUnsortableEnd = true;
            }
        }
        ++fDoneSpans;
        --less;
    }
    int more = 1;
    while (fTs.end() - &span[more - 1] > 1 && xyAtT(&span[more]) == xyAtT(span)) {
        if (span[more - 1].fDone) {
            break;
        }
        double tEndInterval = span[more].fT - newT;
        if (precisely_negative(tEndInterval)) {
            break;
        }
        if (fVerb == SkPath::kCubic_Verb) {
            double tMid = newT - tEndInterval / 2;
            SkDPoint midEndPt = dcubic_xy_at_t(fPts, tMid);
            if (!midEndPt.approximatelyEqual(xyAtT(span))) {
                break;
            }
        }
        span[more - 1].fTiny = true;
        span[more - 1].fDone = true;
        if (approximately_negative(span[more].fT - newT)) {
            if (approximately_greater_than_one(span[more].fT)) {
                span[more + 1].fUnsortableStart = true;
                span[more].fUnsortableEnd = true;
            }
            if (approximately_less_than_zero(newT)) {
                span[more].fUnsortableStart = true;
                span[more - 1].fUnsortableEnd = true;
            }
        }
        ++fDoneSpans;
        ++more;
    }
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
void SkOpSegment::addTCancel(double startT, double endT, SkOpSegment* other,
        double oStartT, double oEndT) {
    SkASSERT(!approximately_negative(endT - startT));
    SkASSERT(!approximately_negative(oEndT - oStartT));
    bool binary = fOperand != other->fOperand;
    int index = 0;
    while (!approximately_negative(startT - fTs[index].fT)) {
        ++index;
    }
    int oIndex = other->fTs.count();
    while (approximately_positive(other->fTs[--oIndex].fT - oEndT))
        ;
    double tRatio = (oEndT - oStartT) / (endT - startT);
    SkOpSpan* test = &fTs[index];
    SkOpSpan* oTest = &other->fTs[oIndex];
    SkSTArray<kOutsideTrackedTCount, double, true> outsideTs;
    SkSTArray<kOutsideTrackedTCount, double, true> oOutsideTs;
    do {
        bool decrement = test->fWindValue && oTest->fWindValue;
        bool track = test->fWindValue || oTest->fWindValue;
        bool bigger = test->fWindValue >= oTest->fWindValue;
        double testT = test->fT;
        double oTestT = oTest->fT;
        SkOpSpan* span = test;
        do {
            if (decrement) {
                if (binary && bigger) {
                    span->fOppValue--;
                } else {
                    decrementSpan(span);
                }
            } else if (track && span->fT < 1 && oTestT < 1) {
                TrackOutside(&outsideTs, span->fT, oTestT);
            }
            span = &fTs[++index];
        } while (approximately_negative(span->fT - testT));
        SkOpSpan* oSpan = oTest;
        double otherTMatchStart = oEndT - (span->fT - startT) * tRatio;
        double otherTMatchEnd = oEndT - (test->fT - startT) * tRatio;
        SkDEBUGCODE(int originalWindValue = oSpan->fWindValue);
        while (approximately_negative(otherTMatchStart - oSpan->fT)
                && !approximately_negative(otherTMatchEnd - oSpan->fT)) {
    #ifdef SK_DEBUG
            SkASSERT(originalWindValue == oSpan->fWindValue);
    #endif
            if (decrement) {
                if (binary && !bigger) {
                    oSpan->fOppValue--;
                } else {
                    other->decrementSpan(oSpan);
                }
            } else if (track && oSpan->fT < 1 && testT < 1) {
                TrackOutside(&oOutsideTs, oSpan->fT, testT);
            }
            if (!oIndex) {
                break;
            }
            oSpan = &other->fTs[--oIndex];
        }
        test = span;
        oTest = oSpan;
    } while (!approximately_negative(endT - test->fT));
    SkASSERT(!oIndex || approximately_negative(oTest->fT - oStartT));
    // FIXME: determine if canceled edges need outside ts added
    if (!done() && outsideTs.count()) {
        double tStart = outsideTs[0];
        double oStart = outsideTs[1];
        addCancelOutsides(tStart, oStart, other, oEndT);
        int count = outsideTs.count();
        if (count > 2) {
            double tStart = outsideTs[count - 2];
            double oStart = outsideTs[count - 1];
            addCancelOutsides(tStart, oStart, other, oEndT);
        }
    }
    if (!other->done() && oOutsideTs.count()) {
        double tStart = oOutsideTs[0];
        double oStart = oOutsideTs[1];
        other->addCancelOutsides(tStart, oStart, this, endT);
    }
}

int SkOpSegment::addSelfT(SkOpSegment* other, const SkPoint& pt, double newT) {
    int result = addT(other, pt, newT);
    SkOpSpan* span = &fTs[result];
    span->fLoop = true;
    return result;
}

int SkOpSegment::addUnsortableT(SkOpSegment* other, bool start, const SkPoint& pt, double newT) {
    int result = addT(other, pt, newT);
    SkOpSpan* span = &fTs[result];
    if (start) {
        if (result > 0) {
            span[result - 1].fUnsortableEnd = true;
        }
        span[result].fUnsortableStart = true;
    } else {
        span[result].fUnsortableEnd = true;
        if (result + 1 < fTs.count()) {
            span[result + 1].fUnsortableStart = true;
        }
    }
    return result;
}

int SkOpSegment::bumpCoincidentThis(const SkOpSpan& oTest, bool opp, int index,
        SkTArray<double, true>* outsideTs) {
    int oWindValue = oTest.fWindValue;
    int oOppValue = oTest.fOppValue;
    if (opp) {
        SkTSwap<int>(oWindValue, oOppValue);
    }
    SkOpSpan* const test = &fTs[index];
    SkOpSpan* end = test;
    const double oStartT = oTest.fT;
    do {
        if (bumpSpan(end, oWindValue, oOppValue)) {
            TrackOutside(outsideTs, end->fT, oStartT);
        }
        end = &fTs[++index];
    } while (approximately_negative(end->fT - test->fT));
    return index;
}

// because of the order in which coincidences are resolved, this and other
// may not have the same intermediate points. Compute the corresponding
// intermediate T values (using this as the master, other as the follower)
// and walk other conditionally -- hoping that it catches up in the end
int SkOpSegment::bumpCoincidentOther(const SkOpSpan& test, double oEndT, int& oIndex,
        SkTArray<double, true>* oOutsideTs) {
    SkOpSpan* const oTest = &fTs[oIndex];
    SkOpSpan* oEnd = oTest;
    const double startT = test.fT;
    const double oStartT = oTest->fT;
    while (!approximately_negative(oEndT - oEnd->fT)
            && approximately_negative(oEnd->fT - oStartT)) {
        zeroSpan(oEnd);
        TrackOutside(oOutsideTs, oEnd->fT, startT);
        oEnd = &fTs[++oIndex];
    }
    return oIndex;
}

// FIXME: need to test this case:
// contourA has two segments that are coincident
// contourB has two segments that are coincident in the same place
// each ends up with +2/0 pairs for winding count
// since logic below doesn't transfer count (only increments/decrements) can this be
// resolved to +4/0 ?

// set spans from start to end to increment the greater by one and decrement
// the lesser
void SkOpSegment::addTCoincident(double startT, double endT, SkOpSegment* other, double oStartT,
                                 double oEndT) {
    SkASSERT(!approximately_negative(endT - startT));
    SkASSERT(!approximately_negative(oEndT - oStartT));
    bool opp = fOperand ^ other->fOperand;
    int index = 0;
    while (!approximately_negative(startT - fTs[index].fT)) {
        ++index;
    }
    int oIndex = 0;
    while (!approximately_negative(oStartT - other->fTs[oIndex].fT)) {
        ++oIndex;
    }
    SkOpSpan* test = &fTs[index];
    SkOpSpan* oTest = &other->fTs[oIndex];
    SkSTArray<kOutsideTrackedTCount, double, true> outsideTs;
    SkSTArray<kOutsideTrackedTCount, double, true> oOutsideTs;
    do {
        // if either span has an opposite value and the operands don't match, resolve first
 //       SkASSERT(!test->fDone || !oTest->fDone);
        if (test->fDone || oTest->fDone) {
            index = advanceCoincidentThis(oTest, opp, index);
            oIndex = other->advanceCoincidentOther(test, oEndT, oIndex);
        } else {
            index = bumpCoincidentThis(*oTest, opp, index, &outsideTs);
            oIndex = other->bumpCoincidentOther(*test, oEndT, oIndex, &oOutsideTs);
        }
        test = &fTs[index];
        oTest = &other->fTs[oIndex];
    } while (!approximately_negative(endT - test->fT));
    SkASSERT(approximately_negative(oTest->fT - oEndT));
    SkASSERT(approximately_negative(oEndT - oTest->fT));
    if (!done() && outsideTs.count()) {
        addCoinOutsides(outsideTs, other, oEndT);
    }
    if (!other->done() && oOutsideTs.count()) {
        other->addCoinOutsides(oOutsideTs, this, endT);
    }
}

// FIXME: this doesn't prevent the same span from being added twice
// fix in caller, SkASSERT here?
void SkOpSegment::addTPair(double t, SkOpSegment* other, double otherT, bool borrowWind,
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
            return;
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
}

void SkOpSegment::addTwoAngles(int start, int end, SkTArray<SkOpAngle, true>* angles) const {
    // add edge leading into junction
    int min = SkMin32(end, start);
    if (fTs[min].fWindValue > 0 || fTs[min].fOppValue != 0) {
        addAngle(angles, end, start);
    }
    // add edge leading away from junction
    int step = SkSign32(end - start);
    int tIndex = nextExactSpan(end, step);
    min = SkMin32(end, tIndex);
    if (tIndex >= 0 && (fTs[min].fWindValue > 0 || fTs[min].fOppValue != 0)) {
        addAngle(angles, end, tIndex);
    }
}

int SkOpSegment::advanceCoincidentThis(const SkOpSpan* oTest, bool opp, int index) {
    SkOpSpan* const test = &fTs[index];
    SkOpSpan* end;
    do {
        end = &fTs[++index];
    } while (approximately_negative(end->fT - test->fT));
    return index;
}

int SkOpSegment::advanceCoincidentOther(const SkOpSpan* test, double oEndT, int oIndex) {
    SkOpSpan* const oTest = &fTs[oIndex];
    SkOpSpan* oEnd = oTest;
    const double oStartT = oTest->fT;
    while (!approximately_negative(oEndT - oEnd->fT)
            && approximately_negative(oEnd->fT - oStartT)) {
        oEnd = &fTs[++oIndex];
    }
    return oIndex;
}

bool SkOpSegment::betweenTs(int lesser, double testT, int greater) const {
    if (lesser > greater) {
        SkTSwap<int>(lesser, greater);
    }
    return approximately_between(fTs[lesser].fT, testT, fTs[greater].fT);
}

void SkOpSegment::buildAngles(int index, SkTArray<SkOpAngle, true>* angles, bool includeOpp) const {
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && (includeOpp || fTs[lesser].fOther->fOperand == fOperand)
            && precisely_negative(referenceT - fTs[lesser].fT)) {
        buildAnglesInner(lesser, angles);
    }
    do {
        buildAnglesInner(index, angles);
    } while (++index < fTs.count() && (includeOpp || fTs[index].fOther->fOperand == fOperand)
            && precisely_negative(fTs[index].fT - referenceT));
}

void SkOpSegment::buildAnglesInner(int index, SkTArray<SkOpAngle, true>* angles) const {
    const SkOpSpan* span = &fTs[index];
    SkOpSegment* other = span->fOther;
// if there is only one live crossing, and no coincidence, continue
// in the same direction
// if there is coincidence, the only choice may be to reverse direction
    // find edge on either side of intersection
    int oIndex = span->fOtherIndex;
    // if done == -1, prior span has already been processed
    int step = 1;
    int next = other->nextExactSpan(oIndex, step);
   if (next < 0) {
        step = -step;
        next = other->nextExactSpan(oIndex, step);
    }
    // add candidate into and away from junction
    other->addTwoAngles(next, oIndex, angles);
}

int SkOpSegment::computeSum(int startIndex, int endIndex, bool binary) {
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
    addTwoAngles(startIndex, endIndex, &angles);
    buildAngles(endIndex, &angles, false);
    // OPTIMIZATION: check all angles to see if any have computed wind sum
    // before sorting (early exit if none)
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
    // FIXME?: Not sure if this sort must be ordered or if the relaxed ordering is OK ...
    bool sortable = SortAngles(angles, &sorted, SkOpSegment::kMustBeOrdered_SortAngleKind);
#if DEBUG_SORT
    sorted[0]->segment()->debugShowSort(__FUNCTION__, sorted, 0, 0, 0, sortable);
#endif
    if (!sortable) {
        return SK_MinS32;
    }
    int angleCount = angles.count();
    const SkOpAngle* angle;
    const SkOpSegment* base;
    int winding;
    int oWinding;
    int firstIndex = 0;
    do {
        angle = sorted[firstIndex];
        base = angle->segment();
        winding = base->windSum(angle);
        if (winding != SK_MinS32) {
            oWinding = base->oppSum(angle);
            break;
        }
        if (++firstIndex == angleCount) {
            return SK_MinS32;
        }
    } while (true);
    // turn winding into contourWinding
    int spanWinding = base->spanSign(angle);
    bool inner = UseInnerWinding(winding + spanWinding, winding);
#if DEBUG_WINDING
    SkDebugf("%s spanWinding=%d winding=%d sign=%d inner=%d result=%d\n", __FUNCTION__,
        spanWinding, winding, angle->sign(), inner,
        inner ? winding + spanWinding : winding);
#endif
    if (inner) {
        winding += spanWinding;
    }
#if DEBUG_SORT
    base->debugShowSort(__FUNCTION__, sorted, firstIndex, winding, oWinding, sortable);
#endif
    int nextIndex = firstIndex + 1;
    int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
    winding -= base->spanSign(angle);
    oWinding -= base->oppSign(angle);
    do {
        if (nextIndex == angleCount) {
            nextIndex = 0;
        }
        angle = sorted[nextIndex];
        SkOpSegment* segment = angle->segment();
        bool opp = base->fOperand ^ segment->fOperand;
        int maxWinding, oMaxWinding;
        int spanSign = segment->spanSign(angle);
        int oppoSign = segment->oppSign(angle);
        if (opp) {
            oMaxWinding = oWinding;
            oWinding -= spanSign;
            maxWinding = winding;
            if (oppoSign) {
                winding -= oppoSign;
            }
        } else {
            maxWinding = winding;
            winding -= spanSign;
            oMaxWinding = oWinding;
            if (oppoSign) {
                oWinding -= oppoSign;
            }
        }
        if (segment->windSum(angle) == SK_MinS32) {
            if (opp) {
                if (UseInnerWinding(oMaxWinding, oWinding)) {
                    oMaxWinding = oWinding;
                }
                if (oppoSign && UseInnerWinding(maxWinding, winding)) {
                    maxWinding = winding;
                }
#ifdef SK_DEBUG
                SkASSERT(abs(maxWinding) <= gDebugMaxWindSum);
                SkASSERT(abs(oMaxWinding) <= gDebugMaxWindSum);
#endif
                (void) segment->markAndChaseWinding(angle, oMaxWinding, maxWinding);
            } else {
                if (UseInnerWinding(maxWinding, winding)) {
                    maxWinding = winding;
                }
                if (oppoSign && UseInnerWinding(oMaxWinding, oWinding)) {
                    oMaxWinding = oWinding;
                }
#ifdef SK_DEBUG
                SkASSERT(abs(maxWinding) <= gDebugMaxWindSum);
                SkASSERT(abs(binary ? oMaxWinding : 0) <= gDebugMaxWindSum);
#endif
                (void) segment->markAndChaseWinding(angle, maxWinding,
                        binary ? oMaxWinding : 0);
            }
        }
    } while (++nextIndex != lastIndex);
    int minIndex = SkMin32(startIndex, endIndex);
    return windSum(minIndex);
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

void SkOpSegment::decrementSpan(SkOpSpan* span) {
    SkASSERT(span->fWindValue > 0);
    if (--(span->fWindValue) == 0) {
        if (!span->fOppValue && !span->fDone) {
            span->fDone = true;
            ++fDoneSpans;
        }
    }
}

bool SkOpSegment::bumpSpan(SkOpSpan* span, int windDelta, int oppDelta) {
    SkASSERT(!span->fDone);
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

// look to see if the curve end intersects an intermediary that intersects the other
void SkOpSegment::checkEnds() {
    debugValidate();
    SkTDArray<SkOpSpan> missingSpans;
    int count = fTs.count();
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = fTs[index];
        const SkOpSegment* other = span.fOther;
        const SkOpSpan* otherSpan = &other->fTs[span.fOtherIndex];
        double otherT = otherSpan->fT;
        if (otherT != 0 && otherT != 1) {
            continue;
        }
        int peekStart = span.fOtherIndex;
        while (peekStart > 0) {
            const SkOpSpan* peeker = &other->fTs[peekStart - 1];
            if (peeker->fT != otherT) {
                break;
            }
            --peekStart;
        }
        int otherLast = other->fTs.count() - 1;
        int peekLast = span.fOtherIndex;
        while (peekLast < otherLast) {
            const SkOpSpan* peeker = &other->fTs[peekLast + 1];
            if (peeker->fT != otherT) {
                break;
            }
            ++peekLast;
        }
        if (peekStart == peekLast) {
            continue;
        }
        double t = span.fT;
        int tStart = index;
        while (tStart > 0 && t == fTs[tStart - 1].fT) {
            --tStart;
        }
        int tLast = index;
        int last = count - 1;
        while (tLast < last && t == fTs[tLast + 1].fT) {
            ++tLast;
        }
        for (int peekIndex = peekStart; peekIndex <= peekLast; ++peekIndex) {
            if (peekIndex == span.fOtherIndex) {
                continue;
            }
            const SkOpSpan& peekSpan = other->fTs[peekIndex];
            SkOpSegment* match = peekSpan.fOther;
            const double matchT = peekSpan.fOtherT;
            for (int tIndex = tStart; tIndex <= tLast; ++tIndex) {
                const SkOpSpan& tSpan = fTs[tIndex];
                if (tSpan.fOther == match && tSpan.fOtherT == matchT) {
                    goto nextPeeker;
                }
            }
            // this segment is missing a entry that the other contains
            // remember so we can add the missing one and recompute the indices
            SkOpSpan* missing = missingSpans.append();
            missing->fT = t;
            missing->fOther = match;
            missing->fOtherT = matchT;
            missing->fPt = peekSpan.fPt;
        }
nextPeeker:
        ;
    }
    int missingCount = missingSpans.count();
    if (missingCount == 0) {
        return;
    }
    debugValidate();
    for (int index = 0; index < missingCount; ++index)  {
        const SkOpSpan& missing = missingSpans[index];
        addTPair(missing.fT, missing.fOther, missing.fOtherT, false, missing.fPt);
    }
    fixOtherTIndex();
    for (int index = 0; index < missingCount; ++index)  {
        const SkOpSpan& missing = missingSpans[index];
        missing.fOther->fixOtherTIndex();
    }
    debugValidate();
}

bool SkOpSegment::equalPoints(int greaterTIndex, int lesserTIndex) {
    SkASSERT(greaterTIndex >= lesserTIndex);
    double greaterT = fTs[greaterTIndex].fT;
    double lesserT = fTs[lesserTIndex].fT;
    if (greaterT == lesserT) {
        return true;
    }
    if (!approximately_negative(greaterT - lesserT)) {
        return false;
    }
    return xyAtT(greaterTIndex) == xyAtT(lesserTIndex);
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
        }
        while (precisely_zero(startT - other->fTs[*nextEnd].fT));
        SkASSERT(step < 0 ? *nextEnd >= 0 : *nextEnd < other->fTs.count());
        if (other->isTiny(SkMin32(*nextStart, *nextEnd))) {
            *unsortable = true;
            return NULL;
        }
        return other;
    }
    // more than one viable candidate -- measure angles to find best
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
    SkASSERT(startIndex - endIndex != 0);
    SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
    addTwoAngles(startIndex, end, &angles);
    buildAngles(end, &angles, true);
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
    bool sortable = SortAngles(angles, &sorted, SkOpSegment::kMustBeOrdered_SortAngleKind);
    int angleCount = angles.count();
    int firstIndex = findStartingEdge(sorted, startIndex, end);
    SkASSERT(firstIndex >= 0);
#if DEBUG_SORT
    debugShowSort(__FUNCTION__, sorted, firstIndex, sortable);
#endif
    if (!sortable) {
        *unsortable = true;
        return NULL;
    }
    SkASSERT(sorted[firstIndex]->segment() == this);
#if DEBUG_WINDING
    SkDebugf("%s firstIndex=[%d] sign=%d\n", __FUNCTION__, firstIndex,
            sorted[firstIndex]->sign());
#endif
    int sumMiWinding = updateWinding(endIndex, startIndex);
    int sumSuWinding = updateOppWinding(endIndex, startIndex);
    if (operand()) {
        SkTSwap<int>(sumMiWinding, sumSuWinding);
    }
    int nextIndex = firstIndex + 1;
    int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
    const SkOpAngle* foundAngle = NULL;
    bool foundDone = false;
    // iterate through the angle, and compute everyone's winding
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        SkASSERT(nextIndex != firstIndex);
        if (nextIndex == angleCount) {
            nextIndex = 0;
        }
        const SkOpAngle* nextAngle = sorted[nextIndex];
        nextSegment = nextAngle->segment();
        int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
        bool activeAngle = nextSegment->activeOp(xorMiMask, xorSuMask, nextAngle->start(),
                nextAngle->end(), op, &sumMiWinding, &sumSuWinding,
                &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
        if (activeAngle) {
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                if (nextSegment->isTiny(nextAngle)) {
                    *unsortable = true;
                    return NULL;
                }
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle) && !nextSegment->isTiny(nextAngle);
            }
        }
        if (nextSegment->done()) {
            continue;
        }
        if (nextSegment->windSum(nextAngle) != SK_MinS32) {
            continue;
        }
        SkOpSpan* last = nextSegment->markAngle(maxWinding, sumWinding, oppMaxWinding,
                oppSumWinding, activeAngle, nextAngle);
        if (last) {
            *chase->append() = last;
#if DEBUG_WINDING
            SkDebugf("%s chase.append id=%d\n", __FUNCTION__,
                    last->fOther->fTs[last->fOtherIndex].fOther->debugID());
#endif
        }
    } while (++nextIndex != lastIndex);
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
        }
        while (precisely_zero(startT - other->fTs[*nextEnd].fT));
        SkASSERT(step < 0 ? *nextEnd >= 0 : *nextEnd < other->fTs.count());
        return other;
    }
    // more than one viable candidate -- measure angles to find best
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
    SkASSERT(startIndex - endIndex != 0);
    SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
    addTwoAngles(startIndex, end, &angles);
    buildAngles(end, &angles, true);
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
    bool sortable = SortAngles(angles, &sorted, SkOpSegment::kMustBeOrdered_SortAngleKind);
    int angleCount = angles.count();
    int firstIndex = findStartingEdge(sorted, startIndex, end);
    SkASSERT(firstIndex >= 0);
#if DEBUG_SORT
    debugShowSort(__FUNCTION__, sorted, firstIndex, sortable);
#endif
    if (!sortable) {
        *unsortable = true;
        return NULL;
    }
    SkASSERT(sorted[firstIndex]->segment() == this);
#if DEBUG_WINDING
    SkDebugf("%s firstIndex=[%d] sign=%d\n", __FUNCTION__, firstIndex,
            sorted[firstIndex]->sign());
#endif
    int sumWinding = updateWinding(endIndex, startIndex);
    int nextIndex = firstIndex + 1;
    int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
    const SkOpAngle* foundAngle = NULL;
    bool foundDone = false;
    // iterate through the angle, and compute everyone's winding
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        SkASSERT(nextIndex != firstIndex);
        if (nextIndex == angleCount) {
            nextIndex = 0;
        }
        const SkOpAngle* nextAngle = sorted[nextIndex];
        nextSegment = nextAngle->segment();
        int maxWinding;
        bool activeAngle = nextSegment->activeWinding(nextAngle->start(), nextAngle->end(),
                &maxWinding, &sumWinding);
        if (activeAngle) {
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                if (nextSegment->isTiny(nextAngle)) {
                    *unsortable = true;
                    return NULL;
                }
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle);
            }
        }
        if (nextSegment->done()) {
            continue;
        }
        if (nextSegment->windSum(nextAngle) != SK_MinS32) {
            continue;
        }
        SkOpSpan* last = nextSegment->markAngle(maxWinding, sumWinding, activeAngle, nextAngle);
        if (last) {
            *chase->append() = last;
#if DEBUG_WINDING
            SkDebugf("%s chase.append id=%d\n", __FUNCTION__,
                    last->fOther->fTs[last->fOtherIndex].fOther->debugID());
#endif
        }
    } while (++nextIndex != lastIndex);
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
    SkASSERT(startIndex < endIndex ? startIndex < count - 1
            : startIndex > 0);
    int step = SkSign32(endIndex - startIndex);
    int end = nextExactSpan(startIndex, step);
    SkASSERT(end >= 0);
    SkOpSpan* endSpan = &fTs[end];
    SkOpSegment* other;
    if (isSimple(end)) {
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
            }
             while (precisely_zero(startT - other->fTs[*nextEnd].fT));
            if (other->fTs[SkMin32(*nextStart, *nextEnd)].fWindValue) {
                break;
            }
#ifdef SK_DEBUG
            SkASSERT(firstLoop);
#endif
            SkDEBUGCODE(firstLoop = false;)
            step = -step;
        } while (true);
        SkASSERT(step < 0 ? *nextEnd >= 0 : *nextEnd < other->fTs.count());
        return other;
    }
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
    SkASSERT(startIndex - endIndex != 0);
    SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
    addTwoAngles(startIndex, end, &angles);
    buildAngles(end, &angles, false);
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
    bool sortable = SortAngles(angles, &sorted, SkOpSegment::kMustBeOrdered_SortAngleKind);
    if (!sortable) {
        *unsortable = true;
#if DEBUG_SORT
        debugShowSort(__FUNCTION__, sorted, findStartingEdge(sorted, startIndex, end), 0, 0,
                sortable);
#endif
        return NULL;
    }
    int angleCount = angles.count();
    int firstIndex = findStartingEdge(sorted, startIndex, end);
    SkASSERT(firstIndex >= 0);
#if DEBUG_SORT
    debugShowSort(__FUNCTION__, sorted, firstIndex, 0, 0, sortable);
#endif
    SkASSERT(sorted[firstIndex]->segment() == this);
    int nextIndex = firstIndex + 1;
    int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
    const SkOpAngle* foundAngle = NULL;
    bool foundDone = false;
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        SkASSERT(nextIndex != firstIndex);
        if (nextIndex == angleCount) {
            nextIndex = 0;
        }
        const SkOpAngle* nextAngle = sorted[nextIndex];
        nextSegment = nextAngle->segment();
        ++activeCount;
        if (!foundAngle || (foundDone && activeCount & 1)) {
            if (nextSegment->isTiny(nextAngle)) {
                *unsortable = true;
                return NULL;
            }
            foundAngle = nextAngle;
            foundDone = nextSegment->done(nextAngle);
        }
        if (nextSegment->done()) {
            continue;
        }
    } while (++nextIndex != lastIndex);
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

int SkOpSegment::findStartingEdge(const SkTArray<SkOpAngle*, true>& sorted, int start, int end) {
    int angleCount = sorted.count();
    int firstIndex = -1;
    for (int angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
        const SkOpAngle* angle = sorted[angleIndex];
        if (angle->segment() == this && angle->start() == end &&
                angle->end() == start) {
            firstIndex = angleIndex;
            break;
        }
    }
    return firstIndex;
}

// FIXME: this is tricky code; needs its own unit test
// note that fOtherIndex isn't computed yet, so it can't be used here
void SkOpSegment::findTooCloseToCall() {
    int count = fTs.count();
    if (count < 3) {  // require t=0, x, 1 at minimum
        return;
    }
    int matchIndex = 0;
    int moCount;
    SkOpSpan* match;
    SkOpSegment* mOther;
    do {
        match = &fTs[matchIndex];
        mOther = match->fOther;
        // FIXME: allow quads, cubics to be near coincident?
        if (mOther->fVerb == SkPath::kLine_Verb) {
            moCount = mOther->fTs.count();
            if (moCount >= 3) {
                break;
            }
        }
        if (++matchIndex >= count) {
            return;
        }
    } while (true);  // require t=0, x, 1 at minimum
    // OPTIMIZATION: defer matchPt until qualifying toCount is found?
    const SkPoint* matchPt = &xyAtT(match);
    // look for a pair of nearby T values that map to the same (x,y) value
    // if found, see if the pair of other segments share a common point. If
    // so, the span from here to there is coincident.
    for (int index = matchIndex + 1; index < count; ++index) {
        SkOpSpan* test = &fTs[index];
        if (test->fDone) {
            continue;
        }
        SkOpSegment* tOther = test->fOther;
        if (tOther->fVerb != SkPath::kLine_Verb) {
            continue;  // FIXME: allow quads, cubics to be near coincident?
        }
        int toCount = tOther->fTs.count();
        if (toCount < 3) {  // require t=0, x, 1 at minimum
            continue;
        }
        const SkPoint* testPt = &xyAtT(test);
        if (*matchPt != *testPt) {
            matchIndex = index;
            moCount = toCount;
            match = test;
            mOther = tOther;
            matchPt = testPt;
            continue;
        }
        int moStart = -1;
        int moEnd = -1;
        double moStartT = 0;
        double moEndT = 0;
        for (int moIndex = 0; moIndex < moCount; ++moIndex) {
            SkOpSpan& moSpan = mOther->fTs[moIndex];
            if (moSpan.fDone) {
                continue;
            }
            if (moSpan.fOther == this) {
                if (moSpan.fOtherT == match->fT) {
                    moStart = moIndex;
                    moStartT = moSpan.fT;
                }
                continue;
            }
            if (moSpan.fOther == tOther) {
                if (tOther->windValueAt(moSpan.fOtherT) == 0) {
                    moStart = -1;
                    break;
                }
                SkASSERT(moEnd == -1);
                moEnd = moIndex;
                moEndT = moSpan.fT;
            }
        }
        if (moStart < 0 || moEnd < 0) {
            continue;
        }
        // FIXME: if moStartT, moEndT are initialized to NaN, can skip this test
        if (approximately_equal(moStartT, moEndT)) {
            continue;
        }
        int toStart = -1;
        int toEnd = -1;
        double toStartT = 0;
        double toEndT = 0;
        for (int toIndex = 0; toIndex < toCount; ++toIndex) {
            SkOpSpan& toSpan = tOther->fTs[toIndex];
            if (toSpan.fDone) {
                continue;
            }
            if (toSpan.fOther == this) {
                if (toSpan.fOtherT == test->fT) {
                    toStart = toIndex;
                    toStartT = toSpan.fT;
                }
                continue;
            }
            if (toSpan.fOther == mOther && toSpan.fOtherT == moEndT) {
                if (mOther->windValueAt(toSpan.fOtherT) == 0) {
                    moStart = -1;
                    break;
                }
                SkASSERT(toEnd == -1);
                toEnd = toIndex;
                toEndT = toSpan.fT;
            }
        }
        // FIXME: if toStartT, toEndT are initialized to NaN, can skip this test
        if (toStart <= 0 || toEnd <= 0) {
            continue;
        }
        if (approximately_equal(toStartT, toEndT)) {
            continue;
        }
        // test to see if the segment between there and here is linear
        if (!mOther->isLinear(moStart, moEnd)
                || !tOther->isLinear(toStart, toEnd)) {
            continue;
        }
        bool flipped = (moStart - moEnd) * (toStart - toEnd) < 1;
        if (flipped) {
            mOther->addTCancel(moStartT, moEndT, tOther, toEndT, toStartT);
        } else {
            mOther->addTCoincident(moStartT, moEndT, tOther, toStartT, toEndT);
        }
    }
}

// FIXME: either:
// a) mark spans with either end unsortable as done, or
// b) rewrite findTop / findTopSegment / findTopContour to iterate further
//    when encountering an unsortable span

// OPTIMIZATION : for a pair of lines, can we compute points at T (cached)
// and use more concise logic like the old edge walker code?
// FIXME: this needs to deal with coincident edges
SkOpSegment* SkOpSegment::findTop(int* tIndexPtr, int* endIndexPtr, bool* unsortable,
                                  bool onlySortable) {
    // iterate through T intersections and return topmost
    // topmost tangent from y-min to first pt is closer to horizontal
    SkASSERT(!done());
    int firstT = -1;
    /* SkPoint topPt = */ activeLeftTop(onlySortable, &firstT);
    if (firstT < 0) {
        *unsortable = true;
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
    int end = nextSpan(firstT, step);
    if (end == -1) {
        step = -1;
        end = nextSpan(firstT, step);
        SkASSERT(end != -1);
    }
    // if the topmost T is not on end, or is three-way or more, find left
    // look for left-ness from tLeft to firstT (matching y of other)
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle, true> angles;
    SkASSERT(firstT - end != 0);
    addTwoAngles(end, firstT, &angles);
    buildAngles(firstT, &angles, true);
    SkSTArray<SkOpAngle::kStackBasedCount, SkOpAngle*, true> sorted;
    bool sortable = SortAngles(angles, &sorted, SkOpSegment::kMayBeUnordered_SortAngleKind);
    int first = SK_MaxS32;
    SkScalar top = SK_ScalarMax;
    int count = sorted.count();
    for (int index = 0; index < count; ++index) {
        const SkOpAngle* angle = sorted[index];
        SkOpSegment* next = angle->segment();
        SkPathOpsBounds bounds;
        next->subDivideBounds(angle->end(), angle->start(), &bounds);
        if (approximately_greater(top, bounds.fTop)) {
            top = bounds.fTop;
            first = index;
        }
    }
    SkASSERT(first < SK_MaxS32);
#if DEBUG_SORT  // || DEBUG_SWAP_TOP
    sorted[first]->segment()->debugShowSort(__FUNCTION__, sorted, first, 0, 0, sortable);
#endif
    if (onlySortable && !sortable) {
        *unsortable = true;
        return NULL;
    }
    // skip edges that have already been processed
    firstT = first - 1;
    SkOpSegment* leftSegment;
    do {
        if (++firstT == count) {
            firstT = 0;
        }
        const SkOpAngle* angle = sorted[firstT];
        SkASSERT(!onlySortable || !angle->unsortable());
        leftSegment = angle->segment();
        *tIndexPtr = angle->end();
        *endIndexPtr = angle->start();
    } while (leftSegment->fTs[SkMin32(*tIndexPtr, *endIndexPtr)].fDone);
    if (leftSegment->verb() >= SkPath::kQuad_Verb) {
        const int tIndex = *tIndexPtr;
        const int endIndex = *endIndexPtr;
        if (!leftSegment->clockwise(tIndex, endIndex)) {
            bool swap = !leftSegment->monotonicInY(tIndex, endIndex)
                    && !leftSegment->serpentine(tIndex, endIndex);
    #if DEBUG_SWAP_TOP
            SkDebugf("%s swap=%d serpentine=%d containedByEnds=%d monotonic=%d\n", __FUNCTION__,
                    swap,
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

// FIXME: not crazy about this
// when the intersections are performed, the other index is into an
// incomplete array. As the array grows, the indices become incorrect
// while the following fixes the indices up again, it isn't smart about
// skipping segments whose indices are already correct
// assuming we leave the code that wrote the index in the first place
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
}

void SkOpSegment::initWinding(int start, int end) {
    int local = spanSign(start, end);
    int oppLocal = oppSign(start, end);
    (void) markAndChaseWinding(start, end, local, oppLocal);
    // OPTIMIZATION: the reverse mark and chase could skip the first marking
    (void) markAndChaseWinding(end, start, local, oppLocal);
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
    if (!winding) {
        winding = dx < 0 ? windVal : -windVal;
    } else if (winding * dx < 0) {
        int sideWind = winding + (dx < 0 ? windVal : -windVal);
        if (abs(winding) < abs(sideWind)) {
            winding = sideWind;
        }
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
}

bool SkOpSegment::isLinear(int start, int end) const {
    if (fVerb == SkPath::kLine_Verb) {
        return true;
    }
    if (fVerb == SkPath::kQuad_Verb) {
        SkDQuad qPart = SkDQuad::SubDivide(fPts, fTs[start].fT, fTs[end].fT);
        return qPart.isLinear(0, 2);
    } else {
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        SkDCubic cPart = SkDCubic::SubDivide(fPts, fTs[start].fT, fTs[end].fT);
        return cPart.isLinear(0, 3);
    }
}

// OPTIMIZE: successive calls could start were the last leaves off
// or calls could specialize to walk forwards or backwards
bool SkOpSegment::isMissing(double startT) const {
    size_t tCount = fTs.count();
    for (size_t index = 0; index < tCount; ++index) {
        if (approximately_zero(startT - fTs[index].fT)) {
            return false;
        }
    }
    return true;
}

bool SkOpSegment::isSimple(int end) const {
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
}

// this span is excluded by the winding rule -- chase the ends
// as long as they are unambiguous to mark connections as done
// and give them the same winding value
SkOpSpan* SkOpSegment::markAndChaseDone(int index, int endIndex, int winding) {
    int step = SkSign32(endIndex - index);
    int min = SkMin32(index, endIndex);
    markDone(min, winding);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        other->markDone(min, winding);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAndChaseDoneBinary(const SkOpAngle* angle, int winding, int oppWinding) {
    int index = angle->start();
    int endIndex = angle->end();
    int step = SkSign32(endIndex - index);
    int min = SkMin32(index, endIndex);
    markDoneBinary(min, winding, oppWinding);
    SkOpSpan* last;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&index, step, &min, &last))) {
        other->markDoneBinary(min, winding, oppWinding);
    }
    return last;
}

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

SkOpSpan* SkOpSegment::markAndChaseDoneUnary(const SkOpAngle* angle, int winding) {
    int index = angle->start();
    int endIndex = angle->end();
    return markAndChaseDone(index, endIndex, winding);
}

SkOpSpan* SkOpSegment::markAndChaseWinding(const SkOpAngle* angle, const int winding) {
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

SkOpSpan* SkOpSegment::markAngle(int maxWinding, int sumWinding, bool activeAngle,
                                const SkOpAngle* angle) {
    SkASSERT(angle->segment() == this);
    if (UseInnerWinding(maxWinding, sumWinding)) {
        maxWinding = sumWinding;
    }
    SkOpSpan* last;
    if (activeAngle) {
        last = markAndChaseWinding(angle, maxWinding);
    } else {
        last = markAndChaseDoneUnary(angle, maxWinding);
    }
    return last;
}

SkOpSpan* SkOpSegment::markAngle(int maxWinding, int sumWinding, int oppMaxWinding,
                                 int oppSumWinding, bool activeAngle, const SkOpAngle* angle) {
    SkASSERT(angle->segment() == this);
    if (UseInnerWinding(maxWinding, sumWinding)) {
        maxWinding = sumWinding;
    }
    if (oppMaxWinding != oppSumWinding && UseInnerWinding(oppMaxWinding, oppSumWinding)) {
        oppMaxWinding = oppSumWinding;
    }
    SkOpSpan* last;
    if (activeAngle) {
        last = markAndChaseWinding(angle, maxWinding, oppMaxWinding);
    } else {
        last = markAndChaseDoneBinary(angle, maxWinding, oppMaxWinding);
    }
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
}

void SkOpSegment::markDoneBinary(int index, int winding, int oppWinding) {
  //  SkASSERT(!done());
    SkASSERT(winding || oppWinding);
    double referenceT = fTs[index].fT;
    int lesser = index;
    while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
        markOneDoneBinary(__FUNCTION__, lesser, winding, oppWinding);
    }
    do {
        markOneDoneBinary(__FUNCTION__, index, winding, oppWinding);
    } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
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
}

void SkOpSegment::markOneDone(const char* funName, int tIndex, int winding) {
    SkOpSpan* span = markOneWinding(funName, tIndex, winding);
    if (!span) {
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
    span->fDone = true;
    fDoneSpans++;
}

void SkOpSegment::markOneDoneBinary(const char* funName, int tIndex, int winding, int oppWinding) {
    SkOpSpan* span = markOneWinding(funName, tIndex, winding, oppWinding);
    if (!span) {
        return;
    }
    span->fDone = true;
    fDoneSpans++;
}

void SkOpSegment::markOneDoneUnary(const char* funName, int tIndex) {
    SkOpSpan* span = verifyOneWindingU(funName, tIndex);
    if (!span) {
        return;
    }
    span->fDone = true;
    fDoneSpans++;
}

SkOpSpan* SkOpSegment::markOneWinding(const char* funName, int tIndex, int winding) {
    SkOpSpan& span = fTs[tIndex];
    if (span.fDone) {
        return NULL;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(funName, span, winding);
#endif
    SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
#ifdef SK_DEBUG
    SkASSERT(abs(winding) <= gDebugMaxWindSum);
#endif
    span.fWindSum = winding;
    return &span;
}

SkOpSpan* SkOpSegment::markOneWinding(const char* funName, int tIndex, int winding,
                                      int oppWinding) {
    SkOpSpan& span = fTs[tIndex];
    if (span.fDone) {
        return NULL;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(funName, span, winding, oppWinding);
#endif
    SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
#ifdef SK_DEBUG
    SkASSERT(abs(winding) <= gDebugMaxWindSum);
#endif
    span.fWindSum = winding;
    SkASSERT(span.fOppSum == SK_MinS32 || span.fOppSum == oppWinding);
#ifdef SK_DEBUG
    SkASSERT(abs(oppWinding) <= gDebugMaxWindSum);
#endif
    span.fOppSum = oppWinding;
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
    if (fVerb == SkPath::kLine_Verb) {
        return false;
    }
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
    SkASSERT(span.fWindSum != SK_MinS32);
    SkASSERT(span.fOppSum != SK_MinS32);
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
    SkASSERT(span.fWindSum != SK_MinS32);
    return &span;
}

// note that just because a span has one end that is unsortable, that's
// not enough to mark it done. The other end may be sortable, allowing the
// span to be added.
// FIXME: if abs(start - end) > 1, mark intermediates as unsortable on both ends
void SkOpSegment::markUnsortable(int start, int end) {
    SkOpSpan* span = &fTs[start];
    if (start < end) {
#if DEBUG_UNSORTABLE
        debugShowNewWinding(__FUNCTION__, *span, 0);
#endif
        span->fUnsortableStart = true;
    } else {
        --span;
#if DEBUG_UNSORTABLE
        debugShowNewWinding(__FUNCTION__, *span, 0);
#endif
        span->fUnsortableEnd = true;
    }
    if (!span->fUnsortableStart || !span->fUnsortableEnd || span->fDone) {
        return;
    }
    span->fDone = true;
    fDoneSpans++;
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
}

void SkOpSegment::matchWindingValue(int tIndex, double t, bool borrowWind) {
    int nextDoorWind = SK_MaxS32;
    int nextOppWind = SK_MaxS32;
    if (tIndex > 0) {
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
    if (other->fTs[*min].fTiny) {
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
// OPTIMIZATION splitting this into separate loops for up/down steps
// would allow using precisely_negative instead of precisely_zero
int SkOpSegment::nextExactSpan(int from, int step) const {
    const SkOpSpan& fromSpan = fTs[from];
    int count = fTs.count();
    int to = from;
    while (step > 0 ? ++to < count : --to >= 0) {
        const SkOpSpan& span = fTs[to];
        if (precisely_zero(span.fT - fromSpan.fT)) {
            continue;
        }
        return to;
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
}

// This marks all spans unsortable so that this info is available for early
// exclusion in find top and others. This could be optimized to only mark
// adjacent spans that unsortable. However, this makes it difficult to later
// determine starting points for edge detection in find top and the like.
bool SkOpSegment::SortAngles(const SkTArray<SkOpAngle, true>& angles,
                             SkTArray<SkOpAngle*, true>* angleList,
                             SortAngleKind orderKind) {
    bool sortable = true;
    int angleCount = angles.count();
    int angleIndex;
// FIXME: caller needs to use SkTArray constructor with reserve count
//    angleList->setReserve(angleCount);
    for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
        const SkOpAngle& angle = angles[angleIndex];
        angleList->push_back(const_cast<SkOpAngle*>(&angle));
#if DEBUG_ANGLE
        (*(angleList->end() - 1))->setID(angleIndex);
#endif
        sortable &= !(angle.unsortable() || (orderKind == kMustBeOrdered_SortAngleKind
                    && angle.unorderable()));
    }
    if (sortable) {
        SkTQSort<SkOpAngle>(angleList->begin(), angleList->end() - 1);
        for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
            if (angles[angleIndex].unsortable() || (orderKind == kMustBeOrdered_SortAngleKind
                        && angles[angleIndex].unorderable())) {
                sortable = false;
                break;
            }
        }
    }
    if (!sortable) {
        for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
            const SkOpAngle& angle = angles[angleIndex];
            angle.segment()->markUnsortable(angle.start(), angle.end());
        }
    }
    return sortable;
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

bool SkOpSegment::isTiny(const SkOpAngle* angle) const {
    int start = angle->start();
    int end = angle->end();
    const SkOpSpan& mSpan = fTs[SkMin32(start, end)];
    return mSpan.fTiny;
}

bool SkOpSegment::isTiny(int index) const {
    return fTs[index].fTiny;
}

void SkOpSegment::TrackOutside(SkTArray<double, true>* outsideTs, double end, double start) {
    int outCount = outsideTs->count();
    if (outCount == 0 || !approximately_negative(end - (*outsideTs)[outCount - 2])) {
        outsideTs->push_back(end);
        outsideTs->push_back(start);
    }
}

void SkOpSegment::undoneSpan(int* start, int* end) {
    size_t tCount = fTs.count();
    size_t index;
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
    int spanWinding = spanSign(index, endIndex);
    if (winding && UseInnerWinding(winding - spanWinding, winding) && winding != SK_MaxS32) {
        winding -= spanWinding;
    }
    return winding;
}

int SkOpSegment::updateWinding(const SkOpAngle* angle) const {
    int startIndex = angle->start();
    int endIndex = angle->end();
    return updateWinding(endIndex, startIndex);
}

int SkOpSegment::updateWindingReverse(const SkOpAngle* angle) const {
    int startIndex = angle->start();
    int endIndex = angle->end();
    return updateWinding(startIndex, endIndex);
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

int SkOpSegment::windValue(const SkOpAngle* angle) const {
    int start = angle->start();
    int end = angle->end();
    int index = SkMin32(start, end);
    return windValue(index);
}

int SkOpSegment::windValueAt(double t) const {
    int count = fTs.count();
    for (int index = 0; index < count; ++index) {
        if (fTs[index].fT == t) {
            return fTs[index].fWindValue;
        }
    }
    SkASSERT(0);
    return 0;
}

void SkOpSegment::zeroSpan(SkOpSpan* span) {
    SkASSERT(span->fWindValue > 0 || span->fOppValue != 0);
    span->fWindValue = 0;
    span->fOppValue = 0;
    SkASSERT(!span->fDone);
    span->fDone = true;
    ++fDoneSpans;
}

#if DEBUG_SWAP_TOP
bool SkOpSegment::controlsContainedByEnds(int tStart, int tEnd) const {
    if (fVerb != SkPath::kCubic_Verb) {
        return false;
    }
    SkDCubic dst = SkDCubic::SubDivide(fPts, fTs[tStart].fT, fTs[tEnd].fT);
    return dst.controlsContainedByEnds();
}
#endif

#if DEBUG_CONCIDENT
// SkASSERT if pair has not already been added
void SkOpSegment::debugAddTPair(double t, const SkOpSegment& other, double otherT) const {
    for (int i = 0; i < fTs.count(); ++i) {
        if (fTs[i].fT == t && fTs[i].fOther == &other && fTs[i].fOtherT == otherT) {
            return;
        }
    }
    SkASSERT(0);
}
#endif

#if DEBUG_CONCIDENT
void SkOpSegment::debugShowTs() const {
    SkDebugf("%s id=%d", __FUNCTION__, fID);
    int lastWind = -1;
    int lastOpp = -1;
    double lastT = -1;
    int i;
    for (i = 0; i < fTs.count(); ++i) {
        bool change = lastT != fTs[i].fT || lastWind != fTs[i].fWindValue
                || lastOpp != fTs[i].fOppValue;
        if (change && lastWind >= 0) {
            SkDebugf(" t=%1.3g %1.9g,%1.9g w=%d o=%d]",
                    lastT, xyAtT(i - 1).fX, xyAtT(i - 1).fY, lastWind, lastOpp);
        }
        if (change) {
            SkDebugf(" [o=%d", fTs[i].fOther->fID);
            lastWind = fTs[i].fWindValue;
            lastOpp = fTs[i].fOppValue;
            lastT = fTs[i].fT;
        } else {
            SkDebugf(",%d", fTs[i].fOther->fID);
        }
    }
    if (i <= 0) {
        return;
    }
    SkDebugf(" t=%1.3g %1.9g,%1.9g w=%d o=%d]",
            lastT, xyAtT(i - 1).fX, xyAtT(i - 1).fY, lastWind, lastOpp);
    if (fOperand) {
        SkDebugf(" operand");
    }
    if (done()) {
        SkDebugf(" done");
    }
    SkDebugf("\n");
}
#endif

#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
void SkOpSegment::debugShowActiveSpans() const {
    debugValidate();
    if (done()) {
        return;
    }
#if DEBUG_ACTIVE_SPANS_SHORT_FORM
    int lastId = -1;
    double lastT = -1;
#endif
    for (int i = 0; i < fTs.count(); ++i) {
        if (fTs[i].fDone) {
            continue;
        }
        SkASSERT(i < fTs.count() - 1);
#if DEBUG_ACTIVE_SPANS_SHORT_FORM
        if (lastId == fID && lastT == fTs[i].fT) {
            continue;
        }
        lastId = fID;
        lastT = fTs[i].fT;
#endif
        SkDebugf("%s id=%d", __FUNCTION__, fID);
        SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
        for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
        }
        const SkOpSpan* span = &fTs[i];
        SkDebugf(") t=%1.9g (%1.9g,%1.9g)", span->fT, xAtT(span), yAtT(span));
        int iEnd = i + 1;
        while (fTs[iEnd].fT < 1 && approximately_equal(fTs[i].fT, fTs[iEnd].fT)) {
            ++iEnd;
        }
        SkDebugf(" tEnd=%1.9g", fTs[iEnd].fT);
        const SkOpSegment* other = fTs[i].fOther;
        SkDebugf(" other=%d otherT=%1.9g otherIndex=%d windSum=",
                other->fID, fTs[i].fOtherT, fTs[i].fOtherIndex);
        if (fTs[i].fWindSum == SK_MinS32) {
            SkDebugf("?");
        } else {
            SkDebugf("%d", fTs[i].fWindSum);
        }
        SkDebugf(" windValue=%d oppValue=%d\n", fTs[i].fWindValue, fTs[i].fOppValue);
    }
}
#endif


#if DEBUG_MARK_DONE || DEBUG_UNSORTABLE
void SkOpSegment::debugShowNewWinding(const char* fun, const SkOpSpan& span, int winding) {
    const SkPoint& pt = xyAtT(&span);
    SkDebugf("%s id=%d", fun, fID);
    SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
    for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
        SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
    }
    SkASSERT(&span == &span.fOther->fTs[span.fOtherIndex].fOther->
            fTs[span.fOther->fTs[span.fOtherIndex].fOtherIndex]);
    SkDebugf(") t=%1.9g [%d] (%1.9g,%1.9g) tEnd=%1.9g newWindSum=%d windSum=",
            span.fT, span.fOther->fTs[span.fOtherIndex].fOtherIndex, pt.fX, pt.fY,
            (&span)[1].fT, winding);
    if (span.fWindSum == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", span.fWindSum);
    }
    SkDebugf(" windValue=%d\n", span.fWindValue);
}

void SkOpSegment::debugShowNewWinding(const char* fun, const SkOpSpan& span, int winding,
                                      int oppWinding) {
    const SkPoint& pt = xyAtT(&span);
    SkDebugf("%s id=%d", fun, fID);
    SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
    for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
        SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
    }
    SkASSERT(&span == &span.fOther->fTs[span.fOtherIndex].fOther->
            fTs[span.fOther->fTs[span.fOtherIndex].fOtherIndex]);
    SkDebugf(") t=%1.9g [%d] (%1.9g,%1.9g) tEnd=%1.9g newWindSum=%d newOppSum=%d oppSum=",
            span.fT, span.fOther->fTs[span.fOtherIndex].fOtherIndex, pt.fX, pt.fY,
            (&span)[1].fT, winding, oppWinding);
    if (span.fOppSum == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", span.fOppSum);
    }
    SkDebugf(" windSum=");
    if (span.fWindSum == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", span.fWindSum);
    }
    SkDebugf(" windValue=%d\n", span.fWindValue);
}
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
void SkOpSegment::debugShowSort(const char* fun, const SkTArray<SkOpAngle*, true>& angles,
                                int first, const int contourWinding,
                                const int oppContourWinding, bool sortable) const {
    if (--gDebugSortCount < 0) {
        return;
    }
    SkASSERT(angles[first]->segment() == this);
    SkASSERT(!sortable || angles.count() > 1);
    int lastSum = contourWinding;
    int oppLastSum = oppContourWinding;
    const SkOpAngle* firstAngle = angles[first];
    int windSum = lastSum - spanSign(firstAngle);
    int oppoSign = oppSign(firstAngle);
    int oppWindSum = oppLastSum - oppoSign;
    #define WIND_AS_STRING(x) char x##Str[12]; if (!valid_wind(x)) strcpy(x##Str, "?"); \
        else SK_SNPRINTF(x##Str, sizeof(x##Str), "%d", x)
    WIND_AS_STRING(contourWinding);
    WIND_AS_STRING(oppContourWinding);
    SkDebugf("%s %s contourWinding=%s oppContourWinding=%s sign=%d\n", fun, __FUNCTION__,
            contourWindingStr, oppContourWindingStr, spanSign(angles[first]));
    int index = first;
    bool firstTime = true;
    do {
        const SkOpAngle& angle = *angles[index];
        const SkOpSegment& segment = *angle.segment();
        int start = angle.start();
        int end = angle.end();
        const SkOpSpan& sSpan = segment.fTs[start];
        const SkOpSpan& eSpan = segment.fTs[end];
        const SkOpSpan& mSpan = segment.fTs[SkMin32(start, end)];
        bool opp = segment.fOperand ^ fOperand;
        if (!firstTime) {
            oppoSign = segment.oppSign(&angle);
            if (opp) {
                oppLastSum = oppWindSum;
                oppWindSum -= segment.spanSign(&angle);
                if (oppoSign) {
                    lastSum = windSum;
                    windSum -= oppoSign;
                }
            } else {
                lastSum = windSum;
                windSum -= segment.spanSign(&angle);
                if (oppoSign) {
                    oppLastSum = oppWindSum;
                    oppWindSum -= oppoSign;
                }
            }
        }
        SkDebugf("%s [%d] %s", __FUNCTION__, index,
                angle.unsortable() ? "*** UNSORTABLE *** " : "");
    #if DEBUG_SORT_COMPACT
        SkDebugf("id=%d %s start=%d (%1.9g,%1.9g) end=%d (%1.9g,%1.9g)",
                segment.fID, kLVerbStr[SkPathOpsVerbToPoints(segment.fVerb)],
                start, segment.xAtT(&sSpan), segment.yAtT(&sSpan), end,
                segment.xAtT(&eSpan), segment.yAtT(&eSpan));
    #else
        switch (segment.fVerb) {
            case SkPath::kLine_Verb:
                SkDebugf(LINE_DEBUG_STR, LINE_DEBUG_DATA(segment.fPts));
                break;
            case SkPath::kQuad_Verb:
                SkDebugf(QUAD_DEBUG_STR, QUAD_DEBUG_DATA(segment.fPts));
                break;
            case SkPath::kCubic_Verb:
                SkDebugf(CUBIC_DEBUG_STR, CUBIC_DEBUG_DATA(segment.fPts));
                break;
            default:
                SkASSERT(0);
        }
        SkDebugf(" tStart=%1.9g tEnd=%1.9g", sSpan.fT, eSpan.fT);
    #endif
        SkDebugf(" sign=%d windValue=%d windSum=", angle.sign(), mSpan.fWindValue);
        winding_printf(mSpan.fWindSum);
        int last, wind;
        if (opp) {
            last = oppLastSum;
            wind = oppWindSum;
        } else {
            last = lastSum;
            wind = windSum;
        }
        bool useInner = valid_wind(last) && valid_wind(wind) && UseInnerWinding(last, wind);
        WIND_AS_STRING(last);
        WIND_AS_STRING(wind);
        WIND_AS_STRING(lastSum);
        WIND_AS_STRING(oppLastSum);
        WIND_AS_STRING(windSum);
        WIND_AS_STRING(oppWindSum);
        #undef WIND_AS_STRING
        if (!oppoSign) {
            SkDebugf(" %s->%s (max=%s)", lastStr, windStr, useInner ? windStr : lastStr);
        } else {
            SkDebugf(" %s->%s (%s->%s)", lastStr, windStr, opp ? lastSumStr : oppLastSumStr,
                    opp ? windSumStr : oppWindSumStr);
        }
        SkDebugf(" done=%d tiny=%d opp=%d\n", mSpan.fDone, mSpan.fTiny, opp);
#if 0 && DEBUG_ANGLE
        angle.debugShow(segment.xyAtT(&sSpan));
#endif
        ++index;
        if (index == angles.count()) {
            index = 0;
        }
        if (firstTime) {
            firstTime = false;
        }
    } while (index != first);
}

void SkOpSegment::debugShowSort(const char* fun, const SkTArray<SkOpAngle*, true>& angles,
                                int first, bool sortable) {
    const SkOpAngle* firstAngle = angles[first];
    const SkOpSegment* segment = firstAngle->segment();
    int winding = segment->updateWinding(firstAngle);
    int oppWinding = segment->updateOppWinding(firstAngle);
    debugShowSort(fun, angles, first, winding, oppWinding, sortable);
}

#endif

#if DEBUG_SHOW_WINDING
int SkOpSegment::debugShowWindingValues(int slotCount, int ofInterest) const {
    if (!(1 << fID & ofInterest)) {
        return 0;
    }
    int sum = 0;
    SkTArray<char, true> slots(slotCount * 2);
    memset(slots.begin(), ' ', slotCount * 2);
    for (int i = 0; i < fTs.count(); ++i) {
   //     if (!(1 << fTs[i].fOther->fID & ofInterest)) {
   //         continue;
   //     }
        sum += fTs[i].fWindValue;
        slots[fTs[i].fOther->fID - 1] = as_digit(fTs[i].fWindValue);
        sum += fTs[i].fOppValue;
        slots[slotCount + fTs[i].fOther->fID - 1] = as_digit(fTs[i].fOppValue);
    }
    SkDebugf("%s id=%2d %.*s | %.*s\n", __FUNCTION__, fID, slotCount, slots.begin(), slotCount,
            slots.begin() + slotCount);
    return sum;
}
#endif

void SkOpSegment::debugValidate() const {
#if DEBUG_VALIDATE
    int count = fTs.count();
    SkASSERT(count >= 2);
    SkASSERT(fTs[0].fT == 0);
    SkASSERT(fTs[count - 1].fT == 1);
    int done = 0;
    double t = -1;
    for (int i = 0; i < count; ++i) {
        const SkOpSpan& span = fTs[i];
        SkASSERT(t <= span.fT);
        t = span.fT;
        int otherIndex = span.fOtherIndex;
        const SkOpSegment* other = span.fOther;
        const SkOpSpan& otherSpan = other->fTs[otherIndex];
        SkASSERT(otherSpan.fPt == span.fPt);
        SkASSERT(otherSpan.fOtherT == t);
        SkASSERT(&fTs[i] == &otherSpan.fOther->fTs[otherSpan.fOtherIndex]);
        done += span.fDone;
    }
    SkASSERT(done == fDoneSpans);
#endif
}
