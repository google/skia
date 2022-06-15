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
            TextureDataCache::Index textureDataIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->fDrawParams.order().paintOrder().bits()) |
                       StencilIndexField::set(draw->fDrawParams.order().stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))                  |
                       PipelineField::set(pipelineIndex))
        , fUniformKey(GeometryUniformField::set(geomUniformIndex.asUInt())   |
                      ShadingUniformField::set(shadingUniformIndex.asUInt()) |
                      TextureBindingsField::set(textureDataIndex.asUInt()))
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

    uint32_t pipeline() const { return PipelineField::get(fPipelineKey);       }
    UniformDataCache::Index geometryUniforms() const {
        return UniformDataCache::Index(GeometryUniformField::get(fUniformKey));
    }
    UniformDataCache::Index shadingUniforms() const {
        return UniformDataCache::Index(ShadingUniformField::get(fUniformKey));
    }
    TextureDataCache::Index textureBindings() const {
        return TextureDataCache::Index(TextureBindingsField::get(fUniformKey));
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
    UniformBindingCache(DrawBufferManager* bufferMgr, UniformDataCache* uniformDataCache)
            : fBufferMgr(bufferMgr)
            , fUniformDataCache(uniformDataCache) {
    }

    UniformDataCache::Index addUniforms(UniformDataCache::Index uIndex) {
        if (!uIndex.isValid()) {
            return {};
        }

        const SkUniformDataBlock* udb = fUniformDataCache->lookup(uIndex);
        SkASSERT(udb);

        if (fBindings.find(uIndex.asUInt()) == fBindings.end()) {
            // First time encountering this data, so upload to the GPU
            SkASSERT(udb->size());
            auto[writer, bufferInfo] = fBufferMgr->getUniformWriter(udb->size());
            writer.write(udb->data(), udb->size());

            fBindings.insert({uIndex.asUInt(), bufferInfo});
        }

        return uIndex;
    }

    BindBufferInfo getBinding(UniformDataCache::Index uniformDataIndex) {
        auto lookup = fBindings.find(uniformDataIndex.asUInt());
        SkASSERT(lookup != fBindings.end());
        return lookup->second;
    }

private:
    DrawBufferManager* fBufferMgr;
    UniformDataCache* fUniformDataCache;

    std::unordered_map<uint32_t, BindBufferInfo> fBindings;
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
        : fCommands(std::max(1, renderStepCount / 4), SkBlockAllocator::GrowthPolicy::kFibonacci)
        , fTarget(std::move(target))
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
    fCommands.reserve(renderStepCount);
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
    UniformBindingCache geometryUniformBindings(bufferMgr, &geometryUniformDataCache);
    UniformBindingCache shadingUniformBindings(bufferMgr, recorder->priv().uniformDataCache());
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();

    std::unordered_map<const GraphicsPipelineDesc*, uint32_t, Hash, Eq> pipelineDescToIndex;

    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount()); // will not exceed but may use less with occluded draws

    SkShaderCodeDictionary* dict = recorder->priv().resourceProvider()->shaderCodeDictionary();
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);
    SkPipelineDataGatherer gatherer(Layout::kMetal);  // TODO: get the layout from the recorder

    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        // If we have two different descriptors, such that the uniforms from the PaintParams can be
        // bound independently of those used by the rest of the RenderStep, then we can upload now
        // and remember the location for re-use on any RenderStep that does shading.
        SkUniquePaintParamsID shaderID;
        UniformDataCache::Index shadingUniformIndex;
        TextureDataCache::Index textureBindingIndex;
        if (draw.fPaintParams.has_value()) {
            UniformDataCache::Index uniformDataIndex;
            std::tie(shaderID, uniformDataIndex, textureBindingIndex) =
                    ExtractPaintData(recorder, &gatherer, &builder,
                                     draw.fDrawParams.transform().inverse(),
                                     draw.fPaintParams.value());
            shadingUniformIndex = shadingUniformBindings.addUniforms(uniformDataIndex);
        } // else depth-only

        for (int stepIndex = 0; stepIndex < draw.fRenderer.numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer.steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            SkUniquePaintParamsID stepShaderID;
            UniformDataCache::Index stepShadingUniformIndex;
            TextureDataCache::Index stepTextureBindingIndex;
            if (performsShading) {
                stepShaderID = shaderID;
                stepShadingUniformIndex = shadingUniformIndex;
                stepTextureBindingIndex = textureBindingIndex;
            } // else depth-only draw or stencil-only step of renderer so no shading is needed

            UniformDataCache::Index geometryUniformIndex;
            if (step->numUniforms() > 0) {
                UniformDataCache::Index uniformDataIndex;
                uniformDataIndex = ExtractRenderStepData(&geometryUniformDataCache,
                                                         &gatherer,
                                                         step,
                                                         draw.fDrawParams);
                geometryUniformIndex = geometryUniformBindings.addUniforms(uniformDataIndex);
            }

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

            keys.push_back({&draw, stepIndex, pipelineIndex,
                            geometryUniformIndex,
                            stepShadingUniformIndex,
                            stepTextureBindingIndex});
        }

        passBounds.join(draw.fDrawParams.clip().drawBounds());
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
    UniformDataCache::Index lastShadingUniforms;
    TextureDataCache::Index lastTextureBindings;
    UniformDataCache::Index lastGeometryUniforms;
    SkIRect lastScissor = SkIRect::MakeSize(drawPass->fTarget->dimensions());

    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = *key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool geometryUniformChange = key.geometryUniforms().isValid() &&
                                           key.geometryUniforms() != lastGeometryUniforms;
        const bool shadingUniformChange = key.shadingUniforms().isValid() &&
                                          key.shadingUniforms() != lastShadingUniforms;
        const bool textureBindingsChange = key.textureBindings().isValid() &&
                                           key.textureBindings() != lastTextureBindings;

        const bool pipelineChange = key.pipeline() != lastPipeline;
        const bool stateChange = geometryUniformChange ||
                                 shadingUniformChange ||
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
            drawPass->fCommands.emplace_back(BindGraphicsPipeline{key.pipeline()});
            lastPipeline = key.pipeline();
        }
        if (stateChange) {
            if (geometryUniformChange) {
                auto binding = geometryUniformBindings.getBinding(key.geometryUniforms());
                drawPass->fCommands.emplace_back(
                        BindUniformBuffer{binding, UniformSlot::kRenderStep});
                lastGeometryUniforms = key.geometryUniforms();
            }
            if (shadingUniformChange) {
                auto binding = shadingUniformBindings.getBinding(key.shadingUniforms());
                drawPass->fCommands.emplace_back(
                        BindUniformBuffer{binding, UniformSlot::kPaint});
                lastShadingUniforms = key.shadingUniforms();
            }
            if (textureBindingsChange) {
                auto textureDataBlock = textureDataCache->lookup(key.textureBindings());
                BindTexturesAndSamplers bts;
                bts.fNumTexSamplers = textureDataBlock->numTextures();
                // TODO: Remove this assert once BindTexturesAndSamplers doesn't have a fixed size
                // of textures and samplers arrays.
                SkASSERT(bts.fNumTexSamplers <= 32);
                for (int i = 0; i < bts.fNumTexSamplers; ++i) {
                    auto& info = textureDataBlock->texture(i);
                    std::tie(bts.fTextureIndices[i], bts.fSamplerIndices[i]) =
                            get_unique_texture_sampler_indices(drawPass->fSampledTextures,
                                                               drawPass->fSamplerDescs,
                                                               info);
                }

                drawPass->fCommands.push_back(Command(bts));
                lastTextureBindings = key.textureBindings();
            }
            if (draw.fDrawParams.clip().scissor() != lastScissor) {
                drawPass->fCommands.emplace_back(SetScissor{draw.fDrawParams.clip().scissor()});
                lastScissor = draw.fDrawParams.clip().scissor();
            }
        }

        renderStep.writeVertices(&drawWriter, draw.fDrawParams);
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    passBounds.roundOut();
    drawPass->fBounds = SkIRect::MakeLTRB((int) passBounds.left(), (int) passBounds.top(),
                                          (int) passBounds.right(), (int) passBounds.bot());
    return drawPass;
}

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                const RenderPassDesc& renderPassDesc) {
    fFullPipelines.reserve(fPipelineDescs.count());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs.items()) {
        auto pipeline = resourceProvider->findOrCreateGraphicsPipeline(pipelineDesc,
                                                                       renderPassDesc);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create GraphicsPipeline for draw in RenderPass. Droping Pass");
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

bool DrawPass::addCommands(CommandBuffer* buffer) const {
    // TODO: Validate RenderPass state against DrawPass's target and requirements?
    // Generate actual GraphicsPipeline objects combining the target-level properties and each of
    // the GraphicsPipelineDesc's referenced in this DrawPass.

    // Set viewport to the entire texture for now (eventually, we may have logically smaller bounds
    // within an approx-sized texture). It is assumed that this also configures the sk_rtAdjust
    // intrinsic for programs (however the backend chooses to do so).
    buffer->setViewport(0, 0, fTarget->dimensions().width(), fTarget->dimensions().height());

    for (const Command& c : fCommands.items()) {
        switch(c.fType) {
            case CommandType::kBindGraphicsPipeline: {
                auto& d = c.fBindGraphicsPipeline;
                buffer->bindGraphicsPipeline(fFullPipelines[d.fPipelineIndex]);
            } break;
            case CommandType::kBindUniformBuffer: {
                auto& d = c.fBindUniformBuffer;
                buffer->bindUniformBuffer(d.fSlot, sk_ref_sp(d.fInfo.fBuffer), d.fInfo.fOffset);
            } break;
            case CommandType::kBindTexturesAndSamplers: {
                auto& d = c.fBindTexturesAndSamplers;

                for (int i = 0; i < d.fNumTexSamplers; ++i) {
                    SkASSERT(fSampledTextures[d.fTextureIndices[i]]);
                    SkASSERT(fSampledTextures[d.fTextureIndices[i]]->texture());
                    SkASSERT(fSamplers[d.fSamplerIndices[i]]);
                    buffer->bindTextureAndSampler(
                            fSampledTextures[d.fTextureIndices[i]]->refTexture(),
                            fSamplers[d.fSamplerIndices[i]],
                            i);
                }

            } break;
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
                                   d.fScissor.width(), d.fScissor.height());
                break;
            }
        }
    }

    return true;
}

} // namespace skgpu::graphite
