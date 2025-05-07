/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_DynamicInstancesPatchAllocator_DEFINED
#define skgpu_graphite_render_DynamicInstancesPatchAllocator_DEFINED

#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/tessellate/LinearTolerances.h"

namespace skgpu::graphite {

// An adapter around DrawWriter::DynamicInstances that fits the API requirements of
// skgpu::tess::PatchWriter's PatchAllocator template parameter.
//
// FixedCountVariant should be one of FixedCountCurves, FixedCountWedges, or FixedCountStrokes
// (technically any class with a static VertexCount(const LinearTolerances&) function).
template <typename FixedCountVariant>
class DynamicInstancesPatchAllocator {
public:
    // 'stride' is provided by PatchWriter.
    // 'writer' is the DrawWriter that the RenderStep can append instance data to,
    // 'fixedVertexBuffer' and 'fixedIndexBuffer' are the bindings for the instance template that
    // is passed to DrawWriter::DynamicInstances.
    DynamicInstancesPatchAllocator(size_t stride,
                                   DrawWriter& writer,
                                   BindBufferInfo fixedVertexBuffer,
                                   BindBufferInfo fixedIndexBuffer,
                                   unsigned int reserveCount)
            : fInstances(writer, fixedVertexBuffer, fixedIndexBuffer) {
        SkASSERT(stride == writer.appendStride());
        // TODO: Is it worth re-reserving large chunks after this preallocation is used up? Or will
        // appending 1 at a time be fine since it's coming from a large vertex buffer alloc anyway?
        fInstances.reserve(reserveCount);
    }

    VertexWriter append(const tess::LinearTolerances& tolerances) {
        return fInstances.append(tolerances, 1);
    }

private:
    struct LinearToleranceProxy {
        operator uint32_t() const { return FixedCountVariant::VertexCount(fTolerances); }
        void operator <<(const tess::LinearTolerances& t) { fTolerances.accumulate(t); }

        tess::LinearTolerances fTolerances;
    };

    DrawWriter::DynamicInstances<LinearToleranceProxy> fInstances;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_DynamicInstancesPatchAllocator_DEFINED
