/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsBounds.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkPathOpsTriangle.h"

void CubicToQuads(const SkDCubic& cubic, double precision, SkTArray<SkDQuad, true>& quads) {
    SkTArray<double, true> ts;
    cubic.toQuadraticTs(precision, &ts);
    if (ts.count() <= 0) {
        SkDQuad quad = cubic.toQuad();
        quads.push_back(quad);
        return;
    }
    double tStart = 0;
    for (int i1 = 0; i1 <= ts.count(); ++i1) {
        const double tEnd = i1 < ts.count() ? ts[i1] : 1;
        SkDCubic part = cubic.subDivide(tStart, tEnd);
        SkDQuad quad = part.toQuad();
        quads.push_back(quad);
        tStart = tEnd;
    }
}

static bool SkDoubleIsNaN(double x) {
    return x != x;
}

bool ValidBounds(const SkPathOpsBounds& bounds) {
    if (SkScalarIsNaN(bounds.fLeft)) {
        return false;
    }
    if (SkScalarIsNaN(bounds.fTop)) {
        return false;
    }
    if (SkScalarIsNaN(bounds.fRight)) {
        return false;
    }
    return !SkScalarIsNaN(bounds.fBottom);
}

bool ValidCubic(const SkDCubic& cubic) {
    for (int index = 0; index < 4; ++index) {
        if (!ValidPoint(cubic[index])) {
            return false;
        }
    }
    return true;
}

bool ValidLine(const SkDLine& line) {
    for (int index = 0; index < 2; ++index) {
        if (!ValidPoint(line[index])) {
            return false;
        }
    }
    return true;
}

bool ValidPoint(const SkDPoint& pt) {
    if (SkDoubleIsNaN(pt.fX)) {
        return false;
    }
    return !SkDoubleIsNaN(pt.fY);
}

bool ValidPoints(const SkPoint* pts, int count) {
    for (int index = 0; index < count; ++index) {
        if (SkScalarIsNaN(pts[index].fX)) {
            return false;
        }
        if (SkScalarIsNaN(pts[index].fY)) {
            return false;
        }
    }
    return true;
}

bool ValidQuad(const SkDQuad& quad) {
    for (int index = 0; index < 3; ++index) {
        if (!ValidPoint(quad[index])) {
            return false;
        }
    }
    return true;
}

bool ValidTriangle(const SkDTriangle& triangle) {
    for (int index = 0; index < 3; ++index) {
        if (!ValidPoint(triangle.fPts[index])) {
            return false;
        }
    }
    return true;
}

bool ValidVector(const SkDVector& v) {
    if (SkDoubleIsNaN(v.fX)) {
        return false;
    }
    return !SkDoubleIsNaN(v.fY);
}
