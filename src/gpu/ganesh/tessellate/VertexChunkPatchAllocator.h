/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VertexChunkPatchAllocator_DEFINED
#define VertexChunkPatchAllocator_DEFINED

#include "src/gpu/ganesh/GrVertexChunkArray.h"

namespace skgpu::v1 {

// An adapter around GrVertexChunkBuilder that fits the API requirements of
// skgpu::tess::PatchWriter's PatchAllocator template parameter.
class VertexChunkPatchAllocator {
public:
    // 'stride' is provided by PatchWriter. 'target', 'chunks', and
    // 'minVerticesPerChunk' must be forwarded through the PatchWriter's ctor.
    VertexChunkPatchAllocator(size_t stride,
                              GrMeshDrawTarget* target,
                              GrVertexChunkArray* chunks,
                              int minVerticesPerChunk)
            : fBuilder(target, chunks, stride, minVerticesPerChunk) {}

    size_t stride() const { return fBuilder.stride(); }
    VertexWriter append() { return fBuilder.appendVertices(1); }

private:
    GrVertexChunkBuilder fBuilder;
};

}  // namespace skgpu::v1

#endif // VertexChunkPatchAllocator_DEFINED
