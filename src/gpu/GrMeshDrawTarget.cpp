/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrMeshDrawTarget.h"

#include "src/gpu/BufferWriter.h"
#include "src/gpu/GrResourceProvider.h"

uint32_t GrMeshDrawTarget::contextUniqueID() const {
    return this->resourceProvider()->contextUniqueID();
}

template<typename W>
static W make_writer(void* p, int count, size_t elementSize) {
    // Don't worry about overflow in calculating byte size if 'p' is not null, presumably the actual
    // allocation validated it, so it should be safe after the fact.
    return p ? W{p, count * elementSize} : W{};
}

skgpu::VertexWriter GrMeshDrawTarget::makeVertexWriter(
        size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>* buffer, int* startVertex) {
    void* p = this->makeVertexSpace(vertexSize, vertexCount, buffer, startVertex);
    return make_writer<skgpu::VertexWriter>(p, vertexCount, vertexSize);
}

skgpu::IndexWriter GrMeshDrawTarget::makeIndexWriter(
        int indexCount, sk_sp<const GrBuffer>* buffer, int* startIndex) {
    void* p = this->makeIndexSpace(indexCount, buffer, startIndex);
    return make_writer<skgpu::IndexWriter>(p, indexCount, sizeof(uint16_t));
}

skgpu::VertexWriter GrMeshDrawTarget::makeVertexWriterAtLeast(
        size_t vertexSize, int minVertexCount, int fallbackVertexCount,
        sk_sp<const GrBuffer>* buffer, int* startVertex, int* actualVertexCount) {
    void* p = this->makeVertexSpaceAtLeast(vertexSize, minVertexCount, fallbackVertexCount,
                                           buffer, startVertex, actualVertexCount);
    return make_writer<skgpu::VertexWriter>(p, *actualVertexCount, vertexSize);
}

skgpu::IndexWriter GrMeshDrawTarget::makeIndexWriterAtLeast(
        int minIndexCount, int fallbackIndexCount, sk_sp<const GrBuffer>* buffer,
        int* startIndex, int* actualIndexCount) {
    void* p = this->makeIndexSpaceAtLeast(minIndexCount, fallbackIndexCount, buffer,
                                          startIndex, actualIndexCount);
    return make_writer<skgpu::IndexWriter>(p, *actualIndexCount, sizeof(uint16_t));
}
