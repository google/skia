/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpAngle_DEFINED
#define SkOpAngle_DEFINED

#include "SkLineParameters.h"
#include "SkPath.h"
#include "SkPathOpsCubic.h"

class SkOpSegment;
struct SkOpSpan;

// sorting angles
// given angles of {dx dy ddx ddy dddx dddy} sort them
class SkOpAngle {
public:
    enum { kStackBasedCount = 8 }; // FIXME: determine what this should be
    enum IncludeType {
        kUnaryWinding,
        kUnaryXor,
        kBinarySingle,
        kBinaryOpp,
    };

    bool operator<(const SkOpAngle& rh) const;

    bool calcSlop(double x, double y, double rx, double ry, bool* result) const;

    double dx() const {
        return fTangentPart.dx();
    }

    double dy() const {
        return fTangentPart.dy();
    }

    int end() const {
        return fEnd;
    }

    bool isHorizontal() const;

    SkOpSpan* lastMarked() const {
        return fLastMarked;
    }

    void set(const SkOpSegment* segment, int start, int end);

    void setLastMarked(SkOpSpan* marked) {
        fLastMarked = marked;
    }

    SkOpSegment* segment() const {
        return const_cast<SkOpSegment*>(fSegment);
    }

    int sign() const {
        return SkSign32(fStart - fEnd);
    }

    int start() const {
        return fStart;
    }

    bool unorderable() const {
        return fUnorderable;
    }

    bool unsortable() const {
        return fUnsortable;
    }

#ifdef SK_DEBUG
    void dump() const;
#endif

#if DEBUG_ANGLE
    void setID(int id) {
        fID = id;
    }
#endif

private:
    bool lengthen(const SkOpAngle& );
    void setSpans();

    SkDCubic fCurvePart; // the curve from start to end
    SkDCubic fCurveHalf; // the curve from start to 1 or 0
    double fSide;
    double fSide2;
    SkLineParameters fTangentPart;
    SkLineParameters fTangentHalf;
    const SkOpSegment* fSegment;
    SkOpSpan* fLastMarked;
    int fStart;
    int fEnd;
    bool fComputed; // tangent is computed, may contain some error
    // if subdividing a quad or cubic causes the tangent to go from the maximum angle to the
    // minimum, mark it unorderable. It still can be sorted, which is good enough for find-top
    // but can't be ordered, and therefore can't be used to compute winding
    bool fUnorderable;
    mutable bool fUnsortable;  // this alone is editable by the less than operator
#if DEBUG_ANGLE
    int fID;
#endif
};

#endif
