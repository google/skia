/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawPass.h"

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/GraphicsPipeline.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/UniformCache.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/BoundsManager.h"

#include "src/core/SkMathPriv.h"
#include "src/core/SkTBlockList.h"
#include "src/gpu/BufferWriter.h"

#include <algorithm>
#include <unordered_map>

namespace skgpu {

// Helper to manage packed fields within a uint64_t
template <uint64_t Bits, uint64_t Offset>
struct Bitfield {
    static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
    static constexpr uint64_t kOffset = Offset;
    static constexpr uint64_t kBits = Bits;

    static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
    static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
};

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
            uint32_t pipelineIndex,
            uint32_t geomUniformIndex,
            uint32_t shadingUniformIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->fOrder.paintOrder().bits()) |
                       StencilIndexField::set(draw->fOrder.stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))     |
                       PipelineField::set(pipelineIndex))
        , fUniformKey(GeometryUniformField::set(geomUniformIndex) |
                      ShadingUniformField::set(shadingUniformIndex))
        , fDraw(draw) {
        SkASSERT(renderStep <= draw->fRenderer.numRenderSteps());
    }

    bool operator<(const SortKey& k) const {
        return fPipelineKey < k.fPipelineKey ||
               (fPipelineKey == k.fPipelineKey && fUniformKey < k.fUniformKey);
    }

    const RenderStep& renderStep() const {
        return *fDraw->fRenderer.steps()[RenderStepField::get(fPipelineKey)];
    }

    const DrawList::Draw* draw() const { return fDraw; }

    uint32_t pipeline()          const { return PipelineField::get(fPipelineKey);       }
    uint32_t geometryUniforms()  const { return GeometryUniformField::get(fUniformKey); }
    uint32_t shadingUniforms()   const { return ShadingUniformField::get(fUniformKey);  }

private:
    // Fields are ordered from most-significant to least when sorting by 128-bit value.
    // NOTE: We don't use bit fields because field ordering is implementation defined and we need
    // to sort consistently.
    using ColorDepthOrderField = Bitfield<16, 48>; // sizeof(CompressedPaintersOrder)
    using StencilIndexField    = Bitfield<16, 32>; // sizeof(DisjointStencilIndex)
    using RenderStepField      = Bitfield<2,  30>; // bits >= log2(Renderer::kMaxRenderSteps)
    using PipelineField        = Bitfield<30, 0>;  // bits >= log2(max steps*DrawList::kMaxDraws)
    uint64_t fPipelineKey;

    using GeometryUniformField = Bitfield<32, 32>; // bits >= log2(max steps * max draw count)
    using ShadingUniformField  = Bitfield<32, 0>;  //  ""
    uint64_t fUniformKey;

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
    static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
    static_assert(RenderStepField::kBits      >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(PipelineField::kBits        >=
                        SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(GeometryUniformField::kBits >= PipelineField::kBits);
    static_assert(ShadingUniformField::kBits  >= PipelineField::kBits);
};

class DrawPass::Drawer final : public DrawDispatcher {
public:
    Drawer(DrawPass* drawPass) : fPass(drawPass) {}
    ~Drawer() override = default;

    void bindDrawBuffers(BindBufferInfo vertexAttribs,
                         BindBufferInfo instanceAttribs,
                         BindBufferInfo indices) override {
        fPass->fCommands.emplace_back(BindDrawBuffers{vertexAttribs, instanceAttribs, indices});
    }

    void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) override {
        fPass->fCommands.emplace_back(Draw{type, baseVertex, vertexCount});
    }

    void drawIndexed(PrimitiveType type, unsigned int baseIndex,
                     unsigned int indexCount, unsigned int baseVertex) override {
        fPass->fCommands.emplace_back(DrawIndexed{type, baseIndex, indexCount, baseVertex});
    }

    void drawInstanced(PrimitiveType type,
                       unsigned int baseVertex, unsigned int vertexCount,
                       unsigned int baseInstance, unsigned int instanceCount) override {
        fPass->fCommands.emplace_back(DrawInstanced{type, baseVertex, vertexCount,
                                                    baseInstance, instanceCount});
    }

    void drawIndexedInstanced(PrimitiveType type,
                              unsigned int baseIndex, unsigned int indexCount,
                              unsigned int baseVertex, unsigned int baseInstance,
                              unsigned int instanceCount) override {
        fPass->fCommands.emplace_back(DrawIndexedInstanced{type, baseIndex, indexCount, baseVertex,
                                                           baseInstance, instanceCount});
    }

private:
    DrawPass* fPass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

class UniformBindingCache {
public:
    UniformBindingCache(DrawBufferManager* bufferMgr, UniformCache* cache)
            : fBufferMgr(bufferMgr), fCache(cache) {}

    uint32_t addUniforms(sk_sp<UniformData> data) {
        if (!data) {
            return UniformCache::kInvalidUniformID;
        }

        uint32_t index = fCache->insert(data);
        if (fBindings.find(index) == fBindings.end()) {
            // First time encountering this data, so upload to the GPU
            auto [writer, bufferInfo] = fBufferMgr->getUniformWriter(data->dataSize());
            writer.write(data->data(), data->dataSize());
            fBindings.insert({index, bufferInfo});
        }

        return index;
    }

    BindBufferInfo getBinding(uint32_t uniformIndex) {
        auto lookup = fBindings.find(uniformIndex);
        SkASSERT(lookup != fBindings.end());
        return lookup->second;
    }

private:
    DrawBufferManager* fBufferMgr;
    UniformCache*      fCache;

    std::unordered_map<uint32_t, BindBufferInfo> fBindings;
};

// std::unordered_map implementation for GraphicsPipelineDesc* that de-reference the pointers.
struct Hash {
    size_t operator()(const skgpu::GraphicsPipelineDesc* desc) const noexcept {
        return skgpu::GraphicsPipelineDesc::Hash()(*desc);
    }
};

struct Eq {
    bool operator()(const skgpu::GraphicsPipelineDesc* a,
                    const skgpu::GraphicsPipelineDesc* b) const noexcept {
        return *a == *b;
    }
};

} // anonymous namespace

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor,
                   int renderStepCount)
        : fCommands(std::max(1, renderStepCount / 4), SkBlockAllocator::GrowthPolicy::kFibonacci)
        , fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor) {
    // TODO: Tune this estimate and the above "itemPerBlock" value for the command buffer sequence
    // After merging, etc. one pipeline per recorded draw+step combo is likely unnecessary.
    fPipelineDescs.reserve(renderStepCount);
    fCommands.reserve(renderStepCount);
}

DrawPass::~DrawPass() = default;

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         std::pair<LoadOp, StoreOp> ops,
                                         std::array<float, 4> clearColor,
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

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(std::move(target), ops, clearColor,
                                                    draws->renderStepCount()));

    Rect passBounds = Rect::InfiniteInverted();

    DrawBufferManager* bufferMgr = recorder->drawBufferManager();
    UniformCache geometryUniforms;
    UniformBindingCache geometryUniformBindings(bufferMgr, &geometryUniforms);
    UniformBindingCache shadingUniformBindings(bufferMgr, recorder->uniformCache());

    std::unordered_map<const GraphicsPipelineDesc*, uint32_t, Hash, Eq> pipelineDescToIndex;

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
        Combination shader;
        sk_sp<UniformData> shadingUniforms = nullptr;
        uint32_t shadingIndex = UniformCache::kInvalidUniformID;
        if (draw.fPaintParams.has_value()) {
            std::tie(shader, shadingUniforms) = ExtractCombo(draw.fPaintParams.value());
            shadingIndex = shadingUniformBindings.addUniforms(shadingUniforms);
        } // else depth-only

        for (int stepIndex = 0; stepIndex < draw.fRenderer.numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer.steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            Combination stepShader;
            uint32_t stepShadingIndex = UniformCache::kInvalidUniformID;
            if (performsShading) {
                stepShader = shader;
                stepShadingIndex = shadingIndex;
            } // else depth-only draw or stencil-only step of renderer so no shading is needed

            uint32_t geometryIndex = UniformCache::kInvalidUniformID;
            if (step->numUniforms() > 0) {
                // TODO: Get layout from the GPU
                auto uniforms = step->writeUniforms(Layout::kMetal,
                                                    draw.fClip.scissor(),
                                                    draw.fTransform,
                                                    draw.fShape);
                geometryIndex = geometryUniformBindings.addUniforms(std::move(uniforms));
            }

            GraphicsPipelineDesc desc;
            desc.setProgram(step, stepShader);
            uint32_t pipelineIndex = 0;
            auto pipelineLookup = pipelineDescToIndex.find(&desc);
            if (pipelineLookup == pipelineDescToIndex.end()) {
                // Assign new index to first appearance of this pipeline description
                pipelineIndex = SkTo<uint32_t>(drawPass->fPipelineDescs.count());
                const GraphicsPipelineDesc& finalDesc = drawPass->fPipelineDescs.push_back(desc);
                pipelineDescToIndex.insert({&finalDesc, pipelineIndex});
            } else {
                // Reuse the existing pipeline description for better batching after sorting
                pipelineIndex = pipelineLookup->second;
            }

            keys.push_back({&draw, stepIndex, pipelineIndex, geometryIndex, stepShadingIndex});
        }

        passBounds.join(draw.fClip.drawBounds());
        drawPass->fDepthStencilFlags |= draw.fRenderer.depthStencilFlags();
        drawPass->fRequiresMSAA |= draw.fRenderer.requiresMSAA();
    }

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(keys.begin(), keys.end());

    // Used to record vertex/instance data, buffer binds, and draw calls
    Drawer drawer(drawPass.get());
    DrawWriter drawWriter(&drawer, bufferMgr);

    // Used to track when a new pipeline or dynamic state needs recording between draw steps.
    // Setting to # render steps ensures the very first time through the loop will bind a pipeline.
    uint32_t lastPipeline = draws->renderStepCount();
    uint32_t lastShadingUniforms = UniformCache::kInvalidUniformID;
    uint32_t lastGeometryUniforms = UniformCache::kInvalidUniformID;
    SkIRect lastScissor = SkIRect::MakeSize(drawPass->fTarget->dimensions());

    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = *key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipeline() != lastPipeline;
        const bool stateChange = key.geometryUniforms() != lastGeometryUniforms ||
                                 key.shadingUniforms() != lastShadingUniforms ||
                                 draw.fClip.scissor() != lastScissor;

        // Update DrawWriter *before* we actually change any state so that accumulated draws from
        // the previous state use the proper state.
        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep.primitiveType(),
                                        renderStep.vertexStride(),
                                        renderStep.instanceStride());
        } else if (stateChange) {
            drawWriter.newDynamicState();
        }

        // Make state changes before accumulating new draw data
        if (pipelineChange) {
            drawPass->fCommands.emplace_back(BindGraphicsPipeline{key.pipeline()});
            lastPipeline = key.pipeline();
            lastShadingUniforms = UniformCache::kInvalidUniformID;
            lastGeometryUniforms = UniformCache::kInvalidUniformID;
        }
        if (stateChange) {
            if (key.geometryUniforms() != lastGeometryUniforms) {
                if (key.geometryUniforms() != UniformCache::kInvalidUniformID) {
                    auto binding = geometryUniformBindings.getBinding(key.geometryUniforms());
                    drawPass->fCommands.emplace_back(
                            BindUniformBuffer{binding, UniformSlot::kRenderStep});
                }
                lastGeometryUniforms = key.geometryUniforms();
            }
            if (key.shadingUniforms() != lastShadingUniforms) {
                if (key.shadingUniforms() != UniformCache::kInvalidUniformID) {
                    auto binding = shadingUniformBindings.getBinding(key.shadingUniforms());
                    drawPass->fCommands.emplace_back(
                            BindUniformBuffer{binding, UniformSlot::kPaint});
                }
                lastShadingUniforms = key.shadingUniforms();
            }
            if (draw.fClip.scissor() != lastScissor) {
                drawPass->fCommands.emplace_back(SetScissor{draw.fClip.scissor()});
                lastScissor = draw.fClip.scissor();
            }
        }

        renderStep.writeVertices(&drawWriter, draw.fClip.scissor(), draw.fTransform, draw.fShape);
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    passBounds.roundOut();
    drawPass->fBounds = SkIRect::MakeLTRB((int) passBounds.left(), (int) passBounds.top(),
                                          (int) passBounds.right(), (int) passBounds.bot());
    return drawPass;
}

void DrawPass::addCommands(CommandBuffer* buffer, ResourceProvider* resourceProvider) const {
    // TODO: Validate RenderPass state against DrawPass's target and requirements?
    // Generate actual GraphicsPipeline objects combining the target-level properties and each of
    // the GraphicsPipelineDesc's referenced in this DrawPass.

    // Use a vector instead of SkTBlockList for the full pipelines so that random access is fast.
    std::vector<sk_sp<GraphicsPipeline>> fullPipelines;
    fullPipelines.reserve(fPipelineDescs.count());
    for (const GraphicsPipelineDesc& desc : fPipelineDescs.items()) {
        fullPipelines.push_back(resourceProvider->findOrCreateGraphicsPipeline(desc));
    }

    // Set viewport to the entire texture for now (eventually, we may have logically smaller bounds
    // within an approx-sized texture). It is assumed that this also configures the sk_rtAdjust
    // intrinsic for programs (however the backend chooses to do so).
    buffer->setViewport(0, 0, fTarget->dimensions().width(), fTarget->dimensions().height());

    for (const Command& c : fCommands.items()) {
        switch(c.fType) {
            case CommandType::kBindGraphicsPipeline: {
                auto& d = c.fBindGraphicsPipeline;
                buffer->bindGraphicsPipeline(fullPipelines[d.fPipelineIndex]);
                break; }
            case CommandType::kBindUniformBuffer: {
                auto& d = c.fBindUniformBuffer;
                buffer->bindUniformBuffer(d.fSlot, sk_ref_sp(d.fInfo.fBuffer), d.fInfo.fOffset);
                break; }
            case CommandType::kBindDrawBuffers: {
                auto& d = c.fBindDrawBuffers;
                buffer->bindDrawBuffers(d.fVertices, d.fInstances, d.fIndices);
                break; }
            case CommandType::kDraw: {
                auto& d = c.fDraw;
                buffer->draw(d.fType, d.fBaseVertex, d.fVertexCount);
                break; }
            case CommandType::kDrawIndexed: {
                auto& d = c.fDrawIndexed;
                buffer->drawIndexed(d.fType, d.fBaseIndex, d.fIndexCount, d.fBaseVertex);
                break; }
            case CommandType::kDrawInstanced: {
                auto& d = c.fDrawInstanced;
                buffer->drawInstanced(d.fType, d.fBaseVertex, d.fVertexCount,
                                      d.fBaseInstance, d.fInstanceCount);
                break; }
            case CommandType::kDrawIndexedInstanced: {
                auto& d = c.fDrawIndexedInstanced;
                buffer->drawIndexedInstanced(d.fType, d.fBaseIndex, d.fIndexCount, d.fBaseVertex,
                                             d.fBaseInstance, d.fInstanceCount);
                break; }
            case CommandType::kSetScissor: {
                auto& d = c.fSetScissor;
                buffer->setScissor(d.fScissor.fLeft, d.fScissor.fTop,
                                   d.fScissor.fRight, d.fScissor.fBottom);
                break;
            }
        }
    }
}

} // namespace skgpu
