/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "IntersectionUtilities.h"

/*
Given a quadratic q, t1, and t2, find a small quadratic segment.

The new quadratic is defined by A, B, and C, where
 A = c[0]*(1 - t1)*(1 - t1) + 2*c[1]*t1*(1 - t1) + c[2]*t1*t1
 C = c[3]*(1 - t1)*(1 - t1) + 2*c[2]*t1*(1 - t1) + c[1]*t1*t1

To find B, compute the point halfway between t1 and t2:

q(at (t1 + t2)/2) == D

Next, compute where D must be if we know the value of B:

_12 = A/2 + B/2
12_ = B/2 + C/2
123 = A/4 + B/2 + C/4
    = D

Group the known values on one side:

B   = D*2 - A/2 - C/2
*/

static double interp_quad_coords(const double* src, double t)
{
    double ab = interp(src[0], src[2], t);
    double bc = interp(src[2], src[4], t);
    double abc = interp(ab, bc, t);
    return abc;
}

void sub_divide(const Quadratic& src, double t1, double t2, Quadratic& dst) {
    double ax = dst[0].x = interp_quad_coords(&src[0].x, t1);
    double ay = dst[0].y = interp_quad_coords(&src[0].y, t1);
    double dx = interp_quad_coords(&src[0].x, (t1 + t2) / 2);
    double dy = interp_quad_coords(&src[0].y, (t1 + t2) / 2);
    double cx = dst[2].x = interp_quad_coords(&src[0].x, t2);
    double cy = dst[2].y = interp_quad_coords(&src[0].y, t2);
    /* bx = */ dst[1].x = 2*dx - (ax + cx)/2;
    /* by = */ dst[1].y = 2*dy - (ay + cy)/2;
}

/* classic one t subdivision */
static void interp_quad_coords(const double* src, double* dst, double t)
{
    double ab = interp(src[0], src[2], t);
    double bc = interp(src[2], src[4], t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = interp(ab, bc, t);
    dst[6] = bc;
    dst[8] = src[4];
}

void chop_at(const Quadratic& src, QuadraticPair& dst, double t)
{
    interp_quad_coords(&src[0].x, &dst.pts[0].x, t);
    interp_quad_coords(&src[0].y, &dst.pts[0].y, t);
}
