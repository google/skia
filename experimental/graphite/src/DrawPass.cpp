/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawPass.h"

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/ProgramCache.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/geom/BoundsManager.h"

#include "src/core/SkMathPriv.h"
#include "src/core/SkUtils.h"

#include <algorithm>

namespace {

// Retrieve the program ID and uniformData ID
std::tuple<uint32_t, uint32_t> get_ids_from_paint(skgpu::Recorder* recorder,
                                                  skgpu::PaintParams params) {
    // TODO: add an ExtractCombo that takes PaintParams directly?
    SkPaint p;

    p.setColor(params.color());
    p.setBlendMode(params.blendMode());
    p.setShader(params.refShader());

    // TODO: perhaps just return the ids here rather than the sk_sps?
    auto [ combo, uniformData] = ExtractCombo(recorder->uniformCache(), p);
    auto programInfo = recorder->programCache()->findOrCreateProgram(combo);

    return { programInfo->id(), uniformData->id() };
}

} // anonymous namespace

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

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
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

    bool requiresStencil = false;
    bool requiresMSAA = false;
    Rect passBounds = Rect::InfiniteInverted();

    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount()); // will not exceed but may use less with occluded draws

    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        if (occlusionCuller && occlusionCuller->isOccluded(draw.fClip.drawBounds(),
                                                           draw.fOrder.depth())) {
            continue;
        }

        // If we have two different descriptors, such that the uniforms from the PaintParams can be
        // bound independently of those used by the rest of the RenderStep, then we can upload now
        // and remember the location for re-use on any RenderStep that does shading.
        uint32_t programID = ProgramCache::kInvalidProgramID;
        uint32_t shadingID = UniformData::kInvalidUniformID;
        if (draw.fPaintParams.has_value()) {
            std::tie(programID, shadingID) = get_ids_from_paint(recorder,
                                                                draw.fPaintParams.value());
        }

        for (int stepIndex = 0; stepIndex < draw.fRenderer.numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer.steps()[stepIndex];

            // TODO ask step to generate a pipeline description based on the above shading code, and
            // have pipelineIndex point to that description in the accumulated list of descs
            int pipelineIndex = 0;
            // TODO step writes out geometry uniforms and have geomIndex point to that buffer data,
            // providing shape, transform, scissor, and paint depth to RenderStep
            int geometryIndex = 0;

            int shadingIndex = -1;

            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();
            if (performsShading) {
                // TODO: we need to combine the 'programID' with the RenderPass info and the
                // geometric rendering method to get the true 'pipelineIndex'
                pipelineIndex = programID;
                shadingIndex = shadingID;
            } else {
                // TODO: fill in 'pipelineIndex' for Chris' stencil/depth draws
            }

            keys.push_back({&draw, stepIndex, pipelineIndex, geometryIndex, shadingIndex});
        }

        passBounds.join(draw.fClip.drawBounds());
        requiresStencil |= draw.fRenderer.requiresStencil();
        requiresMSAA |= draw.fRenderer.requiresMSAA();
    }

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(keys.begin(), keys.end());

    int lastPipeline = -1;
    int lastShadingUniforms = -1;
    int lastGeometryUniforms = -1;
    SkIRect lastScissor = SkIRect::MakeSize(target->dimensions());
    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = *key.draw();
        // TODO: Have the render step write out vertices and figure out what draw call function and
        // primitive type it uses. The vertex buffer binding/offset and draw params will be examined
        // to determine if the active draw can be updated to include the new vertices, or if it has
        // to be ended and a new one begun for this step. In addition to checking this state, must
        // also check if pipeline, uniform, scissor etc. would require the active draw to end.
        //
        // const RenderStep* const step = draw.fRenderer.steps()[key.renderStep()];

        if (key.pipeline() != lastPipeline) {
            // TODO: Look up pipeline description from key's index and record binding it
            lastPipeline = key.pipeline();
            lastShadingUniforms = -1;
            lastGeometryUniforms = -1;
        }
        if (key.geometryUniforms() != lastGeometryUniforms) {
            // TODO: Look up uniform buffer binding info corresponding to key's index and record it
            lastGeometryUniforms = key.geometryUniforms();
        }
        if (key.shadingUniforms() != lastShadingUniforms) {
            // TODO: As above, but for shading uniforms (assuming we have two descriptor
            // sets for the different uniform sources).)
            lastShadingUniforms = key.shadingUniforms();
        }

        if (draw.fClip.scissor() != lastScissor) {
            // TODO: Record new scissor rectangle
        }

        // TODO: Write vertex and index data for the draw step
    }

    // if (currentDraw) {
        // TODO: End the current draw if it has pending vertices
    // }

    passBounds.roundOut();
    SkIRect pxPassBounds = SkIRect::MakeLTRB((int) passBounds.left(), (int) passBounds.top(),
                                             (int) passBounds.right(), (int) passBounds.bot());
    return std::unique_ptr<DrawPass>(new DrawPass(std::move(target), pxPassBounds,
                                                  requiresStencil, requiresMSAA));
}

} // namespace skgpu
