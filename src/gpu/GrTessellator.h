/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellator_DEFINED
#define GrTessellator_DEFINED

#include "GrColor.h"
#include "SkTDArray.h"
#include "SkColorData.h"
#include "SkPoint.h"

class SkMatrix;
class SkPath;
struct SkRect;

/**
 * Provides utility functions for converting paths to a collection of triangles.
 */

#define TESSELLATOR_WIREFRAME 0

namespace GrTessellator {

class VertexAllocator {
public:
    VertexAllocator(size_t stride) : fStride(stride) {}
    virtual ~VertexAllocator() {}
    virtual void* lock(int vertexCount) = 0;
    virtual void unlock(int actualCount) = 0;
    size_t stride() const { return fStride; }
private:
    size_t fStride;
};

struct WindingVertex {
    SkPoint fPos;
    int fWinding;
};

// Triangulates a path to an array of vertices. Each triangle is represented as a set of three
// WindingVertex entries, each of which contains the position and winding count (which is the same
// for all three vertices of a triangle). The 'verts' out parameter is set to point to the resultant
// vertex array. CALLER IS RESPONSIBLE for deleting this buffer to avoid a memory leak!
int PathToVertices(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                   WindingVertex** verts);

int PathToTriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                    VertexAllocator*, bool antialias, const GrColor& color,
                    bool canTweakAlphaForCoverage, bool *isLinear);

// Offsets the transformed path both inwards and outwards, setting color on resulting contours to
// innerColor and outerColor respectively, and tessellating between them. Will also tessellate the
// inner contour as well if fillCenter is true.
// Currently only supports single-contour non-intersecting paths with similarly simple offsets.
// Returns true on success, false otherwise.
bool PathToShadowVertices(const SkPath& path, const SkMatrix& transform, SkScalar offset,
                          SkColor color, bool fillCenter, SkTDArray<SkPoint>* positions,
                          SkTDArray<uint16_t>* indices, SkTDArray<SkColor>* colors);
}

#endif
