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
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/ResourceTypes.h"

struct SkSamplingOptions;
class SkShaderCodeDictionary;

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class BackendTexture;
class Buffer;
class Caps;
class GlobalCache;
class Gpu;
class GraphicsPipeline;
class GraphiteResourceKey;
class ResourceCache;
class Sampler;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    virtual sk_sp<CommandBuffer> createCommandBuffer() = 0;

    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(const GraphicsPipelineDesc&,
                                                         const RenderPassDesc&);

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

#if GRAPHITE_TEST_UTILS
    ResourceCache* resourceCache() { return fResourceCache.get(); }
    const Gpu* gpu() { return fGpu; }
#endif

protected:
    ResourceProvider(const Gpu* gpu, sk_sp<GlobalCache>, SingleOwner* singleOwner);

    const Gpu* fGpu;

private:
    virtual sk_sp<GraphicsPipeline> onCreateGraphicsPipeline(const GraphicsPipelineDesc&,
                                                             const RenderPassDesc&) = 0;
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&, SkBudgeted) = 0;
    virtual sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) = 0;

    virtual sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                         SkTileMode xTileMode,
                                         SkTileMode yTileMode) = 0;

    sk_sp<Texture> findOrCreateTextureWithKey(SkISize dimensions,
                                              const TextureInfo& info,
                                              const GraphiteResourceKey& key,
                                              SkBudgeted);

    class GraphicsPipelineCache {
    public:
        GraphicsPipelineCache(ResourceProvider* resourceProvider);
        ~GraphicsPipelineCache();

        void release();
        sk_sp<GraphicsPipeline> refPipeline(const Caps* caps,
                                            const GraphicsPipelineDesc&,
                                            const RenderPassDesc&);

    private:
        struct Entry;
        struct KeyHash {
            uint32_t operator()(const UniqueKey& key) const {
                return key.hash();
            }
        };
        SkLRUCache<UniqueKey, std::unique_ptr<Entry>, KeyHash> fMap;

        ResourceProvider* fResourceProvider;
    };

    sk_sp<ResourceCache> fResourceCache;
    sk_sp<GlobalCache> fGlobalCache;

    // Cache of GraphicsPipelines
    // TODO: Move this onto GlobalCache
    std::unique_ptr<GraphicsPipelineCache> fGraphicsPipelineCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceProvider_DEFINED
