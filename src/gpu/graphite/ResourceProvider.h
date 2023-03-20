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
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceTypes.h"

struct SkSamplingOptions;

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
class RuntimeEffectDictionary;
class ShaderCodeDictionary;
class Sampler;
class SharedContext;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    // The runtime effect dictionary provides a link between SkCodeSnippetIds referenced in the
    // paint key and the current SkRuntimeEffect that provides the SkSL for that id.
    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(const RuntimeEffectDictionary*,
                                                         const GraphicsPipelineDesc&,
                                                         const RenderPassDesc&);

    sk_sp<ComputePipeline> findOrCreateComputePipeline(const ComputePipelineDesc&);

    sk_sp<Texture> findOrCreateScratchTexture(SkISize, const TextureInfo&, skgpu::Budgeted);
    virtual sk_sp<Texture> createWrappedTexture(const BackendTexture&) = 0;

    sk_sp<Texture> findOrCreateDepthStencilAttachment(SkISize dimensions,
                                                      const TextureInfo&);

    sk_sp<Texture> findOrCreateDiscardableMSAAAttachment(SkISize dimensions,
                                                         const TextureInfo&);

    sk_sp<Buffer> findOrCreateBuffer(size_t size, BufferType type, PrioritizeGpuReads);

    sk_sp<Sampler> findOrCreateCompatibleSampler(const SkSamplingOptions&,
                                                 SkTileMode xTileMode,
                                                 SkTileMode yTileMode);

    SkSL::Compiler* skslCompiler() { return fCompiler.get(); }

    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);
    void deleteBackendTexture(BackendTexture&);

#if GRAPHITE_TEST_UTILS
    ResourceCache* resourceCache() { return fResourceCache.get(); }
    const SharedContext* sharedContext() { return fSharedContext; }
#endif

protected:
    ResourceProvider(SharedContext* sharedContext,
                     SingleOwner* singleOwner);

    SharedContext* fSharedContext;

private:
    virtual sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                           const GraphicsPipelineDesc&,
                                                           const RenderPassDesc&) = 0;
    virtual sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) = 0;
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&, skgpu::Budgeted) = 0;
    virtual sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) = 0;

    virtual sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                         SkTileMode xTileMode,
                                         SkTileMode yTileMode) = 0;

    sk_sp<Texture> findOrCreateTextureWithKey(SkISize dimensions,
                                              const TextureInfo& info,
                                              const GraphiteResourceKey& key,
                                              skgpu::Budgeted);

    virtual BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) = 0;
    virtual void onDeleteBackendTexture(BackendTexture&) = 0;

    // Each ResourceProvider owns one local cache; for some resources it also refers out to the
    // global cache of the SharedContext, which is assumed to outlive the ResourceProvider.
    sk_sp<ResourceCache> fResourceCache;

    // Compiler used for compiling SkSL into backend shader code. We only want to create the
    // compiler once, as there is significant overhead to the first compile.
    std::unique_ptr<SkSL::Compiler> fCompiler;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceProvider_DEFINED
