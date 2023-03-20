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
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/BoundsManager.h"

#include "src/base/SkMathPriv.h"
#include "src/base/SkTBlockList.h"

#include <algorithm>
#include <unordered_map>

namespace skgpu::graphite {

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

// This class maps objects to a dense index which can then be used to look them up later
template <typename T, typename V = T, typename C = V>
class DenseBiMap {
public:
    using Index = uint32_t;

    // See note below in GeometryUniformField. This value can be round-tripped within the SortKey
    // packing for all fields but will not be produced when recording actual draw data.
    static constexpr Index kInvalidIndex{1 << SkNextLog2_portable(Renderer::kMaxRenderSteps *
                                                                  DrawList::kMaxDraws)};

    bool empty() const { return fIndexToData.empty(); }
    size_t size() const { return fIndexToData.size(); }

    Index insert(const T& data) {
        Index* index = fDataToIndex.find(data);
        if (!index) {
            SkASSERT(SkToU32(fIndexToData.size()) < kInvalidIndex - 1);
            index = fDataToIndex.set(data, (Index) fIndexToData.size());
            fIndexToData.push_back(C{data});
        }
        return *index;
    }

    const V& lookup(Index index) {
        SkASSERT(index < kInvalidIndex);
        return fIndexToData[index];
    }

    SkSpan<V> data() { return {fIndexToData.data(), fIndexToData.size()}; }

    SkTArray<V>&& detach() { return std::move(fIndexToData); }

private:
    SkTHashMap<T, Index> fDataToIndex;
    SkTArray<V> fIndexToData;
};

// Tracks uniform data on the CPU and then its transition to storage in a GPU buffer (ubo or ssbo).
struct CpuOrGpuData {
    union {
        const UniformDataBlock* fCpuData;
        BindBufferInfo fGpuData;
    };

    // Can only start from CPU data
    CpuOrGpuData(const UniformDataBlock* cpuData) : fCpuData(cpuData) {}
};

// Tracks the combination of textures from the paint and from the RenderStep to describe the full
// binding that needs to be in the command list.
struct TextureBinding {
    const TextureDataBlock* fPaintTextures;
    const TextureDataBlock* fStepTextures;

    bool operator==(const TextureBinding& other) const {
        return fPaintTextures == other.fPaintTextures &&
               fStepTextures == other.fStepTextures;
    }
    bool operator!=(const TextureBinding& other) const { return !(*this == other); }

    int numTextures() const {
        return (fPaintTextures ? fPaintTextures->numTextures() : 0) +
               (fStepTextures ? fStepTextures->numTextures() : 0);
    }
};

using UniformSsboCache = DenseBiMap<const UniformDataBlock*, CpuOrGpuData>;
using TextureBindingCache = DenseBiMap<TextureBinding>;
using GraphicsPipelineCache = DenseBiMap<GraphicsPipelineDesc>;

// Automatically merges and manages texture bindings and uniform bindings sourced from either the
// paint or the RenderStep. Tracks the bound state based on last-provided unique index to write
// Bind commands to a CommandList when necessary.
class TextureBindingTracker {
public:
    TextureBindingCache::Index trackTextures(const TextureDataBlock* paintTextures,
                                             const TextureDataBlock* stepTextures) {
        if (!paintTextures && !stepTextures) {
            return TextureBindingCache::kInvalidIndex;
        }
        return fBindingCache.insert({paintTextures, stepTextures});
    }

    bool setCurrentTextureBindings(TextureBindingCache::Index bindingIndex) {
        if (bindingIndex < TextureBindingCache::kInvalidIndex && fLastIndex != bindingIndex) {
            fLastIndex = bindingIndex;
            return true;
        }
        // No binding change
        return false;
    }

    void bindTextures(DrawPassCommands::List* commandList) {
        SkASSERT(fLastIndex < TextureBindingCache::kInvalidIndex);
        const TextureBinding& binding = fBindingCache.lookup(fLastIndex);

        auto [texIndices, samplerIndices] =
                commandList->bindDeferredTexturesAndSamplers(binding.numTextures());

        if (binding.fPaintTextures) {
            for (int i = 0; i < binding.fPaintTextures->numTextures(); ++i) {
                auto [tex, sampler] = binding.fPaintTextures->texture(i);
                *texIndices++     = fProxyCache.insert(tex.get());
                *samplerIndices++ = fSamplerCache.insert(sampler);
            }
        }
        if (binding.fStepTextures) {
            for (int i = 0; i < binding.fStepTextures->numTextures(); ++i) {
                auto [tex, sampler] = binding.fStepTextures->texture(i);
                *texIndices++     = fProxyCache.insert(tex.get());
                *samplerIndices++ = fSamplerCache.insert(sampler);
            }
        }
    }

    SkTArray<sk_sp<TextureProxy>>&& detachTextures() { return fProxyCache.detach(); }
    SkTArray<SamplerDesc>&& detachSamplers() { return fSamplerCache.detach(); }

private:
    struct ProxyRef {
        const TextureProxy* fProxy;
        operator sk_sp<TextureProxy>() const { return sk_ref_sp(fProxy); }
    };
    using TextureProxyCache = DenseBiMap<const TextureProxy*, sk_sp<TextureProxy>, ProxyRef>;
    using SamplerDescCache = DenseBiMap<SamplerDesc>;

    TextureBindingCache fBindingCache;

    TextureProxyCache fProxyCache;
    SamplerDescCache fSamplerCache;

    TextureBindingCache::Index fLastIndex = TextureBindingCache::kInvalidIndex;
};

// Collects and writes uniform data either to uniform buffers or to shared storage buffers, and
// tracks when bindings need to change between draws.
class UniformSsboTracker {
public:
    UniformSsboTracker(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    // Maps a given {pipeline index, uniform data cache index} pair to an SSBO index within the
    // pipeline's accumulated array of uniforms.
    UniformSsboCache::Index trackUniforms(GraphicsPipelineCache::Index pipelineIndex,
                                          const UniformDataBlock* cpuData) {
        if (!cpuData) {
            return UniformSsboCache::kInvalidIndex;
        }

        if (pipelineIndex >= SkToU32(fPerPipelineCaches.size())) {
            fPerPipelineCaches.resize(pipelineIndex + 1);
        }

        return fPerPipelineCaches[pipelineIndex].insert(cpuData);
    }

    // Writes all tracked uniform data into buffers, tracking the bindings for the written buffers
    // by GraphicsPipelineCache::Index and possibly the UniformSsboCache::Index (when not using
    // SSBOs). When using SSBos, the buffer is the same for all UniformSsboCache::Indices that share
    // the same pipeline (and is stored in index 0).
    void writeUniforms(DrawBufferManager* bufferMgr) {
        for (UniformSsboCache& cache : fPerPipelineCaches) {
            if (cache.empty()) {
                continue;
            }
            // All data blocks for the same pipeline have the same size, so peek the first
            // to determine the total buffer size
            size_t udbSize = cache.lookup(0).fCpuData->size();
            size_t udbDataSize = udbSize;
            if (!fUseStorageBuffers) {
                udbSize = bufferMgr->alignUniformBlockSize(udbSize);
            }
            auto [writer, bufferInfo] =
                    fUseStorageBuffers ? bufferMgr->getSsboWriter(udbSize * cache.size())
                                       : bufferMgr->getUniformWriter(udbSize * cache.size());

            for (CpuOrGpuData& dataBlock : cache.data()) {
                SkASSERT(dataBlock.fCpuData->size() == udbDataSize);
                writer.write(dataBlock.fCpuData->data(), udbDataSize);
                // Swap from tracking the CPU data to the location of the GPU data
                dataBlock.fGpuData = bufferInfo;
                if (!fUseStorageBuffers) {
                    bufferInfo.fOffset += udbSize;
                    writer.skipBytes(udbSize - udbDataSize);
                } // else keep bufferInfo pointing to the start of the array
            }
        }
    }

    // Updates the current tracked pipeline and ssbo index and returns whether or not bindBuffers()
    // needs to be called, depending on if 'fUseStorageBuffers' is true or not.
    bool setCurrentUniforms(GraphicsPipelineCache::Index pipelineIndex,
                            UniformSsboCache::Index ssboIndex) {
        if (ssboIndex >= UniformSsboCache::kInvalidIndex) {
            return false;
        }
        SkASSERT(pipelineIndex < SkToU32(fPerPipelineCaches.size()) &&
                 ssboIndex < fPerPipelineCaches[pipelineIndex].size());

        if (fUseStorageBuffers) {
            ssboIndex = 0; // The specific index has no effect on binding
        }
        if (fLastPipeline != pipelineIndex || fLastIndex != ssboIndex) {
            fLastPipeline = pipelineIndex;
            fLastIndex = ssboIndex;
            return true;
        } else {
            return false;
        }
    }

    // Binds a new uniform or storage buffer, based on most recently provided batch key and uniform
    // data cache index.
    void bindUniforms(UniformSlot slot, DrawPassCommands::List* commandList) {
        SkASSERT(fLastPipeline < GraphicsPipelineCache::kInvalidIndex &&
                 fLastIndex < UniformSsboCache::kInvalidIndex);
        SkASSERT(!fUseStorageBuffers || fLastIndex == 0);
        const BindBufferInfo& binding =
                fPerPipelineCaches[fLastPipeline].lookup(fLastIndex).fGpuData;
        commandList->bindUniformBuffer(binding, slot);
    }

private:
    // Access first by pipeline index. The final UniformSsboCache::Index is either used to select
    // the BindBufferInfo for a draw using UBOs, or it's the real index into a packed array of
    // uniforms in a storage buffer object (whose binding is stored in index 0).
    SkTArray<UniformSsboCache> fPerPipelineCaches;

    const bool fUseStorageBuffers;

    GraphicsPipelineCache::Index fLastPipeline = GraphicsPipelineCache::kInvalidIndex;
    UniformSsboCache::Index fLastIndex = UniformSsboCache::kInvalidIndex;
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
            GraphicsPipelineCache::Index pipelineIndex,
            UniformSsboCache::Index geomSsboIndex,
            UniformSsboCache::Index shadingSsboIndex,
            TextureBindingCache::Index textureBindingIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->fDrawParams.order().paintOrder().bits()) |
                       StencilIndexField::set(draw->fDrawParams.order().stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))                  |
                       PipelineField::set(pipelineIndex))
        , fUniformKey(GeometryUniformField::set(geomSsboIndex)   |
                      ShadingUniformField::set(shadingSsboIndex) |
                      TextureBindingsField::set(textureBindingIndex))
        , fDraw(draw) {
        SkASSERT(pipelineIndex < GraphicsPipelineCache::kInvalidIndex);
        SkASSERT(renderStep <= draw->fRenderer->numRenderSteps());
    }

    bool operator<(const SortKey& k) const {
        return fPipelineKey < k.fPipelineKey ||
               (fPipelineKey == k.fPipelineKey && fUniformKey < k.fUniformKey);
    }

    const RenderStep& renderStep() const {
        return fDraw->fRenderer->step(RenderStepField::get(fPipelineKey));
    }

    const DrawList::Draw& draw() const { return *fDraw; }

    GraphicsPipelineCache::Index pipelineIndex() const {
        return PipelineField::get(fPipelineKey);
    }
    UniformSsboCache::Index geometrySsboIndex() const {
        return GeometryUniformField::get(fUniformKey);
    }
    UniformSsboCache::Index shadingSsboIndex() const {
        return ShadingUniformField::get(fUniformKey);
    }
    TextureBindingCache::Index textureBindingIndex() const {
        return TextureBindingsField::get(fUniformKey);
    }

private:
    // Fields are ordered from most-significant to least when sorting by 128-bit value.
    // NOTE: We don't use C++ bit fields because field ordering is implementation defined and we
    // need to sort consistently.
    using ColorDepthOrderField = Bitfield<16, 48>; // sizeof(CompressedPaintersOrder)
    using StencilIndexField    = Bitfield<16, 32>; // sizeof(DisjointStencilIndex)
    using RenderStepField      = Bitfield<2,  30>; // bits >= log2(Renderer::kMaxRenderSteps)
    using PipelineField        = Bitfield<30, 0>;  // bits >= log2(max steps*DrawList::kMaxDraws)
    uint64_t fPipelineKey;

    // The uniform/texture index fields need 1 extra bit to encode "no-data". Values that are
    // greater than or equal to 2^(bits-1) represent "no-data", while values between
    // [0, 2^(bits-1)-1] can access data arrays without extra logic.
    using GeometryUniformField = Bitfield<22, 42>; // bits >= 1+log2(max steps * max draw count)
    using ShadingUniformField  = Bitfield<21, 21>; // bits >= 1+log2(max steps * max draw count)
    using TextureBindingsField = Bitfield<21, 0>;  // bits >= 1+log2(max steps * max draw count)
    uint64_t fUniformKey;

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
    static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
    static_assert(RenderStepField::kBits      >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(PipelineField::kBits        >=
                          SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(GeometryUniformField::kBits >=
                          1 + SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(ShadingUniformField::kBits  >=
                          1 + SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
    static_assert(TextureBindingsField::kBits >=
                          1 + SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));
};

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor) {}

DrawPass::~DrawPass() = default;

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         const SkImageInfo& targetInfo,
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
    std::unique_ptr<DrawPass> drawPass(new DrawPass(std::move(target), ops, clearColor));

    Rect passBounds = Rect::InfiniteInverted();

    // We don't expect the uniforms from the renderSteps to reappear multiple times across a
    // recorder's lifetime so we only de-dupe them w/in a given DrawPass.
    UniformDataCache geometryUniformDataCache;
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();

    GraphicsPipelineCache pipelineCache;

    // Geometry uniforms are currently always UBO-backed.
    const ResourceBindingRequirements& bindingReqs =
            recorder->priv().caps()->resourceBindingRequirements();
    Layout geometryUniformLayout = bindingReqs.fUniformBufferLayout;
    UniformSsboTracker geometrySsboTracker(/*useStorageBuffers=*/false);

    bool useStorageBuffers = recorder->priv().caps()->storageBufferPreferred();
    Layout shadingUniformLayout =
            useStorageBuffers ? bindingReqs.fStorageBufferLayout : bindingReqs.fUniformBufferLayout;
    UniformSsboTracker shadingSsboTracker(useStorageBuffers);
    TextureBindingTracker textureBindingTracker;

    ShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);

    // The initial layout we pass here is not important as it will be re-assigned when writing
    // shading and geometry uniforms below.
    PipelineDataGatherer gatherer(shadingUniformLayout);

    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount());
    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        // If we have two different descriptors, such that the uniforms from the PaintParams can be
        // bound independently of those used by the rest of the RenderStep, then we can upload now
        // and remember the location for re-use on any RenderStep that does shading.
        UniquePaintParamsID shaderID;
        const UniformDataBlock* shadingUniforms = nullptr;
        const TextureDataBlock* paintTextures = nullptr;
        if (draw.fPaintParams.has_value()) {
            std::tie(shaderID, shadingUniforms, paintTextures) =
                    ExtractPaintData(recorder,
                                     &gatherer,
                                     &builder,
                                     shadingUniformLayout,
                                     draw.fDrawParams.transform(),
                                     draw.fPaintParams.value(),
                                     targetInfo.colorInfo());
        } // else depth-only

        for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            GraphicsPipelineCache::Index pipelineIndex = pipelineCache.insert(
                    {step, performsShading ? shaderID : UniquePaintParamsID::InvalidID()});
            auto [geometryUniforms, stepTextures] = ExtractRenderStepData(&geometryUniformDataCache,
                                                                          textureDataCache,
                                                                          &gatherer,
                                                                          geometryUniformLayout,
                                                                          step,
                                                                          draw.fDrawParams);

            UniformSsboCache::Index geomSsboIndex = geometrySsboTracker.trackUniforms(
                    pipelineIndex, geometryUniforms);
            UniformSsboCache::Index shadingSsboIndex = shadingSsboTracker.trackUniforms(
                    pipelineIndex, performsShading ? shadingUniforms : nullptr);
            TextureBindingCache::Index textureIndex = textureBindingTracker.trackTextures(
                    performsShading ? paintTextures : nullptr, stepTextures);

            keys.push_back({&draw, stepIndex, pipelineIndex,
                            geomSsboIndex, shadingSsboIndex, textureIndex});
        }

        passBounds.join(draw.fDrawParams.clip().drawBounds());
        drawPass->fDepthStencilFlags |= draw.fRenderer->depthStencilFlags();
        drawPass->fRequiresMSAA |= draw.fRenderer->requiresMSAA();
    }

    geometrySsboTracker.writeUniforms(bufferMgr);
    shadingSsboTracker.writeUniforms(bufferMgr);

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
    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    SkIRect lastScissor = SkIRect::MakeSize(targetInfo.dimensions());

    SkASSERT(!drawPass->fTarget->isInstantiated() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;

        const bool geomBindingChange    = geometrySsboTracker.setCurrentUniforms(
                key.pipelineIndex(), key.geometrySsboIndex());
        const bool shadingBindingChange  = shadingSsboTracker.setCurrentUniforms(
                key.pipelineIndex(), key.shadingSsboIndex());
        const bool textureBindingsChange = textureBindingTracker.setCurrentTextureBindings(
                key.textureBindingIndex());
        const SkIRect* newScissor        = draw.fDrawParams.clip().scissor() != lastScissor ?
                &draw.fDrawParams.clip().scissor() : nullptr;

        const bool stateChange = geomBindingChange ||
                                 shadingBindingChange ||
                                 textureBindingsChange ||
                                 SkToBool(newScissor);

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
            drawPass->fCommandList.bindGraphicsPipeline(key.pipelineIndex());
            lastPipeline = key.pipelineIndex();
        }
        if (stateChange) {
            if (geomBindingChange) {
                geometrySsboTracker.bindUniforms(UniformSlot::kRenderStep, &drawPass->fCommandList);
            }
            if (shadingBindingChange) {
                shadingSsboTracker.bindUniforms(UniformSlot::kPaint, &drawPass->fCommandList);
            }
            if (textureBindingsChange) {
                textureBindingTracker.bindTextures(&drawPass->fCommandList);
            }
            if (newScissor) {
                drawPass->fCommandList.setScissor(*newScissor);
                lastScissor = *newScissor;
            }
        }

        renderStep.writeVertices(&drawWriter, draw.fDrawParams, key.shadingSsboIndex());
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    drawPass->fBounds = passBounds.roundOut().asSkIRect();

    drawPass->fPipelineDescs   = pipelineCache.detach();
    drawPass->fSamplerDescs    = textureBindingTracker.detachSamplers();
    drawPass->fSampledTextures = textureBindingTracker.detachTextures();

    return drawPass;
}

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* runtimeDict,
                                const RenderPassDesc& renderPassDesc) {
    fFullPipelines.reserve_back(fPipelineDescs.size());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs) {
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
    fPipelineDescs.clear();

    for (int i = 0; i < fSampledTextures.size(); ++i) {
        // TODO: We need to remove this check once we are creating valid SkImages from things like
        // snapshot, save layers, etc. Right now we only support SkImages directly made for graphite
        // and all others have a TextureProxy with an invalid TextureInfo.
        if (!fSampledTextures[i]->textureInfo().isValid()) {
            SKGPU_LOG_W("Failed to validate sampled texture. Will not create renderpass!");
            return false;
        }
        if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fSampledTextures[i].get())) {
            SKGPU_LOG_W("Failed to instantiate sampled texture. Will not create renderpass!");
            return false;
        }
    }

    fSamplers.reserve_back(fSamplerDescs.size());
    for (int i = 0; i < fSamplerDescs.size(); ++i) {
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
    for (int i = 0; i < fFullPipelines.size(); ++i) {
        commandBuffer->trackResource(fFullPipelines[i]);
    }
    for (int i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackResource(fSampledTextures[i]->refTexture());
    }
    for (int i = 0; i < fSamplers.size(); ++i) {
        commandBuffer->trackResource(fSamplers[i]);
    }
}

const Texture* DrawPass::getTexture(size_t index) const {
    SkASSERT(index < SkToSizeT(fSampledTextures.size()));
    SkASSERT(fSampledTextures[index]);
    SkASSERT(fSampledTextures[index]->texture());
    return fSampledTextures[index]->texture();
}
const Sampler* DrawPass::getSampler(size_t index) const {
    SkASSERT(index < SkToSizeT(fSamplers.size()));
    SkASSERT(fSamplers[index]);
    return fSamplers[index].get();
}

} // namespace skgpu::graphite
