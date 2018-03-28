/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkInsetConvexPolygon_DEFINED
#define SkInsetConvexPolygon_DEFINED

#include <functional>

#include "SkTDArray.h"
#include "SkPoint.h"

/**
 * Generates a polygon that is inset a given distance from the boundary of a given convex polygon.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 *  It should be convex and have no coincident points.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param insetDistanceFunc  How far we wish to inset the polygon for a given index in the array.
 *  This should return a positive value.
 * @param insetPolygon  The resulting inset polygon, if any.
 * @return true if an inset polygon exists, false otherwise.
 */
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          std::function<SkScalar(int index)> insetDistanceFunc,
                          SkTDArray<SkPoint>* insetPolygon);

inline bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                                 SkScalar inset,
                                 SkTDArray<SkPoint>* insetPolygon) {
    return SkInsetConvexPolygon(inputPolygonVerts, inputPolygonSize,
                                [inset](int) { return inset; },
                                insetPolygon);
}

/**
 * Offset a segment by the given distance at each point.
 * Uses the outer tangents of two circles centered on each endpoint.
 * See: https://en.wikipedia.org/wiki/Tangent_lines_to_circles
 *
 * @param p0  First endpoint.
 * @param p1  Second endpoint.
 * @param d0  Offset distance from first endpoint.
 * @param d1  Offset distance from second endpoint.
 * @param side  Indicates whether we want to offset to the left (1) or right (-1) side of segment.
 * @param offset0  First endpoint of offset segment.
 * @param offset1  Second endpoint of offset segment.
 * @return true if an offset segment exists, false otherwise.
 */
bool SkOffsetSegment(const SkPoint& p0, const SkPoint& p1, SkScalar d0, SkScalar d1,
                     int side, SkPoint* offset0, SkPoint* offset1);

#endif
