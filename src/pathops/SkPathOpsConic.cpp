/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkLineParameters.h"
#include "SkPathOpsConic.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsQuad.h"

// cribbed from the float version in SkGeometry.cpp
static void conic_deriv_coeff(const double src[],
                              SkScalar w,
                              double coeff[3]) {
    const double P20 = src[4] - src[0];
    const double P10 = src[2] - src[0];
    const double wP10 = w * P10;
    coeff[0] = w * P20 - P20;
    coeff[1] = P20 - 2 * wP10;
    coeff[2] = wP10;
}

static double conic_eval_tan(const double coord[], SkScalar w, double t) {
    double coeff[3];
    conic_deriv_coeff(coord, w, coeff);
    return t * (t * coeff[0] + coeff[1]) + coeff[2];
}

int SkDConic::FindExtrema(const double src[], SkScalar w, double t[1]) {
    double coeff[3];
    conic_deriv_coeff(src, w, coeff);

    double tValues[2];
    int roots = SkDQuad::RootsValidT(coeff[0], coeff[1], coeff[2], tValues);
    SkASSERT(0 == roots || 1 == roots);

    if (1 == roots) {
        t[0] = tValues[0];
        return 1;
    }
    return 0;
}

SkDVector SkDConic::dxdyAtT(double t) const {
    SkDVector result = {
        conic_eval_tan(&fPts[0].fX, fWeight, t),
        conic_eval_tan(&fPts[0].fY, fWeight, t)
    };
    return result;
}

static double conic_eval_numerator(const double src[], SkScalar w, double t) {
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= 1);
    double src2w = src[2] * w;
    double C = src[0];
    double A = src[4] - 2 * src2w + C;
    double B = 2 * (src2w - C);
    return (A * t + B) * t + C;
}


static double conic_eval_denominator(SkScalar w, double t) {
    double B = 2 * (w - 1);
    double C = 1;
    double A = -B;
    return (A * t + B) * t + C;
}

bool SkDConic::hullIntersects(const SkDCubic& cubic, bool* isLinear) const {
    return cubic.hullIntersects(*this, isLinear);
}

SkDPoint SkDConic::ptAtT(double t) const {
    if (t == 0) {
        return fPts[0];
    }
    if (t == 1) {
        return fPts[2];
    }
    double denominator = conic_eval_denominator(fWeight, t);
    SkDPoint result = {
        conic_eval_numerator(&fPts[0].fX, fWeight, t) / denominator,
        conic_eval_numerator(&fPts[0].fY, fWeight, t) / denominator
    };
    return result;
}

/* see quad subdivide for rationale */
SkDConic SkDConic::subDivide(double t1, double t2) const {
    double ax, ay, az;
    if (t1 == 0) {
        ax = fPts[0].fX;
        ay = fPts[0].fY;
        az = 1;
    } else if (t1 != 1) {
        ax = conic_eval_numerator(&fPts[0].fX, fWeight, t1);
        ay = conic_eval_numerator(&fPts[0].fY, fWeight, t1);
        az = conic_eval_denominator(fWeight, t1);
    } else {
        ax = fPts[2].fX;
        ay = fPts[2].fY;
        az = 1;
    }
    double midT = (t1 + t2) / 2;
    double dx = conic_eval_numerator(&fPts[0].fX, fWeight, midT);
    double dy = conic_eval_numerator(&fPts[0].fY, fWeight, midT);
    double dz = conic_eval_denominator(fWeight, midT);
    double cx, cy, cz;
    if (t2 == 1) {
        cx = fPts[2].fX;
        cy = fPts[2].fY;
        cz = 1;
    } else if (t2 != 0) {
        cx = conic_eval_numerator(&fPts[0].fX, fWeight, t2);
        cy = conic_eval_numerator(&fPts[0].fY, fWeight, t2);
        cz = conic_eval_denominator(fWeight, t2);
    } else {
        cx = fPts[0].fX;
        cy = fPts[0].fY;
        cz = 1;
    }
    double bx = 2 * dx - (ax + cx) / 2;
    double by = 2 * dy - (ay + cy) / 2;
    double bz = 2 * dz - (az + cz) / 2;
    double dt = t2 - t1;
    double dt_1 = 1 - dt;
    SkScalar w = SkDoubleToScalar((1 + dt * (fWeight - 1))
            / sqrt(dt * dt + 2 * dt * dt_1 * fWeight + dt_1 * dt_1));
    SkDConic dst = {{{{ax / az, ay / az}, {bx / bz, by / bz}, {cx / cz, cy / cz}}}, w };
    return dst;
}

SkDPoint SkDConic::subDivide(const SkDPoint& a, const SkDPoint& c, double t1, double t2,
        SkScalar* weight) const {
    SkDConic chopped = this->subDivide(t1, t2);
    *weight = chopped.fWeight;
    return chopped[1];
}
