/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpSegment_DEFINE
#define SkOpSegment_DEFINE

#include "SkOpAngle.h"
#include "SkOpSpan.h"
#include "SkPathOpsBounds.h"
#include "SkPathOpsCurve.h"
#include "SkTArray.h"
#include "SkTDArray.h"

class SkPathWriter;

class SkOpSegment {
public:
    SkOpSegment() {
#if DEBUG_DUMP
        fID = ++gSegmentID;
#endif
    }

    bool operator<(const SkOpSegment& rh) const {
        return fBounds.fTop < rh.fBounds.fTop;
    }

    const SkPathOpsBounds& bounds() const {
        return fBounds;
    }

    // OPTIMIZE
    // when the edges are initially walked, they don't automatically get the prior and next
    // edges assigned to positions t=0 and t=1. Doing that would remove the need for this check,
    // and would additionally remove the need for similar checks in condition edges. It would
    // also allow intersection code to assume end of segment intersections (maybe?)
    bool complete() const {
        int count = fTs.count();
        return count > 1 && fTs[0].fT == 0 && fTs[--count].fT == 1;
    }

    int count() const {
        return fTs.count();
    }

    bool done() const {
        SkASSERT(fDoneSpans <= fTs.count());
        return fDoneSpans == fTs.count();
    }

    bool done(int min) const {
        return fTs[min].fDone;
    }

    bool done(const SkOpAngle* angle) const {
        return done(SkMin32(angle->start(), angle->end()));
    }

    SkVector dxdy(int index) const {
        return (*CurveSlopeAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, fTs[index].fT);
    }

    SkScalar dy(int index) const {
        return dxdy(index).fY;
    }

    bool intersected() const {
        return fTs.count() > 0;
    }

    bool isCanceled(int tIndex) const {
        return fTs[tIndex].fWindValue == 0 && fTs[tIndex].fOppValue == 0;
    }

    bool isConnected(int startIndex, int endIndex) const {
        return fTs[startIndex].fWindSum != SK_MinS32 || fTs[endIndex].fWindSum != SK_MinS32;
    }

    bool isHorizontal() const {
        return fBounds.fTop == fBounds.fBottom;
    }

    bool isVertical() const {
        return fBounds.fLeft == fBounds.fRight;
    }

    bool isVertical(int start, int end) const {
        return (*CurveIsVertical[SkPathOpsVerbToPoints(fVerb)])(fPts, start, end);
    }

    bool operand() const {
        return fOperand;
    }

    int oppSign(const SkOpAngle* angle) const {
        SkASSERT(angle->segment() == this);
        return oppSign(angle->start(), angle->end());
    }

    int oppSign(int startIndex, int endIndex) const {
        int result = startIndex < endIndex ? -fTs[startIndex].fOppValue : fTs[endIndex].fOppValue;
#if DEBUG_WIND_BUMP
        SkDebugf("%s oppSign=%d\n", __FUNCTION__, result);
#endif
        return result;
    }

    int oppSum(int tIndex) const {
        return fTs[tIndex].fOppSum;
    }

    int oppSum(const SkOpAngle* angle) const {
        int lesser = SkMin32(angle->start(), angle->end());
        return fTs[lesser].fOppSum;
    }

    int oppValue(int tIndex) const {
        return fTs[tIndex].fOppValue;
    }

    int oppValue(const SkOpAngle* angle) const {
        int lesser = SkMin32(angle->start(), angle->end());
        return fTs[lesser].fOppValue;
    }

    const SkOpSegment* other(int index) const {
        return fTs[index].fOther;
    }

    // was used only by right angle winding finding
    SkPoint ptAtT(double mid) const {
        return (*CurvePointAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, mid);
    }

    const SkPoint* pts() const {
        return fPts;
    }

    void reset() {
        init(NULL, (SkPath::Verb) -1, false, false);
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fTs.reset();
    }

    void setOppXor(bool isOppXor) {
        fOppXor = isOppXor;
    }

    void setUpWinding(int index, int endIndex, int* maxWinding, int* sumWinding) {
        int deltaSum = spanSign(index, endIndex);
        *maxWinding = *sumWinding;
        *sumWinding -= deltaSum;
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    const SkOpSpan& span(int tIndex) const {
        return fTs[tIndex];
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    const SkTDArray<SkOpSpan>& spans() const {
        return fTs;
    }

    int spanSign(const SkOpAngle* angle) const {
        SkASSERT(angle->segment() == this);
        return spanSign(angle->start(), angle->end());
    }

    int spanSign(int startIndex, int endIndex) const {
        int result = startIndex < endIndex ? -fTs[startIndex].fWindValue : fTs[endIndex].fWindValue;
#if DEBUG_WIND_BUMP
        SkDebugf("%s spanSign=%d\n", __FUNCTION__, result);
#endif
        return result;
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    double t(int tIndex) const {
        return fTs[tIndex].fT;
    }

    double tAtMid(int start, int end, double mid) const {
        return fTs[start].fT * (1 - mid) + fTs[end].fT * mid;
    }

    bool unsortable(int index) const {
        return fTs[index].fUnsortableStart || fTs[index].fUnsortableEnd;
    }

    void updatePts(const SkPoint pts[]) {
        fPts = pts;
    }

    SkPath::Verb verb() const {
        return fVerb;
    }

    int windSum(int tIndex) const {
        return fTs[tIndex].fWindSum;
    }

    int windValue(int tIndex) const {
        return fTs[tIndex].fWindValue;
    }

    SkScalar xAtT(int index) const {
        return xAtT(&fTs[index]);
    }

    SkScalar xAtT(const SkOpSpan* span) const {
        return xyAtT(span).fX;
    }

    const SkPoint& xyAtT(const SkOpSpan* span) const {
        return span->fPt;
    }

    const SkPoint& xyAtT(int index) const {
        return xyAtT(&fTs[index]);
    }

    SkScalar yAtT(int index) const {
        return yAtT(&fTs[index]);
    }

    SkScalar yAtT(const SkOpSpan* span) const {
        return xyAtT(span).fY;
    }

    bool activeAngle(int index, int* done, SkTArray<SkOpAngle, true>* angles);
    SkPoint activeLeftTop(bool onlySortable, int* firstT) const;
    bool activeOp(int index, int endIndex, int xorMiMask, int xorSuMask, SkPathOp op);
    bool activeOp(int xorMiMask, int xorSuMask, int index, int endIndex, SkPathOp op,
                  int* sumMiWinding, int* sumSuWinding, int* maxWinding, int* sumWinding,
                  int* oppMaxWinding, int* oppSumWinding);
    bool activeWinding(int index, int endIndex);
    bool activeWinding(int index, int endIndex, int* maxWinding, int* sumWinding);
    void addCubic(const SkPoint pts[4], bool operand, bool evenOdd);
    void addCurveTo(int start, int end, SkPathWriter* path, bool active) const;
    void addLine(const SkPoint pts[2], bool operand, bool evenOdd);
    void addOtherT(int index, double otherT, int otherIndex);
    void addQuad(const SkPoint pts[3], bool operand, bool evenOdd);
    int addSelfT(SkOpSegment* other, const SkPoint& pt, double newT);
    int addT(SkOpSegment* other, const SkPoint& pt, double newT);
    void addTCancel(double startT, double endT, SkOpSegment* other, double oStartT, double oEndT);
    void addTCoincident(double startT, double endT, SkOpSegment* other, double oStartT,
                        double oEndT);
    void addTPair(double t, SkOpSegment* other, double otherT, bool borrowWind, const SkPoint& pt);
    void addTPair(double t, SkOpSegment* other, double otherT, bool borrowWind, const SkPoint& pt,
                  const SkPoint& oPt);
    int addUnsortableT(SkOpSegment* other, bool start, const SkPoint& pt, double newT);
    bool betweenTs(int lesser, double testT, int greater) const;
    void checkEnds();
    int computeSum(int startIndex, int endIndex, bool binary);
    int crossedSpanY(const SkPoint& basePt, SkScalar* bestY, double* hitT, bool* hitSomething,
                     double mid, bool opp, bool current) const;
    SkOpSegment* findNextOp(SkTDArray<SkOpSpan*>* chase, int* nextStart, int* nextEnd,
                            bool* unsortable, SkPathOp op, const int xorMiMask,
                            const int xorSuMask);
    SkOpSegment* findNextWinding(SkTDArray<SkOpSpan*>* chase, int* nextStart, int* nextEnd,
                                 bool* unsortable);
    SkOpSegment* findNextXor(int* nextStart, int* nextEnd, bool* unsortable);
    void findTooCloseToCall();
    SkOpSegment* findTop(int* tIndex, int* endIndex, bool* unsortable, bool onlySortable);
    void fixOtherTIndex();
    void initWinding(int start, int end);
    void initWinding(int start, int end, double tHit, int winding, SkScalar hitDx, int oppWind,
            SkScalar hitOppDx);
    bool isLinear(int start, int end) const;
    bool isMissing(double startT) const;
    bool isSimple(int end) const;
    bool isTiny(const SkOpAngle* angle) const;
    bool isTiny(int index) const;
    SkOpSpan* markAndChaseDoneBinary(int index, int endIndex);
    SkOpSpan* markAndChaseDoneUnary(int index, int endIndex);
    SkOpSpan* markAndChaseWinding(const SkOpAngle* angle, int winding, int oppWinding);
    SkOpSpan* markAngle(int maxWinding, int sumWinding, int oppMaxWinding, int oppSumWinding,
                        bool activeAngle, const SkOpAngle* angle);
    void markDone(int index, int winding);
    void markDoneBinary(int index);
    void markDoneUnary(int index);
    SkOpSpan* markOneWinding(const char* funName, int tIndex, int winding);
    SkOpSpan* markOneWinding(const char* funName, int tIndex, int winding, int oppWinding);
    void markWinding(int index, int winding);
    void markWinding(int index, int winding, int oppWinding);
    bool nextCandidate(int* start, int* end) const;
    int nextExactSpan(int from, int step) const;
    int nextSpan(int from, int step) const;
    void setUpWindings(int index, int endIndex, int* sumMiWinding, int* sumSuWinding,
            int* maxWinding, int* sumWinding, int* oppMaxWinding, int* oppSumWinding);
    enum SortAngleKind {
        kMustBeOrdered_SortAngleKind, // required for winding calc
        kMayBeUnordered_SortAngleKind // ok for find top
    };
    static bool SortAngles(const SkTArray<SkOpAngle, true>& angles,
                           SkTArray<SkOpAngle*, true>* angleList,
                           SortAngleKind );
    bool subDivide(int start, int end, SkPoint edge[4]) const;
    bool subDivide(int start, int end, SkDCubic* result) const;
    void undoneSpan(int* start, int* end);
    int updateOppWindingReverse(const SkOpAngle* angle) const;
    int updateWindingReverse(const SkOpAngle* angle) const;
    static bool UseInnerWinding(int outerWinding, int innerWinding);
    int windingAtT(double tHit, int tIndex, bool crossOpp, SkScalar* dx) const;
    int windSum(const SkOpAngle* angle) const;
    int windValue(const SkOpAngle* angle) const;

#if DEBUG_DUMP
    int debugID() const {
        return fID;
    }
#endif
#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
    void debugShowActiveSpans() const;
#endif
#if DEBUG_SORT || DEBUG_SWAP_TOP
    void debugShowSort(const char* fun, const SkTArray<SkOpAngle*, true>& angles, int first,
            const int contourWinding, const int oppContourWinding, bool sortable) const;
    void debugShowSort(const char* fun, const SkTArray<SkOpAngle*, true>& angles, int first,
            bool sortable);
#endif
#if DEBUG_CONCIDENT
    void debugShowTs() const;
#endif
#if DEBUG_SHOW_WINDING
    int debugShowWindingValues(int slotCount, int ofInterest) const;
#endif

private:
    bool activeAngleOther(int index, int* done, SkTArray<SkOpAngle, true>* angles);
    bool activeAngleInner(int index, int* done, SkTArray<SkOpAngle, true>* angles);
    void addAngle(SkTArray<SkOpAngle, true>* angles, int start, int end) const;
    void addCancelOutsides(double tStart, double oStart, SkOpSegment* other, double oEnd);
    void addCoinOutsides(const SkTArray<double, true>& outsideTs, SkOpSegment* other, double oEnd);
    void addTwoAngles(int start, int end, SkTArray<SkOpAngle, true>* angles) const;
    int advanceCoincidentOther(const SkOpSpan* test, double oEndT, int oIndex);
    int advanceCoincidentThis(const SkOpSpan* oTest, bool opp, int index);
    void buildAngles(int index, SkTArray<SkOpAngle, true>* angles, bool includeOpp) const;
    void buildAnglesInner(int index, SkTArray<SkOpAngle, true>* angles) const;
    int bumpCoincidentThis(const SkOpSpan& oTest, bool opp, int index,
                           SkTArray<double, true>* outsideTs);
    int bumpCoincidentOther(const SkOpSpan& test, double oEndT, int& oIndex,
                            SkTArray<double, true>* oOutsideTs);
    bool bumpSpan(SkOpSpan* span, int windDelta, int oppDelta);
    bool clockwise(int tStart, int tEnd) const;
    void decrementSpan(SkOpSpan* span);
    bool equalPoints(int greaterTIndex, int lesserTIndex);
    int findStartingEdge(const SkTArray<SkOpAngle*, true>& sorted, int start, int end);
    void init(const SkPoint pts[], SkPath::Verb verb, bool operand, bool evenOdd);
    void matchWindingValue(int tIndex, double t, bool borrowWind);
    SkOpSpan* markAndChaseDone(int index, int endIndex, int winding);
    SkOpSpan* markAndChaseDoneBinary(const SkOpAngle* angle, int winding, int oppWinding);
    SkOpSpan* markAndChaseWinding(const SkOpAngle* angle, const int winding);
    SkOpSpan* markAndChaseWinding(int index, int endIndex, int winding, int oppWinding);
    SkOpSpan* markAngle(int maxWinding, int sumWinding, bool activeAngle, const SkOpAngle* angle);
    void markDoneBinary(int index, int winding, int oppWinding);
    SkOpSpan* markAndChaseDoneUnary(const SkOpAngle* angle, int winding);
    void markOneDone(const char* funName, int tIndex, int winding);
    void markOneDoneBinary(const char* funName, int tIndex);
    void markOneDoneBinary(const char* funName, int tIndex, int winding, int oppWinding);
    void markOneDoneUnary(const char* funName, int tIndex);
    void markUnsortable(int start, int end);
    bool monotonicInY(int tStart, int tEnd) const;
    bool multipleSpans(int end) const;
    SkOpSegment* nextChase(int* index, const int step, int* min, SkOpSpan** last);
    bool serpentine(int tStart, int tEnd) const;
    void subDivideBounds(int start, int end, SkPathOpsBounds* bounds) const;
    static void TrackOutside(SkTArray<double, true>* outsideTs, double end, double start);
    int updateOppWinding(int index, int endIndex) const;
    int updateOppWinding(const SkOpAngle* angle) const;
    int updateWinding(int index, int endIndex) const;
    int updateWinding(const SkOpAngle* angle) const;
    SkOpSpan* verifyOneWinding(const char* funName, int tIndex);
    SkOpSpan* verifyOneWindingU(const char* funName, int tIndex);
    int windValueAt(double t) const;
    void zeroSpan(SkOpSpan* span);

#if DEBUG_SWAP_TOP
    bool controlsContainedByEnds(int tStart, int tEnd) const;
#endif
#if DEBUG_CONCIDENT
     void debugAddTPair(double t, const SkOpSegment& other, double otherT) const;
#endif
#if DEBUG_MARK_DONE || DEBUG_UNSORTABLE
    void debugShowNewWinding(const char* fun, const SkOpSpan& span, int winding);
    void debugShowNewWinding(const char* fun, const SkOpSpan& span, int winding, int oppWinding);
#endif
#if DEBUG_WINDING
    static char as_digit(int value) {
        return value < 0 ? '?' : value <= 9 ? '0' + value : '+';
    }
#endif
    void debugValidate() const;

    const SkPoint* fPts;
    SkPathOpsBounds fBounds;
    // FIXME: can't convert to SkTArray because it uses insert
    SkTDArray<SkOpSpan> fTs;  // two or more (always includes t=0 t=1)
    // OPTIMIZATION: could pack donespans, verb, operand, xor into 1 int-sized value
    int fDoneSpans;  // quick check that segment is finished
    // OPTIMIZATION: force the following to be byte-sized
    SkPath::Verb fVerb;
    bool fOperand;
    bool fXor;  // set if original contour had even-odd fill
    bool fOppXor;  // set if opposite operand had even-odd fill
#if DEBUG_DUMP
    int fID;
#endif
};

#endif
