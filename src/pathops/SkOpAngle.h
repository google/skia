/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpAngle_DEFINED
#define SkOpAngle_DEFINED

#include "SkLineParameters.h"
#include "SkOpSpan.h"
#include "SkPath.h"
#include "SkPathOpsCubic.h"
#include "SkTDArray.h"

// sorting angles
// given angles of {dx dy ddx ddy dddx dddy} sort them
class SkOpAngle {
public:
    bool operator<(const SkOpAngle& rh) const;

    double dx() const {
        return fTangent1.dx();
    }

    double dy() const {
        return fTangent1.dy();
    }

    int end() const {
        return fEnd;
    }

    bool isHorizontal() const {
        return dy() == 0 && fVerb == SkPath::kLine_Verb;
    }

    bool lengthen();
    bool reverseLengthen();

    void set(const SkPoint* orig, SkPath::Verb verb, const SkOpSegment* segment,
            int start, int end, const SkTDArray<SkOpSpan>& spans);

    void setSpans();

    SkOpSegment* segment() const {
        return const_cast<SkOpSegment*>(fSegment);
    }

    int sign() const {
        return SkSign32(fStart - fEnd);
    }

    const SkTDArray<SkOpSpan>* spans() const {
        return fSpans;
    }

    int start() const {
        return fStart;
    }

    bool unsortable() const {
        return fUnsortable;
    }

#if DEBUG_ANGLE
    const SkPoint* pts() const {
        return fPts;
    }

    SkPath::Verb verb() const {
        return fVerb;
    }

    void debugShow(const SkPoint& a) const {
        SkDebugf("    d=(%1.9g,%1.9g) side=%1.9g\n", dx(), dy(), fSide);
    }
#endif

private:
    const SkPoint* fPts;
    SkDCubic fCurvePart;
    SkPath::Verb fVerb;
    double fSide;
    SkLineParameters fTangent1;
    const SkTDArray<SkOpSpan>* fSpans;
    const SkOpSegment* fSegment;
    int fStart;
    int fEnd;
    bool fReversed;
    mutable bool fUnsortable;  // this alone is editable by the less than operator
};

#endif
