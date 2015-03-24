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

void CubicPathToQuads(const SkPath& cubicPath, SkPath* quadPath) {
    quadPath->reset();
    SkDCubic cubic;
    SkTArray<SkDQuad, true> quads;
    SkPath::RawIter iter(cubicPath);
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                quadPath->moveTo(pts[0].fX, pts[0].fY);
                continue;
            case SkPath::kLine_Verb:
                quadPath->lineTo(pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                quadPath->quadTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                quads.reset();
                cubic.set(pts);
                CubicToQuads(cubic, cubic.calcPrecision(), quads);
                for (int index = 0; index < quads.count(); ++index) {
                    SkPoint qPts[2] = {
                        quads[index][1].asSkPoint(),
                        quads[index][2].asSkPoint()
                    };
                    quadPath->quadTo(qPts[0].fX, qPts[0].fY, qPts[1].fX, qPts[1].fY);
                }
                break;
            case SkPath::kClose_Verb:
                 quadPath->close();
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
}

void CubicPathToSimple(const SkPath& cubicPath, SkPath* simplePath) {
    simplePath->reset();
    SkDCubic cubic;
    SkPath::RawIter iter(cubicPath);
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                simplePath->moveTo(pts[0].fX, pts[0].fY);
                continue;
            case SkPath::kLine_Verb:
                simplePath->lineTo(pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                simplePath->quadTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb: {
                cubic.set(pts);
                double tInflects[2];
                int inflections = cubic.findInflections(tInflects);
                if (inflections > 1 && tInflects[0] > tInflects[1]) {
                    SkTSwap(tInflects[0], tInflects[1]);
                }
                double lo = 0;
                for (int index = 0; index <= inflections; ++index) {
                    double hi = index < inflections ? tInflects[index] : 1;
                    SkDCubic part = cubic.subDivide(lo, hi);
                    SkPoint cPts[3];
                    cPts[0] = part[1].asSkPoint();
                    cPts[1] = part[2].asSkPoint();
                    cPts[2] = part[3].asSkPoint();
                    simplePath->cubicTo(cPts[0].fX, cPts[0].fY, cPts[1].fX, cPts[1].fY,
                            cPts[2].fX, cPts[2].fY);
                    lo = hi;
                }
                break;
            } 
            case SkPath::kClose_Verb:
                 simplePath->close();
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
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
