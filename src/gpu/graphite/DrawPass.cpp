/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawPass.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkTBlockList.h"
#include "src/base/SkVx.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineCreationTask.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/Resource.h"  // IWYU pragma: keep
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
#include "src/gpu/graphite/RenderPassDesc.h"
#endif

#include <algorithm>
#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>

using namespace skia_private;

namespace skgpu::graphite {

class ShaderCodeDictionary;

namespace {

// Helper to manage packed fields within a uint64_t
template <uint64_t Bits, uint64_t Offset>
struct Bitfield {
    static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
    static constexpr uint64_t kOffset = Offset;
    static constexpr uint64_t kBits = Bits;

    static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
    static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
};

// NOTE: TextureBinding's use as a key type in DenseBiMap relies on the fact that the underlying
// data has been de-duplicated by a PipelineDataCache earlier, so that the bit identity of the data
// blocks (e.g. address+size) is equivalent to the content equality of the texture lists.

// Tracks the combination of textures from the paint and from the RenderStep to describe the full
// binding that needs to be in the command list.
struct TextureBinding {
    TextureDataBlock fPaintTextures;
    TextureDataBlock fStepTextures;

    bool operator==(const TextureBinding& other) const {
        return fPaintTextures == other.fPaintTextures &&
               fStepTextures == other.fStepTextures;
    }
    bool operator!=(const TextureBinding& other) const { return !(*this == other); }

    int numTextures() const {
        return (fPaintTextures ? fPaintTextures.numTextures() : 0) +
               (fStepTextures ? fStepTextures.numTextures() : 0);
    }
};

// Writes uniform data either to uniform buffers or to shared storage buffers, and tracks when
// bindings need to change between draws.
class UniformTracker {
public:
    UniformTracker(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    bool writeUniforms(UniformDataCache& uniformCache,
                       DrawBufferManager* bufferMgr,
                       UniformDataCache::Index index) {
        if (index >= UniformDataCache::kInvalidIndex) {
            return false;
        }

        if (index == fLastIndex) {
            return false;
        }
        fLastIndex = index;

        UniformDataCache::Entry& uniformData = uniformCache.lookup(index);
        const size_t uniformDataSize = uniformData.fCpuData.size();

        // Upload the uniform data if we haven't already.
        // Alternatively, re-upload the uniform data to avoid a rebind if we're using storage
        // buffers. This will result in more data uploaded, but the tradeoff seems worthwhile.
        if (!uniformData.fBufferBinding.fBuffer ||
            (fUseStorageBuffers && uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer)) {
            UniformWriter writer;
            std::tie(writer, uniformData.fBufferBinding) =
                    fUseStorageBuffers ? bufferMgr->getAlignedSsboWriter(1, uniformDataSize)
                                       : bufferMgr->getUniformWriter(1, uniformDataSize);

            // Early out if buffer mapping failed.
            if (!writer) {
                return {};
            }

            writer.write(uniformData.fCpuData.data(), uniformDataSize);

            if (fUseStorageBuffers) {
                // When using storage buffers, store the SSBO index in the binding's offset field
                // and always use the entire buffer's size in the size field.
                SkASSERT(uniformData.fBufferBinding.fOffset % uniformDataSize == 0);
                uniformData.fBufferBinding.fOffset /= uniformDataSize;
                uniformData.fBufferBinding.fSize = uniformData.fBufferBinding.fBuffer->size();
            }
        }

        const bool needsRebind =
                uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer ||
                (!fUseStorageBuffers && uniformData.fBufferBinding.fOffset != fLastBinding.fOffset);

        fLastBinding = uniformData.fBufferBinding;

        return needsRebind;
    }

    void bindUniforms(UniformSlot slot, DrawPassCommands::List* commandList) {
        BindBufferInfo binding = fLastBinding;
        if (fUseStorageBuffers) {
            // Track the SSBO index in fLastBinding, but set offset = 0 in the actual used binding.
            binding.fOffset = 0;
        }
        commandList->bindUniformBuffer(binding, slot);
    }

    uint32_t ssboIndex() const {
        // The SSBO index for the last-bound storage buffer is stored in the binding's offset field.
        return fLastBinding.fOffset;
    }

private:
    // Internally track the last binding returned, so that we know whether new uploads or rebindings
    // are necessary. If we're using SSBOs, this is treated specially -- the fOffset field holds the
    // index in the storage buffer of the last-written uniforms, and the offsets used for actual
    // bindings are always zero.
    BindBufferInfo fLastBinding;

    // This keeps track of the last index used for writing uniforms from a provided uniform cache.
    // If a provided index matches the last index, the uniforms are assumed to already be written
    // and no additional uploading is performed. This assumes a UniformTracker will always be
    // provided with the same uniform cache.
    UniformDataCache::Index fLastIndex = UniformDataCache::kInvalidIndex;

    const bool fUseStorageBuffers;
};

// Tracks when to issue BindTexturesAndSamplers commands to a command list and converts
// TextureDataBlocks to that representation as needed.
class TextureTracker {
public:
    TextureTracker(TextureDataCache* textureCache)
            : fTextureCache(textureCache) {}

    bool setCurrentTextureBindings(TextureDataCache::Index bindingIndex) {
        if (bindingIndex < TextureDataCache::kInvalidIndex && fLastIndex != bindingIndex) {
            fLastIndex = bindingIndex;
            return true;
        }
        // No binding change
        return false;
    }

    void bindTextures(DrawPassCommands::List* commandList) {
        SkASSERT(fLastIndex < TextureDataCache::kInvalidIndex);
        TextureDataBlock binding = fTextureCache->lookup(fLastIndex);

        auto [textures, samplers] =
                commandList->bindDeferredTexturesAndSamplers(binding.numTextures());

        for (int i = 0; i < binding.numTextures(); ++i) {
            auto [t, s] = binding.texture(i);
            textures[i] = t.get();
            samplers[i] = s;
        }
    }

private:
    TextureDataCache::Index fLastIndex = TextureDataCache::kInvalidIndex;

    TextureDataCache* const fTextureCache;
};

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Each Draw in a DrawList might be processed by multiple RenderSteps (determined by the Draw's
 * Renderer), which can be sorted independently. Each (step, draw) pair produces its own SortKey.
 *
 * The goal of sorting draws for the DrawPass is to minimize pipeline transitions and dynamic binds
 * within a pipeline, while still respecting the overall painter's order. This decreases the number
 * of low-level draw commands in a command buffer and increases the size of those, allowing the GPU
 * to operate more efficiently and have fewer bubbles within its own instruction stream.
 *
 * The Draw's CompresssedPaintersOrder and DisjointStencilIndex represent the most significant bits
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
            GraphicsPipelineCache::Index pipelineIndex,
            UniformDataCache::Index geomUniformIndex,
            UniformDataCache::Index shadingUniformIndex,
            TextureDataCache::Index textureBindingIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->drawParams().order().paintOrder().bits()) |
                       StencilIndexField::set(draw->drawParams().order().stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))                  |
                       PipelineField::set(pipelineIndex))
        , fUniformKey(GeometryUniformField::set(geomUniformIndex)   |
                      ShadingUniformField::set(shadingUniformIndex) |
                      TextureBindingsField::set(textureBindingIndex))
        , fDraw(draw) {
        SkASSERT(pipelineIndex < GraphicsPipelineCache::kInvalidIndex);
        SkASSERT(renderStep <= draw->renderer()->numRenderSteps());
    }

    bool operator<(const SortKey& k) const {
        return fPipelineKey < k.fPipelineKey ||
               (fPipelineKey == k.fPipelineKey && fUniformKey < k.fUniformKey);
    }

    const RenderStep& renderStep() const {
        return fDraw->renderer()->step(RenderStepField::get(fPipelineKey));
    }

    const DrawList::Draw& draw() const { return *fDraw; }

    GraphicsPipelineCache::Index pipelineIndex() const {
        return PipelineField::get(fPipelineKey);
    }
    UniformDataCache::Index geometryUniformIndex() const {
        return GeometryUniformField::get(fUniformKey);
    }
    UniformDataCache::Index shadingUniformIndex() const {
        return ShadingUniformField::get(fUniformKey);
    }
    TextureDataCache::Index textureBindingIndex() const {
        return TextureBindingsField::get(fUniformKey);
    }

private:
    // Fields are ordered from most-significant to least when sorting by 128-bit value.
    // NOTE: We don't use C++ bit fields because field ordering is implementation defined and we
    // need to sort consistently.
    using ColorDepthOrderField = Bitfield<16, 48>; // sizeof(CompressedPaintersOrder)
    using StencilIndexField    = Bitfield<16, 32>; // sizeof(DisjointStencilIndex)
    using RenderStepField      = Bitfield<2,  30>; // bits >= log2(Renderer::kMaxRenderSteps)
    using PipelineField        = Bitfield<30, 0>;  // bits >= log2(max total steps in draw list)
    uint64_t fPipelineKey;

    // The uniform/texture index fields need 1 extra bit to encode "no-data". Values that are
    // greater than or equal to 2^(bits-1) represent "no-data", while values between
    // [0, 2^(bits-1)-1] can access data arrays without extra logic.
    using GeometryUniformField = Bitfield<17, 47>; // bits >= 1+log2(max total steps)
    using ShadingUniformField  = Bitfield<17, 30>; // bits >= 1+log2(max total steps)
    using TextureBindingsField = Bitfield<30, 0>;  // bits >= 1+log2(max total steps)
    uint64_t fUniformKey;

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
    static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
    static_assert(RenderStepField::kBits      >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(PipelineField::kBits        >= SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(GeometryUniformField::kBits >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(ShadingUniformField::kBits  >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(TextureBindingsField::kBits >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
};

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor,
                   sk_sp<FloatStorageManager> floatStorageManager)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor)
        , fFloatStorageManager(floatStorageManager) {}

DrawPass::~DrawPass() = default;

namespace {
bool paint_uses_advanced_blend_equation(std::optional<PaintParams> drawPaintParams) {
    if (!drawPaintParams.has_value() || !drawPaintParams.value().asFinalBlendMode().has_value()) {
        return false;
    }

    return (int)drawPaintParams.value().asFinalBlendMode().value() >
           (int)SkBlendMode::kLastCoeffMode;
}
} // anonymous

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         const SkImageInfo& targetInfo,
                                         std::pair<LoadOp, StoreOp> ops,
                                         std::array<float, 4> clearColor,
                                         const DstReadStrategy dstReadStrategy) {
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
    static_assert(sizeof(DrawPass::SortKey) ==
                  SkAlignTo(16 + sizeof(void*), alignof(DrawPass::SortKey)));

    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", draws->fDraws.count());

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(target, ops, clearColor,
                                                    recorder->priv().refFloatStorageManager()));

    Rect passBounds = Rect::InfiniteInverted();

    UniformDataCache geometryUniformDataCache;
    UniformDataCache shadingUniformDataCache;
    TextureDataCache textureDataCache;
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    if (bufferMgr->hasMappingFailed()) {
        SKGPU_LOG_W("Buffer mapping has already failed; dropping draw pass!");
        return nullptr;
    }

    GraphicsPipelineCache pipelineCache;

    // Geometry uniforms are currently always UBO-backed.
    const Caps* caps = recorder->priv().caps();
    const bool useStorageBuffers = caps->storageBufferSupport();
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    Layout uniformLayout =
            useStorageBuffers ? bindingReqs.fStorageBufferLayout : bindingReqs.fUniformBufferLayout;

    ShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);

    // The initial layout we pass here is not important as it will be re-assigned when writing
    // shading and geometry uniforms below.
    PipelineDataGatherer gatherer(uniformLayout);
    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount());

    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        gatherer.resetForDraw();

        UniquePaintParamsID shaderID = UniquePaintParamsID::Invalid();
        UniformDataCache::Index shadingUniformIndex = UniformDataCache::kInvalidIndex;

        if (draw.paintParams().has_value()) {
            SkDEBUGCODE(builder.checkReset());
            SkDEBUGCODE(gatherer.checkReset());

            auto& geometry = draw.drawParams().geometry();
            KeyContext keyContext(recorder,
                                  drawPass->floatStorageManager(),
                                  &builder,
                                  &gatherer,
                                  draw.drawParams().transform(),
                                  targetInfo.colorInfo(),
                                  geometry.isShape() || geometry.isEdgeAAQuad()
                                    ? KeyGenFlags::kDefault
                                    : KeyGenFlags::kDisableSamplingOptimization,
                                  draw.paintParams().value().color());
#if defined(SK_DEBUG)
            auto result = draw.paintParams().value().toKey(keyContext);
            auto [dependsOnDst, dstReadReq, usesAdvancedBlend] = *result;
#else
            draw.paintParams().value().toKey(keyContext);
#endif
            SkASSERT(dependsOnDst == draw.dependsOnDst());
            SkASSERT(dstReadReq == draw.dstReadReq());
            SkASSERT(usesAdvancedBlend == paint_uses_advanced_blend_equation(draw.paintParams()));

            shaderID = recorder->priv().shaderCodeDictionary()->findOrCreate(&builder);
            if (shaderID.isValid()) {
                UniformDataBlock paintUniforms = gatherer.endPaintData();
                if (paintUniforms) {
                    shadingUniformIndex = shadingUniformDataCache.insert(paintUniforms);
                }
            }
        } // else depth-only, no paint data

        // Create a sort key for every render step in this draw, extracting out any
        // RenderStep-specific data.
        gatherer.setRenderStepManagerActive();
        for (int stepIndex = 0; stepIndex < draw.renderer()->numRenderSteps(); ++stepIndex) {
            gatherer.rewindForRenderStep();

            const RenderStep* const step = draw.renderer()->steps()[stepIndex];
            const bool performsShading = draw.paintParams().has_value() && step->performsShading();

            GraphicsPipelineCache::Index pipelineIndex = pipelineCache.insert(
                    { step->renderStepID(),
                    performsShading ? shaderID : UniquePaintParamsID::Invalid() });

            step->writeUniformsAndTextures(draw.drawParams(), &gatherer);
            auto [stepUniforms, combinedTextures] = gatherer.endRenderStepData(performsShading);

            UniformDataCache::Index geomUniformIndex = stepUniforms ?
                    geometryUniformDataCache.insert(stepUniforms) : UniformDataCache::kInvalidIndex;
            TextureDataCache::Index textureBindingIndex = combinedTextures ?
                    textureDataCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

            keys.push_back({&draw, stepIndex, pipelineIndex,
                            geomUniformIndex, shadingUniformIndex, textureBindingIndex});
        }

        passBounds.join(draw.drawParams().clip().drawBounds());
    }

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(keys.begin(), keys.end());
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);
    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    const SkIRect targetBounds = SkIRect::MakeSize(targetInfo.dimensions());
    SkIRect lastScissor = targetBounds;

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    UniformTracker geometryUniformTracker(useStorageBuffers);
    UniformTracker shadingUniformTracker(useStorageBuffers);

    // TODO(b/372953722): Remove this forced binding command behavior once dst copies are always
    // bound separately from the rest of the textures.
    const bool rebindTexturesOnPipelineChange = dstReadStrategy == DstReadStrategy::kTextureCopy;
    // Keep track of the prior draw's PaintOrder. If the current draw requires barriers and there
    // is no pipeline or state change, then we must compare the current and prior draw's PaintOrders
    // to determine if the draws overlap. If they do, we must inject a flush between them such that
    // the barrier addition and draw commands are ordered correctly.
    CompressedPaintersOrder priorDrawPaintOrder {};

    // If a draw uses an advanced blend mode and the device supports this via noncoherent blending,
    // then we must insert the appropriate barrier and ensure that the draws do not overlap.
    const bool advancedBlendsRequireBarrier =
            caps->blendEquationSupport() == Caps::BlendEquationSupport::kAdvancedNoncoherent;

#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
    // Accumulate rough pixel area touched by each pipeline as we iterate the SortKeys
    drawPass->fPipelineDrawAreas.push_back_n(pipelineCache.count(), 0.f);
#endif

    TextureTracker textureBindingTracker(&textureDataCache);
    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;
#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
        drawPass->fPipelineDrawAreas[key.pipelineIndex()] +=
                draw.drawParams().clip().drawBounds().area();
#endif

        const bool geomBindingChange = geometryUniformTracker.writeUniforms(
                geometryUniformDataCache, bufferMgr, key.geometryUniformIndex());
        const bool shadingBindingChange = shadingUniformTracker.writeUniforms(
                shadingUniformDataCache, bufferMgr, key.shadingUniformIndex());

        // TODO(b/372953722): The Dawn and Vulkan CommandBuffer implementations currently append any
        // dst copy to the texture bind group/descriptor set automatically when processing a
        // BindTexturesAndSamplers call because they use a single group to contain all textures.
        // However, from the DrawPass POV, we can run into the scenario where two pipelines have the
        // same textures+samplers except one requires a dst-copy and the other does not. In this
        // case we wouldn't necessarily insert a new command when the pipeline changed and then
        // end up with layout validation errors.
        const bool textureBindingsChange = textureBindingTracker.setCurrentTextureBindings(
                key.textureBindingIndex()) ||
                (rebindTexturesOnPipelineChange && pipelineChange &&
                 key.textureBindingIndex() != TextureDataCache::kInvalidIndex);

        std::optional<SkIRect> newScissor =
                renderStep.getScissor(draw.drawParams(), lastScissor, targetBounds);

        // Determine + analyze draw properties to inform whether we need to issue barriers before
        // issuing draw calls.
        bool drawsOverlap = priorDrawPaintOrder != draw.drawParams().order().paintOrder();
        bool drawUsesAdvancedBlendMode = paint_uses_advanced_blend_equation(draw.paintParams());

        std::optional<BarrierType> barrierToAddBeforeDraws = std::nullopt;
        if (dstReadStrategy == DstReadStrategy::kReadFromInput && draw.dstReadReq()) {
            barrierToAddBeforeDraws = BarrierType::kReadDstFromInput;
        }
        if (drawUsesAdvancedBlendMode &&
            caps->supportsHardwareAdvancedBlending() &&
            advancedBlendsRequireBarrier) {
            // A draw should only read from the dst OR use hardware for advanced blend modes.
            SkASSERT(!draw.dstReadReq());

            barrierToAddBeforeDraws = BarrierType::kAdvancedNoncoherentBlend;
        }

        const bool stateChange = geomBindingChange ||
                                 shadingBindingChange ||
                                 textureBindingsChange ||
                                 newScissor.has_value();

        // Update DrawWriter *before* we actually change any state so that accumulated draws from
        // the previous state use the proper state.
        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep.primitiveType(),
                                        renderStep.staticDataStride(),
                                        renderStep.appendDataStride(),
                                        renderStep.getRenderStateFlags(),
                                        barrierToAddBeforeDraws);
        } else if (stateChange) {
            drawWriter.newDynamicState();
        } else if (barrierToAddBeforeDraws.has_value() && drawsOverlap) {
            // Even if there is no pipeline or state change, we must consider whether a
            // DrawPassCommand to add barriers must be inserted before any draw commands. If so,
            // then determine if the current and prior draws overlap (ie, their PaintOrders are
            // unequal). If so, perform a flush() to make sure the draw and add barrier commands are
            // appended to the command list in the proper order.
            drawWriter.flush();
        }

        // Make state changes before accumulating new draw data
        if (pipelineChange) {
            drawPass->fCommandList.bindGraphicsPipeline(key.pipelineIndex());
            lastPipeline = key.pipelineIndex();
        }
        if (stateChange) {
            if (geomBindingChange) {
                geometryUniformTracker.bindUniforms(UniformSlot::kRenderStep,
                                                    &drawPass->fCommandList);
            }
            if (shadingBindingChange) {
                shadingUniformTracker.bindUniforms(UniformSlot::kPaint, &drawPass->fCommandList);
            }
            if (textureBindingsChange) {
                textureBindingTracker.bindTextures(&drawPass->fCommandList);
            }
            if (newScissor.has_value()) {
                drawPass->fCommandList.setScissor(*newScissor);
                lastScissor = *newScissor;
            }
        }

        uint32_t geometrySsboIndex = useStorageBuffers ? geometryUniformTracker.ssboIndex() : 0;
        uint32_t shadingSsboIndex = useStorageBuffers ? shadingUniformTracker.ssboIndex() : 0;
        skvx::uint2 ssboIndices = {geometrySsboIndex, shadingSsboIndex};
        renderStep.writeVertices(&drawWriter, draw.drawParams(), ssboIndices);

        if (bufferMgr->hasMappingFailed()) {
            SKGPU_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            return nullptr;
        }

        // Update priorDrawPaintOrder value before iterating to analyze the next draw.
        priorDrawPaintOrder = draw.drawParams().order().paintOrder();
    }
    // Finish recording draw calls for any collected data still pending at end of the loop
    drawWriter.flush();

    drawPass->fBounds = passBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs   = pipelineCache.detach();
    drawPass->fSampledTextures = textureDataCache.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    return drawPass;
}

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                sk_sp<const RuntimeEffectDictionary> runtimeDict,
                                const RenderPassDesc& renderPassDesc) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fPipelineHandles.reserve(fPipelineDescs.size());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs) {
        fPipelineHandles.push_back(
                resourceProvider->createGraphicsPipelineHandle(pipelineDesc,
                                                               renderPassDesc,
                                                               PipelineCreationFlags::kNone));
        resourceProvider->startPipelineCreationTask(runtimeDict, fPipelineHandles.back());
    }

    // The DrawPass may be long lived on a Recording and we no longer need the GraphicPipelineDescs
    // once we've created pipeline handles, so we drop the storage for them here.
    fPipelineDescs.clear();

    // TODO(robertphillips): move this resolvePipeline loop to addResourceRefs
    fFullPipelines.reserve(fPipelineHandles.size());
    for (const GraphicsPipelineHandle& handle : fPipelineHandles) {
        sk_sp<GraphicsPipeline> pipeline = resourceProvider->resolveHandle(handle);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create GraphicsPipeline for draw in RenderPass. Dropping pass!");
            return false;
        }
        fFullPipelines.push_back(std::move(pipeline));
    }
    fPipelineHandles.clear();

    for (int i = 0; i < fSampledTextures.size(); ++i) {
        // It should not have been possible to draw an Image that has an invalid texture info
        SkASSERT(fSampledTextures[i]->textureInfo().isValid());
        // Tasks should have been ordered to instantiate any scratch textures already, or any
        // client-owned image will have been instantiated at creation. However, if a TextureProxy
        // was cached for reuse across Recordings, it's possible that the initializing Recording
        // failed, leaving the TextureProxy in a bad state (and currently with no way to reconstruct
        // the tasks required to initialize it).
        // TODO(b/409888039): Once TextureProxies track their dependendent tasks to include in all
        // Recordings, this "should" be able to changed to asserts.
        if (!fSampledTextures[i]->isInstantiated() && !fSampledTextures[i]->isLazy()) {
            SKGPU_LOG_W("Cannot sample from an uninstantiated TextureProxy, label %s",
                        fSampledTextures[i]->label());
            return false;
        }
    }

#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "GraphitePipelineUse");
        TRACE_EVENT0_ALWAYS("skia.shaders", TRACE_STR_COPY(renderPassDesc.toString().c_str()));
        for (int i = 0 ; i < fFullPipelines.size(); ++i) {
            TRACE_EVENT_INSTANT1_ALWAYS(
                    "skia.shaders",
                    TRACE_STR_COPY(fFullPipelines[i]->getPipelineInfo().fLabel.c_str()),
                    TRACE_EVENT_SCOPE_THREAD,
                    "area", sk_float_saturate2int(fPipelineDrawAreas[i]));
        }
    }
#endif

    return true;
}

bool DrawPass::addResourceRefs(ResourceProvider* resourceProvider,
                               CommandBuffer* commandBuffer) {
    for (int i = 0; i < fFullPipelines.size(); ++i) {
        commandBuffer->trackResource(fFullPipelines[i]);
    }
    for (int i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackCommandBufferResource(fSampledTextures[i]->refTexture());
    }

    return true;
}

} // namespace skgpu::graphite
