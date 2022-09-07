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
#include "src/gpu/graphite/Caps.h"
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

namespace {

// See note below in GeometryUniformField. This value can be round-tripped within the SortKey
// packing but will not be produced when recording actual draw data.
static constexpr uint32_t kInvalidIndex =
        1 << (SkNextLog2_portable(Renderer::kMaxRenderSteps * DrawList::kMaxDraws));

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
    const SkTextureDataBlock* fPaintTextures;
    const SkTextureDataBlock* fStepTextures;

    bool operator==(const TextureBindingBlock& other) const {
        return fPaintTextures == other.fPaintTextures &&
               fStepTextures == other.fStepTextures;
    }
    bool operator!=(const TextureBindingBlock& other) const { return !(*this == other);  }

    int numTextures() const {
        return (fPaintTextures ? fPaintTextures->numTextures() : 0) +
               (fStepTextures ? fStepTextures->numTextures() : 0);
    }
    // TODO: Switch to reusable dense T->index map for tracking proxies and samplers
    void writeIndices(int* textureIndices,
                      int* samplerIndices,
                      std::vector<sk_sp<TextureProxy>>& proxies,
                      std::vector<SamplerDesc>& samplers) const {
        // TODO: These helpers will go away
        auto getTextureIndex = [&](sk_sp<TextureProxy> texture) {
            for (size_t i = 0; i < proxies.size(); ++i) {
                if (proxies[i].get() == texture.get()) {
                    return (int) i;
                }
            }
            proxies.push_back(std::move(texture));
            return (int) proxies.size() - 1;
        };
        auto getSamplerIndex = [&](const SamplerDesc& desc) {
            for (size_t i = 0; i < samplers.size(); ++i) {
                if (samplers[i] == desc) {
                    return (int) i;
                }
            }
            samplers.push_back(desc);
            return (int) samplers.size() - 1;
        };
        if (fPaintTextures) {
            for (int i = 0; i < fPaintTextures->numTextures(); ++i) {
                *textureIndices++ = getTextureIndex(std::get<0>(fPaintTextures->texture(i)));
                *samplerIndices++ = getSamplerIndex(std::get<1>(fPaintTextures->texture(i)));
            }
        }
        if (fStepTextures) {
            for (int i = 0; i < fStepTextures->numTextures(); ++i) {
                *textureIndices++ = getTextureIndex(std::get<0>(fStepTextures->texture(i)));
                *samplerIndices++ = getSamplerIndex(std::get<1>(fStepTextures->texture(i)));
            }
        }
    }
};

class TextureBindingCache {
public:
    uint32_t trackTextures(const SkTextureDataBlock* paintTextures,
                           const SkTextureDataBlock* stepTextures) {
        if (!paintTextures && !stepTextures) {
            return kInvalidIndex;
        }
        TextureBindingBlock block{paintTextures, stepTextures};
        uint32_t* existing = fBindingToIndex.find(block);
        if (!existing) {
            SkASSERT(fIndexToBinding.size() < kInvalidIndex - 1);
            existing = fBindingToIndex.set(block, fIndexToBinding.count());
            fIndexToBinding.push_back(block);
        }
        return *existing;
    }

    const TextureBindingBlock& lookup(uint32_t index) const {
        SkASSERT(index < kInvalidIndex);
        return fIndexToBinding[index];
    }

private:
    // TODO(michaelludwig): This pattern can be extracted and re-used for uniform writing,
    // pipeline index tracking, sampler tracking, texture tracking, and texture binding tracking.
    SkTHashMap<TextureBindingBlock, uint32_t> fBindingToIndex;
    SkTArray<TextureBindingBlock> fIndexToBinding;
};

} // namespace

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
            uint32_t geomSsboIndex,
            uint32_t shadingSsboIndex,
            uint32_t textureBindingIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->fDrawParams.order().paintOrder().bits()) |
                       StencilIndexField::set(draw->fDrawParams.order().stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))                  |
                       PipelineField::set(pipelineIndex))
        , fUniformKey(GeometryUniformField::set(geomSsboIndex)   |
                      ShadingUniformField::set(shadingSsboIndex) |
                      TextureBindingsField::set(textureBindingIndex))
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

    uint32_t pipelineIndex()       const { return PipelineField::get(fPipelineKey);       }
    uint32_t geometrySsboIndex()   const { return GeometryUniformField::get(fUniformKey); }
    uint32_t shadingSsboIndex()    const { return ShadingUniformField::get(fUniformKey);  }
    uint32_t textureBindingIndex() const { return TextureBindingsField::get(fUniformKey); }

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

namespace {

// Collects and writes uniform data either to uniform buffers or to shared storage buffers.
class DrawPassUniformWriter {
public:
    DrawPassUniformWriter(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    // Maps a given {pipeline index, uniform data cache index} pair to an SSBO index within the
    // pipeline's accumulated array of uniforms.
    uint32_t trackUniforms(uint32_t pipelineIndex, const SkUniformDataBlock* cpuData) {
        if (!cpuData) {
            return kInvalidIndex;
        }

        SkASSERT(fDataToSsboIndex.size() == fSsboIndexToData.size());
        if (pipelineIndex >= fDataToSsboIndex.size()) {
            fDataToSsboIndex.resize(pipelineIndex + 1);
            fSsboIndexToData.resize(pipelineIndex + 1);
        }

        uint32_t* ssboIndex = fDataToSsboIndex[pipelineIndex].find(cpuData);
        if (ssboIndex) {
            // Validate and return duplicate data reference
            SkASSERT(fSsboIndexToData[pipelineIndex][*ssboIndex].fCpuData == cpuData);
            return *ssboIndex;
        } else {
            // Record new data with an incremented index.
            uint32_t newSsboIndex = fSsboIndexToData[pipelineIndex].size();
            fDataToSsboIndex[pipelineIndex].set(cpuData, newSsboIndex);
            fSsboIndexToData[pipelineIndex].emplace_back(cpuData);
            return newSsboIndex;
        }
    }

    // Writes all tracked uniform data into buffers, tracking the bindings for the written buffers
    // by their batch key (if using SSBOs) or uniform data cache indices (if not using SSBOs).
    void writeUniforms(DrawBufferManager* bufferMgr) {
        for (SkTArray<CpuOrGpuData>& ssboData : fSsboIndexToData) {
            if (ssboData.empty()) {
                continue;
            }
            // All data blocks for the same batch key have the same size, so peek the first
            // to determine the total buffer size
            size_t udbSize = ssboData[0].fCpuData->size();
            auto [writer, bufferInfo] =
                    fUseStorageBuffers ? bufferMgr->getSsboWriter(udbSize * ssboData.size())
                                       : bufferMgr->getUniformWriter(udbSize * ssboData.size());

            for (auto&& dataBlock : ssboData) {
                SkASSERT(dataBlock.fCpuData->size() == udbSize);
                writer.write(dataBlock.fCpuData->data(), udbSize);
                // Swap from tracking the CPU data to the location of the GPU data
                dataBlock.fGpuData = bufferInfo;
                if (!fUseStorageBuffers) {
                    bufferInfo.fOffset += udbSize;
                } // else keep bufferInfo pointing to the start of the array
            }
        }
    }

    // Updates the current tracked pipeline and ssbo index and returns whether or not bindBuffers()
    // needs to be called, depending on if 'fUseStorageBuffers' is true or not.
    bool setCurrentUniforms(uint32_t pipelineIndex, uint32_t ssboIndex) {
        if (ssboIndex >= kInvalidIndex) {
            return false;
        }
        SkASSERT(pipelineIndex <= fSsboIndexToData.size() &&
                 (size_t) ssboIndex <= fSsboIndexToData[pipelineIndex].size());

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
    void bindBuffer(UniformSlot slot, DrawPassCommands::List& commandList) {
        SkASSERT(fLastPipeline <= DrawList::kMaxDraws && fLastIndex < kInvalidIndex);
        SkASSERT(fLastPipeline <= fSsboIndexToData.size() &&
                 ((fUseStorageBuffers && fLastIndex == 0) ||
                  (!fUseStorageBuffers && fLastIndex <= fSsboIndexToData[fLastPipeline].size())));
        commandList.bindUniformBuffer(fSsboIndexToData[fLastPipeline][fLastIndex].fGpuData, slot);
    }

private:
    struct CpuOrGpuData {
        union {
            const SkUniformDataBlock* fCpuData;
            BindBufferInfo fGpuData;
        };

        // Can only start from CPU data
        CpuOrGpuData(const SkUniformDataBlock* cpuData) : fCpuData(cpuData) {}
    };

    // Access first by pipeline index, then UniformDataCache::Index. The returned int is the index
    // into the pipeline's corresponding array of CpuOrGpuData. This int is either used to lookup
    // the specific BindBufferInfo for a draw that accesses UBOs, or it's the real ssbo index that
    // the draw must upload to indirectly access the GPU data in the shader.
    SkTArray<SkTHashMap<const SkUniformDataBlock*, uint32_t>> fDataToSsboIndex;

    // Access first by pipeline index, then by the ssbo index (returned by trackUniforms() and
    // stored in SortKey). Initially the data is on the CPU and 'fCpuData' is active. After
    // writeUniforms() all CPU data will have been written to a GPU buffer and 'fGpuData' is active.
    // When using real SSBOs, only the first element will have a valid BindBufferInfo, which points
    // to the start of the dense GPU array that can be accessed using the ssbo index in a shader.
    SkTArray<SkTArray<CpuOrGpuData>> fSsboIndexToData;

    const bool fUseStorageBuffers;

    uint32_t fLastPipeline = std::numeric_limits<uint32_t>::max();
    uint32_t fLastIndex = kInvalidIndex;
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

    // TODO(b/242076321) Use storage buffers for shading uniforms, if supported by GPU.
    DrawPassUniformWriter geometryUniformWriter(/*useStorageBuffers=*/false);
    DrawPassUniformWriter shadingUniformWriter(
            /*useStorageBuffers=*/recorder->priv().caps()->storageBufferPreferred());
    TextureBindingCache textureBindingIndices;

    SkTHashMap<GraphicsPipelineDesc, uint32_t> pipelineDescToIndex;

    SkShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    SkPaintParamsKeyBuilder builder(dict);
    SkPipelineDataGatherer gatherer(Layout::kMetal);  // TODO: get the layout from the recorder

    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount());
    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        // If we have two different descriptors, such that the uniforms from the PaintParams can be
        // bound independently of those used by the rest of the RenderStep, then we can upload now
        // and remember the location for re-use on any RenderStep that does shading.
        SkUniquePaintParamsID shaderID;
        const SkUniformDataBlock* shadingUniforms = nullptr;
        const SkTextureDataBlock* paintTextures = nullptr;
        if (draw.fPaintParams.has_value()) {
            std::tie(shaderID, shadingUniforms, paintTextures) =
                    ExtractPaintData(recorder, &gatherer, &builder,
                                     draw.fDrawParams.transform().inverse(),
                                     draw.fPaintParams.value());
        } // else depth-only

        for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            GraphicsPipelineDesc desc{step, performsShading ? shaderID
                                                            : SkUniquePaintParamsID::InvalidID()};
            uint32_t* pipelineIndex = pipelineDescToIndex.find(desc);
            if (!pipelineIndex) {
                pipelineIndex = pipelineDescToIndex.set(desc, pipelineDescToIndex.count());
                drawPass->fPipelineDescs.push_back(desc);
            }

            auto [geometryUniforms, stepTextures] = ExtractRenderStepData(&geometryUniformDataCache,
                                                                          textureDataCache,
                                                                          &gatherer,
                                                                          step,
                                                                          draw.fDrawParams);

            uint32_t geomSsboIndex = geometryUniformWriter.trackUniforms(
                    *pipelineIndex, geometryUniforms);
            uint32_t shadingSsboIndex = shadingUniformWriter.trackUniforms(
                    *pipelineIndex, performsShading ? shadingUniforms : nullptr);
            uint32_t textureIndex = textureBindingIndices.trackTextures(
                    performsShading ? paintTextures : nullptr, stepTextures);

            keys.push_back({&draw, stepIndex, *pipelineIndex,
                            geomSsboIndex, shadingSsboIndex, textureIndex});
        }

        passBounds.join(draw.fDrawParams.clip().drawBounds());
        drawPass->fDepthStencilFlags |= draw.fRenderer->depthStencilFlags();
        drawPass->fRequiresMSAA |= draw.fRenderer->requiresMSAA();
    }

    geometryUniformWriter.writeUniforms(bufferMgr);
    shadingUniformWriter.writeUniforms(bufferMgr);

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
    uint32_t lastTextureBindings = kInvalidIndex;
    SkIRect lastScissor = SkIRect::MakeSize(drawPass->fTarget->dimensions());

    // Set viewport to the entire texture for now (eventually, we may have logically smaller bounds
    // within an approx-sized texture). It is assumed that this also configures the sk_rtAdjust
    // intrinsic for programs (however the backend chooses to do so).
    drawPass->fCommandList.setViewport(SkRect::Make(drawPass->fTarget->dimensions()));

    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = *key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;

        const bool textureBindingsChange = key.textureBindingIndex() < kInvalidIndex &&
                                           key.textureBindingIndex() != lastTextureBindings;

        const bool geomBindingChange = geometryUniformWriter.setCurrentUniforms(
                key.pipelineIndex(), key.geometrySsboIndex());
        const bool shadingBindingChange = shadingUniformWriter.setCurrentUniforms(
                key.pipelineIndex(), key.shadingSsboIndex());

        const bool stateChange = geomBindingChange ||
                                 shadingBindingChange ||
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
            drawPass->fCommandList.bindGraphicsPipeline(key.pipelineIndex());
            lastPipeline = key.pipelineIndex();
        }

        if (stateChange) {
            if (geomBindingChange) {
                geometryUniformWriter.bindBuffer(UniformSlot::kRenderStep, drawPass->fCommandList);
            }
            if (shadingBindingChange) {
                shadingUniformWriter.bindBuffer(UniformSlot::kPaint, drawPass->fCommandList);
            }
            if (textureBindingsChange) {
                auto binding = textureBindingIndices.lookup(key.textureBindingIndex());

                auto [texIndices, samplerIndices] =
                    drawPass->fCommandList.bindDeferredTexturesAndSamplers(binding.numTextures());
                binding.writeIndices(texIndices, samplerIndices,
                                     drawPass->fSampledTextures, drawPass->fSamplerDescs);

                lastTextureBindings = key.textureBindingIndex();
            }
            if (draw.fDrawParams.clip().scissor() != lastScissor) {
                drawPass->fCommandList.setScissor(draw.fDrawParams.clip().scissor());
                lastScissor = draw.fDrawParams.clip().scissor();
            }
        }

        renderStep.writeVertices(&drawWriter, draw.fDrawParams, key.shadingSsboIndex());
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
