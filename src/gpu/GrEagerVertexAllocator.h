/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrEagerVertexAllocator_DEFINED
#define GrEagerVertexAllocator_DEFINED

#include "src/gpu/GrThreadSafeCache.h"

class GrMeshDrawTarget;

// This interface is used to allocate and map GPU vertex data before the exact number of required
// vertices is known. Usage pattern:
//
//   1. Call lock(eagerCount) with an upper bound on the number of required vertices.
//   2. Compute and write vertex data to the returned pointer (if not null).
//   3. Call unlock(actualCount) and provide the actual number of vertices written during step #2.
//
// On step #3, the implementation will attempt to shrink the underlying GPU memory slot to fit the
// actual vertex count.
class GrEagerVertexAllocator {
public:
    template<typename T> T* lock(int eagerCount) {
        return static_cast<T*>(this->lock(sizeof(T), eagerCount));
    }
    virtual void* lock(size_t stride, int eagerCount) = 0;

    virtual void unlock(int actualCount) = 0;

    virtual ~GrEagerVertexAllocator() {}
};

// GrEagerVertexAllocator implementation that uses GrMeshDrawTarget::makeVertexSpace and
// GrMeshDrawTarget::putBackVertices.
class GrEagerDynamicVertexAllocator : public GrEagerVertexAllocator {
public:
    GrEagerDynamicVertexAllocator(GrMeshDrawTarget* target,
                                  sk_sp<const GrBuffer>* vertexBuffer,
                                  int* baseVertex)
            : fTarget(target)
            , fVertexBuffer(vertexBuffer)
            , fBaseVertex(baseVertex) {
    }

#ifdef SK_DEBUG
    ~GrEagerDynamicVertexAllocator() override {
        SkASSERT(!fLockCount);
    }
#endif

    // Un-shadow GrEagerVertexAllocator::lock<T>.
    using GrEagerVertexAllocator::lock;

    // Mark "final" as a hint for the compiler to not use the vtable.
    void* lock(size_t stride, int eagerCount) final;

    // Mark "final" as a hint for the compiler to not use the vtable.
    void unlock(int actualCount) final;

private:
    GrMeshDrawTarget* const fTarget;
    sk_sp<const GrBuffer>* const fVertexBuffer;
    int* const fBaseVertex;

    size_t fLockStride;
    int fLockCount = 0;
};

class GrCpuVertexAllocator : public GrEagerVertexAllocator {
public:
    GrCpuVertexAllocator() = default;

#ifdef SK_DEBUG
    ~GrCpuVertexAllocator() override {
        SkASSERT(!fLockStride && !fVertices && !fVertexData);
    }
#endif

    void* lock(size_t stride, int eagerCount) override;
    void unlock(int actualCount) override;

    sk_sp<GrThreadSafeCache::VertexData> detachVertexData();

private:
    sk_sp<GrThreadSafeCache::VertexData> fVertexData;

    void*  fVertices = nullptr;
    size_t fLockStride = 0;
};

#endif
