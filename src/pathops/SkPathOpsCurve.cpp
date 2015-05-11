/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsBounds.h"
#include "SkPathOpsRect.h"
#include "SkPathOpsCurve.h"

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
