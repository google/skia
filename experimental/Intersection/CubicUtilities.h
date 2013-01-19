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
double cube_root(double x);
int cubic_to_quadratics(const Cubic& cubic, double precision,
        SkTDArray<Quadratic>& quadratics);
void cubic_to_quadratics(const Cubic& cubic, double precision, SkTDArray<double>& ts);
void coefficients(const double* cubic, double& A, double& B, double& C, double& D);
int cubicRoots(double A, double B, double C, double D, double t[3]);
int cubicRootsX(double A, double B, double C, double D, double s[3]);
void demote_cubic_to_quad(const Cubic& cubic, Quadratic& quad);
double dx_at_t(const Cubic& , double t);
double dy_at_t(const Cubic& , double t);
void dxdy_at_t(const Cubic& , double t, _Point& y);
int find_cubic_inflections(const Cubic& src, double tValues[]);
bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath);
void xy_at_t(const Cubic& , double t, double& x, double& y);

extern const int precisionUnit;
#endif
