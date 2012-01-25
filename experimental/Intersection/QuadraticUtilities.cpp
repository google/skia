#include "QuadraticUtilities.h"

int quadraticRoots(double A, double B, double C, double t[2]) {
    B *= 2;
    double square = B * B - 4 * A * C;
    if (square < 0) {
        return 0;
    }
    double squareRt = sqrt(square);
    double Q = (B + (B < 0 ? -squareRt : squareRt)) / -2;
    int foundRoots = 0;
    if ((Q <= A) ^ (Q < 0)) {
        t[foundRoots++] = Q / A;
    }
    if ((C <= Q) ^ (C < 0)) {
        t[foundRoots++] = C / Q;
    }
    return foundRoots;
}


