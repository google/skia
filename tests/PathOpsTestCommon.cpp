/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsBounds.h"
#include "SkPathOpsConic.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkReduceOrder.h"
#include "SkTSort.h"

static double calc_t_div(const SkDCubic& cubic, double precision, double start) {
    const double adjust = sqrt(3.) / 36;
    SkDCubic sub;
    const SkDCubic* cPtr;
    if (start == 0) {
        cPtr = &cubic;
    } else {
        // OPTIMIZE: special-case half-split ?
        sub = cubic.subDivide(start, 1);
        cPtr = &sub;
    }
    const SkDCubic& c = *cPtr;
    double dx = c[3].fX - 3 * (c[2].fX - c[1].fX) - c[0].fX;
    double dy = c[3].fY - 3 * (c[2].fY - c[1].fY) - c[0].fY;
    double dist = sqrt(dx * dx + dy * dy);
    double tDiv3 = precision / (adjust * dist);
    double t = SkDCubeRoot(tDiv3);
    if (start > 0) {
        t = start + (1 - start) * t;
    }
    return t;
}

static bool add_simple_ts(const SkDCubic& cubic, double precision, SkTArray<double, true>* ts) {
    double tDiv = calc_t_div(cubic, precision, 0);
    if (tDiv >= 1) {
        return true;
    }
    if (tDiv >= 0.5) {
        ts->push_back(0.5);
        return true;
    }
    return false;
}

static void addTs(const SkDCubic& cubic, double precision, double start, double end,
        SkTArray<double, true>* ts) {
    double tDiv = calc_t_div(cubic, precision, 0);
    double parts = ceil(1.0 / tDiv);
    for (double index = 0; index < parts; ++index) {
        double newT = start + (index / parts) * (end - start);
        if (newT > 0 && newT < 1) {
            ts->push_back(newT);
        }
    }
}

static void toQuadraticTs(const SkDCubic* cubic, double precision, SkTArray<double, true>* ts) {
    SkReduceOrder reducer;
    int order = reducer.reduce(*cubic, SkReduceOrder::kAllow_Quadratics);
    if (order < 3) {
        return;
    }
    double inflectT[5];
    int inflections = cubic->findInflections(inflectT);
    SkASSERT(inflections <= 2);
    if (!cubic->endsAreExtremaInXOrY()) {
        inflections += cubic->findMaxCurvature(&inflectT[inflections]);
        SkASSERT(inflections <= 5);
    }
    SkTQSort<double>(inflectT, &inflectT[inflections - 1]);
    // OPTIMIZATION: is this filtering common enough that it needs to be pulled out into its
    // own subroutine?
    while (inflections && approximately_less_than_zero(inflectT[0])) {
        memmove(inflectT, &inflectT[1], sizeof(inflectT[0]) * --inflections);
    }
    int start = 0;
    int next = 1;
    while (next < inflections) {
        if (!approximately_equal(inflectT[start], inflectT[next])) {
            ++start;
        ++next;
            continue;
        }
        memmove(&inflectT[start], &inflectT[next], sizeof(inflectT[0]) * (--inflections - start));
    }

    while (inflections && approximately_greater_than_one(inflectT[inflections - 1])) {
        --inflections;
    }
    SkDCubicPair pair;
    if (inflections == 1) {
        pair = cubic->chopAt(inflectT[0]);
        int orderP1 = reducer.reduce(pair.first(), SkReduceOrder::kNo_Quadratics);
        if (orderP1 < 2) {
            --inflections;
        } else {
            int orderP2 = reducer.reduce(pair.second(), SkReduceOrder::kNo_Quadratics);
            if (orderP2 < 2) {
                --inflections;
            }
        }
    }
    if (inflections == 0 && add_simple_ts(*cubic, precision, ts)) {
        return;
    }
    if (inflections == 1) {
        pair = cubic->chopAt(inflectT[0]);
        addTs(pair.first(), precision, 0, inflectT[0], ts);
        addTs(pair.second(), precision, inflectT[0], 1, ts);
        return;
    }
    if (inflections > 1) {
        SkDCubic part = cubic->subDivide(0, inflectT[0]);
        addTs(part, precision, 0, inflectT[0], ts);
        int last = inflections - 1;
        for (int idx = 0; idx < last; ++idx) {
            part = cubic->subDivide(inflectT[idx], inflectT[idx + 1]);
            addTs(part, precision, inflectT[idx], inflectT[idx + 1], ts);
        }
        part = cubic->subDivide(inflectT[last], 1);
        addTs(part, precision, inflectT[last], 1, ts);
        return;
    }
    addTs(*cubic, precision, 0, 1, ts);
}

void CubicToQuads(const SkDCubic& cubic, double precision, SkTArray<SkDQuad, true>& quads) {
    SkTArray<double, true> ts;
    toQuadraticTs(&cubic, precision, &ts);
    if (ts.count() <= 0) {
        SkDQuad quad = cubic.toQuad();
        quads.push_back(quad);
        return;
    }
    double tStart = 0;
    for (int i1 = 0; i1 <= ts.count(); ++i1) {
        const double tEnd = i1 < ts.count() ? ts[i1] : 1;
        SkDRect bounds;
        bounds.setBounds(cubic);
        SkDCubic part = cubic.subDivide(tStart, tEnd);
        SkDQuad quad = part.toQuad();
        if (quad[1].fX < bounds.fLeft) {
            quad[1].fX = bounds.fLeft;
        } else if (quad[1].fX > bounds.fRight) {
            quad[1].fX = bounds.fRight;
        }
        if (quad[1].fY < bounds.fTop) {
            quad[1].fY = bounds.fTop;
        } else if (quad[1].fY > bounds.fBottom) {
            quad[1].fY = bounds.fBottom;
        }
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

bool ValidConic(const SkDConic& conic) {
    for (int index = 0; index < SkDConic::kPointCount; ++index) {
        if (!ValidPoint(conic[index])) {
            return false;
        }
    }
    if (SkDoubleIsNaN(conic.fWeight)) {
        return false;
    }
    return true;
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

bool ValidVector(const SkDVector& v) {
    if (SkDoubleIsNaN(v.fX)) {
        return false;
    }
    return !SkDoubleIsNaN(v.fY);
}
