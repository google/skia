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

#if defined(SK_DEBUG) || !FORCE_RELEASE
#include "SkThread.h"
#endif

struct SkCoincidence;
class SkPathWriter;

class SkOpSegment {
public:
    SkOpSegment() {
#if defined(SK_DEBUG) || !FORCE_RELEASE
        fID = sk_atomic_inc(&SkPathOpsDebug::gSegmentID);
#endif
    }

    bool operator<(const SkOpSegment& rh) const {
        return fBounds.fTop < rh.fBounds.fTop;
    }

    struct AlignedSpan  {
        double fOldT;
        double fT;
        SkPoint fOldPt;
        SkPoint fPt;
        const SkOpSegment* fSegment;
        const SkOpSegment* fOther1;
        const SkOpSegment* fOther2;
    };

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

    SkDPoint dPtAtT(double mid) const {
        return (*CurveDPointAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, mid);
    }

    SkVector dxdy(int index) const {
        return (*CurveSlopeAtT[SkPathOpsVerbToPoints(fVerb)])(fPts, fTs[index].fT);
    }

    SkScalar dy(int index) const {
        return dxdy(index).fY;
    }

    bool hasMultiples() const {
        return fMultiples;
    }

    bool hasSmall() const {
        return fSmall;
    }

    bool hasTiny() const {
        return fTiny;
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

#if DEBUG_VALIDATE
    bool oppXor() const {
        return fOppXor;
    }
#endif

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

    const SkOpSpan& span(int tIndex) const {
        return fTs[tIndex];
    }

    const SkOpAngle* spanToAngle(int tStart, int tEnd) const {
        SkASSERT(tStart != tEnd);
        const SkOpSpan& span = fTs[tStart];
        return tStart < tEnd ? span.fToAngle : span.fFromAngle;
    }

    // FIXME: create some sort of macro or template that avoids casting
    SkOpAngle* spanToAngle(int tStart, int tEnd) {
        const SkOpAngle* cAngle = (const_cast<const SkOpSegment*>(this))->spanToAngle(tStart, tEnd);
        return const_cast<SkOpAngle*>(cAngle);
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

    double t(int tIndex) const {
        return fTs[tIndex].fT;
    }

    double tAtMid(int start, int end, double mid) const {
        return fTs[start].fT * (1 - mid) + fTs[end].fT * mid;
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

#if defined(SK_DEBUG) || DEBUG_WINDING
    SkScalar xAtT(int index) const {
        return xAtT(&fTs[index]);
    }
#endif

#if DEBUG_VALIDATE
    bool _xor() const {  // FIXME: used only by SkOpAngle::debugValidateLoop()
        return fXor;
    }
#endif

    const SkPoint& xyAtT(const SkOpSpan* span) const {
        return span->fPt;
    }

    const SkPoint& xyAtT(int index) const {
        return xyAtT(&fTs[index]);
    }

#if defined(SK_DEBUG) || DEBUG_WINDING
    SkScalar yAtT(int index) const {
        return yAtT(&fTs[index]);
    }
#endif

    const SkOpAngle* activeAngle(int index, int* start, int* end, bool* done,
                                 bool* sortable) const;
    SkPoint activeLeftTop(int* firstT) const;
    bool activeOp(int index, int endIndex, int xorMiMask, int xorSuMask, SkPathOp op);
    bool activeWinding(int index, int endIndex);
    void addCubic(const SkPoint pts[4], bool operand, bool evenOdd);
    void addCurveTo(int start, int end, SkPathWriter* path, bool active) const;
    void addEndSpan(int endIndex);
    void addLine(const SkPoint pts[2], bool operand, bool evenOdd);
    void addOtherT(int index, double otherT, int otherIndex);
    void addQuad(const SkPoint pts[3], bool operand, bool evenOdd);
    void addSimpleAngle(int endIndex);
    int addSelfT(const SkPoint& pt, double newT);
    void addStartSpan(int endIndex);
    int addT(SkOpSegment* other, const SkPoint& pt, double newT);
    void addTCancel(const SkPoint& startPt, const SkPoint& endPt, SkOpSegment* other);
    void addTCoincident(const SkPoint& startPt, const SkPoint& endPt, double endT,
                        SkOpSegment* other);
    const SkOpSpan* addTPair(double t, SkOpSegment* other, double otherT, bool borrowWind,
                             const SkPoint& pt);
    const SkOpSpan* addTPair(double t, SkOpSegment* other, double otherT, bool borrowWind,
                             const SkPoint& pt, const SkPoint& oPt);
    void alignMultiples(SkTDArray<AlignedSpan>* aligned);
    bool alignSpan(int index, double thisT, const SkPoint& thisPt);
    void alignSpanState(int start, int end);
    bool betweenTs(int lesser, double testT, int greater) const;
    void blindCancel(const SkCoincidence& coincidence, SkOpSegment* other);
    void blindCoincident(const SkCoincidence& coincidence, SkOpSegment* other);
    bool calcAngles();
    double calcMissingTEnd(const SkOpSegment* ref, double loEnd, double min, double max,
                           double hiEnd, const SkOpSegment* other, int thisEnd);
    double calcMissingTStart(const SkOpSegment* ref, double loEnd, double min, double max,
                             double hiEnd, const SkOpSegment* other, int thisEnd);
    void checkDuplicates();
    void checkEnds();
    void checkMultiples();
    void checkSmall();
    bool checkSmall(int index) const;
    void checkTiny();
    int computeSum(int startIndex, int endIndex, SkOpAngle::IncludeType includeType);
    bool containsPt(const SkPoint& , int index, int endIndex) const;
    int crossedSpanY(const SkPoint& basePt, SkScalar* bestY, double* hitT, bool* hitSomething,
                     double mid, bool opp, bool current) const;
    bool findCoincidentMatch(const SkOpSpan* span, const SkOpSegment* other, int oStart, int oEnd,
                             int step, SkPoint* startPt, SkPoint* endPt, double* endT) const;
    SkOpSegment* findNextOp(SkTDArray<SkOpSpan*>* chase, int* nextStart, int* nextEnd,
                            bool* unsortable, SkPathOp op, int xorMiMask, int xorSuMask);
    SkOpSegment* findNextWinding(SkTDArray<SkOpSpan*>* chase, int* nextStart, int* nextEnd,
                                 bool* unsortable);
    SkOpSegment* findNextXor(int* nextStart, int* nextEnd, bool* unsortable);
    int findExactT(double t, const SkOpSegment* ) const;
    int findOtherT(double t, const SkOpSegment* ) const;
    int findT(double t, const SkPoint& , const SkOpSegment* ) const;
    SkOpSegment* findTop(int* tIndex, int* endIndex, bool* unsortable, bool firstPass);
    void fixOtherTIndex();
    void initWinding(int start, int end, SkOpAngle::IncludeType angleIncludeType);
    void initWinding(int start, int end, double tHit, int winding, SkScalar hitDx, int oppWind,
                     SkScalar hitOppDx);
    bool isMissing(double startT, const SkPoint& pt) const;
    bool isTiny(const SkOpAngle* angle) const;
    bool joinCoincidence(SkOpSegment* other, double otherT, const SkPoint& otherPt, int step,
                         bool cancel);
    SkOpSpan* markAndChaseDoneBinary(int index, int endIndex);
    SkOpSpan* markAndChaseDoneUnary(int index, int endIndex);
    SkOpSpan* markAndChaseWinding(const SkOpAngle* angle, int winding, int oppWinding);
    SkOpSpan* markAngle(int maxWinding, int sumWinding, int oppMaxWinding, int oppSumWinding,
                        const SkOpAngle* angle);
    void markDone(int index, int winding);
    void markDoneBinary(int index);
    void markDoneUnary(int index);
    bool nextCandidate(int* start, int* end) const;
    int nextSpan(int from, int step) const;
    void pinT(const SkPoint& pt, double* t);
    void setUpWindings(int index, int endIndex, int* sumMiWinding, int* sumSuWinding,
            int* maxWinding, int* sumWinding, int* oppMaxWinding, int* oppSumWinding);
    void sortAngles();
    bool subDivide(int start, int end, SkPoint edge[4]) const;
    bool subDivide(int start, int end, SkDCubic* result) const;
    void undoneSpan(int* start, int* end);
    int updateOppWindingReverse(const SkOpAngle* angle) const;
    int updateWindingReverse(const SkOpAngle* angle) const;
    static bool UseInnerWinding(int outerWinding, int innerWinding);
    static bool UseInnerWindingReverse(int outerWinding, int innerWinding);
    int windingAtT(double tHit, int tIndex, bool crossOpp, SkScalar* dx) const;
    int windSum(const SkOpAngle* angle) const;
// available for testing only
#if defined(SK_DEBUG) || !FORCE_RELEASE
    int debugID() const {
        return fID;
    }
#else
    int debugID() const {
        return -1;
    }
#endif
#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
    void debugShowActiveSpans() const;
#endif
#if DEBUG_CONCIDENT
    void debugShowTs(const char* prefix) const;
#endif
#if DEBUG_SHOW_WINDING
    int debugShowWindingValues(int slotCount, int ofInterest) const;
#endif
    const SkTDArray<SkOpSpan>& debugSpans() const;
    void debugValidate() const;
    // available to testing only
    const SkOpAngle* debugLastAngle() const;
    void dumpAngles() const;
    void dumpContour(int firstID, int lastID) const;
    void dumpPts() const;
    void dumpSpans() const;

private:
    struct MissingSpan  {
        double fT;
        double fEndT;
        SkOpSegment* fSegment;
        SkOpSegment* fOther;
        double fOtherT;
        SkPoint fPt;
    };

    const SkOpAngle* activeAngleInner(int index, int* start, int* end, bool* done,
                                      bool* sortable) const;
    const SkOpAngle* activeAngleOther(int index, int* start, int* end, bool* done,
                                      bool* sortable) const;
    bool activeOp(int xorMiMask, int xorSuMask, int index, int endIndex, SkPathOp op,
                  int* sumMiWinding, int* sumSuWinding);
    bool activeWinding(int index, int endIndex, int* sumWinding);
    void addCancelOutsides(const SkPoint& startPt, const SkPoint& endPt, SkOpSegment* other);
    void addCoinOutsides(const SkPoint& startPt, const SkPoint& endPt, SkOpSegment* other);
    SkOpAngle* addSingletonAngleDown(SkOpSegment** otherPtr, SkOpAngle** );
    SkOpAngle* addSingletonAngleUp(SkOpSegment** otherPtr, SkOpAngle** );
    SkOpAngle* addSingletonAngles(int step);
    void alignSpan(const SkPoint& newPt, double newT, const SkOpSegment* other, double otherT,
                   const SkOpSegment* other2, SkOpSpan* oSpan, SkTDArray<AlignedSpan>* );
    bool betweenPoints(double midT, const SkPoint& pt1, const SkPoint& pt2) const;
    void bumpCoincidentBlind(bool binary, int index, int last);
    void bumpCoincidentThis(const SkOpSpan& oTest, bool binary, int* index,
                           SkTArray<SkPoint, true>* outsideTs);
    void bumpCoincidentOBlind(int index, int last);
    void bumpCoincidentOther(const SkOpSpan& oTest, int* index,
                           SkTArray<SkPoint, true>* outsideTs);
    bool bumpSpan(SkOpSpan* span, int windDelta, int oppDelta);
    bool calcLoopSpanCount(const SkOpSpan& thisSpan, int* smallCounts);
    bool checkForSmall(const SkOpSpan* span, const SkPoint& pt, double newT,
                       int* less, int* more) const;
    void checkLinks(const SkOpSpan* ,
                    SkTArray<MissingSpan, true>* missingSpans) const;
    static void CheckOneLink(const SkOpSpan* test, const SkOpSpan* oSpan,
                             const SkOpSpan* oFirst, const SkOpSpan* oLast,
                             const SkOpSpan** missingPtr,
                             SkTArray<MissingSpan, true>* missingSpans);
    int checkSetAngle(int tIndex) const;
    void checkSmallCoincidence(const SkOpSpan& span, SkTArray<MissingSpan, true>* );
    bool coincidentSmall(const SkPoint& pt, double t, const SkOpSegment* other) const;
    bool clockwise(int tStart, int tEnd, bool* swap) const;
    static void ComputeOneSum(const SkOpAngle* baseAngle, SkOpAngle* nextAngle,
                              SkOpAngle::IncludeType );
    static void ComputeOneSumReverse(const SkOpAngle* baseAngle, SkOpAngle* nextAngle,
                                     SkOpAngle::IncludeType );
    bool containsT(double t, const SkOpSegment* other, double otherT) const;
    bool decrementSpan(SkOpSpan* span);
    int findEndSpan(int endIndex) const;
    int findStartSpan(int startIndex) const;
    int firstActive(int tIndex) const;
    const SkOpSpan& firstSpan(const SkOpSpan& thisSpan) const;
    void init(const SkPoint pts[], SkPath::Verb verb, bool operand, bool evenOdd);
    bool inCoincidentSpan(double t, const SkOpSegment* other) const;
    bool inLoop(const SkOpAngle* baseAngle, int spanCount, int* indexPtr) const;
#if OLD_CHASE
    bool isSimple(int end) const;
#else
    SkOpSegment* isSimple(int* end, int* step);
#endif
    bool isTiny(int index) const;
    const SkOpSpan& lastSpan(const SkOpSpan& thisSpan) const;
    void matchWindingValue(int tIndex, double t, bool borrowWind);
    SkOpSpan* markAndChaseDone(int index, int endIndex, int winding);
    SkOpSpan* markAndChaseDoneBinary(const SkOpAngle* angle, int winding, int oppWinding);
    SkOpSpan* markAndChaseWinding(const SkOpAngle* angle, int winding);
    SkOpSpan* markAndChaseWinding(int index, int endIndex, int winding);
    SkOpSpan* markAndChaseWinding(int index, int endIndex, int winding, int oppWinding);
    SkOpSpan* markAngle(int maxWinding, int sumWinding, const SkOpAngle* angle);
    void markDoneBinary(int index, int winding, int oppWinding);
    SkOpSpan* markAndChaseDoneUnary(const SkOpAngle* angle, int winding);
    void markOneDone(const char* funName, int tIndex, int winding);
    void markOneDoneBinary(const char* funName, int tIndex);
    void markOneDoneBinary(const char* funName, int tIndex, int winding, int oppWinding);
    void markOneDoneUnary(const char* funName, int tIndex);
    SkOpSpan* markOneWinding(const char* funName, int tIndex, int winding);
    SkOpSpan* markOneWinding(const char* funName, int tIndex, int winding, int oppWinding);
    void markWinding(int index, int winding);
    void markWinding(int index, int winding, int oppWinding);
    bool monotonicInY(int tStart, int tEnd) const;

    bool multipleEnds() const { return fTs[count() - 2].fT == 1; }
    bool multipleStarts() const { return fTs[1].fT == 0; }

    SkOpSegment* nextChase(int* index, int* step, int* min, SkOpSpan** last);
    int nextExactSpan(int from, int step) const;
    bool serpentine(int tStart, int tEnd) const;
    void setCoincidentRange(const SkPoint& startPt, const SkPoint& endPt,  SkOpSegment* other);
    void setFromAngle(int endIndex, SkOpAngle* );
    void setToAngle(int endIndex, SkOpAngle* );
    void setUpWindings(int index, int endIndex, int* sumMiWinding,
            int* maxWinding, int* sumWinding);
    void subDivideBounds(int start, int end, SkPathOpsBounds* bounds) const;
    static void TrackOutsidePair(SkTArray<SkPoint, true>* outsideTs, const SkPoint& endPt,
            const SkPoint& startPt);
    static void TrackOutside(SkTArray<SkPoint, true>* outsideTs, const SkPoint& startPt);
    int updateOppWinding(int index, int endIndex) const;
    int updateOppWinding(const SkOpAngle* angle) const;
    int updateWinding(int index, int endIndex) const;
    int updateWinding(const SkOpAngle* angle) const;
    int updateWindingReverse(int index, int endIndex) const;
    SkOpSpan* verifyOneWinding(const char* funName, int tIndex);
    SkOpSpan* verifyOneWindingU(const char* funName, int tIndex);

    SkScalar xAtT(const SkOpSpan* span) const {
        return xyAtT(span).fX;
    }

    SkScalar yAtT(const SkOpSpan* span) const {
        return xyAtT(span).fY;
    }

    void zeroSpan(SkOpSpan* span);

#if DEBUG_SWAP_TOP
    bool controlsContainedByEnds(int tStart, int tEnd) const;
#endif
    void debugAddAngle(int start, int end);
#if DEBUG_CONCIDENT
    void debugAddTPair(double t, const SkOpSegment& other, double otherT) const;
#endif
#if DEBUG_ANGLE
    void debugCheckPointsEqualish(int tStart, int tEnd) const;
#endif
#if DEBUG_SWAP_TOP
    int debugInflections(int index, int endIndex) const;
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
    // available to testing only
    void debugConstruct();
    void debugConstructCubic(SkPoint shortQuad[4]);
    void debugConstructLine(SkPoint shortQuad[2]);
    void debugConstructQuad(SkPoint shortQuad[3]);
    void debugReset();
    void dumpDPts() const;
    void dumpSpan(int index) const;

    const SkPoint* fPts;
    SkPathOpsBounds fBounds;
    // FIXME: can't convert to SkTArray because it uses insert
    SkTDArray<SkOpSpan> fTs;  // 2+ (always includes t=0 t=1) -- at least (number of spans) + 1
    SkOpAngleSet fAngles;  // empty or 2+ -- (number of non-zero spans) * 2
    // OPTIMIZATION: could pack donespans, verb, operand, xor into 1 int-sized value
    int fDoneSpans;  // quick check that segment is finished
    // OPTIMIZATION: force the following to be byte-sized
    SkPath::Verb fVerb;
    bool fLoop;   // set if cubic intersects itself
    bool fMultiples;  // set if curve intersects multiple other curves at one interior point
    bool fOperand;
    bool fXor;  // set if original contour had even-odd fill
    bool fOppXor;  // set if opposite operand had even-odd fill
    bool fSmall;  // set if some span is small
    bool fTiny;  // set if some span is tiny
#if defined(SK_DEBUG) || !FORCE_RELEASE
    int fID;
#endif

    friend class PathOpsSegmentTester;
};

#endif
