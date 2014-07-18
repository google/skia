/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsDebug.h"
#include "SkPath.h"

#if defined SK_DEBUG || !FORCE_RELEASE

const char* SkPathOpsDebug::kLVerbStr[] = {"", "line", "quad", "cubic"};

#if defined(SK_DEBUG) || !FORCE_RELEASE
int SkPathOpsDebug::gContourID = 0;
int SkPathOpsDebug::gSegmentID = 0;
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
int SkPathOpsDebug::gSortCountDefault = SK_MaxS32;
int SkPathOpsDebug::gSortCount;
#endif

#if DEBUG_ACTIVE_OP
const char* SkPathOpsDebug::kPathOpStr[] = {"diff", "sect", "union", "xor"};
#endif

bool SkPathOpsDebug::ChaseContains(const SkTDArray<SkOpSpan *>& chaseArray,
        const SkOpSpan* span) {
    for (int index = 0; index < chaseArray.count(); ++index) {
        const SkOpSpan* entry = chaseArray[index];
        if (entry == span) {
            return true;
        }
    }
    return false;
}

void SkPathOpsDebug::MathematicaIze(char* str, size_t bufferLen) {
    size_t len = strlen(str);
    bool num = false;
    for (size_t idx = 0; idx < len; ++idx) {
        if (num && str[idx] == 'e') {
            if (len + 2 >= bufferLen) {
                return;
            }
            memmove(&str[idx + 2], &str[idx + 1], len - idx);
            str[idx] = '*';
            str[idx + 1] = '^';
            ++len;
        }
        num = str[idx] >= '0' && str[idx] <= '9';
    }
}

bool SkPathOpsDebug::ValidWind(int wind) {
    return wind > SK_MinS32 + 0xFFFF && wind < SK_MaxS32 - 0xFFFF;
}

void SkPathOpsDebug::WindingPrintf(int wind) {
    if (wind == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", wind);
    }
}

#if DEBUG_SHOW_TEST_NAME
void* SkPathOpsDebug::CreateNameStr() {
    return SkNEW_ARRAY(char, DEBUG_FILENAME_STRING_LENGTH);
}

void SkPathOpsDebug::DeleteNameStr(void* v) {
    SkDELETE_ARRAY(reinterpret_cast<char* >(v));
}

void SkPathOpsDebug::BumpTestName(char* test) {
    char* num = test + strlen(test);
    while (num[-1] >= '0' && num[-1] <= '9') {
        --num;
    }
    if (num[0] == '\0') {
        return;
    }
    int dec = atoi(num);
    if (dec == 0) {
        return;
    }
    ++dec;
    SK_SNPRINTF(num, DEBUG_FILENAME_STRING_LENGTH - (num - test), "%d", dec);
}
#endif

#if !DEBUG_SHOW_TEST_NAME  // enable when building without extended test
void SkPathOpsDebug::ShowPath(const SkPath& one, const SkPath& two, SkPathOp op, const char* name) {
}
#endif

#endif //  defined SK_DEBUG || !FORCE_RELEASE

#include "SkOpAngle.h"
#include "SkOpSegment.h"

#if DEBUG_SORT
void SkOpAngle::debugLoop() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->dumpOne(true);
        SkDebugf("\n");
        next = next->fNext;
    } while (next && next != first);
}
#endif

#if DEBUG_ANGLE
void SkOpAngle::debugSameAs(const SkOpAngle* compare) const {
    SK_ALWAYSBREAK(fSegment == compare->fSegment);
    const SkOpSpan& startSpan = fSegment->span(fStart);
    const SkOpSpan& oStartSpan = fSegment->span(compare->fStart);
    SK_ALWAYSBREAK(startSpan.fToAngle == oStartSpan.fToAngle);
    SK_ALWAYSBREAK(startSpan.fFromAngle == oStartSpan.fFromAngle);
    const SkOpSpan& endSpan = fSegment->span(fEnd);
    const SkOpSpan& oEndSpan = fSegment->span(compare->fEnd);
    SK_ALWAYSBREAK(endSpan.fToAngle == oEndSpan.fToAngle);
    SK_ALWAYSBREAK(endSpan.fFromAngle == oEndSpan.fFromAngle);
}
#endif

#if DEBUG_VALIDATE
void SkOpAngle::debugValidateNext() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = first;
    SkTDArray<const SkOpAngle*>(angles);
    do {
//        SK_ALWAYSBREAK(next->fSegment->debugContains(next));
        angles.push(next);
        next = next->next();
        if (next == first) {
            break;
        }
        SK_ALWAYSBREAK(!angles.contains(next));
        if (!next) {
            return;
        }
    } while (true);
}

void SkOpAngle::debugValidateLoop() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = first;
    SK_ALWAYSBREAK(first->next() != first);
    int signSum = 0;
    int oppSum = 0;
    bool firstOperand = fSegment->operand();
    bool unorderable = false;
    do {
        unorderable |= next->fUnorderable;
        const SkOpSegment* segment = next->fSegment;
        bool operandsMatch = firstOperand == segment->operand();
        signSum += operandsMatch ? segment->spanSign(next) : segment->oppSign(next);
        oppSum += operandsMatch ? segment->oppSign(next) : segment->spanSign(next);
        const SkOpSpan& span = segment->span(SkMin32(next->fStart, next->fEnd));
        if (segment->_xor()) {
//            SK_ALWAYSBREAK(span.fWindValue == 1);
//            SK_ALWAYSBREAK(span.fWindSum == SK_MinS32 || span.fWindSum == 1);
        }
        if (segment->oppXor()) {
            SK_ALWAYSBREAK(span.fOppValue == 0 || abs(span.fOppValue) == 1);
//            SK_ALWAYSBREAK(span.fOppSum == SK_MinS32 || span.fOppSum == 0 || abs(span.fOppSum) == 1);
        }
        next = next->next();
        if (!next) {
            return;
        }
    } while (next != first);
    if (unorderable) {
        return;
    }
    SK_ALWAYSBREAK(!signSum || fSegment->_xor());
    SK_ALWAYSBREAK(!oppSum || fSegment->oppXor());
    int lastWinding;
    int lastOppWinding;
    int winding;
    int oppWinding;
    do {
        const SkOpSegment* segment = next->fSegment;
        const SkOpSpan& span = segment->span(SkMin32(next->fStart, next->fEnd));
        winding = span.fWindSum;
        if (winding != SK_MinS32) {
//            SK_ALWAYSBREAK(winding != 0);
            SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(winding));
            lastWinding = winding;
            int diffWinding = segment->spanSign(next);
            if (!segment->_xor()) {
                SK_ALWAYSBREAK(diffWinding != 0);
                bool sameSign = (winding > 0) == (diffWinding > 0);
                winding -= sameSign ? diffWinding : -diffWinding;
                SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(winding));
                SK_ALWAYSBREAK(abs(winding) <= abs(lastWinding));
                if (!sameSign) {
                    SkTSwap(winding, lastWinding);
                }
            }
            lastOppWinding = oppWinding = span.fOppSum;
            if (oppWinding != SK_MinS32 && !segment->oppXor()) {
                int oppDiffWinding = segment->oppSign(next);
//                SK_ALWAYSBREAK(abs(oppDiffWinding) <= abs(diffWinding) || segment->_xor());
                if (oppDiffWinding) {
                    bool oppSameSign = (oppWinding > 0) == (oppDiffWinding > 0);
                    oppWinding -= oppSameSign ? oppDiffWinding : -oppDiffWinding;
                    SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(oppWinding));
                    SK_ALWAYSBREAK(abs(oppWinding) <= abs(lastOppWinding));
                    if (!oppSameSign) {
                        SkTSwap(oppWinding, lastOppWinding);
                    }
                }
            }
            firstOperand = segment->operand();
            break;
        }
        SK_ALWAYSBREAK(span.fOppSum == SK_MinS32);
        next = next->next();
    } while (next != first);
    if (winding == SK_MinS32) {
        return;
    }
    SK_ALWAYSBREAK(oppWinding == SK_MinS32 || SkPathOpsDebug::ValidWind(oppWinding));
    first = next;
    next = next->next();
    do {
        const SkOpSegment* segment = next->fSegment;
        lastWinding = winding;
        lastOppWinding = oppWinding;
        bool operandsMatch = firstOperand == segment->operand();
        if (operandsMatch) {
            if (!segment->_xor()) {
                winding -= segment->spanSign(next);
                SK_ALWAYSBREAK(winding != lastWinding);
                SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(winding));
            }
            if (!segment->oppXor()) {
                int oppDiffWinding = segment->oppSign(next);
                if (oppWinding != SK_MinS32) {
                    oppWinding -= oppDiffWinding;
                    SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(oppWinding));
                } else {
                    SK_ALWAYSBREAK(oppDiffWinding == 0);
                }
            }
        } else {
            if (!segment->oppXor()) {
                winding -= segment->oppSign(next);
                SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(winding));
            }
            if (!segment->_xor()) {
                oppWinding -= segment->spanSign(next);
                SK_ALWAYSBREAK(oppWinding != lastOppWinding);
                SK_ALWAYSBREAK(SkPathOpsDebug::ValidWind(oppWinding));
            }
        }
        bool useInner = SkOpSegment::UseInnerWinding(lastWinding, winding);
        int sumWinding = useInner ? winding : lastWinding;
        bool oppUseInner = SkOpSegment::UseInnerWinding(lastOppWinding, oppWinding);
        int oppSumWinding = oppUseInner ? oppWinding : lastOppWinding;
        if (!operandsMatch) {
            SkTSwap(useInner, oppUseInner);
            SkTSwap(sumWinding, oppSumWinding);
        }
        const SkOpSpan& span = segment->span(SkMin32(next->fStart, next->fEnd));
        if (winding == -lastWinding) {
            if (span.fWindSum != SK_MinS32) {
                SkDebugf("%s useInner=%d spanSign=%d lastWinding=%d winding=%d windSum=%d\n",
                        __FUNCTION__,
                        useInner, segment->spanSign(next), lastWinding, winding, span.fWindSum);
            }
        }
        if (oppWinding != SK_MinS32) {
            if (span.fOppSum != SK_MinS32) {
                SK_ALWAYSBREAK(span.fOppSum == oppSumWinding || segment->oppXor() || segment->_xor());
            }
        } else {
            SK_ALWAYSBREAK(!firstOperand);
            SK_ALWAYSBREAK(!segment->operand());
            SK_ALWAYSBREAK(!span.fOppValue);
        }
        next = next->next();
    } while (next != first);
}
#endif

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
// SK_ALWAYSBREAK if pair has not already been added
void SkOpSegment::debugAddTPair(double t, const SkOpSegment& other, double otherT) const {
    for (int i = 0; i < fTs.count(); ++i) {
        if (fTs[i].fT == t && fTs[i].fOther == &other && fTs[i].fOtherT == otherT) {
            return;
        }
    }
    SK_ALWAYSBREAK(0);
}
#endif

#if DEBUG_ANGLE
void SkOpSegment::debugCheckPointsEqualish(int tStart, int tEnd) const {
    const SkPoint& basePt = fTs[tStart].fPt;
    while (++tStart < tEnd) {
       const SkPoint& cmpPt = fTs[tStart].fPt;
       SK_ALWAYSBREAK(SkDPoint::ApproximatelyEqual(basePt, cmpPt));
    }
}
#endif

#if DEBUG_SWAP_TOP
int SkOpSegment::debugInflections(int tStart, int tEnd) const {
    if (fVerb != SkPath::kCubic_Verb) {
        return false;
    }
    SkDCubic dst = SkDCubic::SubDivide(fPts, fTs[tStart].fT, fTs[tEnd].fT);
    double inflections[2];
    return dst.findInflections(inflections);
}
#endif

const SkOpAngle* SkOpSegment::debugLastAngle() const {
    const SkOpAngle* result = NULL;
    for (int index = 0; index < count(); ++index) {
        const SkOpSpan& span = this->span(index);
        if (span.fToAngle) {
            SkASSERT(!result);
            result = span.fToAngle;
        }
    }
    SkASSERT(result);
    return result;
}

void SkOpSegment::debugReset() {
    fTs.reset();
    fAngles.reset();
}

#if DEBUG_CONCIDENT
void SkOpSegment::debugShowTs(const char* prefix) const {
    SkDebugf("%s %s id=%d", __FUNCTION__, prefix, fID);
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
        SK_ALWAYSBREAK(i < fTs.count() - 1);
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
    SK_ALWAYSBREAK(&span == &span.fOther->fTs[span.fOtherIndex].fOther->
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
    SK_ALWAYSBREAK(&span == &span.fOther->fTs[span.fOtherIndex].fOther->
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
    SkDebugf(" windValue=%d oppValue=%d\n", span.fWindValue, span.fOppValue);
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
    SK_ALWAYSBREAK(count >= 2);
    SK_ALWAYSBREAK(fTs[0].fT == 0);
    SK_ALWAYSBREAK(fTs[count - 1].fT == 1);
    int done = 0;
    double t = -1;
    const SkOpSpan* last = NULL;
    bool tinyTFound = false;
    bool hasLoop = false;
    for (int i = 0; i < count; ++i) {
        const SkOpSpan& span = fTs[i];
        SK_ALWAYSBREAK(t <= span.fT);
        t = span.fT;
        int otherIndex = span.fOtherIndex;
        const SkOpSegment* other = span.fOther;
        SK_ALWAYSBREAK(other != this || fVerb == SkPath::kCubic_Verb);
        const SkOpSpan& otherSpan = other->fTs[otherIndex];
        SK_ALWAYSBREAK(otherSpan.fPt == span.fPt);
        SK_ALWAYSBREAK(otherSpan.fOtherT == t);
        SK_ALWAYSBREAK(&fTs[i] == &otherSpan.fOther->fTs[otherSpan.fOtherIndex]);
        done += span.fDone;
        if (last) {
            SK_ALWAYSBREAK(last->fT != span.fT || last->fOther != span.fOther);
            bool tsEqual = last->fT == span.fT;
            bool tsPreciselyEqual = precisely_equal(last->fT, span.fT);
            SK_ALWAYSBREAK(!tsEqual || tsPreciselyEqual);
            bool pointsEqual = last->fPt == span.fPt;
            bool pointsNearlyEqual = AlmostEqualUlps(last->fPt, span.fPt);
#if 0  // bufferOverflow test triggers this
            SK_ALWAYSBREAK(!tsPreciselyEqual || pointsNearlyEqual);
#endif
//            SK_ALWAYSBREAK(!last->fTiny || !tsPreciselyEqual || span.fTiny || tinyTFound);
            SK_ALWAYSBREAK(last->fTiny || tsPreciselyEqual || !pointsEqual || hasLoop);
            SK_ALWAYSBREAK(!last->fTiny || pointsEqual);
            SK_ALWAYSBREAK(!last->fTiny || last->fDone);
            SK_ALWAYSBREAK(!last->fSmall || pointsNearlyEqual);
            SK_ALWAYSBREAK(!last->fSmall || last->fDone);
//            SK_ALWAYSBREAK(!last->fSmall || last->fTiny);
//            SK_ALWAYSBREAK(last->fTiny || !pointsEqual || last->fDone == span.fDone);
            if (last->fTiny) {
                tinyTFound |= !tsPreciselyEqual;
            } else {
                tinyTFound = false;
            }
        }
        last = &span;
        hasLoop |= last->fLoop;
    }
    SK_ALWAYSBREAK(done == fDoneSpans);
//    if (fAngles.count() ) {
//        fAngles.begin()->debugValidateLoop();
//    }
#endif
}
