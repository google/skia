/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTriangulator_DEFINED
#define GrTriangulator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrColor.h"

class GrEagerVertexAllocator;
struct SkRect;

/**
 * Provides utility functions for converting paths to a collection of triangles.
 */

#define TRIANGULATOR_WIREFRAME 0

namespace GrTriangulator {

enum class Mode {
    kNormal,

    // Surround path edges with coverage ramps for antialiasing.
    kEdgeAntialias,

    // Triangulate only each contour's inner polygon. The inner polygons connect the endpoints of
    // each verb. (i.e., they are the path that would result from collapsing all curves to single
    // lines.)
    //
    // If the inner polygons are not simple (e.g., self intersection, double winding), then the
    // tessellator aborts and returns 0.
    kSimpleInnerPolygons
};

// Encapsulates the list of inputs for triangulating a path. fIsLinear gets set by GrTriangulator
// internals and is valid to read once triangulation is finished.
struct Args {
    Args(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
         GrEagerVertexAllocator* vertexAllocator, Mode mode)
            : fPath(path)
            , fTolerance(tolerance)
            , fClipBounds(clipBounds)
            , fVertexAllocator(vertexAllocator)
            , fMode(mode) {
    }
    const SkPath fPath;
    const float fTolerance;
    const SkRect fClipBounds;
    GrEagerVertexAllocator* const fVertexAllocator;
    const Mode fMode;
    bool fIsLinear;
};

struct WindingVertex {
    SkPoint fPos;
    int fWinding;
};

// *DEPRECATED*: Once CCPR is removed this method will go away.
//   - Args.fVertexAllocator must be null.
//   - Args.fMode must be kNormal.
//
// Triangulates a path to an array of vertices. Each triangle is represented as a set of three
// WindingVertex entries, each of which contains the position and winding count (which is the same
// for all three vertices of a triangle). The 'verts' out parameter is set to point to the resultant
// vertex array. CALLER IS RESPONSIBLE for deleting this buffer to avoid a memory leak!
int PathToVertices(Args*, WindingVertex**);

constexpr size_t GetVertexStride(Mode mode) {
    return sizeof(SkPoint) + ((Mode::kEdgeAntialias == mode) ? sizeof(float) : 0);
}

int PathToTriangles(Args*);

}  // namespace GrTriangulator

#endif
