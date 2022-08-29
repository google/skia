/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawPass.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawBufferManager.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/BoundsManager.h"

#include "src/core/SkMathPriv.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkTBlockList.h"

#include <algorithm>
#include <unordered_map>

namespace skgpu::graphite {

// Helper to manage packed fields within a uint64_t
template <uint64_t Bits, uint64_t Offset>
struct Bitfield {
    static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
    static constexpr uint64_t kOffset = Offset;
    static constexpr uint64_t kBits = Bits;

    static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
    static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
};

struct TextureBindingBlock {
    TextureDataCache::Index fPaintTextureIndex;
    TextureDataCache::Index fStepTextureIndex;

    static std::unique_ptr<TextureBindingBlock> Make(const TextureBindingBlock& other,
                                                     SkArenaAlloc*) {
        return std::make_unique<TextureBindingBlock>(other);
    }

    bool operator==(const TextureBindingBlock& other) const {
        return fPaintTextureIndex == other.fPaintTextureIndex &&
               fStepTextureIndex == other.fStepTextureIndex;
    }
    bool operator!=(const TextureBindingBlock& other) const { return !(*this == other);  }

    uint32_t hash() const {
        uint32_t hash = 0;
        uint32_t index = fPaintTextureIndex.asUInt();
        hash = SkOpts::hash_fn(&index, sizeof(index), hash);
        index = fStepTextureIndex.asUInt();
        hash = SkOpts::hash_fn(&index, sizeof(index), hash);

        return hash;
    }
};
using TextureBindingCache =
        PipelineDataCache<std::unique_ptr<TextureBindingBlock>, TextureBindingBlock>;

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
            UniformDataCache::Index geomUniformIndex,
            UniformDataCache::Index shadingUniformIndex,
            TextureBindingCache::Index textureBindingIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->fDrawParams.order().paintOrder().bits()) |
                       StencilIndexField::set(draw->fDrawParams.order().stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))                  |
                       PipelineField::set(pipelineIndex))
        , fUniformKey(GeometryUniformField::set(geomUniformIndex.asUInt())   |
                      ShadingUniformField::set(shadingUniformIndex.asUInt()) |
                      TextureBindingsField::set(textureBindingIndex.asUInt()))
        , fDraw(draw) {
        SkASSERT(renderStep <= draw->fRenderer->numRenderSteps());
    }

    bool operator<(const SortKey& k) const {
        return fPipelineKey < k.fPipelineKey ||
               (fPipelineKey == k.fPipelineKey && fUniformKey < k.fUniformKey);
    }

    const RenderStep& renderStep() const {
        return fDraw->fRenderer->step(RenderStepField::get(fPipelineKey));
    }

    const DrawList::Draw* draw() const { return fDraw; }

    uint32_t pipeline() const { return PipelineField::get(fPipelineKey); }
    UniformDataCache::Index geometryUniforms() const {
        return UniformDataCache::Index(GeometryUniformField::get(fUniformKey));
    }
    UniformDataCache::Index shadingUniforms() const {
        return UniformDataCache::Index(ShadingUniformField::get(fUniformKey));
    }
    TextureBindingCache::Index textureBindings() const {
        return TextureBindingCache::Index(TextureBindingsField::get(fUniformKey));
    }

private:
    // Fields are ordered from most-significant to least when sorting by 128-bit value.
    // NOTE: We don't use bit fields because field ordering is implementation defined and we need
    // to sort consistently.
    using ColorDepthOrderField = Bitfield<16, 48>; // sizeof(CompressedPaintersOrder)
    using StencilIndexField    = Bitfield<16, 32>; // sizeof(DisjointStencilIndex)
    using RenderStepField      = Bitfield<2,  30>; // bits >= log2(Renderer::kMaxRenderSteps)
    using PipelineField        = Bitfield<30, 0>;  // bits >= log2(max steps*DrawList::kMaxDraws)
    uint64_t fPipelineKey;

    using GeometryUniformField = Bitfield<22, 42>; // bits >= log2(max steps * max draw count)
    using ShadingUniformField  = Bitfield<21, 21>; //  ""
    using TextureBindingsField = Bitfield<21, 0>;  //  ""
    uint64_t fUniformKey;

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
    static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
    static_assert(RenderStepField::kBits      >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(PipelineField::kBits        >=
                          SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(GeometryUniformField::kBits >=
                          SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(ShadingUniformField::kBits  >=
                          SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(TextureBindingsField::kBits >=
                          SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
};

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

// Collects and writes uniform data either to uniform buffers or to shared storage buffers.
class DrawPassUniformWriter {
public:
    DrawPassUniformWriter(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    // Maps a given pipeline index to a batch key, and maps a given uniform data cache index to an
    // SSBO index in the given batch key's batch.
    void trackUniforms(uint32_t pipelineIndex,
                       uintptr_t batchKey,
                       UniformDataCache::Index uniformIndex) {
        if (!uniformIndex.isValid()) {
            return;
        }

        if (fPipelineToBatchKey.find(pipelineIndex) == fPipelineToBatchKey.end()) {
            fPipelineToBatchKey.insert({pipelineIndex, batchKey});
        }
        SkASSERT(fPipelineToBatchKey[pipelineIndex] == batchKey);

        uint32_t uniformIndexInt = uniformIndex.asUInt();
        std::unordered_map<uint32_t, int>& ssboIndices = fBatchedSsboIndices[batchKey];
        if (ssboIndices.find(uniformIndexInt) == ssboIndices.end()) {
            ssboIndices.insert({uniformIndexInt, ssboIndices.size()});
        }
    }

    // Writes all tracked uniform data into buffers, tracking the bindings for the written buffers
    // by their batch key (if using SSBOs) or uniform data cache indices (if not using SSBOs).
    void writeUniforms(DrawBufferManager* bufferMgr, UniformDataCache* uniformDataCache) {
        if (fUseStorageBuffers) {
            std::vector<const SkUniformDataBlock*> udbs;
            for (auto [batchKey, ssboIndices] : fBatchedSsboIndices) {
                SkASSERT(ssboIndices.size());
                size_t udbSize = 0;
                udbs.resize(ssboIndices.size());
                for (auto [uniformDataIndex, ssboIndex] : ssboIndices) {
                    udbs[ssboIndex] =
                            uniformDataCache->lookup(UniformDataCache::Index(uniformDataIndex));
                    if (udbSize == 0) {
                        udbSize = udbs[ssboIndex]->size();
                    }
                    SkASSERT(udbs[ssboIndex]->size() == udbSize);
                }
                auto [writer, bufferInfo] = bufferMgr->getSsboWriter(udbSize * ssboIndices.size());
                for (const SkUniformDataBlock* udb : udbs) {
                    writer.write(udb->data(), udbSize);
                }
                fBufferBindings.insert({batchKey, bufferInfo});
            }

        } else {  // !fUseStorageBuffer
            for (auto [batchKey, ssboIndices] : fBatchedSsboIndices) {
                for (auto [uniformDataIndex, ssboIndex] : ssboIndices) {
                    if (fBufferBindings.find(uniformDataIndex) != fBufferBindings.end()) {
                        continue;
                    }
                    const SkUniformDataBlock* udb =
                            uniformDataCache->lookup(UniformDataCache::Index(uniformDataIndex));
                    SkASSERT(udb->size());
                    auto [writer, bufferInfo] = bufferMgr->getUniformWriter(udb->size());
                    writer.write(udb->data(), udb->size());
                    fBufferBindings.insert({uniformDataIndex, bufferInfo});
                }
            }
        }
    }

    // Updates the current batch key (based on a given pipeline) and uniform data cache index, whose
    // corresponding buffer binding will be bound in the next call to bindBuffer.
    void setCurrentUniforms(uint32_t pipelineIndex, UniformDataCache::Index uniformIndex) {
        bool uniformIndexChanged = uniformIndex.isValid() && uniformIndex != fUniformIndex;
        fUniformIndex = uniformIndex;

        if (fUseStorageBuffers) {
            auto key = fPipelineToBatchKey.find(pipelineIndex);
            bool batchKeyChanged = key != fPipelineToBatchKey.end() && key->second != fBatchKey;
            if (batchKeyChanged) {
                fBatchKey = key->second;
            }
            if (batchKeyChanged || uniformIndexChanged) {
                SkASSERT(fBatchedSsboIndices.find(fBatchKey) != fBatchedSsboIndices.end() &&
                         fBatchedSsboIndices[fBatchKey].find(fUniformIndex.asUInt()) !=
                                 fBatchedSsboIndices[fBatchKey].end());
                fSsboIndex = fBatchedSsboIndices[fBatchKey][fUniformIndex.asUInt()];
            }
            fNeedBufferBinding = batchKeyChanged;

        } else {  // !fUseStorageBuffers
            fNeedBufferBinding = uniformIndexChanged;
        }
    }

    // Binds a new uniform or storage buffer, based on most recently provided batch key and uniform
    // data cache index.
    void bindBuffer(UniformSlot slot, DrawPassCommands::List& commandList) {
        if (fUseStorageBuffers) {
            auto binding = fBufferBindings.find(fBatchKey);
            if (binding != fBufferBindings.end()) {
                commandList.bindUniformBuffer(binding->second, slot);
            }

        } else {  // !fUseStorageBuffers
            auto binding = fBufferBindings.find(fUniformIndex.asUInt());
            if (binding != fBufferBindings.end()) {
                commandList.bindUniformBuffer(binding->second, slot);
            }
        }

        fNeedBufferBinding = false;
    }

    // Returns whether the most recently provided buffer binding key is different than the one
    // before.
    bool needBufferBinding() const { return fNeedBufferBinding; }

    // Returns the current SSBO index, based on the most recently provided batch key and uniform
    // data cache index.
    int ssboIndex() const { return fSsboIndex; }

private:
    // Maps some batch key to a map of UniformDataCache indices -> SSBO indices.
    std::unordered_map<uintptr_t, std::unordered_map<uint32_t, int>> fBatchedSsboIndices;

    // If using storage buffers, maps batch keys to buffer bindings. If not using storage buffers,
    // maps uniform data cache indices to buffer bindings.
    std::unordered_map<uintptr_t, BindBufferInfo> fBufferBindings;

    // Maps pipeline indices to batch keys, which can be shared between pipelines.
    std::unordered_map<uint32_t, uintptr_t> fPipelineToBatchKey;

    const bool fUseStorageBuffers;

    // A key to a batch of draws that should share a storage buffer. This is used to bind a specific
    // storage buffer when bindBuffer is called. From the writer's perspective this is an opaque
    // key, but in practice it represents either a render step or a paint params unique ID,
    // depending on the slot that the buffer binding is applied to (kRenderStep or kPaint).
    uintptr_t fBatchKey = 0;

    UniformDataCache::Index fUniformIndex;
    bool fNeedBufferBinding = false;
    int fSsboIndex = 0;
};

// std::unordered_map implementation for GraphicsPipelineDesc* that de-reference the pointers.
struct Hash {
    size_t operator()(const GraphicsPipelineDesc* desc) const noexcept {
        return GraphicsPipelineDesc::Hash()(*desc);
    }
};

struct Eq {
    bool operator()(const GraphicsPipelineDesc* a,
                    const GraphicsPipelineDesc* b) const noexcept {
        return *a == *b;
    }
};

} // anonymous namespace

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor,
                   int renderStepCount)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor) {
    // TODO: Tune this estimate and the above "itemPerBlock" value for the command buffer sequence
    // After merging, etc. one pipeline per recorded draw+step combo is likely unnecessary.
    fPipelineDescs.reserve(renderStepCount);
    // TODO: Figure out how to tune the number of different sampler objects we may have. In general
    // many draws should be using a similar small set of samplers.
    static constexpr int kReserveSamplerCnt = 8;
    fSamplerDescs.reserve(kReserveSamplerCnt);
}

DrawPass::~DrawPass() = default;

struct SamplerDesc {
    SkSamplingOptions fSamplingOptions;
    SkTileMode fTileModes[2];

    bool isEqual(const SkTextureDataBlock::TextureInfo& info) {
        return fSamplingOptions == info.fSamplingOptions &&
               fTileModes[0] == info.fTileModes[0] &&
               fTileModes[1] == info.fTileModes[1];
    }
};

namespace {

std::pair<int, int> get_unique_texture_sampler_indices(
        std::vector<sk_sp<TextureProxy>>& sampledTextures,
        std::vector<SamplerDesc>& samplerDescs,
        const SkTextureDataBlock::TextureInfo& info) {
    int texIndex = -1;
    for (size_t i = 0; i < sampledTextures.size(); ++i) {
        if (sampledTextures[i].get() == info.fProxy.get()) {
            texIndex = i;
            break;
        }
    }
    if (texIndex == -1) {
        sampledTextures.push_back(info.fProxy);
        texIndex = sampledTextures.size() - 1;
    }

    int samplerIndex = -1;
    for (size_t i = 0; i < samplerDescs.size(); ++i) {
        if (samplerDescs[i].isEqual(info)) {
            samplerIndex = i;
            break;
        }
    }
    if (samplerIndex == -1) {
        samplerDescs.push_back({info.fSamplingOptions,
                                {info.fTileModes[0], info.fTileModes[1]}});
        samplerIndex = samplerDescs.size() - 1;
    }
    SkASSERT(texIndex >=0 && samplerIndex >=0);
    return std::make_pair(texIndex, samplerIndex);
}

} // anonymous namespace

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         std::pair<LoadOp, StoreOp> ops,
                                         std::array<float, 4> clearColor) {
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

    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();

    // We don't expect the uniforms from the renderSteps to reappear multiple times across a
    // recorder's lifetime so we only de-dupe them w/in a given DrawPass.
    UniformDataCache geometryUniformDataCache;
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();
    TextureBindingCache textureBindingIndices;
    // TODO(b/242076321) Use storage buffers for shading uniforms, if supported by GPU.
    DrawPassUniformWriter geometryUniformWriter(/*useStorageBuffers=*/false);
    DrawPassUniformWriter shadingUniformWriter(/*useStorageBuffers=*/false);

    std::unordered_map<const GraphicsPipelineDesc*, uint32_t, Hash, Eq> pipelineDescToIndex;

    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount()); // will not exceed but may use less with occluded draws

    SkShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);
    SkPipelineDataGatherer gatherer(Layout::kMetal);  // TODO: get the layout from the recorder

    int maxTexturesInSingleDraw = 0;

    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        // If we have two different descriptors, such that the uniforms from the PaintParams can be
        // bound independently of those used by the rest of the RenderStep, then we can upload now
        // and remember the location for re-use on any RenderStep that does shading.
        SkUniquePaintParamsID shaderID;
        UniformDataCache::Index shadingUniformIndex;
        TextureDataCache::Index paintTextureDataIndex;
        if (draw.fPaintParams.has_value()) {
            std::tie(shaderID, shadingUniformIndex, paintTextureDataIndex) =
                    ExtractPaintData(recorder, &gatherer, &builder,
                                     draw.fDrawParams.transform().inverse(),
                                     draw.fPaintParams.value());
        } // else depth-only

        for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            UniformDataCache::Index geometryUniformIndex;
            TextureDataCache::Index stepTextureDataIndex;
            if (step->numUniforms() > 0 || step->hasTextures()) {
                std::tie(geometryUniformIndex, stepTextureDataIndex) =
                        ExtractRenderStepData(&geometryUniformDataCache,
                                              textureDataCache,
                                              &gatherer,
                                              step,
                                              draw.fDrawParams);
            }

            SkUniquePaintParamsID stepShaderID;
            UniformDataCache::Index stepShadingUniformIndex;
            TextureBindingCache::Index stepTextureBindingIndex;
            if (performsShading) {
                stepShaderID = shaderID;
                stepShadingUniformIndex = shadingUniformIndex;
                if (paintTextureDataIndex.isValid() || stepTextureDataIndex.isValid()) {
                    stepTextureBindingIndex = textureBindingIndices.insert({paintTextureDataIndex,
                                                                            stepTextureDataIndex});
                    int numTextures = 0;
                    if (paintTextureDataIndex.isValid()) {
                        auto textureDataBlock = textureDataCache->lookup(paintTextureDataIndex);
                        numTextures = textureDataBlock->numTextures();
                    }
                    if (stepTextureDataIndex.isValid()) {
                        auto textureDataBlock = textureDataCache->lookup(stepTextureDataIndex);
                        numTextures += textureDataBlock->numTextures();
                    }
                    maxTexturesInSingleDraw = std::max(maxTexturesInSingleDraw, numTextures);
                }
            } // else depth-only draw or stencil-only step of renderer so no shading is needed

            GraphicsPipelineDesc desc;
            desc.setProgram(step, stepShaderID);
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

            geometryUniformWriter.trackUniforms(
                    pipelineIndex, reinterpret_cast<uintptr_t>(step), geometryUniformIndex);
            shadingUniformWriter.trackUniforms(
                    pipelineIndex, desc.paintParamsID().asUInt(), stepShadingUniformIndex);

            keys.push_back({&draw, stepIndex, pipelineIndex,
                            geometryUniformIndex,
                            stepShadingUniformIndex,
                            stepTextureBindingIndex});
        }

        passBounds.join(draw.fDrawParams.clip().drawBounds());
        drawPass->fDepthStencilFlags |= draw.fRenderer->depthStencilFlags();
        drawPass->fRequiresMSAA |= draw.fRenderer->requiresMSAA();
    }

    geometryUniformWriter.writeUniforms(bufferMgr, &geometryUniformDataCache);
    shadingUniformWriter.writeUniforms(bufferMgr, recorder->priv().uniformDataCache());

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(keys.begin(), keys.end());

    // Used to record vertex/instance data, buffer binds, and draw calls
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);

    // Used to track when a new pipeline or dynamic state needs recording between draw steps.
    // Setting to # render steps ensures the very first time through the loop will bind a pipeline.
    uint32_t lastPipeline = draws->renderStepCount();
    TextureBindingCache::Index lastTextureBindings;
    SkIRect lastScissor = SkIRect::MakeSize(drawPass->fTarget->dimensions());
    // We will reuse these vectors for all the draws as they are just meant for temporary storage
    // as we are creating commands on the fCommandList.
    std::vector<int> textureIndices(maxTexturesInSingleDraw);
    std::vector<int> samplerIndices(maxTexturesInSingleDraw);

    // Set viewport to the entire texture for now (eventually, we may have logically smaller bounds
    // within an approx-sized texture). It is assumed that this also configures the sk_rtAdjust
    // intrinsic for programs (however the backend chooses to do so).
    drawPass->fCommandList.setViewport(SkRect::Make(drawPass->fTarget->dimensions()));

    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = *key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool textureBindingsChange = key.textureBindings().isValid() &&
                                           key.textureBindings() != lastTextureBindings;

        const bool pipelineChange = key.pipeline() != lastPipeline;

        geometryUniformWriter.setCurrentUniforms(key.pipeline(), key.geometryUniforms());
        shadingUniformWriter.setCurrentUniforms(key.pipeline(), key.shadingUniforms());

        const bool stateChange = geometryUniformWriter.needBufferBinding() ||
                                 shadingUniformWriter.needBufferBinding() ||
                                 textureBindingsChange ||
                                 draw.fDrawParams.clip().scissor() != lastScissor;

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
            drawPass->fCommandList.bindGraphicsPipeline(key.pipeline());
            lastPipeline = key.pipeline();
        }

        if (stateChange) {
            if (geometryUniformWriter.needBufferBinding()) {
                geometryUniformWriter.bindBuffer(UniformSlot::kRenderStep, drawPass->fCommandList);
            }
            if (shadingUniformWriter.needBufferBinding()) {
                shadingUniformWriter.bindBuffer(UniformSlot::kPaint, drawPass->fCommandList);
            }
            if (textureBindingsChange) {
                auto textureIndexBlock = textureBindingIndices.lookup(key.textureBindings());

                auto collect_textures = [](TextureDataCache* cache,
                                           TextureDataCache::Index cacheIndex,
                                           DrawPass* drawPass,
                                           int* numTextures,
                                           std::vector<int>* textureIndices,
                                           std::vector<int>* samplerIndices) {
                    if (cacheIndex.isValid()) {
                        auto textureDataBlock = cache->lookup(cacheIndex);
                        for (int i = 0; i < textureDataBlock->numTextures(); ++i) {
                            auto& info = textureDataBlock->texture(i);
                            std::tie((*textureIndices)[i + *numTextures],
                                     (*samplerIndices)[i + *numTextures]) =
                                    get_unique_texture_sampler_indices(drawPass->fSampledTextures,
                                                                       drawPass->fSamplerDescs,
                                                                       info);
                        }
                        *numTextures += textureDataBlock->numTextures();
                    }
                };

                int numTextures = 0;
                collect_textures(textureDataCache, textureIndexBlock->fPaintTextureIndex,
                                 drawPass.get(), &numTextures, &textureIndices, &samplerIndices);
                collect_textures(textureDataCache, textureIndexBlock->fStepTextureIndex,
                                 drawPass.get(), &numTextures, &textureIndices, &samplerIndices);
                SkASSERT(numTextures <= maxTexturesInSingleDraw);
                drawPass->fCommandList.bindTexturesAndSamplers(numTextures,
                                                               textureIndices.data(),
                                                               samplerIndices.data());
                lastTextureBindings = key.textureBindings();
            }
            if (draw.fDrawParams.clip().scissor() != lastScissor) {
                drawPass->fCommandList.setScissor(draw.fDrawParams.clip().scissor());
                lastScissor = draw.fDrawParams.clip().scissor();
            }
        }

        renderStep.writeVertices(&drawWriter, draw.fDrawParams, shadingUniformWriter.ssboIndex());
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    passBounds.roundOut();
    drawPass->fBounds = SkIRect::MakeLTRB((int) passBounds.left(), (int) passBounds.top(),
                                          (int) passBounds.right(), (int) passBounds.bot());
    return drawPass;
}

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                const SkRuntimeEffectDictionary* runtimeDict,
                                const RenderPassDesc& renderPassDesc) {
    fFullPipelines.reserve(fPipelineDescs.count());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs.items()) {
        auto pipeline = resourceProvider->findOrCreateGraphicsPipeline(runtimeDict,
                                                                       pipelineDesc,
                                                                       renderPassDesc);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create GraphicsPipeline for draw in RenderPass. Dropping pass!");
            return false;
        }
        fFullPipelines.push_back(std::move(pipeline));
    }
    // The DrawPass may be long lived on a Recording and we no longer need the GraphicPipelineDescs
    // once we've created pipelines, so we drop the storage for them here.
    fPipelineDescs.reset();

    for (size_t i = 0; i < fSampledTextures.size(); ++i) {
        // TODO: We need to remove this check once we are creating valid SkImages from things like
        // snapshot, save layers, etc. Right now we only support SkImages directly made for graphite
        // and all others have a TextureProxy with an invalid TextureInfo.
        if (!fSampledTextures[i]->textureInfo().isValid()) {
            SKGPU_LOG_W("Failed to validate sampled texture. Will not create renderpass!");
            return false;
        }
        if (!fSampledTextures[i]->instantiate(resourceProvider)) {
            SKGPU_LOG_W("Failed to instantiate sampled texture. Will not create renderpass!");
            return false;
        }
    }
    for (size_t i = 0; i < fSamplerDescs.size(); ++i) {
        sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(
                fSamplerDescs[i].fSamplingOptions,
                fSamplerDescs[i].fTileModes[0],
                fSamplerDescs[i].fTileModes[1]);
        if (!sampler) {
            SKGPU_LOG_W("Failed to create sampler. Will not create renderpass!");
            return false;
        }
        fSamplers.push_back(std::move(sampler));
    }
    // The DrawPass may be long lived on a Recording and we no longer need the SamplerDescs
    // once we've created Samplers, so we drop the storage for them here.
    fSamplerDescs.clear();

    return true;
}

void DrawPass::addResourceRefs(CommandBuffer* commandBuffer) const {
    for (size_t i = 0; i < fFullPipelines.size(); ++i) {
        commandBuffer->trackResource(fFullPipelines[i]);
    }
    for (size_t i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackResource(fSampledTextures[i]->refTexture());
    }
    for (size_t i = 0; i < fSamplers.size(); ++i) {
        commandBuffer->trackResource(fSamplers[i]);
    }
}

const Texture* DrawPass::getTexture(size_t index) const {
    SkASSERT(index < fSampledTextures.size());
    SkASSERT(fSampledTextures[index]);
    SkASSERT(fSampledTextures[index]->texture());
    return fSampledTextures[index]->texture();
}
const Sampler* DrawPass::getSampler(size_t index) const {
    SkASSERT(index < fSamplers.size());
    SkASSERT(fSamplers[index]);
    return fSamplers[index].get();
}

} // namespace skgpu::graphite
