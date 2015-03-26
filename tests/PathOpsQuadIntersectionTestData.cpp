/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsQuadIntersectionTestData.h"

const SkDQuad quadraticPoints[] = {
    {{{0, 0}, {1, 0}, {0, 0}}},
    {{{0, 0}, {0, 1}, {0, 0}}},
    {{{0, 0}, {1, 1}, {0, 0}}},
    {{{1, 1}, {2, 2}, {1, 1}}},
};

const size_t quadraticPoints_count = SK_ARRAY_COUNT(quadraticPoints);

const SkDQuad quadraticLines[] = {
    {{{0, 0}, {0, 0}, {1, 0}}},
    {{{1, 0}, {0, 0}, {0, 0}}},
    {{{1, 0}, {2, 0}, {3, 0}}},
    {{{0, 0}, {0, 0}, {0, 1}}},
    {{{0, 1}, {0, 0}, {0, 0}}},
    {{{0, 1}, {0, 2}, {0, 3}}},
    {{{0, 0}, {0, 0}, {1, 1}}},
    {{{1, 1}, {0, 0}, {0, 0}}},
    {{{1, 1}, {2, 2}, {3, 3}}},
    {{{1, 1}, {3, 3}, {3, 3}}},
    {{{1, 1}, {1, 1}, {2, 2}}},
    {{{1, 1}, {1, 1}, {3, 3}}},
    {{{1, 1}, {2, 2}, {4, 4}}},  // no coincident
    {{{1, 1}, {3, 3}, {4, 4}}},
    {{{1, 1}, {3, 3}, {2, 2}}},
    {{{1, 1}, {4, 4}, {2, 2}}},
    {{{1, 1}, {4, 4}, {3, 3}}},
    {{{2, 2}, {1, 1}, {3, 3}}},
    {{{2, 2}, {1, 1}, {4, 4}}},
    {{{2, 2}, {3, 3}, {1, 1}}},
    {{{2, 2}, {3, 3}, {4, 4}}},
    {{{2, 2}, {4, 4}, {1, 1}}},
    {{{2, 2}, {4, 4}, {3, 3}}},
};

const size_t quadraticLines_count = SK_ARRAY_COUNT(quadraticLines);

static const double F = FLT_EPSILON * 32;
static const double H = FLT_EPSILON * 32;
static const double J = FLT_EPSILON * 32;
static const double K = FLT_EPSILON * 32;  // INVESTIGATE: why are larger multiples necessary?

const SkDQuad quadraticModEpsilonLines[] = {
    {{{0, F}, {0, 0}, {1, 0}}},
    {{{0, 0}, {1, 0}, {0, F}}},
    {{{1, 0}, {0, F}, {0, 0}}},
    {{{1, H}, {2, 0}, {3, 0}}},
//  {{{F, 0}, {0, 0}, {0, 1}}},  // INVESTIGATE: even substituting K for F, quad is still linear.
//  {{{0, 0}, {0, 1}, {F, 0}}},
//  {{{0, 1}, {F, 0}, {0, 0}}},
//  {{{H, 1}, {0, 2}, {0, 3}}},
    {{{0, F}, {0, 0}, {1, 1}}},
    {{{0, 0}, {1, 1}, {F, 0}}},
    {{{1, 1}, {F, 0}, {0, 0}}},
    {{{1, 1+J}, {2, 2}, {3, 3}}},
    {{{1, 1}, {3, 3}, {3+F, 3}}},
    {{{1, 1}, {1+F, 1}, {2, 2}}},
    {{{1, 1}, {2, 2}, {1, 1+K}}},
    {{{1, 1}, {1, 1+F}, {3, 3}}},
    {{{1+H, 1}, {2, 2}, {4, 4}}},  // no coincident
    {{{1, 1+K}, {3, 3}, {4, 4}}},
    {{{1, 1}, {3+F, 3}, {2, 2}}},
    {{{1, 1}, {4, 4+F}, {2, 2}}},
    {{{1, 1}, {4, 4}, {3+F, 3}}},
    {{{2, 2}, {1, 1}, {3, 3+F}}},
    {{{2+F, 2}, {1, 1}, {4, 4}}},
    {{{2, 2+F}, {3, 3}, {1, 1}}},
    {{{2, 2}, {3+F, 3}, {4, 4}}},
    {{{2, 2}, {4, 4+F}, {1, 1}}},
    {{{2, 2}, {4, 4}, {3+F, 3}}},
};

const size_t quadraticModEpsilonLines_count =
        SK_ARRAY_COUNT(quadraticModEpsilonLines);

const SkDQuad quadraticTests[][2] = {
    {  // one intersection
     {{{0, 0},
      {0, 1},
      {1, 1}}},
     {{{0, 1},
      {0, 0},
      {1, 0}}}
    },
    {  // four intersections
     {{{1, 0},
      {2, 6},
      {3, 0}}},
     {{{0, 1},
      {6, 2},
      {0, 3}}}
    }
};

const size_t quadraticTests_count = SK_ARRAY_COUNT(quadraticTests);
