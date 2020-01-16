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

// Returns the maximum possible number of vertices that can be written by EmitCenterWedges() for the
// given path.
inline int MaxPossibleWedgeVertices(const SkPath& path) {
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
// NOTE: The incoming patchData must have allocated at least MaxWedgeVertices() vertices.
int EmitCenterWedges(const SkPath&, SkPoint* patchData);

}  // namespace

#endif
