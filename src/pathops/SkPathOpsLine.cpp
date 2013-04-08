/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsLine.h"

SkDLine SkDLine::subDivide(double t1, double t2) const {
    SkDVector delta = tangent();
    SkDLine dst = {{{
            fPts[0].fX - t1 * delta.fX, fPts[0].fY - t1 * delta.fY}, {
            fPts[0].fX - t2 * delta.fX, fPts[0].fY - t2 * delta.fY}}};
    return dst;
}

// may have this below somewhere else already:
// copying here because I thought it was clever

// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that a class is already given for the object:
//    Point with coordinates {float x, y;}
//===================================================================

// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
//    return (float) ((P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y));
double SkDLine::isLeft(const SkDPoint& pt) const {
    SkDVector p0 = fPts[1] - fPts[0];
    SkDVector p2 = pt - fPts[0];
    return p0.cross(p2);
}

SkDPoint SkDLine::xyAtT(double t) const {
    double one_t = 1 - t;
    SkDPoint result = { one_t * fPts[0].fX + t * fPts[1].fX, one_t * fPts[0].fY + t * fPts[1].fY };
    return result;
}
