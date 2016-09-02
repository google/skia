/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsBounds.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsCurve.h"

 // this cheats and assumes that the perpendicular to the point is the closest ray to the curve
 // this case (where the line and the curve are nearly coincident) may be the only case that counts
double SkDCurve::nearPoint(SkPath::Verb verb, const SkDPoint& xy, const SkDPoint& opp) const {
    int count = SkPathOpsVerbToPoints(verb);
    double minX = fCubic.fPts[0].fX;
    double maxX = minX;
    for (int index = 0; index < count; ++index) {
        minX = SkTMin(minX, fCubic.fPts[index].fX);
        maxX = SkTMax(maxX, fCubic.fPts[index].fX);
    }
    if (!AlmostBetweenUlps(minX, xy.fX, maxX)) {
        return -1;
    }
    double minY = fCubic.fPts[0].fY;
    double maxY = minY;
    for (int index = 0; index < count; ++index) {
        minY = SkTMin(minY, fCubic.fPts[index].fY);
        maxY = SkTMax(maxY, fCubic.fPts[index].fY);
    }
    if (!AlmostBetweenUlps(minY, xy.fY, maxY)) {
        return -1;
    }
    SkIntersections i;
    SkDLine perp = {{ xy, { xy.fX + opp.fY - xy.fY, xy.fY + xy.fX - opp.fX }}};
    (*CurveDIntersectRay[verb])(*this, perp, &i);
    int minIndex = -1;
    double minDist = FLT_MAX;
    for (int index = 0; index < i.used(); ++index) {
        double dist = xy.distance(i.pt(index));
        if (minDist > dist) {
            minDist = dist;
            minIndex = index;
        }
    }
    if (minIndex < 0) {
        return -1;
    }
    double largest = SkTMax(SkTMax(maxX, maxY), -SkTMin(minX, minY));
    if (!AlmostEqualUlps_Pin(largest, largest + minDist)) { // is distance within ULPS tolerance?
        return -1;
    }
    return SkPinT(i[0][minIndex]);
}

void SkDCurve::offset(SkPath::Verb verb, const SkDVector& off) {
    int count = SkPathOpsVerbToPoints(verb);
    for (int index = 0; index < count; ++index) {
        fCubic.fPts[index] += off;
    }
}

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

void SkDCurve::setQuadBounds(const SkPoint curve[3], SkScalar ,
        double tStart, double tEnd, SkPathOpsBounds* bounds) {
    SkDQuad dCurve;
    dCurve.set(curve);
    SkDRect dRect;
    dRect.setBounds(dCurve, fQuad, tStart, tEnd);
    bounds->set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}
