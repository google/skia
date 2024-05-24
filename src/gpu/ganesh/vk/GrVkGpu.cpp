/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkGpu.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkBackendSemaphore.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrGpuResourceCacheAccess.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"
#include "src/gpu/ganesh/vk/GrVkBuffer.h"
#include "src/gpu/ganesh/vk/GrVkCommandBuffer.h"
#include "src/gpu/ganesh/vk/GrVkCommandPool.h"
#include "src/gpu/ganesh/vk/GrVkFramebuffer.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkOpsRenderPass.h"
#include "src/gpu/ganesh/vk/GrVkPipeline.h"
#include "src/gpu/ganesh/vk/GrVkPipelineState.h"
#include "src/gpu/ganesh/vk/GrVkRenderPass.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkSemaphore.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"
#include "src/gpu/ganesh/vk/GrVkTextureRenderTarget.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanMemory.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#if defined(SK_USE_VMA)
#include "src/gpu/vk/VulkanAMDMemoryAllocator.h"
#endif

using namespace skia_private;

#define VK_CALL(X) GR_VK_CALL(this->vkInterface(), X)
#define VK_CALL_RET(RET, X) GR_VK_CALL_RESULT(this, RET, X)

std::unique_ptr<GrGpu> GrVkGpu::Make(const GrVkBackendContext& backendContext,
                                     const GrContextOptions& options,
                                     GrDirectContext* direct) {
    if (backendContext.fInstance == VK_NULL_HANDLE ||
        backendContext.fPhysicalDevice == VK_NULL_HANDLE ||
        backendContext.fDevice == VK_NULL_HANDLE ||
        backendContext.fQueue == VK_NULL_HANDLE) {
        return nullptr;
    }
    if (!backendContext.fGetProc) {
        return nullptr;
    }

    PFN_vkEnumerateInstanceVersion localEnumerateInstanceVersion =
            reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
                    backendContext.fGetProc("vkEnumerateInstanceVersion",
                                            VK_NULL_HANDLE, VK_NULL_HANDLE));
    uint32_t instanceVersion = 0;
    if (!localEnumerateInstanceVersion) {
        instanceVersion = VK_MAKE_VERSION(1, 0, 0);
    } else {
        VkResult err = localEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            SkDebugf("Failed to enumerate instance version. Err: %d\n", err);
            return nullptr;
        }
    }

    PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                    backendContext.fGetProc("vkGetPhysicalDeviceProperties",
                                            backendContext.fInstance,
                                            VK_NULL_HANDLE));

    if (!localGetPhysicalDeviceProperties) {
        return nullptr;
    }
    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
    uint32_t physDevVersion = physDeviceProperties.apiVersion;

    uint32_t apiVersion = backendContext.fMaxAPIVersion ? backendContext.fMaxAPIVersion
                                                        : instanceVersion;

    instanceVersion = std::min(instanceVersion, apiVersion);
    physDevVersion = std::min(physDevVersion, apiVersion);

    sk_sp<const skgpu::VulkanInterface> interface;

    if (backendContext.fVkExtensions) {
        interface.reset(new skgpu::VulkanInterface(backendContext.fGetProc,
                                                   backendContext.fInstance,
                                                   backendContext.fDevice,
                                                   instanceVersion,
                                                   physDevVersion,
                                                   backendContext.fVkExtensions));
        if (!interface->validate(instanceVersion, physDevVersion, backendContext.fVkExtensions)) {
            return nullptr;
        }
    } else {
        skgpu::VulkanExtensions extensions;
        // The only extension flag that may effect the vulkan backend is the swapchain extension. We
        // need to know if this is enabled to know if we can transition to a present layout when
        // flushing a surface.
        if (backendContext.fExtensions & kKHR_swapchain_GrVkExtensionFlag) {
            const char* swapChainExtName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            extensions.init(backendContext.fGetProc, backendContext.fInstance,
                            backendContext.fPhysicalDevice, 0, nullptr, 1, &swapChainExtName);
        }
        interface.reset(new skgpu::VulkanInterface(backendContext.fGetProc,
                                                   backendContext.fInstance,
                                                   backendContext.fDevice,
                                                   instanceVersion,
                                                   physDevVersion,
                                                   &extensions));
        if (!interface->validate(instanceVersion, physDevVersion, &extensions)) {
            return nullptr;
        }
    }

    sk_sp<GrVkCaps> caps;
    if (backendContext.fDeviceFeatures2) {
        caps.reset(new GrVkCaps(options, interface.get(), backendContext.fPhysicalDevice,
                                *backendContext.fDeviceFeatures2, instanceVersion, physDevVersion,
                                *backendContext.fVkExtensions, backendContext.fProtectedContext));
    } else if (backendContext.fDeviceFeatures) {
        VkPhysicalDeviceFeatures2 features2;
        features2.pNext = nullptr;
        features2.features = *backendContext.fDeviceFeatures;
        caps.reset(new GrVkCaps(options, interface.get(), backendContext.fPhysicalDevice,
                                features2, instanceVersion, physDevVersion,
                                *backendContext.fVkExtensions, backendContext.fProtectedContext));
    } else {
        VkPhysicalDeviceFeatures2 features;
        memset(&features, 0, sizeof(VkPhysicalDeviceFeatures2));
        features.pNext = nullptr;
        if (backendContext.fFeatures & kGeometryShader_GrVkFeatureFlag) {
            features.features.geometryShader = true;
        }
        if (backendContext.fFeatures & kDualSrcBlend_GrVkFeatureFlag) {
            features.features.dualSrcBlend = true;
        }
        if (backendContext.fFeatures & kSampleRateShading_GrVkFeatureFlag) {
            features.features.sampleRateShading = true;
        }
        skgpu::VulkanExtensions extensions;
        // The only extension flag that may effect the vulkan backend is the swapchain extension. We
        // need to know if this is enabled to know if we can transition to a present layout when
        // flushing a surface.
        if (backendContext.fExtensions & kKHR_swapchain_GrVkExtensionFlag) {
            const char* swapChainExtName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            extensions.init(backendContext.fGetProc, backendContext.fInstance,
                            backendContext.fPhysicalDevice, 0, nullptr, 1, &swapChainExtName);
        }
        caps.reset(new GrVkCaps(options, interface.get(), backendContext.fPhysicalDevice,
                                features, instanceVersion, physDevVersion, extensions,
                                backendContext.fProtectedContext));
    }

    if (!caps) {
        return nullptr;
    }

    sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator = backendContext.fMemoryAllocator;
#if defined(SK_USE_VMA)
    if (!memoryAllocator) {
        // We were not given a memory allocator at creation
        memoryAllocator = skgpu::VulkanAMDMemoryAllocator::Make(backendContext.fInstance,
                                                                backendContext.fPhysicalDevice,
                                                                backendContext.fDevice,
                                                                physDevVersion,
                                                                backendContext.fVkExtensions,
                                                                interface.get(),
                                                                skgpu::ThreadSafe::kNo);
    }
#endif
    if (!memoryAllocator) {
        SkDEBUGFAIL("No supplied vulkan memory allocator and unable to create one internally.");
        return nullptr;
    }

    std::unique_ptr<GrVkGpu> vkGpu(new GrVkGpu(direct,
                                               backendContext,
                                               std::move(caps),
                                               interface,
                                               instanceVersion,
                                               physDevVersion,
                                               std::move(memoryAllocator)));
    if (backendContext.fProtectedContext == GrProtected::kYes &&
        !vkGpu->vkCaps().supportsProtectedContent()) {
        return nullptr;
    }
    return vkGpu;
}

////////////////////////////////////////////////////////////////////////////////

GrVkGpu::GrVkGpu(GrDirectContext* direct,
                 const GrVkBackendContext& backendContext,
                 sk_sp<GrVkCaps> caps,
                 sk_sp<const skgpu::VulkanInterface> interface,
                 uint32_t instanceVersion,
                 uint32_t physicalDeviceVersion,
                 sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator)
        : INHERITED(direct)
        , fInterface(std::move(interface))
        , fMemoryAllocator(std::move(memoryAllocator))
        , fVkCaps(std::move(caps))
        , fPhysicalDevice(backendContext.fPhysicalDevice)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fQueueIndex(backendContext.fGraphicsQueueIndex)
        , fResourceProvider(this)
        , fStagingBufferManager(this)
        , fDisconnected(false)
        , fProtectedContext(backendContext.fProtectedContext)
        , fDeviceLostContext(backendContext.fDeviceLostContext)
        , fDeviceLostProc(backendContext.fDeviceLostProc) {
    SkASSERT(!backendContext.fOwnsInstanceAndDevice);
    SkASSERT(fMemoryAllocator);

    this->initCaps(fVkCaps);

    VK_CALL(GetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &fPhysDevProps));
    VK_CALL(GetPhysicalDeviceMemoryProperties(backendContext.fPhysicalDevice, &fPhysDevMemProps));

    fResourceProvider.init();

    fMainCmdPool = fResourceProvider.findOrCreateCommandPool();
    if (fMainCmdPool) {
        fMainCmdBuffer = fMainCmdPool->getPrimaryCommandBuffer();
        SkASSERT(this->currentCommandBuffer());
        this->currentCommandBuffer()->begin(this);
    }
}

void GrVkGpu::destroyResources() {
    if (fMainCmdPool) {
        fMainCmdPool->getPrimaryCommandBuffer()->end(this, /*abandoningBuffer=*/true);
        fMainCmdPool->close();
    }

    // wait for all commands to finish
    this->finishOutstandingGpuWork();

    if (fMainCmdPool) {
        fMainCmdPool->unref();
        fMainCmdPool = nullptr;
    }

    for (int i = 0; i < fSemaphoresToWaitOn.size(); ++i) {
        fSemaphoresToWaitOn[i]->unref();
    }
    fSemaphoresToWaitOn.clear();

    for (int i = 0; i < fSemaphoresToSignal.size(); ++i) {
        fSemaphoresToSignal[i]->unref();
    }
    fSemaphoresToSignal.clear();

    fStagingBufferManager.reset();

    fMSAALoadManager.destroyResources(this);

    // must call this just before we destroy the command pool and VkDevice
    fResourceProvider.destroyResources();
}

GrVkGpu::~GrVkGpu() {
    if (!fDisconnected) {
        this->destroyResources();
    }
    // We don't delete the memory allocator until the very end of the GrVkGpu lifetime so that
    // clients can continue to delete backend textures even after a context has been abandoned.
    fMemoryAllocator.reset();
}


void GrVkGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);
    if (!fDisconnected) {
        this->destroyResources();

        fSemaphoresToWaitOn.clear();
        fSemaphoresToSignal.clear();
        fMainCmdBuffer = nullptr;
        fDisconnected = true;
    }
}

GrThreadSafePipelineBuilder* GrVkGpu::pipelineBuilder() {
    return fResourceProvider.pipelineStateCache();
}

sk_sp<GrThreadSafePipelineBuilder> GrVkGpu::refPipelineBuilder() {
    return fResourceProvider.refPipelineStateCache();
}

///////////////////////////////////////////////////////////////////////////////

GrOpsRenderPass* GrVkGpu::onGetOpsRenderPass(
        GrRenderTarget* rt,
        bool useMSAASurface,
        GrAttachment* stencil,
        GrSurfaceOrigin origin,
        const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
        const TArray<GrSurfaceProxy*, true>& sampledProxies,
        GrXferBarrierFlags renderPassXferBarriers) {
    if (!fCachedOpsRenderPass) {
        fCachedOpsRenderPass = std::make_unique<GrVkOpsRenderPass>(this);
    }

    // For the given render target and requested render pass features we need to find a compatible
    // framebuffer to use for the render pass. Technically it is the underlying VkRenderPass that
    // is compatible, but that is part of the framebuffer that we get here.
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);

    SkASSERT(!useMSAASurface ||
             rt->numSamples() > 1 ||
             (this->vkCaps().supportsDiscardableMSAAForDMSAA() &&
              vkRT->resolveAttachment() &&
              vkRT->resolveAttachment()->supportsInputAttachmentUsage()));

    // Covert the GrXferBarrierFlags into render pass self dependency flags
    GrVkRenderPass::SelfDependencyFlags selfDepFlags = GrVkRenderPass::SelfDependencyFlags::kNone;
    if (renderPassXferBarriers & GrXferBarrierFlags::kBlend) {
        selfDepFlags |= GrVkRenderPass::SelfDependencyFlags::kForNonCoherentAdvBlend;
    }
    if (renderPassXferBarriers & GrXferBarrierFlags::kTexture) {
        selfDepFlags |= GrVkRenderPass::SelfDependencyFlags::kForInputAttachment;
    }

    // Figure out if we need a resolve attachment for this render pass. A resolve attachment is
    // needed if we are using msaa to draw with a discardable msaa attachment. If we are in this
    // case we also need to update the color load/store ops since we don't want to ever load or
    // store the msaa color attachment, but may need to for the resolve attachment.
    GrOpsRenderPass::LoadAndStoreInfo localColorInfo = colorInfo;
    bool withResolve = false;
    GrVkRenderPass::LoadFromResolve loadFromResolve = GrVkRenderPass::LoadFromResolve::kNo;
    GrOpsRenderPass::LoadAndStoreInfo resolveInfo{GrLoadOp::kLoad, GrStoreOp::kStore, {}};
    if (useMSAASurface && this->vkCaps().renderTargetSupportsDiscardableMSAA(vkRT)) {
        withResolve = true;
        localColorInfo.fStoreOp = GrStoreOp::kDiscard;
        if (colorInfo.fLoadOp == GrLoadOp::kLoad) {
            loadFromResolve = GrVkRenderPass::LoadFromResolve::kLoad;
            localColorInfo.fLoadOp = GrLoadOp::kDiscard;
        } else {
            resolveInfo.fLoadOp = GrLoadOp::kDiscard;
        }
    }

    // Get the framebuffer to use for the render pass
   sk_sp<GrVkFramebuffer> framebuffer;
    if (vkRT->wrapsSecondaryCommandBuffer()) {
        framebuffer = vkRT->externalFramebuffer();
    } else {
        auto fb = vkRT->getFramebuffer(withResolve, SkToBool(stencil), selfDepFlags,
                                       loadFromResolve);
        framebuffer = sk_ref_sp(fb);
    }
    if (!framebuffer) {
        return nullptr;
    }

    if (!fCachedOpsRenderPass->set(rt, std::move(framebuffer), origin, bounds, localColorInfo,
                                   stencilInfo, resolveInfo, selfDepFlags, loadFromResolve,
                                   sampledProxies)) {
        return nullptr;
    }
    return fCachedOpsRenderPass.get();
}

bool GrVkGpu::submitCommandBuffer(SyncQueue sync) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (!this->currentCommandBuffer()) {
        return false;
    }
    SkASSERT(!fCachedOpsRenderPass || !fCachedOpsRenderPass->isActive());

    if (!this->currentCommandBuffer()->hasWork() && kForce_SyncQueue != sync &&
        fSemaphoresToSignal.empty() && fSemaphoresToWaitOn.empty()) {
        // We may have added finished procs during the flush call. Since there is no actual work
        // we are not submitting the command buffer and may never come back around to submit it.
        // Thus we call all current finished procs manually, since the work has technically
        // finished.
        this->currentCommandBuffer()->callFinishedProcs();
        SkASSERT(fDrawables.empty());
        fResourceProvider.checkCommandBuffers();
        return true;
    }

    fMainCmdBuffer->end(this);
    SkASSERT(fMainCmdPool);
    fMainCmdPool->close();
    bool didSubmit = fMainCmdBuffer->submitToQueue(this, fQueue, fSemaphoresToSignal,
                                                   fSemaphoresToWaitOn);

    if (didSubmit && sync == kForce_SyncQueue) {
        fMainCmdBuffer->forceSync(this);
    }

    // We must delete any drawables that had to wait until submit to destroy.
    fDrawables.clear();

    // If we didn't submit the command buffer then we did not wait on any semaphores. We will
    // continue to hold onto these semaphores and wait on them during the next command buffer
    // submission.
    if (didSubmit) {
        for (int i = 0; i < fSemaphoresToWaitOn.size(); ++i) {
            fSemaphoresToWaitOn[i]->unref();
        }
        fSemaphoresToWaitOn.clear();
    }

    // Even if we did not submit the command buffer, we drop all the signal semaphores since we will
    // not try to recover the work that wasn't submitted and instead just drop it all. The client
    // will be notified that the semaphores were not submit so that they will not try to wait on
    // them.
    for (int i = 0; i < fSemaphoresToSignal.size(); ++i) {
        fSemaphoresToSignal[i]->unref();
    }
    fSemaphoresToSignal.clear();

    // Release old command pool and create a new one
    fMainCmdPool->unref();
    fMainCmdPool = fResourceProvider.findOrCreateCommandPool();
    if (fMainCmdPool) {
        fMainCmdBuffer = fMainCmdPool->getPrimaryCommandBuffer();
        SkASSERT(fMainCmdBuffer);
        fMainCmdBuffer->begin(this);
    } else {
        fMainCmdBuffer = nullptr;
    }
    // We must wait to call checkCommandBuffers until after we get a new command buffer. The
    // checkCommandBuffers may trigger a releaseProc which may cause us to insert a barrier for a
    // released GrVkImage. That barrier needs to be put into a new command buffer and not the old
    // one that was just submitted.
    fResourceProvider.checkCommandBuffers();
    return didSubmit;
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<GrGpuBuffer> GrVkGpu::onCreateBuffer(size_t size,
                                           GrGpuBufferType type,
                                           GrAccessPattern accessPattern) {
#ifdef SK_DEBUG
    switch (type) {
        case GrGpuBufferType::kVertex:
        case GrGpuBufferType::kIndex:
        case GrGpuBufferType::kDrawIndirect:
            SkASSERT(accessPattern == kDynamic_GrAccessPattern ||
                     accessPattern == kStatic_GrAccessPattern);
            break;
        case GrGpuBufferType::kXferCpuToGpu:
            SkASSERT(accessPattern == kDynamic_GrAccessPattern);
            break;
        case GrGpuBufferType::kXferGpuToCpu:
            SkASSERT(accessPattern == kDynamic_GrAccessPattern ||
                     accessPattern == kStream_GrAccessPattern);
            break;
        case GrGpuBufferType::kUniform:
            SkASSERT(accessPattern == kDynamic_GrAccessPattern);
            break;
    }
#endif
    return GrVkBuffer::Make(this, size, type, accessPattern);
}

bool GrVkGpu::onWritePixels(GrSurface* surface,
                            SkIRect rect,
                            GrColorType surfaceColorType,
                            GrColorType srcColorType,
                            const GrMipLevel texels[],
                            int mipLevelCount,
                            bool prepForTexSampling) {
    GrVkTexture* texture = static_cast<GrVkTexture*>(surface->asTexture());
    if (!texture) {
        return false;
    }
    GrVkImage* texImage = texture->textureImage();

    // Make sure we have at least the base level
    if (!mipLevelCount || !texels[0].fPixels) {
        return false;
    }

    SkASSERT(!skgpu::VkFormatIsCompressed(texImage->imageFormat()));
    bool success = false;
    bool linearTiling = texImage->isLinearTiled();
    if (linearTiling) {
        if (mipLevelCount > 1) {
            SkDebugf("Can't upload mipmap data to linear tiled texture");
            return false;
        }
        if (VK_IMAGE_LAYOUT_PREINITIALIZED != texImage->currentLayout()) {
            // Need to change the layout to general in order to perform a host write
            texImage->setImageLayout(this,
                                     VK_IMAGE_LAYOUT_GENERAL,
                                     VK_ACCESS_HOST_WRITE_BIT,
                                     VK_PIPELINE_STAGE_HOST_BIT,
                                     false);
            if (!this->submitCommandBuffer(kForce_SyncQueue)) {
                return false;
            }
        }
        success = this->uploadTexDataLinear(texImage,
                                            rect,
                                            srcColorType,
                                            texels[0].fPixels,
                                            texels[0].fRowBytes);
    } else {
        SkASSERT(mipLevelCount <= (int)texImage->mipLevels());
        success = this->uploadTexDataOptimal(texImage,
                                             rect,
                                             srcColorType,
                                             texels,
                                             mipLevelCount);
        if (1 == mipLevelCount) {
            texture->markMipmapsDirty();
        }
    }

    if (prepForTexSampling) {
        texImage->setImageLayout(this,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      false);
    }

    return success;
}

// When we update vertex/index buffers via transfers we assume that they may have been used
// previously in draws and will be used again in draws afterwards. So we put a barrier before and
// after. If we had a mechanism for gathering the buffers that will be used in a GrVkOpsRenderPass
// *before* we begin a subpass we could do this lazily and non-redundantly by tracking the "last
// usage" on the GrVkBuffer. Then Pass 1 draw, xfer, xfer, xfer, Pass 2 draw  would insert just two
// barriers: one before the first xfer and one before Pass 2. Currently, we'd use six barriers.
// Pass false as "after" before the transfer and true after the transfer.
static void add_transfer_dst_buffer_mem_barrier(GrVkGpu* gpu,
                                                GrVkBuffer* dst,
                                                size_t offset,
                                                size_t size,
                                                bool after) {
    if (dst->intendedType() != GrGpuBufferType::kIndex &&
        dst->intendedType() != GrGpuBufferType::kVertex) {
        return;
    }

    VkAccessFlags srcAccessMask = dst->intendedType() == GrGpuBufferType::kIndex
                                          ? VK_ACCESS_INDEX_READ_BIT
                                          : VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkPipelineStageFlagBits srcPipelineStageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    VkPipelineStageFlagBits dstPipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (after) {
        using std::swap;
        swap(srcAccessMask,         dstAccessMask        );
        swap(srcPipelineStageFlags, dstPipelineStageFlags);
    }

    VkBufferMemoryBarrier bufferMemoryBarrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType
            nullptr,                                  // pNext
            srcAccessMask,                            // srcAccessMask
            dstAccessMask,                            // dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
            dst->vkBuffer(),                          // buffer
            offset,                                   // offset
            size,                                     // size
    };

    gpu->addBufferMemoryBarrier(srcPipelineStageFlags,
                                dstPipelineStageFlags,
                                /*byRegion=*/false,
                                &bufferMemoryBarrier);
}

bool GrVkGpu::onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                           size_t srcOffset,
                                           sk_sp<GrGpuBuffer> dst,
                                           size_t dstOffset,
                                           size_t size) {
    if (!this->currentCommandBuffer()) {
        return false;
    }

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    add_transfer_dst_buffer_mem_barrier(this,
                                        static_cast<GrVkBuffer*>(dst.get()),
                                        dstOffset,
                                        size,
                                        /*after=*/false);
    this->currentCommandBuffer()->copyBuffer(this, std::move(src), dst, 1, &copyRegion);
    add_transfer_dst_buffer_mem_barrier(this,
                                        static_cast<GrVkBuffer*>(dst.get()),
                                        dstOffset,
                                        size,
                                        /*after=*/true);

    return true;
}

bool GrVkGpu::onTransferPixelsTo(GrTexture* texture,
                                 SkIRect rect,
                                 GrColorType surfaceColorType,
                                 GrColorType bufferColorType,
                                 sk_sp<GrGpuBuffer> transferBuffer,
                                 size_t bufferOffset,
                                 size_t rowBytes) {
    if (!this->currentCommandBuffer()) {
        return false;
    }

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (GrBackendFormatBytesPerPixel(texture->backendFormat()) != bpp) {
        return false;
    }

    // Vulkan only supports offsets that are both 4-byte aligned and aligned to a pixel.
    if ((bufferOffset & 0x3) || (bufferOffset % bpp)) {
        return false;
    }
    GrVkTexture* tex = static_cast<GrVkTexture*>(texture);
    if (!tex) {
        return false;
    }
    GrVkImage* vkImage = tex->textureImage();
    VkFormat format = vkImage->imageFormat();

    // Can't transfer compressed data
    SkASSERT(!skgpu::VkFormatIsCompressed(format));

    if (!transferBuffer) {
        return false;
    }

    if (bufferColorType != this->vkCaps().transferColorType(format, surfaceColorType)) {
        return false;
    }
    SkASSERT(skgpu::VkFormatBytesPerBlock(format) == GrColorTypeBytesPerPixel(bufferColorType));

    SkASSERT(SkIRect::MakeSize(texture->dimensions()).contains(rect));

    // Set up copy region
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = bufferOffset;
    region.bufferRowLength = (uint32_t)(rowBytes/bpp);
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { rect.left(), rect.top(), 0 };
    region.imageExtent = { (uint32_t)rect.width(), (uint32_t)rect.height(), 1 };

    // Change layout of our target so it can be copied to
    vkImage->setImageLayout(this,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            false);

    const GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(transferBuffer.get());

    // Copy the buffer to the image.
    this->currentCommandBuffer()->copyBufferToImage(this,
                                                    vkBuffer->vkBuffer(),
                                                    vkImage,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                    1,
                                                    &region);
    this->currentCommandBuffer()->addGrBuffer(std::move(transferBuffer));

    tex->markMipmapsDirty();
    return true;
}

bool GrVkGpu::onTransferPixelsFrom(GrSurface* surface,
                                   SkIRect rect,
                                   GrColorType surfaceColorType,
                                   GrColorType bufferColorType,
                                   sk_sp<GrGpuBuffer> transferBuffer,
                                   size_t offset) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    SkASSERT(surface);
    SkASSERT(transferBuffer);
    if (fProtectedContext == GrProtected::kYes) {
        return false;
    }

    GrVkImage* srcImage;
    if (GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(surface->asRenderTarget())) {
        // Reading from render targets that wrap a secondary command buffer is not allowed since
        // it would require us to know the VkImage, which we don't have, as well as need us to
        // stop and start the VkRenderPass which we don't have access to.
        if (rt->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        if (!rt->nonMSAAAttachment()) {
            return false;
        }
        srcImage = rt->nonMSAAAttachment();
    } else {
        SkASSERT(surface->asTexture());
        srcImage = static_cast<GrVkTexture*>(surface->asTexture())->textureImage();
    }

    VkFormat format = srcImage->imageFormat();
    if (bufferColorType != this->vkCaps().transferColorType(format, surfaceColorType)) {
        return false;
    }
    SkASSERT(skgpu::VkFormatBytesPerBlock(format) == GrColorTypeBytesPerPixel(bufferColorType));

    // Set up copy region
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = offset;
    region.bufferRowLength = rect.width();
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = {rect.left(), rect.top(), 0};
    region.imageExtent = {(uint32_t)rect.width(), (uint32_t)rect.height(), 1};

    srcImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    this->currentCommandBuffer()->copyImageToBuffer(this, srcImage,
                                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                    transferBuffer, 1, &region);

    GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(transferBuffer.get());
    // Make sure the copy to buffer has finished.
    vkBuffer->addMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT,
                               VK_ACCESS_HOST_READ_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_HOST_BIT,
                               false);
    return true;
}

void GrVkGpu::resolveImage(GrSurface* dst, GrVkRenderTarget* src, const SkIRect& srcRect,
                           const SkIPoint& dstPoint) {
    if (!this->currentCommandBuffer()) {
        return;
    }

    SkASSERT(dst);
    SkASSERT(src && src->colorAttachment() && src->colorAttachment()->numSamples() > 1);

    VkImageResolve resolveInfo;
    resolveInfo.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveInfo.srcOffset = {srcRect.fLeft, srcRect.fTop, 0};
    resolveInfo.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveInfo.dstOffset = {dstPoint.fX, dstPoint.fY, 0};
    resolveInfo.extent = {(uint32_t)srcRect.width(), (uint32_t)srcRect.height(), 1};

    GrVkImage* dstImage;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    GrTexture* dstTex = dst->asTexture();
    if (dstTex) {
        dstImage = static_cast<GrVkTexture*>(dstTex)->textureImage();
    } else {
        SkASSERT(dst->asRenderTarget());
        dstImage = static_cast<GrVkRenderTarget*>(dstRT)->nonMSAAAttachment();
    }
    SkASSERT(dstImage);

    dstImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    src->colorAttachment()->setImageLayout(this,
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           VK_ACCESS_TRANSFER_READ_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           false);
    this->currentCommandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(src->colorAttachment()));
    this->currentCommandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(dst));
    this->currentCommandBuffer()->resolveImage(this, *src->colorAttachment(), *dstImage, 1,
                                               &resolveInfo);
}

void GrVkGpu::onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) {
    SkASSERT(target->numSamples() > 1);
    GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(target);
    SkASSERT(rt->colorAttachmentView() && rt->resolveAttachmentView());

    if (this->vkCaps().renderTargetSupportsDiscardableMSAA(rt)) {
        // We would have resolved the RT during the render pass;
        return;
    }

    this->resolveImage(target, rt, resolveRect,
                       SkIPoint::Make(resolveRect.x(), resolveRect.y()));
}

bool GrVkGpu::uploadTexDataLinear(GrVkImage* texImage,
                                  SkIRect rect,
                                  GrColorType dataColorType,
                                  const void* data,
                                  size_t rowBytes) {
    SkASSERT(data);
    SkASSERT(texImage->isLinearTiled());

    SkASSERT(SkIRect::MakeSize(texImage->dimensions()).contains(rect));

    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);
    size_t trimRowBytes = rect.width() * bpp;

    SkASSERT(VK_IMAGE_LAYOUT_PREINITIALIZED == texImage->currentLayout() ||
             VK_IMAGE_LAYOUT_GENERAL == texImage->currentLayout());
    const VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,  // mipLevel
        0,  // arraySlice
    };
    VkSubresourceLayout layout;

    const skgpu::VulkanInterface* interface = this->vkInterface();

    GR_VK_CALL(interface, GetImageSubresourceLayout(fDevice,
                                                    texImage->image(),
                                                    &subres,
                                                    &layout));

    const skgpu::VulkanAlloc& alloc = texImage->alloc();
    if (VK_NULL_HANDLE == alloc.fMemory) {
        return false;
    }
    VkDeviceSize offset = rect.top()*layout.rowPitch + rect.left()*bpp;
    VkDeviceSize size = rect.height()*layout.rowPitch;
    SkASSERT(size + offset <= alloc.fSize);
    auto checkResult = [this](VkResult result) {
        return this->checkVkResult(result);
    };
    auto allocator = this->memoryAllocator();
    void* mapPtr = skgpu::VulkanMemory::MapAlloc(allocator, alloc, checkResult);
    if (!mapPtr) {
        return false;
    }
    mapPtr = reinterpret_cast<char*>(mapPtr) + offset;

    SkRectMemcpy(mapPtr,
                 static_cast<size_t>(layout.rowPitch),
                 data,
                 rowBytes,
                 trimRowBytes,
                 rect.height());

    skgpu::VulkanMemory::FlushMappedAlloc(allocator, alloc, offset, size, checkResult);
    skgpu::VulkanMemory::UnmapAlloc(allocator, alloc);

    return true;
}

// This fills in the 'regions' vector in preparation for copying a buffer to an image.
// 'individualMipOffsets' is filled in as a side-effect.
static size_t fill_in_compressed_regions(GrStagingBufferManager* stagingBufferManager,
                                         TArray<VkBufferImageCopy>* regions,
                                         TArray<size_t>* individualMipOffsets,
                                         GrStagingBufferManager::Slice* slice,
                                         SkTextureCompressionType compression,
                                         VkFormat vkFormat,
                                         SkISize dimensions,
                                         skgpu::Mipmapped mipmapped) {
    SkASSERT(compression != SkTextureCompressionType::kNone);
    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    regions->reserve_exact(regions->size() + numMipLevels);
    individualMipOffsets->reserve_exact(individualMipOffsets->size() + numMipLevels);

    size_t bytesPerBlock = skgpu::VkFormatBytesPerBlock(vkFormat);

    size_t bufferSize = SkCompressedDataSize(
            compression, dimensions, individualMipOffsets, mipmapped == skgpu::Mipmapped::kYes);
    SkASSERT(individualMipOffsets->size() == numMipLevels);

    // Get a staging buffer slice to hold our mip data.
    // Vulkan requires offsets in the buffer to be aligned to multiple of the texel size and 4
    size_t alignment = bytesPerBlock;
    switch (alignment & 0b11) {
        case 0:                     break;   // alignment is already a multiple of 4.
        case 2:     alignment *= 2; break;   // alignment is a multiple of 2 but not 4.
        default:    alignment *= 4; break;   // alignment is not a multiple of 2.
    }
    *slice = stagingBufferManager->allocateStagingBufferSlice(bufferSize, alignment);
    if (!slice->fBuffer) {
        return 0;
    }

    for (int i = 0; i < numMipLevels; ++i) {
        VkBufferImageCopy& region = regions->push_back();
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = slice->fOffset + (*individualMipOffsets)[i];
        SkISize revisedDimensions = skgpu::CompressedDimensions(compression, dimensions);
        region.bufferRowLength = revisedDimensions.width();
        region.bufferImageHeight = revisedDimensions.height();
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(i), 0, 1};
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {SkToU32(dimensions.width()),
                              SkToU32(dimensions.height()), 1};

        dimensions = {std::max(1, dimensions.width() /2),
                      std::max(1, dimensions.height()/2)};
    }

    return bufferSize;
}

bool GrVkGpu::uploadTexDataOptimal(GrVkImage* texImage,
                                   SkIRect rect,
                                   GrColorType dataColorType,
                                   const GrMipLevel texels[],
                                   int mipLevelCount) {
    if (!this->currentCommandBuffer()) {
        return false;
    }

    SkASSERT(!texImage->isLinearTiled());
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(mipLevelCount == 1 || rect == SkIRect::MakeSize(texImage->dimensions()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(mipLevelCount == 1 || mipLevelCount == (int)texImage->mipLevels());

    SkASSERT(!rect.isEmpty());

    SkASSERT(this->vkCaps().surfaceSupportsWritePixels(texImage));

    SkASSERT(this->vkCaps().isVkFormatTexturable(texImage->imageFormat()));
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    // texels is const.
    // But we may need to adjust the fPixels ptr based on the copyRect, or fRowBytes.
    // Because of this we need to make a non-const shallow copy of texels.
    AutoTArray<GrMipLevel> texelsShallowCopy(mipLevelCount);
    std::copy_n(texels, mipLevelCount, texelsShallowCopy.get());

    TArray<size_t> individualMipOffsets;
    size_t combinedBufferSize;
    if (mipLevelCount > 1) {
        combinedBufferSize = GrComputeTightCombinedBufferSize(bpp,
                                                              rect.size(),
                                                              &individualMipOffsets,
                                                              mipLevelCount);
    } else {
        SkASSERT(texelsShallowCopy[0].fPixels && texelsShallowCopy[0].fRowBytes);
        combinedBufferSize = rect.width()*rect.height()*bpp;
        individualMipOffsets.push_back(0);
    }
    SkASSERT(combinedBufferSize);

    // Get a staging buffer slice to hold our mip data.
    // Vulkan requires offsets in the buffer to be aligned to multiple of the texel size and 4
    size_t alignment = bpp;
    switch (alignment & 0b11) {
        case 0:                     break;   // alignment is already a multiple of 4.
        case 2:     alignment *= 2; break;   // alignment is a multiple of 2 but not 4.
        default:    alignment *= 4; break;   // alignment is not a multiple of 2.
    }
    GrStagingBufferManager::Slice slice =
            fStagingBufferManager.allocateStagingBufferSlice(combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }

    int uploadLeft = rect.left();
    int uploadTop = rect.top();

    char* buffer = (char*) slice.fOffsetMapPtr;
    TArray<VkBufferImageCopy> regions(mipLevelCount);

    int currentWidth = rect.width();
    int currentHeight = rect.height();
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texelsShallowCopy[currentMipLevel].fPixels) {
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t rowBytes = texelsShallowCopy[currentMipLevel].fRowBytes;

            // copy data into the buffer, skipping the trailing bytes
            char* dst = buffer + individualMipOffsets[currentMipLevel];
            const char* src = (const char*)texelsShallowCopy[currentMipLevel].fPixels;
            SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);

            VkBufferImageCopy& region = regions.push_back();
            memset(&region, 0, sizeof(VkBufferImageCopy));
            region.bufferOffset = slice.fOffset + individualMipOffsets[currentMipLevel];
            region.bufferRowLength = currentWidth;
            region.bufferImageHeight = currentHeight;
            region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(currentMipLevel), 0, 1};
            region.imageOffset = {uploadLeft, uploadTop, 0};
            region.imageExtent = {(uint32_t)currentWidth, (uint32_t)currentHeight, 1};
        }

        currentWidth  = std::max(1,  currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
    }

    // Change layout of our target so it can be copied to
    texImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    // Copy the buffer to the image. This call takes the raw VkBuffer instead of a GrGpuBuffer
    // because we don't need the command buffer to ref the buffer here. The reason being is that
    // the buffer is coming from the staging manager and the staging manager will make sure the
    // command buffer has a ref on the buffer. This avoids having to add and remove a ref for ever
    // upload in the frame.
    GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(slice.fBuffer);
    this->currentCommandBuffer()->copyBufferToImage(this,
                                                    vkBuffer->vkBuffer(),
                                                    texImage,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                    regions.size(),
                                                    regions.begin());
    return true;
}

// It's probably possible to roll this into uploadTexDataOptimal,
// but for now it's easier to maintain as a separate entity.
bool GrVkGpu::uploadTexDataCompressed(GrVkImage* uploadTexture,
                                      SkTextureCompressionType compression,
                                      VkFormat vkFormat,
                                      SkISize dimensions,
                                      skgpu::Mipmapped mipmapped,
                                      const void* data,
                                      size_t dataSize) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    SkASSERT(data);
    SkASSERT(!uploadTexture->isLinearTiled());
    // For now the assumption is that our rect is the entire texture.
    // Compressed textures are read-only so this should be a reasonable assumption.
    SkASSERT(dimensions.fWidth == uploadTexture->width() &&
             dimensions.fHeight == uploadTexture->height());

    if (dimensions.fWidth == 0 || dimensions.fHeight  == 0) {
        return false;
    }

    SkASSERT(uploadTexture->imageFormat() == vkFormat);
    SkASSERT(this->vkCaps().isVkFormatTexturable(vkFormat));


    GrStagingBufferManager::Slice slice;
    TArray<VkBufferImageCopy> regions;
    TArray<size_t> individualMipOffsets;
    SkDEBUGCODE(size_t combinedBufferSize =) fill_in_compressed_regions(&fStagingBufferManager,
                                                                        &regions,
                                                                        &individualMipOffsets,
                                                                        &slice,
                                                                        compression,
                                                                        vkFormat,
                                                                        dimensions,
                                                                        mipmapped);
    if (!slice.fBuffer) {
        return false;
    }
    SkASSERT(dataSize == combinedBufferSize);

    {
        char* buffer = (char*)slice.fOffsetMapPtr;
        memcpy(buffer, data, dataSize);
    }

    // Change layout of our target so it can be copied to
    uploadTexture->setImageLayout(this,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  false);

    // Copy the buffer to the image. This call takes the raw VkBuffer instead of a GrGpuBuffer
    // because we don't need the command buffer to ref the buffer here. The reason being is that
    // the buffer is coming from the staging manager and the staging manager will make sure the
    // command buffer has a ref on the buffer. This avoids having to add and remove a ref for ever
    // upload in the frame.
    GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(slice.fBuffer);
    this->currentCommandBuffer()->copyBufferToImage(this,
                                                    vkBuffer->vkBuffer(),
                                                    uploadTexture,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                    regions.size(),
                                                    regions.begin());

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// TODO: make this take a skgpu::Mipmapped
sk_sp<GrTexture> GrVkGpu::onCreateTexture(SkISize dimensions,
                                          const GrBackendFormat& format,
                                          GrRenderable renderable,
                                          int renderTargetSampleCnt,
                                          skgpu::Budgeted budgeted,
                                          GrProtected isProtected,
                                          int mipLevelCount,
                                          uint32_t levelClearMask,
                                          std::string_view label) {
    VkFormat pixelFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(format, &pixelFormat));
    SkASSERT(!skgpu::VkFormatIsCompressed(pixelFormat));
    SkASSERT(mipLevelCount > 0);

    GrMipmapStatus mipmapStatus =
            mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;

    sk_sp<GrVkTexture> tex;
    if (renderable == GrRenderable::kYes) {
        tex = GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
                this, budgeted, dimensions, pixelFormat, mipLevelCount, renderTargetSampleCnt,
                mipmapStatus, isProtected, label);
    } else {
        tex = GrVkTexture::MakeNewTexture(this, budgeted, dimensions, pixelFormat,
                                          mipLevelCount, isProtected, mipmapStatus, label);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        if (!this->currentCommandBuffer()) {
            return nullptr;
        }
        STArray<1, VkImageSubresourceRange> ranges;
        bool inRange = false;
        GrVkImage* texImage = tex->textureImage();
        for (uint32_t i = 0; i < texImage->mipLevels(); ++i) {
            if (levelClearMask & (1U << i)) {
                if (inRange) {
                    ranges.back().levelCount++;
                } else {
                    auto& range = ranges.push_back();
                    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    range.baseArrayLayer = 0;
                    range.baseMipLevel = i;
                    range.layerCount = 1;
                    range.levelCount = 1;
                    inRange = true;
                }
            } else if (inRange) {
                inRange = false;
            }
        }
        SkASSERT(!ranges.empty());
        static constexpr VkClearColorValue kZeroClearColor = {};
        texImage->setImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);
        this->currentCommandBuffer()->clearColorImage(this, texImage, &kZeroClearColor,
                                                      ranges.size(), ranges.begin());
    }
    return tex;
}

sk_sp<GrTexture> GrVkGpu::onCreateCompressedTexture(SkISize dimensions,
                                                    const GrBackendFormat& format,
                                                    skgpu::Budgeted budgeted,
                                                    skgpu::Mipmapped mipmapped,
                                                    GrProtected isProtected,
                                                    const void* data,
                                                    size_t dataSize) {
    VkFormat pixelFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(format, &pixelFormat));
    SkASSERT(skgpu::VkFormatIsCompressed(pixelFormat));

    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height())+1;
    }

    GrMipmapStatus mipmapStatus = (mipmapped == skgpu::Mipmapped::kYes)
                                          ? GrMipmapStatus::kValid
                                          : GrMipmapStatus::kNotAllocated;

    auto tex = GrVkTexture::MakeNewTexture(this,
                                           budgeted,
                                           dimensions,
                                           pixelFormat,
                                           numMipLevels,
                                           isProtected,
                                           mipmapStatus,
                                           /*label=*/"VkGpu_CreateCompressedTexture");
    if (!tex) {
        return nullptr;
    }

    SkTextureCompressionType compression = GrBackendFormatToCompressionType(format);
    if (!this->uploadTexDataCompressed(tex->textureImage(), compression, pixelFormat,
                                       dimensions, mipmapped, data, dataSize)) {
        return nullptr;
    }

    return tex;
}

////////////////////////////////////////////////////////////////////////////////

bool GrVkGpu::updateBuffer(sk_sp<GrVkBuffer> buffer, const void* src,
                           VkDeviceSize offset, VkDeviceSize size) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    add_transfer_dst_buffer_mem_barrier(this,
                                        static_cast<GrVkBuffer*>(buffer.get()),
                                        offset,
                                        size,
                                        /*after=*/false);
    this->currentCommandBuffer()->updateBuffer(this, buffer, offset, size, src);
    add_transfer_dst_buffer_mem_barrier(this,
                                        static_cast<GrVkBuffer*>(buffer.get()),
                                        offset,
                                        size,
                                        /*after=*/true);

    return true;
}

bool GrVkGpu::zeroBuffer(sk_sp<GrGpuBuffer> buffer) {
    if (!this->currentCommandBuffer()) {
        return false;
    }

    add_transfer_dst_buffer_mem_barrier(this,
                                        static_cast<GrVkBuffer*>(buffer.get()),
                                        /*offset=*/0,
                                        buffer->size(),
                                        /*after=*/false);
    this->currentCommandBuffer()->fillBuffer(this,
                                             buffer,
                                             /*offset=*/0,
                                             buffer->size(),
                                             /*data=*/0);
    add_transfer_dst_buffer_mem_barrier(this,
                                        static_cast<GrVkBuffer*>(buffer.get()),
                                        /*offset=*/0,
                                        buffer->size(),
                                        /*after=*/true);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool check_image_info(const GrVkCaps& caps,
                             const GrVkImageInfo& info,
                             bool needsAllocation,
                             uint32_t graphicsQueueIndex) {
    if (VK_NULL_HANDLE == info.fImage) {
        return false;
    }

    if (VK_NULL_HANDLE == info.fAlloc.fMemory && needsAllocation) {
        return false;
    }

    if (info.fImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && !caps.supportsSwapchain()) {
        return false;
    }

    if (info.fCurrentQueueFamily != VK_QUEUE_FAMILY_IGNORED &&
        info.fCurrentQueueFamily != VK_QUEUE_FAMILY_EXTERNAL &&
        info.fCurrentQueueFamily != VK_QUEUE_FAMILY_FOREIGN_EXT) {
        if (info.fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
            if (info.fCurrentQueueFamily != graphicsQueueIndex) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (info.fYcbcrConversionInfo.isValid()) {
        if (!caps.supportsYcbcrConversion()) {
            return false;
        }
        if (info.fYcbcrConversionInfo.fExternalFormat != 0) {
            return true;
        }
    }

    // We currently require everything to be made with transfer bits set
    if (!SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) ||
        !SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        return false;
    }

    return true;
}

static bool check_tex_image_info(const GrVkCaps& caps, const GrVkImageInfo& info) {
    // We don't support directly importing multisampled textures for sampling from shaders.
    if (info.fSampleCount != 1) {
        return false;
    }

    if (info.fYcbcrConversionInfo.isValid() && info.fYcbcrConversionInfo.fExternalFormat != 0) {
        return true;
    }
    if (info.fImageTiling == VK_IMAGE_TILING_OPTIMAL) {
        if (!caps.isVkFormatTexturable(info.fFormat)) {
            return false;
        }
    } else if (info.fImageTiling == VK_IMAGE_TILING_LINEAR) {
        if (!caps.isVkFormatTexturableLinearly(info.fFormat)) {
            return false;
        }
    } else if (info.fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        if (!caps.supportsDRMFormatModifiers()) {
            return false;
        }
        // To be technically correct we should query the vulkan support for VkFormat and
        // drmFormatModifier pairs to confirm the required feature support is there. However, we
        // currently don't have our caps and format tables set up to do this effeciently. So
        // instead we just rely on the client's passed in VkImageUsageFlags and assume they we set
        // up using valid features (checked below). In practice this should all be safe because
        // currently we are setting all drm format modifier textures to have a
        // GrTextureType::kExternal so we just really need to be able to read these video VkImage in
        // a shader. The video decoder isn't going to give us VkImages that don't support being
        // sampled.
    } else {
        SkUNREACHABLE;
    }

    // We currently require all textures to be made with sample support
    if (!SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)) {
        return false;
    }

    return true;
}

static bool check_rt_image_info(const GrVkCaps& caps, const GrVkImageInfo& info, bool resolveOnly) {
    if (!caps.isFormatRenderable(info.fFormat, info.fSampleCount)) {
        return false;
    }
    if (!resolveOnly && !SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        return false;
    }
    return true;
}

sk_sp<GrTexture> GrVkGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                               GrWrapOwnership ownership,
                                               GrWrapCacheable cacheable,
                                               GrIOType ioType) {
    GrVkImageInfo imageInfo;
    if (!GrBackendTextures::GetVkImageInfo(backendTex, &imageInfo)) {
        return nullptr;
    }

    if (!check_image_info(this->vkCaps(), imageInfo, kAdopt_GrWrapOwnership == ownership,
                          this->queueIndex())) {
        return nullptr;
    }

    if (!check_tex_image_info(this->vkCaps(), imageInfo)) {
        return nullptr;
    }

    if (backendTex.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }

    sk_sp<skgpu::MutableTextureState> mutableState = backendTex.getMutableState();
    SkASSERT(mutableState);
    return GrVkTexture::MakeWrappedTexture(this, backendTex.dimensions(), ownership, cacheable,
                                           ioType, imageInfo, std::move(mutableState));
}

sk_sp<GrTexture> GrVkGpu::onWrapCompressedBackendTexture(const GrBackendTexture& beTex,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    return this->onWrapBackendTexture(beTex, ownership, cacheable, kRead_GrIOType);
}

sk_sp<GrTexture> GrVkGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         int sampleCnt,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    GrVkImageInfo imageInfo;
    if (!GrBackendTextures::GetVkImageInfo(backendTex, &imageInfo)) {
        return nullptr;
    }

    if (!check_image_info(this->vkCaps(), imageInfo, kAdopt_GrWrapOwnership == ownership,
                          this->queueIndex())) {
        return nullptr;
    }

    if (!check_tex_image_info(this->vkCaps(), imageInfo)) {
        return nullptr;
    }
    // If sampleCnt is > 1 we will create an intermediate MSAA VkImage and then resolve into
    // the wrapped VkImage.
    bool resolveOnly = sampleCnt > 1;
    if (!check_rt_image_info(this->vkCaps(), imageInfo, resolveOnly)) {
        return nullptr;
    }

    if (backendTex.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }

    sampleCnt = this->vkCaps().getRenderTargetSampleCount(sampleCnt, imageInfo.fFormat);

    sk_sp<skgpu::MutableTextureState> mutableState = backendTex.getMutableState();
    SkASSERT(mutableState);

    return GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(this, backendTex.dimensions(),
                                                                   sampleCnt, ownership, cacheable,
                                                                   imageInfo,
                                                                   std::move(mutableState));
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    GrVkImageInfo info;
    if (!GrBackendRenderTargets::GetVkImageInfo(backendRT, &info)) {
        return nullptr;
    }

    if (!check_image_info(this->vkCaps(), info, false, this->queueIndex())) {
        return nullptr;
    }

    // We will always render directly to this VkImage.
    static bool kResolveOnly = false;
    if (!check_rt_image_info(this->vkCaps(), info, kResolveOnly)) {
        return nullptr;
    }

    if (backendRT.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }

    sk_sp<skgpu::MutableTextureState> mutableState = backendRT.getMutableState();
    SkASSERT(mutableState);

    sk_sp<GrVkRenderTarget> tgt = GrVkRenderTarget::MakeWrappedRenderTarget(
            this, backendRT.dimensions(), backendRT.sampleCnt(), info, std::move(mutableState));

    // We don't allow the client to supply a premade stencil buffer. We always create one if needed.
    SkASSERT(!backendRT.stencilBits());
    if (tgt) {
        SkASSERT(tgt->canAttemptStencilAttachment(tgt->numSamples() > 1));
    }

    return tgt;
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo) {
    int maxSize = this->caps()->maxTextureSize();
    if (imageInfo.width() > maxSize || imageInfo.height() > maxSize) {
        return nullptr;
    }

    GrBackendFormat backendFormat = GrBackendFormats::MakeVk(vkInfo.fFormat);
    if (!backendFormat.isValid()) {
        return nullptr;
    }
    int sampleCnt = this->vkCaps().getRenderTargetSampleCount(1, vkInfo.fFormat);
    if (!sampleCnt) {
        return nullptr;
    }

    return GrVkRenderTarget::MakeSecondaryCBRenderTarget(this, imageInfo.dimensions(), vkInfo);
}

bool GrVkGpu::loadMSAAFromResolve(GrVkCommandBuffer* commandBuffer,
                                  const GrVkRenderPass& renderPass,
                                  GrAttachment* dst,
                                  GrVkImage* src,
                                  const SkIRect& srcRect) {
    return fMSAALoadManager.loadMSAAFromResolve(this, commandBuffer, renderPass, dst, src, srcRect);
}

bool GrVkGpu::onRegenerateMipMapLevels(GrTexture* tex) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    auto* vkTex = static_cast<GrVkTexture*>(tex)->textureImage();
    // don't do anything for linearly tiled textures (can't have mipmaps)
    if (vkTex->isLinearTiled()) {
        SkDebugf("Trying to create mipmap for linear tiled texture");
        return false;
    }
    SkASSERT(tex->textureType() == GrTextureType::k2D);

    // determine if we can blit to and from this format
    const GrVkCaps& caps = this->vkCaps();
    if (!caps.formatCanBeDstofBlit(vkTex->imageFormat(), false) ||
        !caps.formatCanBeSrcofBlit(vkTex->imageFormat(), false) ||
        !caps.mipmapSupport()) {
        return false;
    }

    int width = tex->width();
    int height = tex->height();
    VkImageBlit blitRegion;
    memset(&blitRegion, 0, sizeof(VkImageBlit));

    // SkMipmap doesn't include the base level in the level count so we have to add 1
    uint32_t levelCount = SkMipmap::ComputeLevelCount(tex->width(), tex->height()) + 1;
    SkASSERT(levelCount == vkTex->mipLevels());

    // change layout of the layers so we can write to them.
    vkTex->setImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, false);

    // setup memory barrier
    SkASSERT(GrVkFormatIsSupported(vkTex->imageFormat()));
    VkImageMemoryBarrier imageMemoryBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_TRANSFER_WRITE_BIT,            // srcAccessMask
            VK_ACCESS_TRANSFER_READ_BIT,             // dstAccessMask
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // oldLayout
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,    // newLayout
            VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
            vkTex->image(),                          // image
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}  // subresourceRange
    };

    // Blit the miplevels
    uint32_t mipLevel = 1;
    while (mipLevel < levelCount) {
        int prevWidth = width;
        int prevHeight = height;
        width = std::max(1, width / 2);
        height = std::max(1, height / 2);

        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel - 1;
        this->addImageMemoryBarrier(vkTex->resource(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, false, &imageMemoryBarrier);

        blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel - 1, 0, 1 };
        blitRegion.srcOffsets[0] = { 0, 0, 0 };
        blitRegion.srcOffsets[1] = { prevWidth, prevHeight, 1 };
        blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, 0, 1 };
        blitRegion.dstOffsets[0] = { 0, 0, 0 };
        blitRegion.dstOffsets[1] = { width, height, 1 };
        this->currentCommandBuffer()->blitImage(this,
                                                vkTex->resource(),
                                                vkTex->image(),
                                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                vkTex->resource(),
                                                vkTex->image(),
                                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                1,
                                                &blitRegion,
                                                VK_FILTER_LINEAR);
        ++mipLevel;
    }
    if (levelCount > 1) {
        // This barrier logically is not needed, but it changes the final level to the same layout
        // as all the others, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. This makes tracking of the
        // layouts and future layout changes easier. The alternative here would be to track layout
        // and memory accesses per layer which doesn't seem work it.
        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel - 1;
        this->addImageMemoryBarrier(vkTex->resource(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, false, &imageMemoryBarrier);
        vkTex->updateImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<GrAttachment> GrVkGpu::makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                                   SkISize dimensions, int numStencilSamples) {
    VkFormat sFmt = this->vkCaps().preferredStencilFormat();

    fStats.incStencilAttachmentCreates();
    return GrVkImage::MakeStencil(this, dimensions, numStencilSamples, sFmt);
}

sk_sp<GrAttachment> GrVkGpu::makeMSAAAttachment(SkISize dimensions,
                                                const GrBackendFormat& format,
                                                int numSamples,
                                                GrProtected isProtected,
                                                GrMemoryless memoryless) {
    VkFormat pixelFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(format, &pixelFormat));
    SkASSERT(!skgpu::VkFormatIsCompressed(pixelFormat));
    SkASSERT(this->vkCaps().isFormatRenderable(pixelFormat, numSamples));

    fStats.incMSAAAttachmentCreates();
    return GrVkImage::MakeMSAA(this, dimensions, numSamples, pixelFormat, isProtected, memoryless);
}

////////////////////////////////////////////////////////////////////////////////

bool copy_src_data(char* mapPtr,
                   VkFormat vkFormat,
                   const TArray<size_t>& individualMipOffsets,
                   const GrPixmap srcData[],
                   int numMipLevels) {
    SkASSERT(srcData && numMipLevels);
    SkASSERT(!skgpu::VkFormatIsCompressed(vkFormat));
    SkASSERT(individualMipOffsets.size() == numMipLevels);
    SkASSERT(mapPtr);

    size_t bytesPerPixel = skgpu::VkFormatBytesPerBlock(vkFormat);

    for (int level = 0; level < numMipLevels; ++level) {
        const size_t trimRB = srcData[level].info().width() * bytesPerPixel;

        SkRectMemcpy(mapPtr + individualMipOffsets[level], trimRB,
                     srcData[level].addr(), srcData[level].rowBytes(),
                     trimRB, srcData[level].height());
    }
    return true;
}

bool GrVkGpu::createVkImageForBackendSurface(VkFormat vkFormat,
                                             SkISize dimensions,
                                             int sampleCnt,
                                             GrTexturable texturable,
                                             GrRenderable renderable,
                                             skgpu::Mipmapped mipmapped,
                                             GrVkImageInfo* info,
                                             GrProtected isProtected) {
    SkASSERT(texturable == GrTexturable::kYes || renderable == GrRenderable::kYes);

    if (fProtectedContext != isProtected) {
        return false;
    }

    if (texturable == GrTexturable::kYes && !fVkCaps->isVkFormatTexturable(vkFormat)) {
        return false;
    }

    // MSAA images are only currently used by createTestingOnlyBackendRenderTarget.
    if (sampleCnt > 1 && (texturable == GrTexturable::kYes || renderable == GrRenderable::kNo)) {
        return false;
    }

    if (renderable == GrRenderable::kYes) {
        sampleCnt = fVkCaps->getRenderTargetSampleCount(sampleCnt, vkFormat);
        if (!sampleCnt) {
            return false;
        }
    }


    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texturable == GrTexturable::kYes) {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (renderable == GrRenderable::kYes) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // We always make our render targets support being used as input attachments
        usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = vkFormat;
    imageDesc.fWidth = dimensions.width();
    imageDesc.fHeight = dimensions.height();
    imageDesc.fLevels = numMipLevels;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    imageDesc.fIsProtected = fProtectedContext;

    if (!GrVkImage::InitImageInfo(this, imageDesc, info)) {
        SkDebugf("Failed to init image info\n");
        return false;
    }

    return true;
}

bool GrVkGpu::onClearBackendTexture(const GrBackendTexture& backendTexture,
                                    sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                    std::array<float, 4> color) {
    GrVkImageInfo info;
    SkAssertResult(GrBackendTextures::GetVkImageInfo(backendTexture, &info));

    sk_sp<skgpu::MutableTextureState> mutableState = backendTexture.getMutableState();
    SkASSERT(mutableState);
    sk_sp<GrVkTexture> texture =
                GrVkTexture::MakeWrappedTexture(this, backendTexture.dimensions(),
                                                kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                                                kRW_GrIOType, info, std::move(mutableState));
    if (!texture) {
        return false;
    }
    GrVkImage* texImage = texture->textureImage();

    GrVkPrimaryCommandBuffer* cmdBuffer = this->currentCommandBuffer();
    if (!cmdBuffer) {
        return false;
    }

    texImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    // CmdClearColorImage doesn't work for compressed formats
    SkASSERT(!skgpu::VkFormatIsCompressed(info.fFormat));

    VkClearColorValue vkColor;
    // If we ever support SINT or UINT formats this needs to be updated to use the int32 and
    // uint32 union members in those cases.
    vkColor.float32[0] = color[0];
    vkColor.float32[1] = color[1];
    vkColor.float32[2] = color[2];
    vkColor.float32[3] = color[3];
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseArrayLayer = 0;
    range.baseMipLevel = 0;
    range.layerCount = 1;
    range.levelCount = info.fLevelCount;
    cmdBuffer->clearColorImage(this, texImage, &vkColor, 1, &range);

    // Change image layout to shader read since if we use this texture as a borrowed
    // texture within Ganesh we require that its layout be set to that
    texImage->setImageLayout(this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                  false);

    if (finishedCallback) {
        this->addFinishedCallback(std::move(finishedCallback));
    }
    return true;
}

GrBackendTexture GrVkGpu::onCreateBackendTexture(SkISize dimensions,
                                                 const GrBackendFormat& format,
                                                 GrRenderable renderable,
                                                 skgpu::Mipmapped mipmapped,
                                                 GrProtected isProtected,
                                                 std::string_view label) {
    const GrVkCaps& caps = this->vkCaps();

    if (fProtectedContext != isProtected) {
        return {};
    }

    VkFormat vkFormat;
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
        return {};
    }

    // TODO: move the texturability check up to GrGpu::createBackendTexture and just assert here
    if (!caps.isVkFormatTexturable(vkFormat)) {
        return {};
    }

    if (skgpu::VkFormatNeedsYcbcrSampler(vkFormat)) {
        return {};
    }

    GrVkImageInfo info;
    if (!this->createVkImageForBackendSurface(vkFormat, dimensions, 1, GrTexturable::kYes,
                                              renderable, mipmapped, &info, isProtected)) {
        return {};
    }

    return GrBackendTextures::MakeVk(dimensions.width(), dimensions.height(), info);
}

GrBackendTexture GrVkGpu::onCreateCompressedBackendTexture(SkISize dimensions,
                                                           const GrBackendFormat& format,
                                                           skgpu::Mipmapped mipmapped,
                                                           GrProtected isProtected) {
    return this->onCreateBackendTexture(dimensions,
                                        format,
                                        GrRenderable::kNo,
                                        mipmapped,
                                        isProtected,
                                        /*label=*/"VkGpu_CreateCompressedBackendTexture");
}

bool GrVkGpu::onUpdateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                               const void* data,
                                               size_t size) {
    GrVkImageInfo info;
    SkAssertResult(GrBackendTextures::GetVkImageInfo(backendTexture, &info));

    sk_sp<skgpu::MutableTextureState> mutableState = backendTexture.getMutableState();
    SkASSERT(mutableState);
    sk_sp<GrVkTexture> texture = GrVkTexture::MakeWrappedTexture(this,
                                                                 backendTexture.dimensions(),
                                                                 kBorrow_GrWrapOwnership,
                                                                 GrWrapCacheable::kNo,
                                                                 kRW_GrIOType,
                                                                 info,
                                                                 std::move(mutableState));
    if (!texture) {
        return false;
    }

    GrVkPrimaryCommandBuffer* cmdBuffer = this->currentCommandBuffer();
    if (!cmdBuffer) {
        return false;
    }
    GrVkImage* image = texture->textureImage();
    image->setImageLayout(this,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          false);

    SkTextureCompressionType compression =
            GrBackendFormatToCompressionType(backendTexture.getBackendFormat());

    TArray<VkBufferImageCopy> regions;
    TArray<size_t> individualMipOffsets;
    GrStagingBufferManager::Slice slice;

    fill_in_compressed_regions(&fStagingBufferManager,
                               &regions,
                               &individualMipOffsets,
                               &slice,
                               compression,
                               info.fFormat,
                               backendTexture.dimensions(),
                               backendTexture.fMipmapped);

    if (!slice.fBuffer) {
        return false;
    }

    memcpy(slice.fOffsetMapPtr, data, size);

    cmdBuffer->addGrSurface(texture);
    // Copy the buffer to the image. This call takes the raw VkBuffer instead of a GrGpuBuffer
    // because we don't need the command buffer to ref the buffer here. The reason being is that
    // the buffer is coming from the staging manager and the staging manager will make sure the
    // command buffer has a ref on the buffer. This avoids having to add and remove a ref for
    // every upload in the frame.
    cmdBuffer->copyBufferToImage(this,
                                 static_cast<GrVkBuffer*>(slice.fBuffer)->vkBuffer(),
                                 image,
                                 image->currentLayout(),
                                 regions.size(),
                                 regions.begin());

    // Change image layout to shader read since if we use this texture as a borrowed
    // texture within Ganesh we require that its layout be set to that
    image->setImageLayout(this,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_READ_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          false);

    if (finishedCallback) {
        this->addFinishedCallback(std::move(finishedCallback));
    }
    return true;
}

void set_layout_and_queue_from_mutable_state(GrVkGpu* gpu, GrVkImage* image,
                                             VkImageLayout newLayout,
                                             uint32_t newQueueFamilyIndex) {
    // Even though internally we use this helper for getting src access flags and stages they
    // can also be used for general dst flags since we don't know exactly what the client
    // plans on using the image for.
    if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        newLayout = image->currentLayout();
    }
    VkPipelineStageFlags dstStage = GrVkImage::LayoutToPipelineSrcStageFlags(newLayout);
    VkAccessFlags dstAccess = GrVkImage::LayoutToSrcAccessMask(newLayout);

    uint32_t currentQueueFamilyIndex = image->currentQueueFamilyIndex();
    auto isSpecialQueue = [](uint32_t queueFamilyIndex) {
        return queueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
               queueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT;
    };
    if (isSpecialQueue(currentQueueFamilyIndex) && isSpecialQueue(newQueueFamilyIndex)) {
        // It is illegal to have both the new and old queue be special queue families (i.e. external
        // or foreign).
        return;
    }

    image->setImageLayoutAndQueueIndex(gpu, newLayout, dstAccess, dstStage, false,
                                       newQueueFamilyIndex);
}

bool GrVkGpu::setBackendSurfaceState(GrVkImageInfo info,
                                     sk_sp<skgpu::MutableTextureState> currentState,
                                     SkISize dimensions,
                                     VkImageLayout newLayout,
                                     uint32_t newQueueFamilyIndex,
                                     skgpu::MutableTextureState* previousState,
                                     sk_sp<skgpu::RefCntedCallback> finishedCallback) {
    sk_sp<GrVkImage> texture = GrVkImage::MakeWrapped(this,
                                                      dimensions,
                                                      info,
                                                      std::move(currentState),
                                                      GrVkImage::UsageFlags::kColorAttachment,
                                                      kBorrow_GrWrapOwnership,
                                                      GrWrapCacheable::kNo,
                                                      "VkGpu_SetBackendSurfaceState",
                                                      /*forSecondaryCB=*/false);
    SkASSERT(texture);
    if (!texture) {
        return false;
    }
    if (previousState) {
        previousState->set(*texture->getMutableState());
    }
    set_layout_and_queue_from_mutable_state(this, texture.get(), newLayout, newQueueFamilyIndex);
    if (finishedCallback) {
        this->addFinishedCallback(std::move(finishedCallback));
    }
    return true;
}

bool GrVkGpu::setBackendTextureState(const GrBackendTexture& backendTeture,
                                     const skgpu::MutableTextureState& newState,
                                     skgpu::MutableTextureState* previousState,
                                     sk_sp<skgpu::RefCntedCallback> finishedCallback) {
    GrVkImageInfo info;
    SkAssertResult(GrBackendTextures::GetVkImageInfo(backendTeture, &info));
    sk_sp<skgpu::MutableTextureState> currentState = backendTeture.getMutableState();
    SkASSERT(currentState);
    SkASSERT(newState.isValid() && newState.backend() == skgpu::BackendApi::kVulkan);
    return this->setBackendSurfaceState(info, std::move(currentState), backendTeture.dimensions(),
                                        skgpu::MutableTextureStates::GetVkImageLayout(newState),
                                        skgpu::MutableTextureStates::GetVkQueueFamilyIndex(newState),
                                        previousState,
                                        std::move(finishedCallback));
}

bool GrVkGpu::setBackendRenderTargetState(const GrBackendRenderTarget& backendRenderTarget,
                                          const skgpu::MutableTextureState& newState,
                                          skgpu::MutableTextureState* previousState,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback) {
    GrVkImageInfo info;
    SkAssertResult(GrBackendRenderTargets::GetVkImageInfo(backendRenderTarget, &info));
    sk_sp<skgpu::MutableTextureState> currentState = backendRenderTarget.getMutableState();
    SkASSERT(currentState);
    SkASSERT(newState.backend() == skgpu::BackendApi::kVulkan);
    return this->setBackendSurfaceState(info, std::move(currentState),
                                        backendRenderTarget.dimensions(),
                                        skgpu::MutableTextureStates::GetVkImageLayout(newState),
                                        skgpu::MutableTextureStates::GetVkQueueFamilyIndex(newState),
                                        previousState, std::move(finishedCallback));
}

void GrVkGpu::xferBarrier(GrRenderTarget* rt, GrXferBarrierType barrierType) {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
    VkPipelineStageFlags dstStage;
    VkAccessFlags dstAccess;
    if (barrierType == kBlend_GrXferBarrierType) {
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
    } else {
        SkASSERT(barrierType == kTexture_GrXferBarrierType);
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }
    GrVkImage* image = vkRT->colorAttachment();
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = dstAccess;
    barrier.oldLayout = image->currentLayout();
    barrier.newLayout = barrier.oldLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image->image();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, image->mipLevels(), 0, 1};
    this->addImageMemoryBarrier(image->resource(),
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                dstStage, true, &barrier);
}

void GrVkGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kVulkan == tex.fBackend);

    GrVkImageInfo info;
    if (GrBackendTextures::GetVkImageInfo(tex, &info)) {
        GrVkImage::DestroyImageInfo(this, const_cast<GrVkImageInfo*>(&info));
    }
}

bool GrVkGpu::compile(const GrProgramDesc& desc, const GrProgramInfo& programInfo) {
    GrVkRenderPass::AttachmentsDescriptor attachmentsDescriptor;
    GrVkRenderPass::AttachmentFlags attachmentFlags;
    GrVkRenderTarget::ReconstructAttachmentsDescriptor(this->vkCaps(), programInfo,
                                                       &attachmentsDescriptor, &attachmentFlags);

    GrVkRenderPass::SelfDependencyFlags selfDepFlags = GrVkRenderPass::SelfDependencyFlags::kNone;
    if (programInfo.renderPassBarriers() & GrXferBarrierFlags::kBlend) {
        selfDepFlags |= GrVkRenderPass::SelfDependencyFlags::kForNonCoherentAdvBlend;
    }
    if (programInfo.renderPassBarriers() & GrXferBarrierFlags::kTexture) {
        selfDepFlags |= GrVkRenderPass::SelfDependencyFlags::kForInputAttachment;
    }

    GrVkRenderPass::LoadFromResolve loadFromResolve = GrVkRenderPass::LoadFromResolve::kNo;
    if (this->vkCaps().programInfoWillUseDiscardableMSAA(programInfo) &&
        programInfo.colorLoadOp() == GrLoadOp::kLoad) {
        loadFromResolve = GrVkRenderPass::LoadFromResolve::kLoad;
    }
    sk_sp<const GrVkRenderPass> renderPass(this->resourceProvider().findCompatibleRenderPass(
            &attachmentsDescriptor, attachmentFlags, selfDepFlags, loadFromResolve));
    if (!renderPass) {
        return false;
    }

    GrThreadSafePipelineBuilder::Stats::ProgramCacheResult stat;

    auto pipelineState = this->resourceProvider().findOrCreateCompatiblePipelineState(
                                    desc,
                                    programInfo,
                                    renderPass->vkRenderPass(),
                                    &stat);
    if (!pipelineState) {
        return false;
    }

    return stat != GrThreadSafePipelineBuilder::Stats::ProgramCacheResult::kHit;
}

#if defined(GR_TEST_UTILS)
bool GrVkGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kVulkan == tex.fBackend);

    GrVkImageInfo backend;
    if (!GrBackendTextures::GetVkImageInfo(tex, &backend)) {
        return false;
    }

    if (backend.fImage && backend.fAlloc.fMemory) {
        VkMemoryRequirements req;
        memset(&req, 0, sizeof(req));
        GR_VK_CALL(this->vkInterface(), GetImageMemoryRequirements(fDevice,
                                                                   backend.fImage,
                                                                   &req));
        // TODO: find a better check
        // This will probably fail with a different driver
        return (req.size > 0) && (req.size <= 8192 * 8192);
    }

    return false;
}

GrBackendRenderTarget GrVkGpu::createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                                    GrColorType ct,
                                                                    int sampleCnt,
                                                                    GrProtected isProtected) {
    if (dimensions.width()  > this->caps()->maxRenderTargetSize() ||
        dimensions.height() > this->caps()->maxRenderTargetSize()) {
        return {};
    }

    VkFormat vkFormat = this->vkCaps().getFormatFromColorType(ct);

    GrVkImageInfo info;
    if (!this->createVkImageForBackendSurface(vkFormat,
                                              dimensions,
                                              sampleCnt,
                                              GrTexturable::kNo,
                                              GrRenderable::kYes,
                                              skgpu::Mipmapped::kNo,
                                              &info,
                                              isProtected)) {
        return {};
    }
    return GrBackendRenderTargets::MakeVk(dimensions.width(), dimensions.height(), info);
}

void GrVkGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kVulkan == rt.fBackend);

    GrVkImageInfo info;
    if (GrBackendRenderTargets::GetVkImageInfo(rt, &info)) {
        // something in the command buffer may still be using this, so force submit
        SkAssertResult(this->submitCommandBuffer(kForce_SyncQueue));
        GrVkImage::DestroyImageInfo(this, const_cast<GrVkImageInfo*>(&info));
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////

void GrVkGpu::addBufferMemoryBarrier(const GrManagedResource* resource,
                                     VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     VkBufferMemoryBarrier* barrier) const {
    if (!this->currentCommandBuffer()) {
        return;
    }
    SkASSERT(resource);
    this->currentCommandBuffer()->pipelineBarrier(this,
                                                  resource,
                                                  srcStageMask,
                                                  dstStageMask,
                                                  byRegion,
                                                  GrVkCommandBuffer::kBufferMemory_BarrierType,
                                                  barrier);
}
void GrVkGpu::addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     VkBufferMemoryBarrier* barrier) const {
    if (!this->currentCommandBuffer()) {
        return;
    }
    // We don't pass in a resource here to the command buffer. The command buffer only is using it
    // to hold a ref, but every place where we add a buffer memory barrier we are doing some other
    // command with the buffer on the command buffer. Thus those other commands will already cause
    // the command buffer to be holding a ref to the buffer.
    this->currentCommandBuffer()->pipelineBarrier(this,
                                                  /*resource=*/nullptr,
                                                  srcStageMask,
                                                  dstStageMask,
                                                  byRegion,
                                                  GrVkCommandBuffer::kBufferMemory_BarrierType,
                                                  barrier);
}

void GrVkGpu::addImageMemoryBarrier(const GrManagedResource* resource,
                                    VkPipelineStageFlags srcStageMask,
                                    VkPipelineStageFlags dstStageMask,
                                    bool byRegion,
                                    VkImageMemoryBarrier* barrier) const {
    // If we are in the middle of destroying or abandoning the context we may hit a release proc
    // that triggers the destruction of a GrVkImage. This could cause us to try and transfer the
    // VkImage back to the original queue. In this state we don't submit anymore work and we may not
    // have a current command buffer. Thus we won't do the queue transfer.
    if (!this->currentCommandBuffer()) {
        return;
    }
    SkASSERT(resource);
    this->currentCommandBuffer()->pipelineBarrier(this,
                                                  resource,
                                                  srcStageMask,
                                                  dstStageMask,
                                                  byRegion,
                                                  GrVkCommandBuffer::kImageMemory_BarrierType,
                                                  barrier);
}

void GrVkGpu::prepareSurfacesForBackendAccessAndStateUpdates(
        SkSpan<GrSurfaceProxy*> proxies,
        SkSurfaces::BackendSurfaceAccess access,
        const skgpu::MutableTextureState* newState) {
    // Submit the current command buffer to the Queue. Whether we inserted semaphores or not does
    // not effect what we do here.
    if (!proxies.empty() && (access == SkSurfaces::BackendSurfaceAccess::kPresent || newState)) {
        // We currently don't support passing in new surface state for multiple proxies here. The
        // only time we have multiple proxies is if we are flushing a yuv SkImage which won't have
        // state updates anyways. Additionally if we have a newState than we must not have any
        // BackendSurfaceAccess.
        SkASSERT(!newState || proxies.size() == 1);
        SkASSERT(!newState || access == SkSurfaces::BackendSurfaceAccess::kNoAccess);
        GrVkImage* image;
        for (GrSurfaceProxy* proxy : proxies) {
            SkASSERT(proxy->isInstantiated());
            if (GrTexture* tex = proxy->peekTexture()) {
                image = static_cast<GrVkTexture*>(tex)->textureImage();
            } else {
                GrRenderTarget* rt = proxy->peekRenderTarget();
                SkASSERT(rt);
                GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
                image = vkRT->externalAttachment();
            }
            if (newState) {
                VkImageLayout newLayout =
                    skgpu::MutableTextureStates::GetVkImageLayout(newState);
                uint32_t newIndex =
                    skgpu::MutableTextureStates::GetVkQueueFamilyIndex(newState);
                set_layout_and_queue_from_mutable_state(this, image, newLayout, newIndex);
            } else {
                SkASSERT(access == SkSurfaces::BackendSurfaceAccess::kPresent);
                image->prepareForPresent(this);
            }
        }
    }
}

void GrVkGpu::addFinishedProc(GrGpuFinishedProc finishedProc,
                              GrGpuFinishedContext finishedContext) {
    SkASSERT(finishedProc);
    this->addFinishedCallback(skgpu::RefCntedCallback::Make(finishedProc, finishedContext));
}

void GrVkGpu::addFinishedCallback(sk_sp<skgpu::RefCntedCallback> finishedCallback) {
    SkASSERT(finishedCallback);
    fResourceProvider.addFinishedProcToActiveCommandBuffers(std::move(finishedCallback));
}

void GrVkGpu::takeOwnershipOfBuffer(sk_sp<GrGpuBuffer> buffer) {
    this->currentCommandBuffer()->addGrBuffer(std::move(buffer));
}

bool GrVkGpu::onSubmitToGpu(GrSyncCpu sync) {
    if (sync == GrSyncCpu::kYes) {
        return this->submitCommandBuffer(kForce_SyncQueue);
    } else {
        return this->submitCommandBuffer(kSkip_SyncQueue);
    }
}

void GrVkGpu::finishOutstandingGpuWork() {
    VK_CALL(QueueWaitIdle(fQueue));

    if (this->vkCaps().mustSyncCommandBuffersWithQueue()) {
        fResourceProvider.forceSyncAllCommandBuffers();
    }
}

void GrVkGpu::onReportSubmitHistograms() {
#if SK_HISTOGRAMS_ENABLED
    uint64_t allocatedMemory = 0, usedMemory = 0;
    std::tie(allocatedMemory, usedMemory) = fMemoryAllocator->totalAllocatedAndUsedMemory();
    SkASSERT(usedMemory <= allocatedMemory);
    if (allocatedMemory > 0) {
        SK_HISTOGRAM_PERCENTAGE("VulkanMemoryAllocator.PercentUsed",
                                (usedMemory * 100) / allocatedMemory);
    }
    // allocatedMemory is in bytes and need to be reported it in kilobytes. SK_HISTOGRAM_MEMORY_KB
    // supports samples up to around 500MB which should support the amounts of memory we allocate.
    SK_HISTOGRAM_MEMORY_KB("VulkanMemoryAllocator.AmountAllocated", allocatedMemory >> 10);
#endif  // SK_HISTOGRAMS_ENABLED
}

void GrVkGpu::copySurfaceAsCopyImage(GrSurface* dst,
                                     GrSurface* src,
                                     GrVkImage* dstImage,
                                     GrVkImage* srcImage,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    if (!this->currentCommandBuffer()) {
        return;
    }

#ifdef SK_DEBUG
    int dstSampleCnt = dstImage->numSamples();
    int srcSampleCnt = srcImage->numSamples();
    bool dstHasYcbcr = dstImage->ycbcrConversionInfo().isValid();
    bool srcHasYcbcr = srcImage->ycbcrConversionInfo().isValid();
    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(dst->backendFormat(), &srcFormat));
    SkASSERT(this->vkCaps().canCopyImage(dstFormat, dstSampleCnt, dstHasYcbcr,
                                         srcFormat, srcSampleCnt, srcHasYcbcr));
#endif
    if (src->isProtected() && !dst->isProtected()) {
        SkDebugf("Can't copy from protected memory to non-protected");
        return;
    }

    // These flags are for flushing/invalidating caches and for the dst image it doesn't matter if
    // the cache is flushed since it is only being written to.
    dstImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    srcImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    VkImageCopy copyRegion;
    memset(&copyRegion, 0, sizeof(VkImageCopy));
    copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.srcOffset = { srcRect.fLeft, srcRect.fTop, 0 };
    copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.dstOffset = { dstPoint.fX, dstPoint.fY, 0 };
    copyRegion.extent = { (uint32_t)srcRect.width(), (uint32_t)srcRect.height(), 1 };

    this->currentCommandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(src));
    this->currentCommandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(dst));
    this->currentCommandBuffer()->copyImage(this,
                                            srcImage,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            dstImage,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                            1,
                                            &copyRegion);

    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
}

void GrVkGpu::copySurfaceAsBlit(GrSurface* dst,
                                GrSurface* src,
                                GrVkImage* dstImage,
                                GrVkImage* srcImage,
                                const SkIRect& srcRect,
                                const SkIRect& dstRect,
                                GrSamplerState::Filter filter) {
    if (!this->currentCommandBuffer()) {
        return;
    }

#ifdef SK_DEBUG
    int dstSampleCnt = dstImage->numSamples();
    int srcSampleCnt = srcImage->numSamples();
    bool dstHasYcbcr = dstImage->ycbcrConversionInfo().isValid();
    bool srcHasYcbcr = srcImage->ycbcrConversionInfo().isValid();
    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(dst->backendFormat(), &srcFormat));
    SkASSERT(this->vkCaps().canCopyAsBlit(dstFormat,
                                          dstSampleCnt,
                                          dstImage->isLinearTiled(),
                                          dstHasYcbcr,
                                          srcFormat,
                                          srcSampleCnt,
                                          srcImage->isLinearTiled(),
                                          srcHasYcbcr));

#endif
    if (src->isProtected() && !dst->isProtected()) {
        SkDebugf("Can't copy from protected memory to non-protected");
        return;
    }

    dstImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    srcImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    VkImageBlit blitRegion;
    memset(&blitRegion, 0, sizeof(VkImageBlit));
    blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    blitRegion.srcOffsets[0] = { srcRect.fLeft, srcRect.fTop, 0 };
    blitRegion.srcOffsets[1] = { srcRect.fRight, srcRect.fBottom, 1 };
    blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    blitRegion.dstOffsets[0] = { dstRect.fLeft, dstRect.fTop, 0 };
    blitRegion.dstOffsets[1] = { dstRect.fRight, dstRect.fBottom, 1 };

    this->currentCommandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(src));
    this->currentCommandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(dst));
    this->currentCommandBuffer()->blitImage(this,
                                            *srcImage,
                                            *dstImage,
                                            1,
                                            &blitRegion,
                                            filter == GrSamplerState::Filter::kNearest ?
                                                    VK_FILTER_NEAREST : VK_FILTER_LINEAR);

    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
}

void GrVkGpu::copySurfaceAsResolve(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    if (src->isProtected() && !dst->isProtected()) {
        SkDebugf("Can't copy from protected memory to non-protected");
        return;
    }
    GrVkRenderTarget* srcRT = static_cast<GrVkRenderTarget*>(src->asRenderTarget());
    this->resolveImage(dst, srcRT, srcRect, dstPoint);
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
}

bool GrVkGpu::onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                            GrSurface* src, const SkIRect& srcRect,
                            GrSamplerState::Filter filter) {
#ifdef SK_DEBUG
    if (GrVkRenderTarget* srcRT = static_cast<GrVkRenderTarget*>(src->asRenderTarget())) {
        SkASSERT(!srcRT->wrapsSecondaryCommandBuffer());
    }
    if (GrVkRenderTarget* dstRT = static_cast<GrVkRenderTarget*>(dst->asRenderTarget())) {
        SkASSERT(!dstRT->wrapsSecondaryCommandBuffer());
    }
#endif
    if (src->isProtected() && !dst->isProtected()) {
        SkDebugf("Can't copy from protected memory to non-protected");
        return false;
    }

    GrVkImage* dstImage;
    GrVkImage* srcImage;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(dstRT);
        if (vkRT->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        // This will technically return true for single sample rts that used DMSAA in which case we
        // don't have to pick the resolve attachment. But in that case the resolve and color
        // attachments will be the same anyways.
        if (this->vkCaps().renderTargetSupportsDiscardableMSAA(vkRT)) {
            dstImage = vkRT->resolveAttachment();
        } else {
            dstImage = vkRT->colorAttachment();
        }
    } else if (dst->asTexture()) {
        dstImage = static_cast<GrVkTexture*>(dst->asTexture())->textureImage();
    } else {
        // The surface in a GrAttachment already
        dstImage = static_cast<GrVkImage*>(dst);
    }
    GrRenderTarget* srcRT = src->asRenderTarget();
    if (srcRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(srcRT);
        // This will technically return true for single sample rts that used DMSAA in which case we
        // don't have to pick the resolve attachment. But in that case the resolve and color
        // attachments will be the same anyways.
        if (this->vkCaps().renderTargetSupportsDiscardableMSAA(vkRT)) {
            srcImage = vkRT->resolveAttachment();
        } else {
            srcImage = vkRT->colorAttachment();
        }
    } else if (src->asTexture()) {
        SkASSERT(src->asTexture());
        srcImage = static_cast<GrVkTexture*>(src->asTexture())->textureImage();
    } else {
        // The surface in a GrAttachment already
        srcImage = static_cast<GrVkImage*>(src);
    }

    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat = srcImage->imageFormat();

    int dstSampleCnt = dstImage->numSamples();
    int srcSampleCnt = srcImage->numSamples();

    bool dstHasYcbcr = dstImage->ycbcrConversionInfo().isValid();
    bool srcHasYcbcr = srcImage->ycbcrConversionInfo().isValid();

    if (srcRect.size() == dstRect.size()) {
        // Prefer resolves or copy-image commands when there is no scaling
        const SkIPoint dstPoint = dstRect.topLeft();
        if (this->vkCaps().canCopyAsResolve(dstFormat, dstSampleCnt, dstHasYcbcr,
                                            srcFormat, srcSampleCnt, srcHasYcbcr)) {
            this->copySurfaceAsResolve(dst, src, srcRect, dstPoint);
            return true;
        }

        if (this->vkCaps().canCopyImage(dstFormat, dstSampleCnt, dstHasYcbcr,
                                        srcFormat, srcSampleCnt, srcHasYcbcr)) {
            this->copySurfaceAsCopyImage(dst, src, dstImage, srcImage, srcRect, dstPoint);
            return true;
        }
    }

    if (this->vkCaps().canCopyAsBlit(dstFormat,
                                     dstSampleCnt,
                                     dstImage->isLinearTiled(),
                                     dstHasYcbcr,
                                     srcFormat,
                                     srcSampleCnt,
                                     srcImage->isLinearTiled(),
                                     srcHasYcbcr)) {
        this->copySurfaceAsBlit(dst, src, dstImage, srcImage, srcRect, dstRect, filter);
        return true;
    }

    return false;
}

bool GrVkGpu::onReadPixels(GrSurface* surface,
                           SkIRect rect,
                           GrColorType surfaceColorType,
                           GrColorType dstColorType,
                           void* buffer,
                           size_t rowBytes) {
    if (surface->isProtected()) {
        return false;
    }

    if (!this->currentCommandBuffer()) {
        return false;
    }

    GrVkImage* image = nullptr;
    GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(surface->asRenderTarget());
    if (rt) {
        // Reading from render targets that wrap a secondary command buffer is not allowed since
        // it would require us to know the VkImage, which we don't have, as well as need us to
        // stop and start the VkRenderPass which we don't have access to.
        if (rt->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        image = rt->nonMSAAAttachment();
    } else {
        image = static_cast<GrVkTexture*>(surface->asTexture())->textureImage();
    }

    if (!image) {
        return false;
    }

    if (dstColorType == GrColorType::kUnknown ||
        dstColorType != this->vkCaps().transferColorType(image->imageFormat(), surfaceColorType)) {
        return false;
    }

    // Change layout of our target so it can be used as copy
    image->setImageLayout(this,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_ACCESS_TRANSFER_READ_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          false);

    size_t bpp = GrColorTypeBytesPerPixel(dstColorType);
    if (skgpu::VkFormatBytesPerBlock(image->imageFormat()) != bpp) {
        return false;
    }
    size_t tightRowBytes = bpp*rect.width();

    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    VkOffset3D offset = { rect.left(), rect.top(), 0 };
    region.imageOffset = offset;
    region.imageExtent = { (uint32_t)rect.width(), (uint32_t)rect.height(), 1 };

    size_t transBufferRowBytes = bpp * region.imageExtent.width;
    size_t imageRows = region.imageExtent.height;
    GrResourceProvider* resourceProvider = this->getContext()->priv().resourceProvider();
    sk_sp<GrGpuBuffer> transferBuffer = resourceProvider->createBuffer(
            transBufferRowBytes * imageRows,
            GrGpuBufferType::kXferGpuToCpu,
            kDynamic_GrAccessPattern,
            GrResourceProvider::ZeroInit::kNo);

    if (!transferBuffer) {
        return false;
    }

    GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(transferBuffer.get());

    // Copy the image to a buffer so we can map it to cpu memory
    region.bufferOffset = 0;
    region.bufferRowLength = 0; // Forces RowLength to be width. We handle the rowBytes below.
    region.bufferImageHeight = 0; // Forces height to be tightly packed. Only useful for 3d images.
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

    this->currentCommandBuffer()->copyImageToBuffer(this,
                                                    image,
                                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                    transferBuffer,
                                                    1,
                                                    &region);

    // make sure the copy to buffer has finished
    vkBuffer->addMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT,
                               VK_ACCESS_HOST_READ_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_HOST_BIT,
                               false);

    // We need to submit the current command buffer to the Queue and make sure it finishes before
    // we can copy the data out of the buffer.
    if (!this->submitCommandBuffer(kForce_SyncQueue)) {
        return false;
    }
    void* mappedMemory = transferBuffer->map();
    if (!mappedMemory) {
        return false;
    }

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, tightRowBytes, rect.height());

    transferBuffer->unmap();
    return true;
}

bool GrVkGpu::beginRenderPass(const GrVkRenderPass* renderPass,
                              sk_sp<const GrVkFramebuffer> framebuffer,
                              const VkClearValue* colorClear,
                              const GrSurface* target,
                              const SkIRect& renderPassBounds,
                              bool forSecondaryCB) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    SkASSERT (!framebuffer->isExternal());

#ifdef SK_DEBUG
    uint32_t index;
    bool result = renderPass->colorAttachmentIndex(&index);
    SkASSERT(result && 0 == index);
    result = renderPass->stencilAttachmentIndex(&index);
    if (result) {
        SkASSERT(1 == index);
    }
#endif
    VkClearValue clears[3];
    int stencilIndex = renderPass->hasResolveAttachment() ? 2 : 1;
    clears[0].color = colorClear->color;
    clears[stencilIndex].depthStencil.depth = 0.0f;
    clears[stencilIndex].depthStencil.stencil = 0;

   return this->currentCommandBuffer()->beginRenderPass(
        this, renderPass, std::move(framebuffer), clears, target, renderPassBounds, forSecondaryCB);
}

void GrVkGpu::endRenderPass(GrRenderTarget* target, GrSurfaceOrigin origin,
                            const SkIRect& bounds) {
    // We had a command buffer when we started the render pass, we should have one now as well.
    SkASSERT(this->currentCommandBuffer());
    this->currentCommandBuffer()->endRenderPass(this);
    this->didWriteToSurface(target, origin, &bounds);
}

bool GrVkGpu::checkVkResult(VkResult result) {
    switch (result) {
        case VK_SUCCESS:
            return true;
        case VK_ERROR_DEVICE_LOST:
            if (!fDeviceIsLost) {
                // Callback should only be invoked once, and device should be marked as lost first.
                fDeviceIsLost = true;
                skgpu::InvokeDeviceLostCallback(vkInterface(),
                                                device(),
                                                fDeviceLostContext,
                                                fDeviceLostProc,
                                                vkCaps().supportsDeviceFaultInfo());
            }
            return false;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            this->setOOMed();
            return false;
        default:
            return false;
    }
}

void GrVkGpu::submitSecondaryCommandBuffer(std::unique_ptr<GrVkSecondaryCommandBuffer> buffer) {
    if (!this->currentCommandBuffer()) {
        return;
    }
    this->currentCommandBuffer()->executeCommands(this, std::move(buffer));
}

void GrVkGpu::submit(GrOpsRenderPass* renderPass) {
    SkASSERT(fCachedOpsRenderPass.get() == renderPass);

    fCachedOpsRenderPass->submit();
    fCachedOpsRenderPass->reset();
}

[[nodiscard]] std::unique_ptr<GrSemaphore> GrVkGpu::makeSemaphore(bool isOwned) {
    return GrVkSemaphore::Make(this, isOwned);
}

std::unique_ptr<GrSemaphore> GrVkGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                           GrSemaphoreWrapType wrapType,
                                                           GrWrapOwnership ownership) {
    return GrVkSemaphore::MakeWrapped(this, GrBackendSemaphores::GetVkSemaphore(semaphore),
                                      wrapType, ownership);
}

void GrVkGpu::insertSemaphore(GrSemaphore* semaphore) {
    SkASSERT(semaphore);

    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore);

    GrVkSemaphore::Resource* resource = vkSem->getResource();
    if (resource->shouldSignal()) {
        resource->ref();
        fSemaphoresToSignal.push_back(resource);
    }
}

void GrVkGpu::waitSemaphore(GrSemaphore* semaphore) {
    SkASSERT(semaphore);

    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore);

    GrVkSemaphore::Resource* resource = vkSem->getResource();
    if (resource->shouldWait()) {
        resource->ref();
        fSemaphoresToWaitOn.push_back(resource);
    }
}

std::unique_ptr<GrSemaphore> GrVkGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(texture);
    GrVkImage* vkTexture = static_cast<GrVkTexture*>(texture)->textureImage();
    vkTexture->setImageLayout(this,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT,
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              false);
    // TODO: should we have a way to notify the caller that this has failed? Currently if the submit
    // fails (caused by DEVICE_LOST) this will just cause us to fail the next use of the gpu.
    // Eventually we will abandon the whole GPU if this fails.
    this->submitToGpu(GrSyncCpu::kNo);

    // The image layout change serves as a barrier, so no semaphore is needed.
    // If we ever decide we need to return a semaphore here, we need to make sure GrVkSemaphore is
    // thread safe so that only the first thread that tries to use the semaphore actually submits
    // it. This additionally would also require thread safety in command buffer submissions to
    // queues in general.
    return nullptr;
}

void GrVkGpu::addDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable) {
    fDrawables.emplace_back(std::move(drawable));
}

void GrVkGpu::storeVkPipelineCacheData() {
    if (this->getContext()->priv().getPersistentCache()) {
        this->resourceProvider().storePipelineCacheData();
    }
}
