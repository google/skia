/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef LineIntersection_DEFINE
#define LineIntersection_DEFINE

#include "Intersections.h"

int horizontalIntersect(const _Line& line, double y, double tRange[2]);
int horizontalLineIntersect(const _Line& line, double left, double right,
        double y, double tRange[2]);
void lineIntersect(const _Line& a, const _Line& b, _Point& p);
int intersect(const _Line& a, const _Line& b, Intersections&);
bool testIntersect(const _Line& a, const _Line& b);
int verticalLineIntersect(const _Line& line, double top, double bottom,
        double x, double tRange[2]);

#endif
