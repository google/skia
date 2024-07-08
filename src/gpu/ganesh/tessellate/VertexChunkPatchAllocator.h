/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VertexChunkPatchAllocator_DEFINED
#define VertexChunkPatchAllocator_DEFINED

#include "src/gpu/BufferWriter.h"
#include "src/gpu/ganesh/GrVertexChunkArray.h"
#include "src/gpu/tessellate/LinearTolerances.h"

#include <cstddef>

class GrMeshDrawTarget;

namespace skgpu::ganesh {

// An adapter around GrVertexChunkBuilder that fits the API requirements of
// skgpu::tess::PatchWriter's PatchAllocator template parameter.
class VertexChunkPatchAllocator {
public:
    // 'stride' is provided by PatchWriter.
    // 'worstCaseTolerances' is used to accumulate the LinearTolerances from each append().
    // 'target', 'chunks', and 'minVerticesPerChunk' are forwarded from the PatchWriter's ctor and
    // are used to construct a GrVertexChunkBuilder matching the PatchWriter's stride.
    VertexChunkPatchAllocator(size_t stride,
                              tess::LinearTolerances* worstCaseTolerances,
                              GrMeshDrawTarget* target,
                              GrVertexChunkArray* chunks,
                              int minVerticesPerChunk)
            : fWorstCaseTolerances(worstCaseTolerances)
            , fBuilder(target, chunks, stride, minVerticesPerChunk) {}

    VertexWriter append(const tess::LinearTolerances& tolerances) {
        fWorstCaseTolerances->accumulate(tolerances);
        return fBuilder.appendVertices(1);
    }

private:
    tess::LinearTolerances* fWorstCaseTolerances;
    GrVertexChunkBuilder    fBuilder;
};

}  // namespace skgpu::ganesh

#endif // VertexChunkPatchAllocator_DEFINED
