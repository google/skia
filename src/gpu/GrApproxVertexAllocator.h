/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrApproxVertexAllocator_DEFINED
#define GrApproxVertexAllocator_DEFINED

#include "src/gpu/ops/GrMeshDrawOp.h"

// This interface is used to allocate and map GPU vertex data before the exact number of required
// vertices is known. Usage pattern:
//
//   1. Call lock() with an upper bound on the number of required vertices.
//   2. Compute and write vertex data to the returned pointer (if not null).
//   3. Call unlock() and provide the actual number of vertices written during step #2.
//
// On step #3, the implementation will attempt to shrink the underlying GPU memory slot to fit the
// actual vertex count.
class GrApproxVertexAllocator {
public:
    template<typename T> T* lock(int lockCount) {
        return static_cast<T*>(this->lock(sizeof(T), lockCount));
    }
    virtual void* lock(size_t stride, int count) = 0;

    virtual void unlock(int actualCount) = 0;

    virtual ~GrApproxVertexAllocator() {}
};

// GrApproxVertexAllocator implementation that uses GrMeshDrawOp::Target::makeVertexSpace and
// GrMeshDrawOp::Target::putBackVertices.
class GrApproxDynamicVertexAllocator : public GrApproxVertexAllocator {
public:
    GrApproxDynamicVertexAllocator(GrMeshDrawOp::Target* target,
                                   sk_sp<const GrBuffer>* vertexBuffer, int* baseVertex)
            : fTarget(target)
            , fVertexBuffer(vertexBuffer)
            , fBaseVertex(baseVertex) {
    }

#ifdef SK_DEBUG
    ~GrApproxDynamicVertexAllocator() override {
        SkASSERT(!fLockCount);
    }
#endif

    void* lock(size_t stride, int count) override {
        SkASSERT(!fLockCount);
        SkASSERT(count);
        if (void* data = fTarget->makeVertexSpace(stride, count, fVertexBuffer, fBaseVertex)) {
            fLockStride = stride;
            fLockCount = count;
            return data;
        }
        fVertexBuffer->reset();
        *fBaseVertex = 0;
        return nullptr;
    }

    void unlock(int actualCount) override {
        SkASSERT(fLockCount);
        SkASSERT(actualCount <= fLockCount);
        fTarget->putBackVertices(fLockCount - actualCount, fLockStride);
        if (!actualCount) {
            fVertexBuffer->reset();
            *fBaseVertex = 0;
        }
        fLockCount = 0;
    }

private:
    GrMeshDrawOp::Target* const fTarget;
    sk_sp<const GrBuffer>* const fVertexBuffer;
    int* const fBaseVertex;

    size_t fLockStride;
    int fLockCount = 0;
};

#endif
