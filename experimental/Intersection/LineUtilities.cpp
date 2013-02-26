/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "LineUtilities.h"

bool implicitLine(const _Line& line, double& slope, double& axisIntercept) {
    _Point delta;
    tangent(line, delta);
    bool moreHorizontal = fabs(delta.x) > fabs(delta.y);
    if (moreHorizontal) {
        slope = delta.y / delta.x;
        axisIntercept = line[0].y - slope * line[0].x;
    } else {
        slope = delta.x / delta.y;
        axisIntercept = line[0].x - slope * line[0].y;
    }
    return moreHorizontal;
}

int reduceOrder(const _Line& line, _Line& reduced) {
    reduced[0] = line[0];
    int different = line[0] != line[1];
    reduced[1] = line[different];
    return 1 + different;
}

void sub_divide(const _Line& line, double t1, double t2, _Line& dst) {
    _Point delta;
    tangent(line, delta);
    dst[0].x = line[0].x - t1 * delta.x;
    dst[0].y = line[0].y - t1 * delta.y;
    dst[1].x = line[0].x - t2 * delta.x;
    dst[1].y = line[0].y - t2 * delta.y;
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
double is_left(const _Line& line, const _Point& pt) {
    _Vector P0 = line[1] - line[0];
    _Vector P2 = pt - line[0];
    return P0.cross(P2);
}

double t_at(const _Line& line, const _Point& pt) {
    double dx = line[1].x - line[0].x;
    double dy = line[1].y - line[0].y;
    if (fabs(dx) > fabs(dy)) {
        if (approximately_zero(dx)) {
            return 0;
        }
        return (pt.x - line[0].x) / dx;
    }
    if (approximately_zero(dy)) {
        return 0;
    }
    return (pt.y - line[0].y) / dy;
}

static void setMinMax(double x, int flags, double& minX, double& maxX) {
    if (minX > x && (flags & (kFindTopMin | kFindBottomMin))) {
        minX = x;
    }
    if (maxX < x && (flags & (kFindTopMax | kFindBottomMax))) {
        maxX = x;
    }
}

void x_at(const _Point& p1, const _Point& p2, double top, double bottom,
        int flags, double& minX, double& maxX) {
    if (AlmostEqualUlps(p1.y, p2.y)) {
        // It should be OK to bail early in this case. There's another edge
        // which shares this end point which can intersect without failing to
        // have a slope ... maybe
        return;
    }

    // p2.x is always greater than p1.x -- the part of points (p1, p2) are
    // moving from the start of the cubic towards its end.
    // if p1.y < p2.y, minX can be affected
    // if p1.y > p2.y, maxX can be affected
    double slope = (p2.x - p1.x) / (p2.y - p1.y);
    int topFlags = flags & (kFindTopMin | kFindTopMax);
    if (topFlags && ((top <= p1.y && top >= p2.y)
            || (top >= p1.y && top <= p2.y))) {
        double x = p1.x + (top - p1.y) * slope;
        setMinMax(x, topFlags, minX, maxX);
    }
    int bottomFlags = flags & (kFindBottomMin | kFindBottomMax);
    if (bottomFlags && ((bottom <= p1.y && bottom >= p2.y)
            || (bottom >= p1.y && bottom <= p2.y))) {
        double x = p1.x + (bottom - p1.y) * slope;
        setMinMax(x, bottomFlags, minX, maxX);
    }
}

void xy_at_t(const _Line& line, double t, double& x, double& y) {
    double one_t = 1 - t;
    if (&x) {
        x = one_t * line[0].x + t * line[1].x;
    }
    if (&y) {
        y = one_t * line[0].y + t * line[1].y;
    }
}

_Point xy_at_t(const _Line& line, double t) {
    double one_t = 1 - t;
    _Point result = { one_t * line[0].x + t * line[1].x, one_t * line[0].y + t * line[1].y };
    return result;
}
