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

// sorting angles
// given angles of {dx dy ddx ddy dddx dddy} sort them
class SkOpAngle {
public:
    enum { kStackBasedCount = 8 }; // FIXME: determine what this should be

    bool operator<(const SkOpAngle& rh) const;

    bool calcSlop(double x, double y, double rx, double ry, bool* result) const;

    double dx() const {
        return fTangent1.dx();
    }

    double dy() const {
        return fTangent1.dy();
    }

    int end() const {
        return fEnd;
    }

    bool isHorizontal() const;

    void set(const SkOpSegment* segment, int start, int end);

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

#if DEBUG_ANGLE
    void debugShow(const SkPoint& a) const {
        SkDebugf("    d=(%1.9g,%1.9g) side=%1.9g\n", dx(), dy(), fSide);
    }

    void setID(int id) {
        fID = id;
    }
#endif

private:
    bool lengthen(const SkOpAngle& );
    void setSpans();

    SkDCubic fCurvePart;
    double fSide;
    SkLineParameters fTangent1;
    const SkOpSegment* fSegment;
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
