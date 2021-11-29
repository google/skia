/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkResourceProvider_DEFINED
#define GrVkResourceProvider_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrResourceHandle.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/vk/GrVkDescriptorPool.h"
#include "src/gpu/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkSampler.h"
#include "src/gpu/vk/GrVkSamplerYcbcrConversion.h"
#include "src/gpu/vk/GrVkUtil.h"

class GrVkCommandPool;
class GrVkGpu;
class GrVkPipeline;
class GrVkPipelineState;
class GrVkPrimaryCommandBuffer;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;
class GrVkUniformHandler;

class GrVkResourceProvider {
public:
    GrVkResourceProvider(GrVkGpu* gpu);
    ~GrVkResourceProvider();

    GrThreadSafePipelineBuilder* pipelineStateCache() {
        return fPipelineStateCache.get();
    }

    sk_sp<GrThreadSafePipelineBuilder> refPipelineStateCache() {
        return fPipelineStateCache;
    }

    // Set up any initial vk objects
    void init();

    sk_sp<const GrVkPipeline> makePipeline(const GrProgramInfo&,
                                           VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                           int shaderStageCount,
                                           VkRenderPass compatibleRenderPass,
                                           VkPipelineLayout layout,
                                           uint32_t subpass);

    GR_DEFINE_RESOURCE_HANDLE_CLASS(CompatibleRPHandle);

    using SelfDependencyFlags = GrVkRenderPass::SelfDependencyFlags;
    using LoadFromResolve = GrVkRenderPass::LoadFromResolve;

    // Finds or creates a simple render pass that matches the target, increments the refcount,
    // and returns. The caller can optionally pass in a pointer to a CompatibleRPHandle. If this is
    // non null it will be set to a handle that can be used in the furutre to quickly return a
    // compatible GrVkRenderPasses without the need inspecting a GrVkRenderTarget.
    const GrVkRenderPass* findCompatibleRenderPass(GrVkRenderTarget* target,
                                                   CompatibleRPHandle* compatibleHandle,
                                                   bool withResolve,
                                                   bool withStencil,
                                                   SelfDependencyFlags selfDepFlags,
                                                   LoadFromResolve);
    const GrVkRenderPass* findCompatibleRenderPass(GrVkRenderPass::AttachmentsDescriptor*,
                                                   GrVkRenderPass::AttachmentFlags,
                                                   SelfDependencyFlags selfDepFlags,
                                                   LoadFromResolve,
                                                   CompatibleRPHandle* compatibleHandle = nullptr);

    const GrVkRenderPass* findCompatibleExternalRenderPass(VkRenderPass,
                                                           uint32_t colorAttachmentIndex);


    // Finds or creates a render pass that matches the target and LoadStoreOps, increments the
    // refcount, and returns. The caller can optionally pass in a pointer to a CompatibleRPHandle.
    // If this is non null it will be set to a handle that can be used in the future to quickly
    // return a GrVkRenderPass without the need to inspect a GrVkRenderTarget.
    // TODO: sk_sp?
    const GrVkRenderPass* findRenderPass(GrVkRenderTarget* target,
                                         const GrVkRenderPass::LoadStoreOps& colorOps,
                                         const GrVkRenderPass::LoadStoreOps& resolveOps,
                                         const GrVkRenderPass::LoadStoreOps& stencilOps,
                                         CompatibleRPHandle* compatibleHandle,
                                         bool withResolve,
                                         bool withStencil,
                                         SelfDependencyFlags selfDepFlags,
                                         LoadFromResolve);

    // The CompatibleRPHandle must be a valid handle previously set by a call to findRenderPass or
    // findCompatibleRenderPass.
    const GrVkRenderPass* findRenderPass(const CompatibleRPHandle& compatibleHandle,
                                         const GrVkRenderPass::LoadStoreOps& colorOps,
                                         const GrVkRenderPass::LoadStoreOps& resolveOps,
                                         const GrVkRenderPass::LoadStoreOps& stencilOps);

    GrVkCommandPool* findOrCreateCommandPool();

    void checkCommandBuffers();

    void forceSyncAllCommandBuffers();

    // We must add the finishedProc to all active command buffers since we may have flushed work
    // that the client cares about before they explicitly called flush and the GPU may reorder
    // command execution. So we make sure all previously submitted work finishes before we call the
    // finishedProc.
    void addFinishedProcToActiveCommandBuffers(sk_sp<GrRefCntedCallback> finishedCallback);

    // Finds or creates a compatible GrVkDescriptorPool for the requested type and count.
    // The refcount is incremented and a pointer returned.
    // TODO: Currently this will just create a descriptor pool without holding onto a ref itself
    //       so we currently do not reuse them. Rquires knowing if another draw is currently using
    //       the GrVkDescriptorPool, the ability to reset pools, and the ability to purge pools out
    //       of our cache of GrVkDescriptorPools.
    GrVkDescriptorPool* findOrCreateCompatibleDescriptorPool(VkDescriptorType type, uint32_t count);

    // Finds or creates a compatible GrVkSampler based on the GrSamplerState and
    // GrVkYcbcrConversionInfo. The refcount is incremented and a pointer returned.
    GrVkSampler* findOrCreateCompatibleSampler(GrSamplerState,
                                               const GrVkYcbcrConversionInfo& ycbcrInfo);

    // Finds or creates a compatible GrVkSamplerYcbcrConversion based on the GrSamplerState and
    // GrVkYcbcrConversionInfo. The refcount is incremented and a pointer returned.
    GrVkSamplerYcbcrConversion* findOrCreateCompatibleSamplerYcbcrConversion(
            const GrVkYcbcrConversionInfo& ycbcrInfo);

    GrVkPipelineState* findOrCreateCompatiblePipelineState(
            GrRenderTarget*,
            const GrProgramInfo&,
            VkRenderPass compatibleRenderPass,
            bool overrideSubpassForResolveLoad);

    GrVkPipelineState* findOrCreateCompatiblePipelineState(
            const GrProgramDesc&,
            const GrProgramInfo&,
            VkRenderPass compatibleRenderPass,
            GrThreadSafePipelineBuilder::Stats::ProgramCacheResult* stat);

    sk_sp<const GrVkPipeline> findOrCreateMSAALoadPipeline(
            const GrVkRenderPass& renderPass,
            int numSamples,
            VkPipelineShaderStageCreateInfo*,
            VkPipelineLayout);

    void getSamplerDescriptorSetHandle(VkDescriptorType type,
                                       const GrVkUniformHandler&,
                                       GrVkDescriptorSetManager::Handle* handle);

    // This is a convenience function to return a descriptor set for zero sammples. When making a
    // VkPipelineLayout we must pass in an array of valid descriptor set handles. However, we have
    // set up our system to have the descriptor sets be in the order uniform, sampler, input. So
    // if we have a uniform and input we will need to have a valid handle for the sampler as well.
    // When using the GrVkMSAALoadManager this is the case, but we also don't have a
    // GrVkUniformHandler to pass into the more general function. Thus we use this call instead.
    void getZeroSamplerDescriptorSetHandle(GrVkDescriptorSetManager::Handle* handle);

    // Returns the compatible VkDescriptorSetLayout to use for uniform buffers. The caller does not
    // own the VkDescriptorSetLayout and thus should not delete it. This function should be used
    // when the caller needs the layout to create a VkPipelineLayout.
    VkDescriptorSetLayout getUniformDSLayout() const;

    // Returns the compatible VkDescriptorSetLayout to use for input attachments. The caller does
    // not own the VkDescriptorSetLayout and thus should not delete it. This function should be used
    // when the caller needs the layout to create a VkPipelineLayout.
    VkDescriptorSetLayout getInputDSLayout() const;

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

    // Returns a GrVkDescriptorSet that can be used for input attachments. The GrVkDescriptorSet
    // is already reffed for the caller.
    const GrVkDescriptorSet* getInputDescriptorSet();

    // Signals that the descriptor set passed it, which is compatible with the passed in handle,
    // can be reused by the next allocation request.
    void recycleDescriptorSet(const GrVkDescriptorSet* descSet,
                              const GrVkDescriptorSetManager::Handle&);

    void storePipelineCacheData();

    // Destroy any cached resources. To be called before destroying the VkDevice.
    // The assumption is that all queues are idle and all command buffers are finished.
    // For resource tracing to work properly, this should be called after unrefing all other
    // resource usages.
    void destroyResources();

    // Currently we just release available command pools (which also releases their buffers). The
    // command buffers and pools take up the most memory. Other objects (e.g. samples,
    // ycbcr conversions, etc.) tend to be fairly light weight and not worth the effort to remove
    // them and then possibly remake them. Additionally many of those objects have refs/handles that
    // are held by other objects that aren't deleted here. Thus the memory wins for removing these
    // objects from the cache are probably not worth the complexity of safely releasing them.
    void releaseUnlockedBackendObjects();

#if GR_TEST_UTILS
    void resetShaderCacheForTesting() const { fPipelineStateCache->release(); }
#endif

private:
    class PipelineStateCache : public GrThreadSafePipelineBuilder {
    public:
        PipelineStateCache(GrVkGpu* gpu);
        ~PipelineStateCache() override;

        void release();
        GrVkPipelineState* findOrCreatePipelineState(GrRenderTarget*,
                                                     const GrProgramInfo&,
                                                     VkRenderPass compatibleRenderPass,
                                                     bool overrideSubpassForResolveLoad);
        GrVkPipelineState* findOrCreatePipelineState(const GrProgramDesc& desc,
                                                     const GrProgramInfo& programInfo,
                                                     VkRenderPass compatibleRenderPass,
                                                     Stats::ProgramCacheResult* stat) {
            return this->findOrCreatePipelineStateImpl(desc, programInfo, compatibleRenderPass,
                                                       false, stat);
        }

    private:
        struct Entry;

        GrVkPipelineState* findOrCreatePipelineStateImpl(const GrProgramDesc&,
                                                         const GrProgramInfo&,
                                                         VkRenderPass compatibleRenderPass,
                                                         bool overrideSubpassForResolveLoad,
                                                         Stats::ProgramCacheResult*);

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GrProgramDesc, std::unique_ptr<Entry>, DescHash> fMap;

        GrVkGpu*                    fGpu;
    };

    class CompatibleRenderPassSet {
    public:
        // This will always construct the basic load store render pass (all attachments load and
        // store their data) so that there is at least one compatible VkRenderPass that can be used
        // with this set.
        CompatibleRenderPassSet(GrVkRenderPass* renderPass);

        bool isCompatible(const GrVkRenderPass::AttachmentsDescriptor&,
                          GrVkRenderPass::AttachmentFlags,
                          SelfDependencyFlags selfDepFlags,
                          LoadFromResolve) const;

        const GrVkRenderPass* getCompatibleRenderPass() const {
            // The first GrVkRenderpass should always exist since we create the basic load store
            // render pass on create
            SkASSERT(fRenderPasses[0]);
            return fRenderPasses[0];
        }

        GrVkRenderPass* getRenderPass(GrVkGpu* gpu,
                                      const GrVkRenderPass::LoadStoreOps& colorOps,
                                      const GrVkRenderPass::LoadStoreOps& resolveOps,
                                      const GrVkRenderPass::LoadStoreOps& stencilOps);

        void releaseResources();

    private:
        SkSTArray<4, GrVkRenderPass*> fRenderPasses;
        int                           fLastReturnedIndex;
    };

    VkPipelineCache pipelineCache();

    GrVkGpu* fGpu;

    // Central cache for creating pipelines
    VkPipelineCache fPipelineCache;

    struct MSAALoadPipeline {
        sk_sp<const GrVkPipeline> fPipeline;
        const GrVkRenderPass* fRenderPass;
    };

    // Cache of previously created msaa load pipelines
    SkTArray<MSAALoadPipeline> fMSAALoadPipelines;

    SkSTArray<4, CompatibleRenderPassSet> fRenderPassArray;

    SkTArray<const GrVkRenderPass*> fExternalRenderPasses;

    // Array of command pools that we are waiting on
    SkSTArray<4, GrVkCommandPool*, true> fActiveCommandPools;

    // Array of available command pools that are not in flight
    SkSTArray<4, GrVkCommandPool*, true> fAvailableCommandPools;

    // Stores GrVkSampler objects that we've already created so we can reuse them across multiple
    // GrVkPipelineStates
    SkTDynamicHash<GrVkSampler, GrVkSampler::Key> fSamplers;

    // Stores GrVkSamplerYcbcrConversion objects that we've already created so we can reuse them.
    SkTDynamicHash<GrVkSamplerYcbcrConversion, GrVkSamplerYcbcrConversion::Key> fYcbcrConversions;

    // Cache of GrVkPipelineStates
    sk_sp<PipelineStateCache> fPipelineStateCache;

    SkSTArray<4, std::unique_ptr<GrVkDescriptorSetManager>> fDescriptorSetManagers;

    GrVkDescriptorSetManager::Handle fUniformDSHandle;
    GrVkDescriptorSetManager::Handle fInputDSHandle;
};

#endif
