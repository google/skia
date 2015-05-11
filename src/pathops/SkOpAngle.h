/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpAngle_DEFINED
#define SkOpAngle_DEFINED

#include "SkLineParameters.h"
#include "SkPathOpsCurve.h"
#if DEBUG_ANGLE
#include "SkString.h"
#endif

class SkOpContour;
class SkOpPtT;
class SkOpSegment;
class SkOpSpanBase;
class SkOpSpan;

struct SkOpAngle {
    enum IncludeType {
        kUnaryWinding,
        kUnaryXor,
        kBinarySingle,
        kBinaryOpp,
    };

    bool after(SkOpAngle* test);
    int allOnOneSide(const SkOpAngle* test);
    bool checkCrossesZero() const;
    void checkNearCoincidence();
    bool checkParallel(SkOpAngle* );
    bool computeSector();
    int convexHullOverlaps(const SkOpAngle* ) const;

    const SkOpAngle* debugAngle(int id) const;
    SkOpContour* debugContour(int id);

    int debugID() const {
        return SkDEBUGRELEASE(fID, -1);
    }

#if DEBUG_SORT
    void debugLoop() const;
#endif

#if DEBUG_ANGLE
    SkString debugPart() const;
#endif
    const SkOpPtT* debugPtT(int id) const;
    const SkOpSegment* debugSegment(int id) const;
    int debugSign() const;
    const SkOpSpanBase* debugSpan(int id) const;
    void debugValidate() const; 
    void debugValidateNext() const;  // in debug builds, verify that angle loop is uncorrupted
    double distEndRatio(double dist) const;
    // available to testing only
    void dump() const;
    void dumpCurves() const;
    void dumpLoop() const;
    void dumpOne(bool functionHeader) const;
    void dumpTo(const SkOpSegment* fromSeg, const SkOpAngle* ) const;
    void dumpTest() const;

    SkOpSpanBase* end() const {
        return fEnd;
    }

    bool endsIntersect(SkOpAngle* );
    bool endToSide(const SkOpAngle* rh, bool* inside) const;
    int findSector(SkPath::Verb verb, double x, double y) const;
    SkOpGlobalState* globalState() const;
    void insert(SkOpAngle* );
    SkOpSpanBase* lastMarked() const;
    bool loopContains(const SkOpAngle* ) const;
    int loopCount() const;
    bool merge(SkOpAngle* );
    double midT() const;
    bool midToSide(const SkOpAngle* rh, bool* inside) const;

    SkOpAngle* next() const {
        return fNext;
    }

    bool oppositePlanes(const SkOpAngle* rh) const;
    bool orderable(SkOpAngle* rh);  // false == this < rh ; true == this > rh
    SkOpAngle* previous() const;

    int sectorEnd() const {
        return fSectorEnd;
    }

    int sectorStart() const {
        return fSectorStart;
    }

    SkOpSegment* segment() const;

    void set(SkOpSpanBase* start, SkOpSpanBase* end);
    void setCurveHullSweep();

    void setID(int id) {
        SkDEBUGCODE(fID = id);
    }

    void setLastMarked(SkOpSpanBase* marked) {
        fLastMarked = marked;
    }

    void setSector();
    void setSpans();

    SkOpSpanBase* start() const {
        return fStart;
    }

    SkOpSpan* starter();
    bool tangentsDiverge(const SkOpAngle* rh, double s0xt0) const;

    bool unorderable() const {
        return fUnorderable;
    }

    SkDCurve fCurvePart;  // the curve from start to end
    double fSide;
    SkLineParameters fTangentHalf;  // used only to sort a pair of lines or line-like sections
    SkOpAngle* fNext;
    SkOpSpanBase* fLastMarked;
    SkDVector fSweep[2];
    SkOpSpanBase* fStart;
    SkOpSpanBase* fEnd;
    SkOpSpanBase* fComputedEnd;
    int fSectorMask;
    int8_t fSectorStart;  // in 32nds of a circle
    int8_t fSectorEnd;
    bool fIsCurve;
    bool fUnorderable;
    bool fUnorderedSweep;  // set when a cubic's first control point between the sweep vectors
    bool fComputeSector;
    bool fComputedSector;
    bool fCheckCoincidence;
    SkDEBUGCODE(int fID);

};



#endif
