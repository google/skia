/*
 *  QuadraticIntersection_TestData.cpp
 *  edge
 *
 *  Created by Cary Clark on 1/10/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "QuadraticIntersection_TestData.h"

const Quadratic quadraticLines[] = {
    {{0, 0}, {0, 0}, {1, 0}},
    {{0, 0}, {1, 0}, {0, 0}},
    {{1, 0}, {0, 0}, {0, 0}},
    {{1, 0}, {2, 0}, {3, 0}},
    {{0, 0}, {0, 0}, {0, 1}},
    {{0, 0}, {0, 1}, {0, 0}},
    {{0, 1}, {0, 0}, {0, 0}},
    {{0, 1}, {0, 2}, {0, 3}},
    {{0, 0}, {0, 0}, {1, 1}},
    {{0, 0}, {1, 1}, {0, 0}},
    {{1, 1}, {0, 0}, {0, 0}},
    {{1, 1}, {2, 2}, {3, 3}},
    {{1, 1}, {3, 3}, {3, 3}},
    {{1, 1}, {1, 1}, {2, 2}},
    {{1, 1}, {2, 2}, {1, 1}},
    {{1, 1}, {1, 1}, {3, 3}},
    {{1, 1}, {2, 2}, {4, 4}}, // no coincident
    {{1, 1}, {3, 3}, {4, 4}},
    {{1, 1}, {3, 3}, {2, 2}},
    {{1, 1}, {4, 4}, {2, 2}},
    {{1, 1}, {4, 4}, {3, 3}},
    {{2, 2}, {1, 1}, {3, 3}},
    {{2, 2}, {1, 1}, {4, 4}},
    {{2, 2}, {3, 3}, {1, 1}},
    {{2, 2}, {3, 3}, {4, 4}},
    {{2, 2}, {4, 4}, {1, 1}},
    {{2, 2}, {4, 4}, {3, 3}},
};

const size_t quadraticLines_count = sizeof(quadraticLines) / sizeof(quadraticLines[0]);

static const double F = PointEpsilon * 3;
static const double H = PointEpsilon * 4;
static const double J = PointEpsilon * 5;
static const double K = PointEpsilon * 8; // INVESTIGATE: why are larger multiples necessary?

const Quadratic quadraticModEpsilonLines[] = {
    {{0, F}, {0, 0}, {1, 0}},
    {{0, 0}, {1, 0}, {0, F}},
    {{1, 0}, {0, F}, {0, 0}},
    {{1, H}, {2, 0}, {3, 0}},
    {{F, 0}, {0, 0}, {0, 1}},
    {{0, 0}, {0, 1}, {F, 0}},
    {{0, 1}, {F, 0}, {0, 0}},
    {{H, 1}, {0, 2}, {0, 3}},
    {{0, F}, {0, 0}, {1, 1}},
    {{0, 0}, {1, 1}, {F, 0}},
    {{1, 1}, {F, 0}, {0, 0}},
    {{1, 1+J}, {2, 2}, {3, 3}},
    {{1, 1}, {3, 3}, {3+F, 3}},
    {{1, 1}, {1+F, 1}, {2, 2}},
    {{1, 1}, {2, 2}, {1, 1+F}},
    {{1, 1}, {1, 1+F}, {3, 3}},
    {{1+H, 1}, {2, 2}, {4, 4}}, // no coincident
    {{1, 1+K}, {3, 3}, {4, 4}},
    {{1, 1}, {3+F, 3}, {2, 2}},
    {{1, 1}, {4, 4+F}, {2, 2}},
    {{1, 1}, {4, 4}, {3+F, 3}},
    {{2, 2}, {1, 1}, {3, 3+F}},
    {{2+F, 2}, {1, 1}, {4, 4}},
    {{2, 2+F}, {3, 3}, {1, 1}},
    {{2, 2}, {3+F, 3}, {4, 4}},
    {{2, 2}, {4, 4+F}, {1, 1}},
    {{2, 2}, {4, 4}, {3+F, 3}},
};

const size_t quadraticModEpsilonLines_count = sizeof(quadraticModEpsilonLines) / sizeof(quadraticModEpsilonLines[0]);

const Quadratic quadraticTests[][2] = {
    { // one intersection
     {{0, 0},
      {0, 1},
      {1, 1}},
     {{0, 1},
      {0, 0},
      {1, 0}}
    },
    { // four intersections
     {{1, 0},
      {2, 6},
      {3, 0}},
     {{0, 1},
      {6, 2},
      {0, 3}}
    }
};

const size_t quadraticTests_count = sizeof(quadraticTests) / sizeof(quadraticTests[0]);
