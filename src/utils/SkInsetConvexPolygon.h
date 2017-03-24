/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkInsetConvexPolygon_DEFINED
#define SkInsetConvexPolygon_DEFINED

#include "SkTDArray.h"
#include "SkPoint.h"

 /**
 * Generates a polygon that is inset a given distance from the boundary of a given convex polygon.
 *
 * @param inputPolygonVerts  Array of points representing the vertices of the original polygon.
 *  It should be convex and have no coincident points.
 * @param inputPolygonSize  Number of vertices in the original polygon.
 * @param insetDistance  How far we wish to inset the polygon. This should be a positive value.
 * @param insetPolygon  The resulting inset polygon, if any.
 * @return true if an inset polygon exists, false otherwise.
 */
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          SkScalar insetDistance, SkTDArray<SkPoint>* insetPolygon);

#endif
