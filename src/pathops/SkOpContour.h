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

class SkIntersections;
class SkOpContour;
class SkPathWriter;

struct SkCoincidence {
    SkOpContour* fContours[2];
    int fSegments[2];
    double fTs[2][2];
    SkPoint fPts[2];
};

class SkOpContour {
public:
    SkOpContour() {
        reset();
#if DEBUG_DUMP
        fID = ++gContourID;
#endif
    }

    bool operator<(const SkOpContour& rh) const {
        return fBounds.fTop == rh.fBounds.fTop
                ? fBounds.fLeft < rh.fBounds.fLeft
                : fBounds.fTop < rh.fBounds.fTop;
    }

    void addCoincident(int index, SkOpContour* other, int otherIndex,
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

    int addQuad(const SkPoint pts[3]) {
        fSegments.push_back().addQuad(pts, fOperand, fXor);
        fContainsCurves = true;
        return fSegments.count();
    }

    int addT(int segIndex, SkOpContour* other, int otherIndex, const SkPoint& pt, double newT) {
        setContainsIntercepts();
        return fSegments[segIndex].addT(&other->fSegments[otherIndex], pt, newT);
    }

    int addSelfT(int segIndex, SkOpContour* other, int otherIndex, const SkPoint& pt, double newT) {
        setContainsIntercepts();
        return fSegments[segIndex].addSelfT(&other->fSegments[otherIndex], pt, newT);
    }

    int addUnsortableT(int segIndex, SkOpContour* other, int otherIndex, bool start,
                       const SkPoint& pt, double newT) {
        return fSegments[segIndex].addUnsortableT(&other->fSegments[otherIndex], start, pt, newT);
    }

    const SkPathOpsBounds& bounds() const {
        return fBounds;
    }

    void calcCoincidentWinding();

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
            fSegments[sIndex].checkEnds();
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

    void findTooCloseToCall() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].findTooCloseToCall();
        }
    }

    void fixOtherTIndex() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].fixOtherTIndex();
        }
    }

    SkOpSegment* nonVerticalSegment(int* start, int* end);

    bool operand() const {
        return fOperand;
    }

    void reset() {
        fSegments.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fContainsCurves = fContainsCubics = fContainsIntercepts = fDone = false;
    }

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

private:
    void setBounds();

    SkTArray<SkOpSegment> fSegments;
    SkTArray<SkOpSegment*, true> fSortedSegments;
    int fFirstSorted;
    SkTArray<SkCoincidence, true> fCoincidences;
    SkTArray<const SkOpContour*, true> fCrosses;
    SkPathOpsBounds fBounds;
    bool fContainsIntercepts;  // FIXME: is this used by anybody?
    bool fContainsCubics;
    bool fContainsCurves;
    bool fDone;
    bool fOperand;  // true for the second argument to a binary operator
    bool fXor;
    bool fOppXor;
#if DEBUG_DUMP
    int fID;
#endif
};

#endif
