/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsTypes_DEFINED
#define SkPathOpsTypes_DEFINED

#include <float.h>  // for FLT_EPSILON
#include <math.h>   // for fabs, sqrt

#include "SkFloatingPoint.h"
#include "SkPathOps.h"
#include "SkPathOpsDebug.h"
#include "SkScalar.h"

enum SkPathOpsMask {
    kWinding_PathOpsMask = -1,
    kNo_PathOpsMask = 0,
    kEvenOdd_PathOpsMask = 1
};

// Use Almost Equal when comparing coordinates. Use epsilon to compare T values.
extern bool AlmostEqualUlps(float A, float B);
inline bool AlmostEqualUlps(double A, double B) {
    return AlmostEqualUlps(SkDoubleToScalar(A), SkDoubleToScalar(B));
}

// FLT_EPSILON == 1.19209290E-07 == 1 / (2 ^ 23)
// DBL_EPSILON == 2.22045e-16
const double FLT_EPSILON_CUBED = FLT_EPSILON * FLT_EPSILON * FLT_EPSILON;
const double FLT_EPSILON_HALF = FLT_EPSILON / 2;
const double FLT_EPSILON_SQUARED = FLT_EPSILON * FLT_EPSILON;
const double FLT_EPSILON_SQRT = sqrt(FLT_EPSILON);
const double FLT_EPSILON_INVERSE = 1 / FLT_EPSILON;
const double DBL_EPSILON_ERR = DBL_EPSILON * 4;  // FIXME: tune -- allow a few bits of error
const double ROUGH_EPSILON = FLT_EPSILON * 64;
const double MORE_ROUGH_EPSILON = FLT_EPSILON * 256;

inline bool approximately_zero(double x) {
    return fabs(x) < FLT_EPSILON;
}

inline bool precisely_zero(double x) {
    return fabs(x) < DBL_EPSILON_ERR;
}

inline bool approximately_zero(float x) {
    return fabs(x) < FLT_EPSILON;
}

inline bool approximately_zero_cubed(double x) {
    return fabs(x) < FLT_EPSILON_CUBED;
}

inline bool approximately_zero_half(double x) {
    return fabs(x) < FLT_EPSILON_HALF;
}

inline bool approximately_zero_squared(double x) {
    return fabs(x) < FLT_EPSILON_SQUARED;
}

inline bool approximately_zero_sqrt(double x) {
    return fabs(x) < FLT_EPSILON_SQRT;
}

inline bool approximately_zero_inverse(double x) {
    return fabs(x) > FLT_EPSILON_INVERSE;
}

// OPTIMIZATION: if called multiple times with the same denom, we want to pass 1/y instead
inline bool approximately_zero_when_compared_to(double x, double y) {
    return x == 0 || fabs(x / y) < FLT_EPSILON;
}

// Use this for comparing Ts in the range of 0 to 1. For general numbers (larger and smaller) use
// AlmostEqualUlps instead.
inline bool approximately_equal(double x, double y) {
    return approximately_zero(x - y);
}

inline bool precisely_equal(double x, double y) {
    return precisely_zero(x - y);
}

inline bool approximately_equal_half(double x, double y) {
    return approximately_zero_half(x - y);
}

inline bool approximately_equal_squared(double x, double y) {
    return approximately_equal(x, y);
}

inline bool approximately_greater(double x, double y) {
    return x - FLT_EPSILON >= y;
}

inline bool approximately_greater_or_equal(double x, double y) {
    return x + FLT_EPSILON > y;
}

inline bool approximately_lesser(double x, double y) {
    return x + FLT_EPSILON <= y;
}

inline bool approximately_lesser_or_equal(double x, double y) {
    return x - FLT_EPSILON < y;
}

inline double approximately_pin(double x) {
    return approximately_zero(x) ? 0 : x;
}

inline float approximately_pin(float x) {
    return approximately_zero(x) ? 0 : x;
}

inline bool approximately_greater_than_one(double x) {
    return x > 1 - FLT_EPSILON;
}

inline bool precisely_greater_than_one(double x) {
    return x > 1 - DBL_EPSILON_ERR;
}

inline bool approximately_less_than_zero(double x) {
    return x < FLT_EPSILON;
}

inline bool precisely_less_than_zero(double x) {
    return x < DBL_EPSILON_ERR;
}

inline bool approximately_negative(double x) {
    return x < FLT_EPSILON;
}

inline bool precisely_negative(double x) {
    return x < DBL_EPSILON_ERR;
}

inline bool approximately_one_or_less(double x) {
    return x < 1 + FLT_EPSILON;
}

inline bool approximately_positive(double x) {
    return x > -FLT_EPSILON;
}

inline bool approximately_positive_squared(double x) {
    return x > -(FLT_EPSILON_SQUARED);
}

inline bool approximately_zero_or_more(double x) {
    return x > -FLT_EPSILON;
}

inline bool approximately_between(double a, double b, double c) {
    return a <= c ? approximately_negative(a - b) && approximately_negative(b - c)
            : approximately_negative(b - a) && approximately_negative(c - b);
}

inline bool precisely_between(double a, double b, double c) {
    return a <= c ? precisely_negative(a - b) && precisely_negative(b - c)
            : precisely_negative(b - a) && precisely_negative(c - b);
}

// returns true if (a <= b <= c) || (a >= b >= c)
inline bool between(double a, double b, double c) {
    SkASSERT(((a <= b && b <= c) || (a >= b && b >= c)) == ((a - b) * (c - b) <= 0));
    return (a - b) * (c - b) <= 0;
}

inline bool more_roughly_equal(double x, double y) {
    return fabs(x - y) < MORE_ROUGH_EPSILON;
}

inline bool roughly_equal(double x, double y) {
    return fabs(x - y) < ROUGH_EPSILON;
}

struct SkDPoint;
struct SkDVector;
struct SkDLine;
struct SkDQuad;
struct SkDTriangle;
struct SkDCubic;
struct SkDRect;

inline double SkDInterp(double A, double B, double t) {
    return A + (B - A) * t;
}

double SkDCubeRoot(double x);

/* Returns -1 if negative, 0 if zero, 1 if positive
*/
inline int SkDSign(double x) {
    return (x > 0) - (x < 0);
}

/* Returns 0 if negative, 1 if zero, 2 if positive
*/
inline int SKDSide(double x) {
    return (x > 0) + (x >= 0);
}

/* Returns 1 if negative, 2 if zero, 4 if positive
*/
inline int SkDSideBit(double x) {
    return 1 << SKDSide(x);
}

#endif
