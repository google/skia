/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpContour_DEFINED
#define SkOpContour_DEFINED

#include "SkOpSegment.h"
#include "SkTArray.h"

#if defined(SK_DEBUG) || !FORCE_RELEASE
#include "SkThread.h"
#endif

class SkIntersections;
class SkOpContour;
class SkPathWriter;

struct SkCoincidence {
    SkOpContour* fOther;
    int fSegments[2];
    double fTs[2][2];
    SkPoint fPts[2][2];
    int fNearly[2];
};

class SkOpContour {
public:
    SkOpContour() {
        reset();
#if defined(SK_DEBUG) || !FORCE_RELEASE
        fID = sk_atomic_inc(&SkPathOpsDebug::gContourID);
#endif
    }

    bool operator<(const SkOpContour& rh) const {
        return fBounds.fTop == rh.fBounds.fTop
                ? fBounds.fLeft < rh.fBounds.fLeft
                : fBounds.fTop < rh.fBounds.fTop;
    }

    bool addCoincident(int index, SkOpContour* other, int otherIndex,
                       const SkIntersections& ts, bool swap);
    void addCoincidentPoints();

    void addCross(const SkOpContour* crosser) {
#ifdef DEBUG_CROSS
        for (int index = 0; index < fCrosses.count(); ++index) {
            SkASSERT(fCrosses[index] != crosser);
        }
#endif
        fCrosses.push_back(crosser);
    }

    void addCubic(const SkPoint pts[4]) {
        fSegments.push_back().addCubic(pts, fOperand, fXor);
        fContainsCurves = fContainsCubics = true;
    }

    int addLine(const SkPoint pts[2]) {
        fSegments.push_back().addLine(pts, fOperand, fXor);
        return fSegments.count();
    }

    void addOtherT(int segIndex, int tIndex, double otherT, int otherIndex) {
        fSegments[segIndex].addOtherT(tIndex, otherT, otherIndex);
    }

    bool addPartialCoincident(int index, SkOpContour* other, int otherIndex,
                       const SkIntersections& ts, int ptIndex, bool swap);

    int addQuad(const SkPoint pts[3]) {
        fSegments.push_back().addQuad(pts, fOperand, fXor);
        fContainsCurves = true;
        return fSegments.count();
    }

    int addT(int segIndex, SkOpContour* other, int otherIndex, const SkPoint& pt, double newT) {
        setContainsIntercepts();
        return fSegments[segIndex].addT(&other->fSegments[otherIndex], pt, newT);
    }

    int addSelfT(int segIndex, const SkPoint& pt, double newT) {
        setContainsIntercepts();
        return fSegments[segIndex].addSelfT(pt, newT);
    }

    void align(const SkOpSegment::AlignedSpan& aligned, bool swap, SkCoincidence* coincidence);
    void alignCoincidence(const SkOpSegment::AlignedSpan& aligned,
            SkTArray<SkCoincidence, true>* coincidences);

    void alignCoincidence(const SkOpSegment::AlignedSpan& aligned) {
        alignCoincidence(aligned, &fCoincidences);
        alignCoincidence(aligned, &fPartialCoincidences);
    }

    void alignMultiples(SkTDArray<SkOpSegment::AlignedSpan>* aligned) {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            SkOpSegment& segment = fSegments[sIndex];
            if (segment.hasMultiples()) {
                segment.alignMultiples(aligned);
            }
        }
    }

    void alignTPt(int segmentIndex, const SkOpContour* other, int otherIndex,
                  bool swap, int tIndex, SkIntersections* ts, SkPoint* point) const;

    const SkPathOpsBounds& bounds() const {
        return fBounds;
    }

    bool calcAngles();
    void calcCoincidentWinding();
    void calcPartialCoincidentWinding();

    void checkDuplicates() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            SkOpSegment& segment = fSegments[sIndex];
            if (segment.count() > 2) {
                segment.checkDuplicates();
            }
        }
    }

    void checkEnds() {
        if (!fContainsCurves) {
            return;
        }
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            SkOpSegment* segment = &fSegments[sIndex];
            if (segment->verb() == SkPath::kLine_Verb) {
                continue;
            }
            if (segment->done()) {
                continue;   // likely coincident, nothing to do
            }
            segment->checkEnds();
        }
    }

    void checkMultiples() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            SkOpSegment& segment = fSegments[sIndex];
            if (segment.count() > 2) {
                segment.checkMultiples();
                fMultiples |= segment.hasMultiples();
            }
        }
    }

    void checkSmall() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            SkOpSegment& segment = fSegments[sIndex];
            // OPTIMIZATION : skip segments that are done?
            if (segment.hasSmall()) {
                segment.checkSmall();
            }
        }
    }

    // if same point has different T values, choose a common T
    void checkTiny() {
        int segmentCount = fSegments.count();
        if (segmentCount <= 2) {
            return;
        }
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            SkOpSegment& segment = fSegments[sIndex];
            if (segment.hasTiny()) {
                segment.checkTiny();
            }
        }
    }

    void complete() {
        setBounds();
        fContainsIntercepts = false;
    }

    bool containsCubics() const {
        return fContainsCubics;
    }

    bool crosses(const SkOpContour* crosser) const {
        for (int index = 0; index < fCrosses.count(); ++index) {
            if (fCrosses[index] == crosser) {
                return true;
            }
        }
        return false;
    }

    bool done() const {
        return fDone;
    }

    const SkPoint& end() const {
        const SkOpSegment& segment = fSegments.back();
        return segment.pts()[SkPathOpsVerbToPoints(segment.verb())];
    }

    void fixOtherTIndex() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].fixOtherTIndex();
        }
    }

    bool hasMultiples() const {
        return fMultiples;
    }

    void joinCoincidence() {
        joinCoincidence(fCoincidences, false);
        joinCoincidence(fPartialCoincidences, true);
    }

    SkOpSegment* nonVerticalSegment(int* start, int* end);

    bool operand() const {
        return fOperand;
    }

    void reset() {
        fSegments.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fContainsCurves = fContainsCubics = fContainsIntercepts = fDone = fMultiples = false;
    }

    void resolveNearCoincidence();

    SkTArray<SkOpSegment>& segments() {
        return fSegments;
    }

    void setContainsIntercepts() {
        fContainsIntercepts = true;
    }

    void setOperand(bool isOp) {
        fOperand = isOp;
    }

    void setOppXor(bool isOppXor) {
        fOppXor = isOppXor;
        int segmentCount = fSegments.count();
        for (int test = 0; test < segmentCount; ++test) {
            fSegments[test].setOppXor(isOppXor);
        }
    }

    void setXor(bool isXor) {
        fXor = isXor;
    }

    void sortAngles();
    void sortSegments();

    const SkPoint& start() const {
        return fSegments.front().pts()[0];
    }

    void toPath(SkPathWriter* path) const;

    void toPartialBackward(SkPathWriter* path) const {
        int segmentCount = fSegments.count();
        for (int test = segmentCount - 1; test >= 0; --test) {
            fSegments[test].addCurveTo(1, 0, path, true);
        }
    }

    void toPartialForward(SkPathWriter* path) const {
        int segmentCount = fSegments.count();
        for (int test = 0; test < segmentCount; ++test) {
            fSegments[test].addCurveTo(0, 1, path, true);
        }
    }

    void topSortableSegment(const SkPoint& topLeft, SkPoint* bestXY, SkOpSegment** topStart);
    SkOpSegment* undoneSegment(int* start, int* end);

    int updateSegment(int index, const SkPoint* pts) {
        SkOpSegment& segment = fSegments[index];
        segment.updatePts(pts);
        return SkPathOpsVerbToPoints(segment.verb()) + 1;
    }

#if DEBUG_TEST
    SkTArray<SkOpSegment>& debugSegments() {
        return fSegments;
    }
#endif

#if DEBUG_ACTIVE_SPANS || DEBUG_ACTIVE_SPANS_FIRST_ONLY
    void debugShowActiveSpans() {
        for (int index = 0; index < fSegments.count(); ++index) {
            fSegments[index].debugShowActiveSpans();
        }
    }
#endif

#if DEBUG_SHOW_WINDING
    int debugShowWindingValues(int totalSegments, int ofInterest);
    static void debugShowWindingValues(const SkTArray<SkOpContour*, true>& contourList);
#endif

    // available to test routines only
    void dump() const;
    void dumpAngles() const;
    void dumpCoincidence(const SkCoincidence& ) const;
    void dumpCoincidences() const;
    void dumpPt(int ) const;
    void dumpPts() const;
    void dumpSpan(int ) const;
    void dumpSpans() const;

private:
    void alignPt(int index, SkPoint* point, int zeroPt) const;
    int alignT(bool swap, int tIndex, SkIntersections* ts) const;
    void calcCommonCoincidentWinding(const SkCoincidence& );
    void checkCoincidentPair(const SkCoincidence& oneCoin, int oneIdx,
                             const SkCoincidence& twoCoin, int twoIdx, bool partial);
    void joinCoincidence(const SkTArray<SkCoincidence, true>& , bool partial);
    void setBounds();

    SkTArray<SkOpSegment> fSegments;
    SkTArray<SkOpSegment*, true> fSortedSegments;
    int fFirstSorted;
    SkTArray<SkCoincidence, true> fCoincidences;
    SkTArray<SkCoincidence, true> fPartialCoincidences;
    SkTArray<const SkOpContour*, true> fCrosses;
    SkPathOpsBounds fBounds;
    bool fContainsIntercepts;  // FIXME: is this used by anybody?
    bool fContainsCubics;
    bool fContainsCurves;
    bool fDone;
    bool fMultiples;  // set if some segment has multiple identical intersections with other curves
    bool fOperand;  // true for the second argument to a binary operator
    bool fXor;
    bool fOppXor;
#if defined(SK_DEBUG) || !FORCE_RELEASE
    int debugID() const { return fID; }
    int fID;
#else
    int debugID() const { return -1; }
#endif
};

#endif
