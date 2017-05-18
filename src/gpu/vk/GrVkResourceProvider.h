/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkResourceProvider_DEFINED
#define GrVkResourceProvider_DEFINED

#include "GrGpu.h"
#include "GrResourceHandle.h"
#include "GrVkDescriptorPool.h"
#include "GrVkDescriptorSetManager.h"
#include "GrVkPipelineState.h"
#include "GrVkRenderPass.h"
#include "GrVkResource.h"
#include "GrVkUtil.h"
#include "SkLRUCache.h"
#include "SkTArray.h"
#include "SkTDynamicHash.h"
#include "SkTInternalLList.h"

#include "vk/GrVkDefines.h"

class GrPipeline;
class GrPrimitiveProcessor;
class GrSamplerParams;
class GrVkCopyPipeline;
class GrVkGpu;
class GrVkPipeline;
class GrVkPrimaryCommandBuffer;
class GrVkRenderTarget;
class GrVkSampler;
class GrVkSecondaryCommandBuffer;
class GrVkUniformHandler;

class GrVkResourceProvider {
public:
    GrVkResourceProvider(GrVkGpu* gpu);
    ~GrVkResourceProvider();

    // Set up any initial vk objects
    void init();

    GrVkPipeline* createPipeline(const GrPipeline& pipeline,
                                 const GrStencilSettings& stencil,
                                 const GrPrimitiveProcessor& primProc,
                                 VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                 int shaderStageCount,
                                 GrPrimitiveType primitiveType,
                                 const GrVkRenderPass& renderPass,
                                 VkPipelineLayout layout);

    GrVkCopyPipeline* findOrCreateCopyPipeline(const GrVkRenderTarget* dst,
                                               VkPipelineShaderStageCreateInfo*,
                                               VkPipelineLayout);

    GR_DEFINE_RESOURCE_HANDLE_CLASS(CompatibleRPHandle);

    // Finds or creates a simple render pass that matches the target, increments the refcount,
    // and returns. The caller can optionally pass in a pointer to a CompatibleRPHandle. If this is
    // non null it will be set to a handle that can be used in the furutre to quickly return a
    // compatible GrVkRenderPasses without the need inspecting a GrVkRenderTarget.
    const GrVkRenderPass* findCompatibleRenderPass(const GrVkRenderTarget& target,
                                                   CompatibleRPHandle* compatibleHandle = nullptr);
    // The CompatibleRPHandle must be a valid handle previously set by a call to
    // findCompatibleRenderPass(GrVkRenderTarget&, CompatibleRPHandle*).
    const GrVkRenderPass* findCompatibleRenderPass(const CompatibleRPHandle& compatibleHandle);

    // Finds or creates a render pass that matches the target and LoadStoreOps, increments the
    // refcount, and returns. The caller can optionally pass in a pointer to a CompatibleRPHandle.
    // If this is non null it will be set to a handle that can be used in the furutre to quickly
    // return a GrVkRenderPasses without the need inspecting a GrVkRenderTarget.
    const GrVkRenderPass* findRenderPass(const GrVkRenderTarget& target,
                                         const GrVkRenderPass::LoadStoreOps& colorOps,
                                         const GrVkRenderPass::LoadStoreOps& stencilOps,
                                         CompatibleRPHandle* compatibleHandle = nullptr);

    // The CompatibleRPHandle must be a valid handle previously set by a call to findRenderPass or
    // findCompatibleRenderPass.
    const GrVkRenderPass* findRenderPass(const CompatibleRPHandle& compatibleHandle,
                                         const GrVkRenderPass::LoadStoreOps& colorOps,
                                         const GrVkRenderPass::LoadStoreOps& stencilOps);

    GrVkPrimaryCommandBuffer* findOrCreatePrimaryCommandBuffer();
    void checkCommandBuffers();

    GrVkSecondaryCommandBuffer* findOrCreateSecondaryCommandBuffer();
    void recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* cb);

    // Finds or creates a compatible GrVkDescriptorPool for the requested type and count.
    // The refcount is incremented and a pointer returned.
    // TODO: Currently this will just create a descriptor pool without holding onto a ref itself
    //       so we currently do not reuse them. Rquires knowing if another draw is currently using
    //       the GrVkDescriptorPool, the ability to reset pools, and the ability to purge pools out
    //       of our cache of GrVkDescriptorPools.
    GrVkDescriptorPool* findOrCreateCompatibleDescriptorPool(VkDescriptorType type, uint32_t count);

    // Finds or creates a compatible GrVkSampler based on the GrSamplerParams.
    // The refcount is incremented and a pointer returned.
    GrVkSampler* findOrCreateCompatibleSampler(const GrSamplerParams&, uint32_t mipLevels);

    sk_sp<GrVkPipelineState> findOrCreateCompatiblePipelineState(const GrPipeline&,
                                                                 const GrPrimitiveProcessor&,
                                                                 GrPrimitiveType,
                                                                 const GrVkRenderPass& renderPass);

    void getSamplerDescriptorSetHandle(VkDescriptorType type,
                                       const GrVkUniformHandler&,
                                       GrVkDescriptorSetManager::Handle* handle);
    void getSamplerDescriptorSetHandle(VkDescriptorType type,
                                       const SkTArray<uint32_t>& visibilities,
                                       GrVkDescriptorSetManager::Handle* handle);

    // Returns the compatible VkDescriptorSetLayout to use for uniform buffers. The caller does not
    // own the VkDescriptorSetLayout and thus should not delete it. This function should be used
    // when the caller needs the layout to create a VkPipelineLayout.
    VkDescriptorSetLayout getUniformDSLayout() const;

    // Returns the compatible VkDescriptorSetLayout to use for a specific sampler handle. The caller
    // does not own the VkDescriptorSetLayout and thus should not delete it. This function should be
    // used when the caller needs the layout to create a VkPipelineLayout.
    VkDescriptorSetLayout getSamplerDSLayout(const GrVkDescriptorSetManager::Handle&) const;

    // Returns a GrVkDescriptorSet that can be used for uniform buffers. The GrVkDescriptorSet
    // is already reffed for the caller.
    const GrVkDescriptorSet* getUniformDescriptorSet();

    // Returns a GrVkDescriptorSet that can be used for sampler descriptors that are compatible with
    // the GrVkDescriptorSetManager::Handle passed in. The GrVkDescriptorSet is already reffed for
    // the caller.
    const GrVkDescriptorSet* getSamplerDescriptorSet(const GrVkDescriptorSetManager::Handle&);


    // Signals that the descriptor set passed it, which is compatible with the passed in handle,
    // can be reused by the next allocation request.
    void recycleDescriptorSet(const GrVkDescriptorSet* descSet,
                              const GrVkDescriptorSetManager::Handle&);

    // Creates or finds free uniform buffer resources of size GrVkUniformBuffer::kStandardSize.
    // Anything larger will need to be created and released by the client.
    const GrVkResource* findOrCreateStandardUniformBufferResource();

    // Signals that the resource passed to it (which should be a uniform buffer resource)
    // can be reused by the next uniform buffer resource request.
    void recycleStandardUniformBufferResource(const GrVkResource*);

    // Destroy any cached resources. To be called before destroying the VkDevice.
    // The assumption is that all queues are idle and all command buffers are finished.
    // For resource tracing to work properly, this should be called after unrefing all other
    // resource usages.
    // If deviceLost is true, then resources will not be checked to see if they've finished
    // before deleting (see section 4.2.4 of the Vulkan spec).
    void destroyResources(bool deviceLost);

    // Abandon any cached resources. To be used when the context/VkDevice is lost.
    // For resource tracing to work properly, this should be called after unrefing all other
    // resource usages.
    void abandonResources();

private:
#ifdef SK_DEBUG
#define GR_PIPELINE_STATE_CACHE_STATS
#endif

    class PipelineStateCache : public ::SkNoncopyable {
    public:
        PipelineStateCache(GrVkGpu* gpu);
        ~PipelineStateCache();

        void abandon();
        void release();
        sk_sp<GrVkPipelineState> refPipelineState(const GrPipeline&,
                                                  const GrPrimitiveProcessor&,
                                                  GrPrimitiveType,
                                                  const GrVkRenderPass& renderPass);

    private:
        enum {
            // We may actually have kMaxEntries+1 PipelineStates in context because we create a new
            // PipelineState before evicting from the cache.
            kMaxEntries = 128,
        };

        struct Entry;

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GrVkPipelineState::Desc, std::unique_ptr<Entry>, DescHash> fMap;

        GrVkGpu*                    fGpu;

#ifdef GR_PIPELINE_STATE_CACHE_STATS
        int                         fTotalRequests;
        int                         fCacheMisses;
#endif
    };

    class CompatibleRenderPassSet {
    public:
        // This will always construct the basic load store render pass (all attachments load and
        // store their data) so that there is at least one compatible VkRenderPass that can be used
        // with this set.
        CompatibleRenderPassSet(const GrVkGpu* gpu, const GrVkRenderTarget& target);

        bool isCompatible(const GrVkRenderTarget& target) const;

        GrVkRenderPass* getCompatibleRenderPass() const {
            // The first GrVkRenderpass should always exist since we create the basic load store
            // render pass on create
            SkASSERT(fRenderPasses[0]);
            return fRenderPasses[0];
        }

        GrVkRenderPass* getRenderPass(const GrVkGpu* gpu,
                                      const GrVkRenderPass::LoadStoreOps& colorOps,
                                      const GrVkRenderPass::LoadStoreOps& stencilOps);

        void releaseResources(const GrVkGpu* gpu);
        void abandonResources();

    private:
        SkSTArray<4, GrVkRenderPass*> fRenderPasses;
        int                           fLastReturnedIndex;
    };

    GrVkGpu* fGpu;

    // Central cache for creating pipelines
    VkPipelineCache fPipelineCache;

    // Cache of previously created copy pipelines
    SkTArray<GrVkCopyPipeline*> fCopyPipelines;

    SkSTArray<4, CompatibleRenderPassSet> fRenderPassArray;

    // Array of PrimaryCommandBuffers that are currently in flight
    SkSTArray<4, GrVkPrimaryCommandBuffer*, true> fActiveCommandBuffers;
    // Array of available primary command buffers that are not in flight
    SkSTArray<4, GrVkPrimaryCommandBuffer*, true> fAvailableCommandBuffers;

    // Array of available secondary command buffers
    SkSTArray<16, GrVkSecondaryCommandBuffer*, true> fAvailableSecondaryCommandBuffers;

    // Array of available uniform buffer resources
    SkSTArray<16, const GrVkResource*, true> fAvailableUniformBufferResources;

    // Stores GrVkSampler objects that we've already created so we can reuse them across multiple
    // GrVkPipelineStates
    SkTDynamicHash<GrVkSampler, uint16_t> fSamplers;

    // Cache of GrVkPipelineStates
    PipelineStateCache* fPipelineStateCache;

    SkSTArray<4, std::unique_ptr<GrVkDescriptorSetManager>> fDescriptorSetManagers;

    GrVkDescriptorSetManager::Handle fUniformDSHandle;
};

#endif
