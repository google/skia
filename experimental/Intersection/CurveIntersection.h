/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef CurveIntersection_DEFINE
#define CurveIntersection_DEFINE

#include "DataTypes.h"

class Intersections;

// unit-testable utilities
double axialIntersect(const Quadratic& q1, const _Point& p, bool vert);
bool bezier_clip(const Cubic& cubic1, const Cubic& cubic2, double& minT, double& maxT);
bool bezier_clip(const Quadratic& q1, const Quadratic& q2, double& minT, double& maxT);
void chop_at(const Cubic& src, CubicPair& dst, double t);
void chop_at(const Quadratic& src, QuadraticPair& dst, double t);
int convex_hull(const Cubic& cubic, char order[4]);
bool convex_x_hull(const Cubic& cubic, char connectTo0[2], char connectTo3[2]);
bool implicit_matches(const Cubic& cubic1, const Cubic& cubic2);
bool implicit_matches(const _Line& line1, const _Line& line2);
bool implicit_matches_ulps(const _Line& one, const _Line& two, int ulps);
bool implicit_matches(const Quadratic& quad1, const Quadratic& quad2);
void sub_divide(const Cubic& src, double t1, double t2, Cubic& dst);
void sub_divide(const _Line& src, double t1, double t2, _Line& dst);
void sub_divide(const Quadratic& src, double t1, double t2, Quadratic& dst);
void tangent(const Cubic& cubic, double t, _Point& result);
void tangent(const _Line& line, _Point& result);
void tangent(const Quadratic& quad, double t, _Point& result);

// main functions
enum ReduceOrder_Flags {
    kReduceOrder_NoQuadraticsAllowed,
    kReduceOrder_QuadraticsAllowed
};
int reduceOrder(const Cubic& cubic, Cubic& reduction, ReduceOrder_Flags );
int reduceOrder(const _Line& line, _Line& reduction);
int reduceOrder(const Quadratic& quad, Quadratic& reduction);
int horizontalIntersect(const Cubic& cubic, double y, double tRange[3]);
int horizontalIntersect(const Cubic& cubic, double left, double right, double y,
        double tRange[3]);
int horizontalIntersect(const Cubic& cubic, double left, double right, double y,
        bool flipped, Intersections&);
int horizontalIntersect(const _Line& line, double left, double right,
        double y, bool flipped, Intersections& );
int horizontalIntersect(const Quadratic& quad, double left, double right,
        double y, double tRange[2]);
int horizontalIntersect(const Quadratic& quad, double left, double right,
        double y, bool flipped, Intersections& );
bool intersect(const Cubic& cubic1, const Cubic& cubic2, Intersections& );
int intersect(const Cubic& cubic, const _Line& line, double cRange[3], double lRange[3]);
bool intersect(const Quadratic& q1, const Quadratic& q2, Intersections& );
int intersect(const Quadratic& quad, const _Line& line, Intersections& );
bool isLinear(const Quadratic& quad, int startIndex, int endIndex);
bool isLinear(const Cubic& cubic, int startIndex, int endIndex);
double leftMostT(const Cubic& , double startT, double endT);
double leftMostT(const _Line& , double startT, double endT);
double leftMostT(const Quadratic& , double startT, double endT);
int verticalIntersect(const Cubic& cubic, double top, double bottom, double x,
        bool flipped, Intersections& );
int verticalIntersect(const _Line& line, double top, double bottom, double x,
        bool flipped, Intersections& );
int verticalIntersect(const Quadratic& quad, double top, double bottom,
        double x, bool flipped, Intersections& );

#endif
