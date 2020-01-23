/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathParser_DEFINED
#define GrPathParser_DEFINED

#include "include/core/SkPath.h"

class GrEagerVertexAllocator;

namespace GrPathParser {

// Writes an array of cubic "wedges" from an SkPath, converting any lines or quadratics to cubics.
// These wedges can then be fed into GrStencilWedgeShader to stencil the path. A wedge is a 5-point
// tessellation patch consisting of 4 cubic control points, plus an anchor point fanning from the
// center of the curve's resident contour.
//
// TODO: Eventually we want to use rational cubic wedges in order to support conics.
//
// Returns the number of vertices written to the array.
int EmitCenterWedgePatches(const SkPath&, GrEagerVertexAllocator*);

// Triangulates and writes an SkPath's inner polygon(s). The inner polygons connect the endpoints of
// each verb. (i.e., they are the path that would result from collapsing all curves to single
// lines.)
//
// This method works by recursively subdividing the path rather than emitting a linear triangle fan
// or strip. This can reduce the load on the rasterizer by a great deal on complex paths.
//
// Returns the number of vertices written to the array.
int EmitInnerPolygonTriangles(const SkPath&, GrEagerVertexAllocator*);

// Writes out an array of cubics from an SkPath as 4-point instances, converting any quadratics to
// cubics.
//
// Returns the number of *instances* written to the array.
int EmitCubicInstances(const SkPath&, GrEagerVertexAllocator*);

}  // namespace

#endif
