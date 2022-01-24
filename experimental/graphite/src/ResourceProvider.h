/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceProvider_DEFINED
#define skgpu_ResourceProvider_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ResourceKey.h"

struct SkSamplingOptions;

namespace skgpu {

class BackendTexture;
class Buffer;
class Gpu;
class GraphicsPipeline;
class Sampler;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    virtual sk_sp<CommandBuffer> createCommandBuffer() = 0;

    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(Context*, const GraphicsPipelineDesc&,
                                                         const RenderPassDesc&);

    sk_sp<Texture> findOrCreateTexture(SkISize, const TextureInfo&);
    virtual sk_sp<Texture> createWrappedTexture(const BackendTexture&) = 0;

    sk_sp<Buffer> findOrCreateBuffer(size_t size, BufferType type, PrioritizeGpuReads);

    sk_sp<Sampler> findOrCreateCompatibleSampler(const SkSamplingOptions&,
                                                 SkTileMode xTileMode,
                                                 SkTileMode yTileMode);

protected:
    ResourceProvider(const Gpu* gpu);

    const Gpu* fGpu;

private:
    virtual sk_sp<GraphicsPipeline> onCreateGraphicsPipeline(Context*,
                                                             const GraphicsPipelineDesc&,
                                                             const RenderPassDesc&) = 0;
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&) = 0;
    virtual sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) = 0;

    virtual sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                         SkTileMode xTileMode,
                                         SkTileMode yTileMode) = 0;

    class GraphicsPipelineCache {
    public:
        GraphicsPipelineCache(ResourceProvider* resourceProvider);
        ~GraphicsPipelineCache();

        void release();
        sk_sp<GraphicsPipeline> refPipeline(Context*, const GraphicsPipelineDesc&,
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

    // Cache of GraphicsPipelines
    std::unique_ptr<GraphicsPipelineCache> fGraphicsPipelineCache;
};

} // namespace skgpu

#endif // skgpu_ResourceProvider_DEFINED
