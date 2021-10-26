/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawPass.h"

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/geom/BoundsManager.h"

#include "src/core/SkMathPriv.h"
#include "src/core/SkUtils.h"

namespace skgpu {

/**
 * Each Draw in a DrawList might be processed by multiple RenderSteps (determined by the Draw's
 * Renderer), which can be sorted independently. Each (step, draw) pair produces its own SortKey.
 *
 * The goal of sorting draws for the DrawPass is to minimize pipeline transitions and dynamic binds
 * within a pipeline, while still respecting the overall painter's order. This decreases the number
 * of low-level draw commands in a command buffer and increases the size of those, allowing the GPU
 * to operate more efficiently and have fewer bubbles within its own instruction stream.
 *
 * The Draw's CompresssedPaintersOrder and DisjointStencilINdex represent the most significant bits
 * of the key, and are shared by all SortKeys produced by the same draw. Next, the pipeline
 * description is encoded in two steps:
 *  1. The index of the RenderStep packed in the high bits to ensure each step for a draw is
 *     ordered correctly.
 *  2. An index into a cache of pipeline descriptions is used to encode the identity of the
 *     pipeline (SortKeys that differ in the bits from #1 necessarily would have different
 *     descriptions, but then the specific ordering of the RenderSteps isn't enforced).
 * Last, the SortKey encodes an index into the set of uniform bindings accumulated for a DrawPass.
 * This allows the SortKey to cluster draw steps that have both a compatible pipeline and do not
 * require rebinding uniform data or other state (e.g. scissor). Since the uniform data index and
 * the pipeline description index are packed into indices and not actual pointers, a given SortKey
 * is only valid for the a specific DrawList->DrawPass conversion.
 */
class DrawPass::SortKey {
public:
    SortKey(const DrawList::Draw* draw,
            int renderStep,
            int pipelineIndex,
            int geomUniformIndex,
            int shadingUniformIndex)
        : fPipelineKey{draw->fOrder.paintOrder().bits(),
                       draw->fOrder.stencilIndex().bits(),
                       static_cast<uint32_t>(renderStep),
                       static_cast<uint32_t>(pipelineIndex)}
        , fUniformKey{geomUniformIndex, shadingUniformIndex} {}

    bool operator<(const SortKey& k) const {
        uint64_t k1 = this->pipelineKey();
        uint64_t k2 = k.pipelineKey();
        return k1 < k2 || (k1 == k2 && this->uniformKey() < k.uniformKey());
    }

    const DrawList::Draw* draw() const { return fDraw; }
    int pipeline() const { return static_cast<int>(fPipelineKey.fPipeline); }
    int renderStep() const { return static_cast<int>(fPipelineKey.fRenderStep); }

    int geometryUniforms() const { return static_cast<int>(fUniformKey.fGeometryIndex); }
    int shadingUniforms() const { return static_cast<int>(fUniformKey.fShadingIndex); }

private:
    // Fields are ordered from most-significant to lowest when sorting by 128-bit value.
    struct {
        uint32_t fColorDepthOrder : 16; // sizeof(CompressedPaintersOrder)
        uint32_t fStencilOrder    : 16; // sizeof(DisjointStencilIndex)
        uint32_t fRenderStep      : 2;  // bits >= log2(Renderer::kMaxRenderSteps)
        uint32_t fPipeline        : 30; // bits >= log2(max steps * DrawList::kMaxDraws)
    } fPipelineKey; // NOTE: named for bit-punning, can't take address of a bit-field

    uint64_t pipelineKey() const { return sk_bit_cast<uint64_t>(fPipelineKey); }

    struct {
        int32_t fGeometryIndex; // bits >= log2(max steps * max draw count)
        int32_t fShadingIndex;  //  ""
    } fUniformKey;

    uint64_t uniformKey() const { return sk_bit_cast<uint64_t>(fUniformKey); }

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(16 >= sizeof(CompressedPaintersOrder));
    static_assert(16 >= sizeof(DisjointStencilIndex));
    static_assert(2  >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(30 >= SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
};

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawPass::DrawPass(sk_sp<TextureProxy> target, const SkIRect& bounds,
                   bool requiresStencil, bool requiresMSAA)
        : fTarget(std::move(target))
        , fBounds(bounds)
        , fRequiresStencil(requiresStencil)
        , fRequiresMSAA(requiresMSAA) {}

DrawPass::~DrawPass() = default;

std::unique_ptr<DrawPass> DrawPass::Make(std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         const BoundsManager* occlusionCuller) {
    // NOTE: This assert is here to ensure SortKey is as tightly packed as possible. Any change to
    // its size should be done with care and good reason. The performance of sorting the keys is
    // heavily tied to the total size.
    //
    // At 24 bytes (current), sorting is about 30% slower than if SortKey could be packed into just
    // 16 bytes. There are several ways this could be done if necessary:
    //  - Restricting the max draw count to 16k (14-bits) and only using a single index to refer to
    //    the uniform data => 8 bytes of key, 8 bytes of pointer.
    //  - Restrict the max draw count to 32k (15-bits), use a single uniform index, and steal the
    //    4 low bits from the Draw* pointer since it's 16 byte aligned.
    //  - Compact the Draw* to an index into the original collection, although that has extra
    //    indirection and does not work as well with SkTBlockList.
    // In pseudo tests, manipulating the pointer or having to mask out indices was about 15% slower
    // than an 8 byte key and unmodified pointer.
    static_assert(sizeof(DrawPass::SortKey) == 16 + sizeof(void*));
    return std::unique_ptr<DrawPass>(new DrawPass(std::move(target), {0, 0, 0, 0}, true, true));
}

} // namespace skgpu
