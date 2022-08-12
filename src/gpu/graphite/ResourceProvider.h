/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ResourceProvider_DEFINED
#define skgpu_graphite_ResourceProvider_DEFINED

#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkRuntimeEffectDictionary.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceTypes.h"

struct SkSamplingOptions;
class SkShaderCodeDictionary;

namespace skgpu {
class SingleOwner;
}

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {

class BackendTexture;
class Buffer;
class Caps;
class ComputePipeline;
class ComputePipelineDesc;
class GlobalCache;
class GraphicsPipeline;
class GraphicsPipelineDesc;
class GraphiteResourceKey;
class ResourceCache;
class Sampler;
class SharedContext;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    virtual sk_sp<CommandBuffer> createCommandBuffer() = 0;

    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(const GraphicsPipelineDesc&,
                                                         const RenderPassDesc&);

    sk_sp<ComputePipeline> findOrCreateComputePipeline(const ComputePipelineDesc&);

    sk_sp<Texture> findOrCreateScratchTexture(SkISize, const TextureInfo&, SkBudgeted);
    virtual sk_sp<Texture> createWrappedTexture(const BackendTexture&) = 0;

    sk_sp<Texture> findOrCreateDepthStencilAttachment(SkISize dimensions,
                                                      const TextureInfo&);

    sk_sp<Texture> findOrCreateDiscardableMSAAAttachment(SkISize dimensions,
                                                         const TextureInfo&);

    sk_sp<Buffer> findOrCreateBuffer(size_t size, BufferType type, PrioritizeGpuReads);

    sk_sp<Sampler> findOrCreateCompatibleSampler(const SkSamplingOptions&,
                                                 SkTileMode xTileMode,
                                                 SkTileMode yTileMode);

    SkShaderCodeDictionary* shaderCodeDictionary() const;

    SkRuntimeEffectDictionary* runtimeEffectDictionary() { return &fRuntimeEffectDictionary; }

    SkSL::Compiler* skslCompiler() { return fCompiler.get(); }

    void resetAfterSnap();

#if GRAPHITE_TEST_UTILS
    ResourceCache* resourceCache() { return fResourceCache.get(); }
    const SharedContext* sharedContext() { return fSharedContext; }
#endif

protected:
    ResourceProvider(const SharedContext* sharedContext,
                     sk_sp<GlobalCache>,
                     SingleOwner* singleOwner);

    const SharedContext* fSharedContext;

private:
    virtual sk_sp<GraphicsPipeline> createGraphicsPipeline(const GraphicsPipelineDesc&,
                                                           const RenderPassDesc&) = 0;
    virtual sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) = 0;
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&, SkBudgeted) = 0;
    virtual sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) = 0;

    virtual sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                         SkTileMode xTileMode,
                                         SkTileMode yTileMode) = 0;

    sk_sp<Texture> findOrCreateTextureWithKey(SkISize dimensions,
                                              const TextureInfo& info,
                                              const GraphiteResourceKey& key,
                                              SkBudgeted);

    sk_sp<ResourceCache> fResourceCache;
    sk_sp<GlobalCache>   fGlobalCache;

    // TODO: To be moved to Recorder
    SkRuntimeEffectDictionary fRuntimeEffectDictionary;
    // Compiler used for compiling SkSL into backend shader code. We only want to create the
    // compiler once, as there is significant overhead to the first compile.
    std::unique_ptr<SkSL::Compiler> fCompiler;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceProvider_DEFINED
