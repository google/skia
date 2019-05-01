/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetPolygon_DEFINED
#define SkOffsetPolygon_DEFINED

#include <functional>

#include "include/core/SkPoint.h"
#include "include/private/SkTDArray.h"

struct SkRect;

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
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          SkScalar inset, SkTDArray<SkPoint>* insetPolygon);

/**
 * Generates a simple polygon (if possible) that is offset a constant distance from the boundary
 * of a given simple polygon.
 * The input polygon must be simple and have no coincident vertices or collinear edges.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param bounds Bounding rectangle for the original polygon.
 * @param offset How far we wish to offset the polygon.
 *   Positive values indicate insetting, negative values outsetting.
 * @param offsetPolgon  The resulting offset polygon, if any.
 * @param polygonIndices  The indices of the original polygon that map to the new one.
 * @return true if an offset simple polygon exists, false otherwise.
 */
bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                           const SkRect& bounds, SkScalar offset, SkTDArray<SkPoint>* offsetPolygon,
                           SkTDArray<int>* polygonIndices = nullptr);

/**
 * Compute the number of points needed for a circular join when offsetting a vertex.
 * The lengths of offset0 and offset1 don't have to equal |offset| -- only the direction matters.
 * The segment lengths will be approximately four pixels.
 *
 * @param offset0  Starting offset vector direction.
 * @param offset1  Ending offset vector direction.
 * @param offset  Offset value (can be negative).
 * @param rotSin  Sine of rotation delta per step.
 * @param rotCos  Cosine of rotation delta per step.
 * @param n  Number of steps to fill out the arc.
 * @return true for success, false otherwise
 */
bool SkComputeRadialSteps(const SkVector& offset0, const SkVector& offset1, SkScalar offset,
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

// Experiment: doesn't handle really big floats (returns false), always returns true for count <= 3
bool SkIsPolyConvex_experimental(const SkPoint[], int count);

#endif
