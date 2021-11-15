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
#include "src/core/SkLRUCache.h"

namespace skgpu {

class Buffer;
class Gpu;
class GraphicsPipeline;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    virtual sk_sp<CommandBuffer> createCommandBuffer() = 0;

    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(const GraphicsPipelineDesc&);

    sk_sp<Texture> findOrCreateTexture(SkISize, const TextureInfo&);

    sk_sp<Buffer> findOrCreateBuffer(size_t size, BufferType type, PrioritizeGpuReads);

protected:
    ResourceProvider(const Gpu* gpu);

    const Gpu* fGpu;

private:
    virtual sk_sp<GraphicsPipeline> onCreateGraphicsPipeline(const GraphicsPipelineDesc&) = 0;
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&) = 0;
    virtual sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) = 0;

    class GraphicsPipelineCache {
    public:
        GraphicsPipelineCache(ResourceProvider* resourceProvider);
        ~GraphicsPipelineCache();

        void release();
        sk_sp<GraphicsPipeline> refPipeline(const GraphicsPipelineDesc&);

    private:
        struct Entry;

        struct DescHash {
            uint32_t operator()(const GraphicsPipelineDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GraphicsPipelineDesc, std::unique_ptr<Entry>, DescHash> fMap;

        ResourceProvider* fResourceProvider;
    };

    // Cache of GraphicsPipelines
    std::unique_ptr<GraphicsPipelineCache> fGraphicsPipelineCache;
};

} // namespace skgpu

#endif // skgpu_ResourceProvider_DEFINED
