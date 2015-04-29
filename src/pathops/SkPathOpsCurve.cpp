/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsBounds.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsCurve.h"

SkDPoint SkDCurve::conicTop(const SkPoint curve[3], SkScalar curveWeight, 
        double startT, double endT, double* topT) {
    SkDPoint topPt = fConic[0];
    *topT = startT;
    if (topPt.fY > fConic[2].fY || (topPt.fY == fConic[2].fY && topPt.fX > fConic[2].fX)) {
        *topT = endT;
        topPt = fConic[2];
    }
    if (!fConic.monotonicInY()) {
        double extremeT;
        if (SkDConic::FindExtrema(&fConic.fPts.fPts[0].fY, fConic.fWeight, &extremeT)) {
            SkDConic dCurve;
            dCurve.set(curve, curveWeight);
            extremeT = startT + (endT - startT) * extremeT;
            SkDPoint test = dCurve.ptAtT(extremeT);
            if (topPt.fY > test.fY || (topPt.fY == test.fY && topPt.fX > test.fX)) {
                *topT = extremeT;
                topPt = test;
            }
        }
    }
    return topPt;
}

SkDPoint SkDCurve::cubicTop(const SkPoint curve[4], SkScalar ,
        double startT, double endT, double* topT) {
    SkDPoint topPt = fCubic[0];
    *topT = startT;
    if (topPt.fY > fCubic[3].fY || (topPt.fY == fCubic[3].fY && topPt.fX > fCubic[3].fX)) {
        *topT = endT;
        topPt = fCubic[3];
    }
    double extremeTs[2];
    if (!fCubic.monotonicInY()) {
        int roots = SkDCubic::FindExtrema(&fCubic.fPts[0].fY, extremeTs);
        SkDCubic dCurve;
        dCurve.set(curve);
        for (int index = 0; index < roots; ++index) {
            double t = startT + (endT - startT) * extremeTs[index];
            SkDPoint mid = dCurve.ptAtT(t);
            if (topPt.fY > mid.fY || (topPt.fY == mid.fY && topPt.fX > mid.fX)) {
                *topT = t;
                topPt = mid;
            }
        }
    }
    return topPt;
}

SkDPoint SkDCurve::lineTop(const SkPoint[2], SkScalar , double startT, double endT, double* topT) {
    SkDPoint topPt = fLine[0];
    *topT = startT;
    if (topPt.fY > fLine[1].fY || (topPt.fY == fLine[1].fY && topPt.fX > fLine[1].fX)) {
        *topT = endT;
        topPt = fLine[1];
    }
    return topPt;
}

SkDPoint SkDCurve::quadTop(const SkPoint curve[3], SkScalar ,
        double startT, double endT, double* topT) {
    SkDPoint topPt = fQuad[0];
    *topT = startT;
    if (topPt.fY > fQuad[2].fY || (topPt.fY == fQuad[2].fY && topPt.fX > fQuad[2].fX)) {
        *topT = endT;
        topPt = fQuad[2];
    }
    if (!fQuad.monotonicInY()) {
        double extremeT;
        if (SkDQuad::FindExtrema(&fQuad.fPts[0].fY, &extremeT)) {
            SkDQuad dCurve;
            dCurve.set(curve);
            extremeT = startT + (endT - startT) * extremeT;
            SkDPoint test = dCurve.ptAtT(extremeT);
            if (topPt.fY > test.fY || (topPt.fY == test.fY && topPt.fX > test.fX)) {
                *topT = extremeT;
                topPt = test;
            }
        }
    }
    return topPt;
}

SkDPoint (SkDCurve::* const Top[])(const SkPoint curve[], SkScalar curveWeight,
        double tStart, double tEnd, double* topT) = {
    NULL,
    &SkDCurve::lineTop,
    &SkDCurve::quadTop,
    &SkDCurve::conicTop,
    &SkDCurve::cubicTop
};

void SkDCurve::setConicBounds(const SkPoint curve[3], SkScalar curveWeight,
        double tStart, double tEnd, SkPathOpsBounds* bounds) {
    SkDConic dCurve;
    dCurve.set(curve, curveWeight);
    SkDRect dRect;
    dRect.setBounds(dCurve, fConic, tStart, tEnd);
    bounds->set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}

void SkDCurve::setCubicBounds(const SkPoint curve[4], SkScalar , 
        double tStart, double tEnd, SkPathOpsBounds* bounds) {
    SkDCubic dCurve;
    dCurve.set(curve);
    SkDRect dRect;
    dRect.setBounds(dCurve, fCubic, tStart, tEnd);
    bounds->set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}

void SkDCurve::setLineBounds(const SkPoint[2], SkScalar , 
        double , double , SkPathOpsBounds* bounds) {
    bounds->setPointBounds(fLine[0]);
    bounds->add(fLine[1]);
}

void SkDCurve::setQuadBounds(const SkPoint curve[3], SkScalar ,
        double tStart, double tEnd, SkPathOpsBounds* bounds) {
    SkDQuad dCurve;
    dCurve.set(curve);
    SkDRect dRect;
    dRect.setBounds(dCurve, fQuad, tStart, tEnd);
    bounds->set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}

void (SkDCurve::* const SetBounds[])(const SkPoint curve[], SkScalar curveWeight,
        double tStart, double tEnd, SkPathOpsBounds* bounds) = {
    NULL,
    &SkDCurve::setLineBounds,
    &SkDCurve::setQuadBounds,
    &SkDCurve::setConicBounds,
    &SkDCurve::setCubicBounds
};
