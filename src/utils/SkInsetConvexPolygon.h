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
 * @param insetDistanceFunc  How far we wish to inset the polygon for a given point.
 *  This should return a positive value.
 * @param insetPolygon  The resulting inset polygon, if any.
 * @return true if an inset polygon exists, false otherwise.
 */
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          std::function<SkScalar(const SkPoint&)> insetDistanceFunc,
                          SkTDArray<SkPoint>* insetPolygon);

inline bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          SkScalar inset,
                          SkTDArray<SkPoint>* insetPolygon) {
    return SkInsetConvexPolygon(inputPolygonVerts, inputPolygonSize,
                                [inset](const SkPoint&) { return inset; },
                                insetPolygon);
}


#endif
