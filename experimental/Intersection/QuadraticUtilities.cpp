#include "QuadraticUtilities.h"
#include <math.h>

/*

Numeric Solutions (5.6) suggests to solve the quadratic by computing

       Q = -1/2(B + sgn(B)Sqrt(B^2 - 4 A C))

and using the roots

      t1 = Q / A
      t2 = C / Q
      
*/

int quadraticRoots(double A, double B, double C, double t[2]) {
    B *= 2;
    double square = B * B - 4 * A * C;
    if (square < 0) {
        return 0;
    }
    double squareRt = sqrt(square);
    double Q = (B + (B < 0 ? -squareRt : squareRt)) / -2;
    int foundRoots = 0;
    double ratio = Q / A;
    if (ratio > -FLT_EPSILON && ratio < 1 + FLT_EPSILON) {
        if (ratio < FLT_EPSILON) {
            ratio = 0;
        } else if (ratio > 1 - FLT_EPSILON) {
            ratio = 1;
        }
        t[foundRoots++] = ratio;
    }
    ratio = C / Q;
    if (ratio > -FLT_EPSILON && ratio < 1 + FLT_EPSILON) {
        if (ratio < FLT_EPSILON) {
            ratio = 0;
        } else if (ratio > 1 - FLT_EPSILON) {
            ratio = 1;
        }
        t[foundRoots++] = ratio;
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
