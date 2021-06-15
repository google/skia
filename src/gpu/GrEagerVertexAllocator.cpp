/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrEagerVertexAllocator.h"

//-------------------------------------------------------------------------------------------------
void* GrCpuVertexAllocator::lock(size_t stride, int eagerCount) {
    SkASSERT(!fLockStride && !fVertices && !fVertexData);
    SkASSERT(stride && eagerCount);

    fVertices = sk_malloc_throw(eagerCount * stride);
    fLockStride = stride;

    return fVertices;
}

void GrCpuVertexAllocator::unlock(int actualCount) {
    SkASSERT(fLockStride && fVertices && !fVertexData);

    fVertices = sk_realloc_throw(fVertices, actualCount * fLockStride);

    fVertexData = GrThreadSafeCache::MakeVertexData(fVertices, actualCount, fLockStride);

    fVertices = nullptr;
    fLockStride = 0;
}

sk_sp<GrThreadSafeCache::VertexData> GrCpuVertexAllocator::detachVertexData() {
    SkASSERT(!fLockStride && !fVertices && fVertexData);

    return std::move(fVertexData);
}
