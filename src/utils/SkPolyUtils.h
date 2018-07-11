/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetPolygon_DEFINED
#define SkOffsetPolygon_DEFINED

#include <functional>

#include "SkTDArray.h"
#include "SkPoint.h"

/**
 * Generates a polygon that is inset a variable distance (controlled by offsetDistanceFunc)
 * from the boundary of a given convex polygon.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 *  It should be convex and have no coincident points.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param insetDistanceFunc  How far we wish to inset the polygon for a given position.
 *  This should return a positive value.
 * @param insetPolygon  The resulting inset polygon, if any.
 * @return true if an inset polygon exists, false otherwise.
 */
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          std::function<SkScalar(const SkPoint&)> insetDistanceFunc,
                          SkTDArray<SkPoint>* insetPolygon);

/**
 * Generates a polygon that is inset a constant from the boundary of a given convex polygon.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 *  It should be convex and have no coincident points.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param inset  How far we wish to inset the polygon. This should be a positive value.
 * @param insetPolygon  The resulting inset polygon, if any.
 * @return true if an inset polygon exists, false otherwise.
 */
inline bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                                 SkScalar inset, SkTDArray<SkPoint>* insetPolygon) {
    return SkInsetConvexPolygon(inputPolygonVerts, inputPolygonSize,
                                [inset](const SkPoint&) { return inset; },
                                insetPolygon);
}

/**
 * Generates a simple polygon (if possible) that is offset a variable distance (controlled by
 * offsetDistanceFunc) from the boundary of a given simple polygon.
 * The input polygon must be simple and have no coincident vertices or collinear edges.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param offsetDistanceFunc  How far we wish to offset the polygon for a given position.
 *   Positive values indicate insetting, negative values outsetting.
 * @param offsetPolgon  The resulting offset polygon, if any.
 * @param polygonIndices  The indices of the original polygon that map to the new one.
 * @return true if an offset simple polygon exists, false otherwise.
 */
bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                           std::function<SkScalar(const SkPoint&)> offsetDistanceFunc,
                           SkTDArray<SkPoint>* offsetPolygon,
                           SkTDArray<int>* polygonIndices = nullptr);

/**
 * Generates a simple polygon (if possible) that is offset a constant distance from the boundary
 * of a given simple polygon.
 * The input polygon must be simple and have no coincident vertices or collinear edges.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param offset How far we wish to offset the polygon.
 *   Positive values indicate insetting, negative values outsetting.
 * @param offsetPolgon  The resulting offset polygon, if any.
 * @param polygonIndices  The indices of the original polygon that map to the new one.
 * @return true if an offset simple polygon exists, false otherwise.
 */
inline bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                                  SkScalar offset, SkTDArray<SkPoint>* offsetPolygon,
                                  SkTDArray<int>* polygonIndices = nullptr) {
    return SkOffsetSimplePolygon(inputPolygonVerts, inputPolygonSize,
                                 [offset](const SkPoint&) { return offset; },
                                 offsetPolygon, polygonIndices);
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

/**
 * Compute the number of points needed for a circular join when offsetting a vertex.
 * The lengths of offset0 and offset1 don't have to equal r -- only the direction matters.
 * The segment lengths will be approximately four pixels.
 *
 * @param offset0  Starting offset vector direction.
 * @param offset1  Ending offset vector direction.
 * @param r  Length of offset.
 * @param rotSin  Sine of rotation delta per step.
 * @param rotCos  Cosine of rotation delta per step.
 * @param n  Number of steps to fill out the arc.
 * @return true for success, false otherwise
 */
bool SkComputeRadialSteps(const SkVector& offset0, const SkVector& offset1, SkScalar r,
                          SkScalar* rotSin, SkScalar* rotCos, int* n);

/**
 * Determine winding direction for a polygon.
 * The input polygon must be simple or the result will be meaningless.
 *
 * @param polygonVerts  Array of points representing the vertices of the polygon.
 * @param polygonSize  Number of vertices in the polygon.
 * @return 1 for cw, -1 for ccw, and 0 if zero signed area (either degenerate or self-intersecting).
 *         The y-axis is assumed to be pointing down.
 */
int SkGetPolygonWinding(const SkPoint* polygonVerts, int polygonSize);

/**
 * Determine whether a polygon is convex or not.
 *
 * @param polygonVerts  Array of points representing the vertices of the polygon.
 * @param polygonSize  Number of vertices in the polygon.
 * @return true if the polygon is convex, false otherwise.
 */
bool SkIsConvexPolygon(const SkPoint* polygonVerts, int polygonSize);

/**
 * Determine whether a polygon is simple (i.e., not self-intersecting) or not.
 * The input polygon must have no coincident vertices or the test will fail.
 *
 * @param polygonVerts  Array of points representing the vertices of the polygon.
 * @param polygonSize  Number of vertices in the polygon.
 * @return true if the polygon is simple, false otherwise.
 */
 bool SkIsSimplePolygon(const SkPoint* polygonVerts, int polygonSize);

 /**
  * Compute indices to triangulate the given polygon.
  * The input polygon must be simple (i.e. it is not self-intersecting)
  * and have no coincident vertices or collinear edges.
  *
  * @param polygonVerts  Array of points representing the vertices of the polygon.
  * @param indexMap Mapping from index in the given array to the final index in the triangulation.
  * @param polygonSize  Number of vertices in the polygon.
  * @param triangleIndices  Indices of the resulting triangulation.
  * @return true if successful, false otherwise.
  */
 bool SkTriangulateSimplePolygon(const SkPoint* polygonVerts, uint16_t* indexMap, int polygonSize,
                                 SkTDArray<uint16_t>* triangleIndices);

#endif
