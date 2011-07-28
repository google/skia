
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkConcaveToTriangles_DEFINED
#define SkConcaveToTriangles_DEFINED

#include "SkPoint.h"
#include "SkTDArray.h"


// Triangulate a polygon.
// The polygon can be convex or concave, and can have holes or multiple contours
// of arbitrary recursion.
// The holes must have opposite orientation of the outer contours, whereas
// islands within the holes must have the same orientation as the outer contour.
// Contours should be joined by zero-thickness double-edges, to mimic a single
// polygon.  The polygon should not be self-intersecting.
// Currently, the outer contour must be right-handed, i.e. it should be oriented
// in the direction that rotates the X-axis to the Y-axis.
bool SkConcaveToTriangles(size_t count,
                          const SkPoint pts[],
                          SkTDArray<SkPoint> *triangles);


#endif  // SkConcaveToTriangles_DEFINED
