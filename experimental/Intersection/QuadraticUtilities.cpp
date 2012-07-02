#include "QuadraticUtilities.h"
#include <math.h>

int quadraticRoots(double A, double B, double C, double t[2]) {
    B *= 2;
    double square = B * B - 4 * A * C;
    if (square < 0) {
        return 0;
    }
    double squareRt = sqrt(square);
    double Q = (B + (B < 0 ? -squareRt : squareRt)) / -2;
    double ratio;
    int foundRoots = 0;
    if ((Q <= A) ^ (Q < 0)) {
        ratio = Q / A;
        if (!isnan(ratio)) {
            t[foundRoots++] = ratio;
        }
    }
    if ((C <= Q) ^ (C < 0)) {
        ratio = C / Q;
        if (!isnan(ratio)) {
            t[foundRoots++] = ratio;
        }
    }
    return foundRoots;
}

void dxdy_at_t(const Quadratic& quad, double t, double& x, double& y) {
    double a = t - 1;
    double b = 1 - 2 * t;
    double c = t;
    if (&x) {
        x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    }
    if (&y) {
        y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
    }
}

void xy_at_t(const Quadratic& quad, double t, double& x, double& y) {
    double one_t = 1 - t;
    double a = one_t * one_t;
    double b = 2 * one_t * t;
    double c = t * t;
    if (&x) {
        x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    }
    if (&y) {
        y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
    }
}
