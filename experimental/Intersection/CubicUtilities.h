/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#if !defined CUBIC_UTILITIES_H
#define CUBIC_UTILITIES_H

#include "DataTypes.h"
#include "SkTDArray.h"

double calcPrecision(const Cubic& cubic);
#if SK_DEBUG
double calcPrecision(const Cubic& cubic, double t, double scale);
#endif
void chop_at(const Cubic& src, CubicPair& dst, double t);
bool clockwise(const Cubic& c);
double cube_root(double x);
int cubic_to_quadratics(const Cubic& cubic, double precision,
        SkTDArray<Quadratic>& quadratics);
void cubic_to_quadratics(const Cubic& cubic, double precision, SkTDArray<double>& ts);
void coefficients(const double* cubic, double& A, double& B, double& C, double& D);
bool controls_contained_by_ends(const Cubic& c);
int cubicRootsValidT(double A, double B, double C, double D, double t[3]);
int cubicRootsReal(double A, double B, double C, double D, double s[3]);
void demote_cubic_to_quad(const Cubic& cubic, Quadratic& quad);
double dx_at_t(const Cubic& , double t);
double dy_at_t(const Cubic& , double t);
//void dxdy_at_t(const Cubic& , double t, _Point& y);
_Vector dxdy_at_t(const Cubic& cubic, double t);
bool ends_are_extrema_in_x_or_y(const Cubic& );
int find_cubic_inflections(const Cubic& src, double tValues[]);
int find_cubic_max_curvature(const Cubic& src, double tValues[]);
bool monotonic_in_y(const Cubic& c);
bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath);
bool serpentine(const Cubic& c);
void sub_divide(const Cubic& src, double t1, double t2, Cubic& dst);
void sub_divide(const Cubic& , const _Point& a, const _Point& d, double t1, double t2, _Point [2]);
_Point top(const Cubic& , double startT, double endT);
void xy_at_t(const Cubic& , double t, double& x, double& y);
_Point xy_at_t(const Cubic& , double t);

extern const int gPrecisionUnit;

#endif
