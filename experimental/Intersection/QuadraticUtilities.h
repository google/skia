/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined QUADRATIC_UTILITIES_H
#define QUADRATIC_UTILITIES_H

#include "DataTypes.h"

int add_valid_ts(double s[], int realRoots, double* t);
void chop_at(const Quadratic& src, QuadraticPair& dst, double t);
double dx_at_t(const Quadratic& , double t);
double dy_at_t(const Quadratic& , double t);
//void dxdy_at_t(const Quadratic& , double t, _Point& xy);
_Vector dxdy_at_t(const Quadratic& , double t);

double nearestT(const Quadratic& , const _Point& );
bool point_in_hull(const Quadratic& , const _Point& );

/* Parameterization form, given A*t*t + 2*B*t*(1-t) + C*(1-t)*(1-t)
 *
 * a = A - 2*B +   C
 * b =     2*B - 2*C
 * c =             C
 */
inline void set_abc(const double* quad, double& a, double& b, double& c) {
    a = quad[0];     // a = A
    b = 2 * quad[2]; // b =     2*B
    c = quad[4];     // c =             C
    b -= c;          // b =     2*B -   C
    a -= b;          // a = A - 2*B +   C
    b -= c;          // b =     2*B - 2*C
}

int quadraticRootsReal(double A, double B, double C, double t[2]);
int quadraticRootsValidT(const double A, const double B, const double C, double s[2]);
void sub_divide(const Quadratic& src, double t1, double t2, Quadratic& dst);
_Point sub_divide(const Quadratic& src, const _Point& a, const _Point& c, double t1, double t2);
void toCubic(const Quadratic& , Cubic& );
_Point top(const Quadratic& , double startT, double endT);
void xy_at_t(const Quadratic& , double t, double& x, double& y);
_Point xy_at_t(const Quadratic& , double t);

#endif
