/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellator_DEFINED
#define GrTessellator_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

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

class DynamicVertexAllocator : public VertexAllocator {
public:
    DynamicVertexAllocator(size_t stride, GrMeshDrawOp::Target* target)
            : VertexAllocator(stride)
            , fTarget(target) {}
    void* lock(int lockCount) override {
        SkASSERT(!fLockCount);
        SkASSERT(lockCount);
        fLockCount = lockCount;
        return fTarget->makeVertexSpace(this->stride(), fLockCount, &fVertexBuffer, &fFirstVertex);
    }
    void unlock(int actualCount) override {
        SkASSERT(fLockCount);
        fTarget->putBackVertices(fLockCount - actualCount, this->stride());
        fLockCount = 0;
    }
    sk_sp<const GrBuffer> detachVertexBuffer() const { return std::move(fVertexBuffer); }
    int firstVertex() const { return fFirstVertex; }

private:
    GrMeshDrawOp::Target* const fTarget;
    int fLockCount = 0;
    sk_sp<const GrBuffer> fVertexBuffer;
    int fFirstVertex;
};

struct WindingVertex {
    SkPoint fPos;
    int fWinding;
};

// Using this tolerance value guarantees that we emit only one straight line per curve, from start
// to end point (rather than linearizing a curve apporximation).
static constexpr float kFlattenCurvesTolerance = std::numeric_limits<float>::infinity();

enum class Flags {
    kNone = 0,

    // Adds coverage ramps on path edges for antialiasing.
    kCoverageAA = 1 << 0,

    // Ensures that triangles with wind > 0 are emitted with device-space clockwise vertices, and
    // triangles with wind < 0 have counter-clockwise vertices.
    //
    // If this flag is not provied, then all triangles will wind clockwise.
    //
    // NOTE: wind direction matching does not apply to kCoverageAA ramps.
    kPreserveWindingDirection = 1 << 1,

    // Ensures that no vertices get added or removed after the path has been linearized. If
    // the linearized path has self intersections that require new vertices, then PathToVertices
    // returns 0. Collinear points are not culled.
    // NOTE: this flag is not compatible with kCoverageAA.
    kPreserveExactEdges = 1 << 2
};

GR_MAKE_BITFIELD_CLASS_OPS(Flags);

// Triangulates a path to an array of vertices. Each triangle is represented as a set of three
// WindingVertex entries, each of which contains the position and winding count (which is the same
// for all three vertices of a triangle). The 'verts' out parameter is set to point to the resultant
// vertex array. CALLER IS RESPONSIBLE for deleting this buffer to avoid a memory leak!
int PathToVertices(const SkPath& path, SkScalar tolerance, Flags, const SkRect& clipBounds,
                   WindingVertex** verts);

int PathToTriangles(const SkPath& path, SkScalar tolerance, Flags,
                    const SkRect& clipBounds, VertexAllocator*, bool *isLinear);
}

#endif
