/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathParser_DEFINED
#define GrPathParser_DEFINED

#include "include/core/SkPath.h"

namespace GrPathParser {

// Returns the maximum number of vertices that can be written by EmitCenterWedges() for the given
// path.
inline int MaxWedgeVertices(const SkPath& path) {
    // No initial moveTo, one wedge per verb, plus an implicit close at the end.
    // Each wedge has 5 vertices.
    return (path.countVerbs() + 1) * 5;
}

// Writes an array of cubic "wedges" from an SkPath, converting any lines or quadratics to cubics.
// These wedges can then be fed into GrTessellateWedgeShader to stencil the path. A wedge is a
// 5-point tessellation patch consisting of 4 cubic control points, plus an anchor point fanning
// from the center of the curve's resident contour.
//
// TODO: Eventually we want to use rational cubic wedges in order to support conics.
//
// Returns the number of vertices written to the array.
//
// The incoming patchData array must have at least MaxWedgeVertices() elements.
int EmitCenterWedgePatches(const SkPath&, SkPoint* patchData);

// Returns the maximum number of vertices required to triangulate the given path's inner polygon(s).
inline int MaxInnerPolygonVertices(const SkPath& path) {
    // No initial moveTo, plus an implicit close at the end; n-2 trianles fill an n-gon.
    // Each triangle has 3 vertices.
    return (path.countVerbs() - 1) * 3;
}

// Triangulates the path's inner polygon(s) and writes the result to "vertexData". The inner
// polygons connect the endpoints of each verb. (i.e., they are the path that would result from
// collapsing all curves to single lines.)
//
// This method works by recursively subdividing the path rather than emitting a linear triangle fan
// or strip. This can reduce the load on the rasterizer by a great deal on complex paths.
//
// Returns the number of vertices written to the array.
//
// The incoming vertexData array must have at least MaxInnerPolygonVertices() elements.
int EmitInnerPolygonTriangles(const SkPath&, SkPoint* vertexData, int* numCurves);

int EmitCubicInstances(const SkPath&, SkPoint* vertexData);

}  // namespace

#endif
