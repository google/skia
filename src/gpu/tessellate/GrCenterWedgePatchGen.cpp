/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrCenterWedgePatchGen.h"

#include "include/core/SkPath.h"
#include "src/core/SkPathPriv.h"

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    return a * (1 - T) + b * T;
}

int GrCenterWedgePatchGen::walkPath(std::array<SkPoint, 5>* wedgeData,
                                    SkTArray<SkPoint, true>* contourMidpoints) {
    fWedgeData = (std::array<SkPoint, 5>*)wedgeData;
    fContourMidpoints = contourMidpoints;

    fLastPt = {0,0};
    fCurrContourInitialPatchCount = 0;
    if (fWedgeData) {
        fCurrContourMidpoint = fContourMidpoints->begin();
    } else {
        SkASSERT(fContourMidpoints->empty());
        fCurrContourFanPtsSum = {0,0};
    }
    fNumWedges = 0;

    const SkPoint* pts = SkPathPriv::PointData(fPath);
    SkPoint contourStartPt = {0,0};
    int ptsIdx = 0;
    for (SkPath::Verb verb : SkPathPriv::Verbs(fPath)) {
        switch (verb) {
            case SkPath::kMove_Verb:
                this->close(contourStartPt);
                contourStartPt = fLastPt = pts[ptsIdx++];
                continue;
            case SkPath::kClose_Verb:
                fLastPt = this->close(contourStartPt);
                continue;
            case SkPath::kLine_Verb:
                fLastPt = this->lineTo(pts[ptsIdx++]);
                continue;
            case SkPath::kQuad_Verb:
                fLastPt = this->quadraticTo(pts[ptsIdx], pts[ptsIdx + 1]);
                ptsIdx += 2;
                continue;
            case SkPath::kCubic_Verb:
                fLastPt = this->cubicTo(pts[ptsIdx], pts[ptsIdx + 1], pts[ptsIdx + 2]);
                ptsIdx += 3;
                continue;
            case SkPath::kConic_Verb:
                // TODO: Eventually we want to use rational cubic wedges in order to support
                // perspective and conics.
            default:
                SK_ABORT("Unexpected path verb.");
        }
    }
    this->close(contourStartPt);
    SkASSERT(!fWedgeData || fCurrContourMidpoint == fContourMidpoints->end());
    return fNumWedges;
}

inline SkPoint GrCenterWedgePatchGen::lineTo(const SkPoint& p1) {
    if (fWedgeData) {
        fWedgeData[fNumWedges] = {
                fLastPt, lerp(fLastPt, p1, 1/3.f), lerp(fLastPt, p1, 2/3.f), p1,
                *fCurrContourMidpoint};
    } else {
        fCurrContourFanPtsSum += p1;
    }
    ++fNumWedges;
    return p1;
}

inline SkPoint GrCenterWedgePatchGen::quadraticTo(const SkPoint& p1, const SkPoint& p2) {
    if (fWedgeData) {
        fWedgeData[fNumWedges] = {
                fLastPt, lerp(fLastPt, p1, 2/3.f), lerp(p1, p2, 1/3.f), p2, *fCurrContourMidpoint};
    } else {
        fCurrContourFanPtsSum += p2;
    }
    ++fNumWedges;
    return p2;
}

inline SkPoint GrCenterWedgePatchGen::cubicTo(const SkPoint& p1, const SkPoint& p2,
                                              const SkPoint& p3) {
    if (fWedgeData) {
        fWedgeData[fNumWedges] = {fLastPt, p1, p2, p3, *fCurrContourMidpoint};
    } else {
        fCurrContourFanPtsSum += p3;
    }
    ++fNumWedges;
    return p3;
}

inline SkPoint GrCenterWedgePatchGen::close(const SkPoint& startPt) {
    if (fLastPt != startPt) {
        this->lineTo(startPt);
    }
    if (fWedgeData) {
        ++fCurrContourMidpoint;
    } else {
        int numPatchesInContour = fNumWedges - fCurrContourInitialPatchCount;
        fContourMidpoints->push_back(fCurrContourFanPtsSum * (1.f/numPatchesInContour));
        fCurrContourInitialPatchCount = fNumWedges;
        fCurrContourFanPtsSum = {0,0};
    }
    return startPt;
}
