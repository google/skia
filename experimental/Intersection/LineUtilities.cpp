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
float isLeft( _Point P0, _Point P1, _Point P2 )
{
    return (float) ((P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y));
}
