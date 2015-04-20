/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsBounds.h"
#include "SkPathOpsConic.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"

void SkPathOpsBounds::setConicBounds(const SkPoint a[3], SkScalar weight) {
    SkDConic conic;
    conic.set(a, weight);
    SkDRect dRect;
    dRect.setBounds(conic);
    set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}

void SkPathOpsBounds::setCubicBounds(const SkPoint a[4], SkScalar ) {
    SkDCubic cubic;
    cubic.set(a);
    SkDRect dRect;
    dRect.setBounds(cubic);
    set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}

void SkPathOpsBounds::setLineBounds(const SkPoint a[2], SkScalar ) {
    setPointBounds(a[0]);
    add(a[1]);
}

void SkPathOpsBounds::setQuadBounds(const SkPoint a[3], SkScalar ) {
    SkDQuad quad;
    quad.set(a);
    SkDRect dRect;
    dRect.setBounds(quad);
    set(SkDoubleToScalar(dRect.fLeft), SkDoubleToScalar(dRect.fTop),
            SkDoubleToScalar(dRect.fRight), SkDoubleToScalar(dRect.fBottom));
}

void (SkPathOpsBounds::* const SetCurveBounds[])(const SkPoint[], SkScalar weight) = {
    NULL,
    &SkPathOpsBounds::setLineBounds,
    &SkPathOpsBounds::setQuadBounds,
    &SkPathOpsBounds::setConicBounds,
    &SkPathOpsBounds::setCubicBounds
};
