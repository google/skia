/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkGpu.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkTo.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrNativeRect.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/vk/GrVkAMDMemoryAllocator.h"
#include "src/gpu/vk/GrVkBuffer.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkCommandPool.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkOpsRenderPass.h"
#include "src/gpu/vk/GrVkPipeline.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkResourceProvider.h"
#include "src/gpu/vk/GrVkSemaphore.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/gpu/vk/GrVkTextureRenderTarget.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkSurface_Gpu.h"

#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkTypes.h"

#include <utility>

#define VK_CALL(X) GR_VK_CALL(this->vkInterface(), X)
#define VK_CALL_RET(RET, X) GR_VK_CALL_RESULT(this, RET, X)

sk_sp<GrGpu> GrVkGpu::Make(const GrVkBackendContext& backendContext,
                           const GrContextOptions& options, GrDirectContext* direct) {
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

    sk_sp<const GrVkInterface> interface;

    if (backendContext.fVkExtensions) {
        interface.reset(new GrVkInterface(backendContext.fGetProc,
                                          backendContext.fInstance,
                                          backendContext.fDevice,
                                          instanceVersion,
                                          physDevVersion,
                                          backendContext.fVkExtensions));
        if (!interface->validate(instanceVersion, physDevVersion, backendContext.fVkExtensions)) {
            return nullptr;
        }
    } else {
        GrVkExtensions extensions;
        // The only extension flag that may effect the vulkan backend is the swapchain extension. We
        // need to know if this is enabled to know if we can transition to a present layout when
        // flushing a surface.
        if (backendContext.fExtensions & kKHR_swapchain_GrVkExtensionFlag) {
            const char* swapChainExtName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            extensions.init(backendContext.fGetProc, backendContext.fInstance,
                            backendContext.fPhysicalDevice, 0, nullptr, 1, &swapChainExtName);
        }
        interface.reset(new GrVkInterface(backendContext.fGetProc,
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
        GrVkExtensions extensions;
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

    sk_sp<GrVkMemoryAllocator> memoryAllocator = backendContext.fMemoryAllocator;
    if (!memoryAllocator) {
        // We were not given a memory allocator at creation
        memoryAllocator = GrVkAMDMemoryAllocator::Make(backendContext.fInstance,
                                                       backendContext.fPhysicalDevice,
                                                       backendContext.fDevice, physDevVersion,
                                                       backendContext.fVkExtensions, interface,
                                                       caps.get());
    }
    if (!memoryAllocator) {
        SkDEBUGFAIL("No supplied vulkan memory allocator and unable to create one internally.");
        return nullptr;
    }

     sk_sp<GrVkGpu> vkGpu(new GrVkGpu(direct, backendContext, std::move(caps), interface,
                                      instanceVersion, physDevVersion,
                                      std::move(memoryAllocator)));
     if (backendContext.fProtectedContext == GrProtected::kYes &&
         !vkGpu->vkCaps().supportsProtectedMemory()) {
         return nullptr;
     }
     return std::move(vkGpu);
}

////////////////////////////////////////////////////////////////////////////////

GrVkGpu::GrVkGpu(GrDirectContext* direct, const GrVkBackendContext& backendContext,
                 sk_sp<GrVkCaps> caps, sk_sp<const GrVkInterface> interface,
                 uint32_t instanceVersion, uint32_t physicalDeviceVersion,
                 sk_sp<GrVkMemoryAllocator> memoryAllocator)
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
        , fProtectedContext(backendContext.fProtectedContext) {
    SkASSERT(!backendContext.fOwnsInstanceAndDevice);
    SkASSERT(fMemoryAllocator);

    this->initCapsAndCompiler(fVkCaps);

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

    for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
        fSemaphoresToWaitOn[i]->unref();
    }
    fSemaphoresToWaitOn.reset();

    for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
        fSemaphoresToSignal[i]->unref();
    }
    fSemaphoresToSignal.reset();

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

        fSemaphoresToWaitOn.reset();
        fSemaphoresToSignal.reset();
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
        GrAttachment* stencil,
        GrSurfaceOrigin origin,
        const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
        GrXferBarrierFlags renderPassXferBarriers) {
    if (!fCachedOpsRenderPass) {
        fCachedOpsRenderPass = std::make_unique<GrVkOpsRenderPass>(this);
    }

    if (!fCachedOpsRenderPass->set(rt, stencil, origin, bounds, colorInfo, stencilInfo,
                                   sampledProxies, renderPassXferBarriers)) {
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
        !fSemaphoresToSignal.count() && !fSemaphoresToWaitOn.count()) {
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
    fDrawables.reset();

    // If we didn't submit the command buffer then we did not wait on any semaphores. We will
    // continue to hold onto these semaphores and wait on them during the next command buffer
    // submission.
    if (didSubmit) {
        for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
            fSemaphoresToWaitOn[i]->unref();
        }
        fSemaphoresToWaitOn.reset();
    }

    // Even if we did not submit the command buffer, we drop all the signal semaphores since we will
    // not try to recover the work that wasn't submitted and instead just drop it all. The client
    // will be notified that the semaphores were not submit so that they will not try to wait on
    // them.
    for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
        fSemaphoresToSignal[i]->unref();
    }
    fSemaphoresToSignal.reset();

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
sk_sp<GrGpuBuffer> GrVkGpu::onCreateBuffer(size_t size, GrGpuBufferType type,
                                           GrAccessPattern accessPattern, const void* data) {
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
    sk_sp<GrGpuBuffer> buff = GrVkBuffer::Make(this, size, type, accessPattern);

    if (data && buff) {
        buff->updateData(data, size);
    }
    return buff;
}

bool GrVkGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType srcColorType,
                            const GrMipLevel texels[], int mipLevelCount,
                            bool prepForTexSampling) {
    GrVkTexture* texture = static_cast<GrVkTexture*>(surface->asTexture());
    if (!texture) {
        return false;
    }
    GrVkAttachment* texAttachment = texture->textureAttachment();

    // Make sure we have at least the base level
    if (!mipLevelCount || !texels[0].fPixels) {
        return false;
    }

    SkASSERT(!GrVkFormatIsCompressed(texAttachment->imageFormat()));
    bool success = false;
    bool linearTiling = texAttachment->isLinearTiled();
    if (linearTiling) {
        if (mipLevelCount > 1) {
            SkDebugf("Can't upload mipmap data to linear tiled texture");
            return false;
        }
        if (VK_IMAGE_LAYOUT_PREINITIALIZED != texAttachment->currentLayout()) {
            // Need to change the layout to general in order to perform a host write
            texAttachment->setImageLayout(this,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          VK_ACCESS_HOST_WRITE_BIT,
                                          VK_PIPELINE_STAGE_HOST_BIT,
                                          false);
            if (!this->submitCommandBuffer(kForce_SyncQueue)) {
                return false;
            }
        }
        success = this->uploadTexDataLinear(texAttachment, left, top, width, height, srcColorType,
                                            texels[0].fPixels, texels[0].fRowBytes);
    } else {
        SkASSERT(mipLevelCount <= (int)texAttachment->mipLevels());
        success = this->uploadTexDataOptimal(texAttachment, left, top, width, height, srcColorType,
                                             texels, mipLevelCount);
        if (1 == mipLevelCount) {
            texture->markMipmapsDirty();
        }
    }

    if (prepForTexSampling) {
        texAttachment->setImageLayout(this,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      false);
    }

    return success;
}

bool GrVkGpu::onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                                 GrColorType surfaceColorType, GrColorType bufferColorType,
                                 sk_sp<GrGpuBuffer> transferBuffer, size_t bufferOffset,
                                 size_t rowBytes) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    if (surfaceColorType != bufferColorType) {
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
    GrVkAttachment* vkTex = tex->textureAttachment();

    // Can't transfer compressed data
    SkASSERT(!GrVkFormatIsCompressed(vkTex->imageFormat()));

    if (!transferBuffer) {
        return false;
    }

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
        SkASSERT(bounds.contains(subRect));
    )

    // Set up copy region
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = bufferOffset;
    region.bufferRowLength = (uint32_t)(rowBytes/bpp);
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { left, top, 0 };
    region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

    // Change layout of our target so it can be copied to
    vkTex->setImageLayout(this,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          false);

    const GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(transferBuffer.get());

    // Copy the buffer to the image.
    this->currentCommandBuffer()->copyBufferToImage(this,
                                                    vkBuffer->vkBuffer(),
                                                    vkTex,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                    1,
                                                    &region);
    this->currentCommandBuffer()->addGrBuffer(std::move(transferBuffer));

    tex->markMipmapsDirty();
    return true;
}

bool GrVkGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                   GrColorType surfaceColorType, GrColorType bufferColorType,
                                   sk_sp<GrGpuBuffer> transferBuffer, size_t offset) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    SkASSERT(surface);
    SkASSERT(transferBuffer);
    if (fProtectedContext == GrProtected::kYes) {
        return false;
    }
    if (surfaceColorType != bufferColorType) {
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
        srcImage = static_cast<GrVkTexture*>(surface->asTexture())->textureAttachment();
    }

    if (GrVkFormatBytesPerBlock(srcImage->imageFormat()) !=
        GrColorTypeBytesPerPixel(surfaceColorType)) {
        return false;
    }

    // Set up copy region
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = offset;
    region.bufferRowLength = width;
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { left, top, 0 };
    region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

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
        dstImage = static_cast<GrVkTexture*>(dstTex)->textureAttachment();
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

    if (this->vkCaps().preferDiscardableMSAAAttachment() && rt->resolveAttachment() &&
        rt->resolveAttachment()->supportsInputAttachmentUsage()) {
        // We would have resolved the RT during the render pass;
        return;
    }

    this->resolveImage(target, rt, resolveRect,
                       SkIPoint::Make(resolveRect.x(), resolveRect.y()));
}

bool GrVkGpu::uploadTexDataLinear(GrVkAttachment* texAttachment, int left, int top, int width,
                                  int height, GrColorType dataColorType, const void* data,
                                  size_t rowBytes) {
    SkASSERT(data);
    SkASSERT(texAttachment->isLinearTiled());

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texAttachment->width(), texAttachment->height());
        SkASSERT(bounds.contains(subRect));
    )
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);
    size_t trimRowBytes = width * bpp;

    SkASSERT(VK_IMAGE_LAYOUT_PREINITIALIZED == texAttachment->currentLayout() ||
             VK_IMAGE_LAYOUT_GENERAL == texAttachment->currentLayout());
    const VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,  // mipLevel
        0,  // arraySlice
    };
    VkSubresourceLayout layout;

    const GrVkInterface* interface = this->vkInterface();

    GR_VK_CALL(interface, GetImageSubresourceLayout(fDevice,
                                                    texAttachment->image(),
                                                    &subres,
                                                    &layout));

    const GrVkAlloc& alloc = texAttachment->alloc();
    if (VK_NULL_HANDLE == alloc.fMemory) {
        return false;
    }
    VkDeviceSize offset = top * layout.rowPitch + left * bpp;
    VkDeviceSize size = height*layout.rowPitch;
    SkASSERT(size + offset <= alloc.fSize);
    void* mapPtr = GrVkMemory::MapAlloc(this, alloc);
    if (!mapPtr) {
        return false;
    }
    mapPtr = reinterpret_cast<char*>(mapPtr) + offset;

    SkRectMemcpy(mapPtr, static_cast<size_t>(layout.rowPitch), data, rowBytes, trimRowBytes,
                 height);

    GrVkMemory::FlushMappedAlloc(this, alloc, offset, size);
    GrVkMemory::UnmapAlloc(this, alloc);

    return true;
}

// This fills in the 'regions' vector in preparation for copying a buffer to an image.
// 'individualMipOffsets' is filled in as a side-effect.
static size_t fill_in_regions(GrStagingBufferManager* stagingBufferManager,
                              SkTArray<VkBufferImageCopy>* regions,
                              SkTArray<size_t>* individualMipOffsets,
                              GrStagingBufferManager::Slice* slice,
                              SkImage::CompressionType compression,
                              VkFormat vkFormat, SkISize dimensions, GrMipmapped mipMapped) {
    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    regions->reserve_back(numMipLevels);
    individualMipOffsets->reserve_back(numMipLevels);

    size_t bytesPerBlock = GrVkFormatBytesPerBlock(vkFormat);

    size_t combinedBufferSize;
    if (compression == SkImage::CompressionType::kNone) {
        combinedBufferSize = GrComputeTightCombinedBufferSize(bytesPerBlock, dimensions,
                                                              individualMipOffsets,
                                                              numMipLevels);
    } else {
        combinedBufferSize = SkCompressedDataSize(compression, dimensions, individualMipOffsets,
                                                  mipMapped == GrMipmapped::kYes);
    }
    SkASSERT(individualMipOffsets->count() == numMipLevels);

    // Get a staging buffer slice to hold our mip data.
    // Vulkan requires offsets in the buffer to be aligned to multiple of the texel size and 4
    size_t alignment = bytesPerBlock;
    switch (alignment & 0b11) {
        case 0:                     break;   // alignment is already a multiple of 4.
        case 2:     alignment *= 2; break;   // alignment is a multiple of 2 but not 4.
        default:    alignment *= 4; break;   // alignment is not a multiple of 2.
    }
    *slice = stagingBufferManager->allocateStagingBufferSlice(combinedBufferSize, alignment);
    if (!slice->fBuffer) {
        return 0;
    }

    for (int i = 0; i < numMipLevels; ++i) {
        VkBufferImageCopy& region = regions->push_back();
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = slice->fOffset + (*individualMipOffsets)[i];
        SkISize revisedDimensions = GrCompressedDimensions(compression, dimensions);
        region.bufferRowLength = revisedDimensions.width();
        region.bufferImageHeight = revisedDimensions.height();
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(i), 0, 1};
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {SkToU32(dimensions.width()),
                              SkToU32(dimensions.height()), 1};

        dimensions = {std::max(1, dimensions.width() /2),
                      std::max(1, dimensions.height()/2)};
    }

    return combinedBufferSize;
}

bool GrVkGpu::uploadTexDataOptimal(GrVkAttachment* texAttachment, int left, int top, int width,
                                   int height, GrColorType dataColorType, const GrMipLevel texels[],
                                   int mipLevelCount) {
    if (!this->currentCommandBuffer()) {
        return false;
    }

    SkASSERT(!texAttachment->isLinearTiled());
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == texAttachment->width() &&
              height == texAttachment->height()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(1 == mipLevelCount || mipLevelCount == (int)texAttachment->mipLevels());

    if (width == 0 || height == 0) {
        return false;
    }

    SkASSERT(this->vkCaps().surfaceSupportsWritePixels(texAttachment));
    SkASSERT(this->vkCaps().areColorTypeAndFormatCompatible(
            dataColorType, texAttachment->backendFormat()));

    // For RGB_888x src data we are uploading it first to an RGBA texture and then copying it to the
    // dst RGB texture. Thus we do not upload mip levels for that.
    if (dataColorType == GrColorType::kRGB_888x &&
        texAttachment->imageFormat() == VK_FORMAT_R8G8B8_UNORM) {
        // First check that we'll be able to do the copy to the to the R8G8B8 image in the end via a
        // blit or draw.
        if (!this->vkCaps().formatCanBeDstofBlit(VK_FORMAT_R8G8B8_UNORM,
                                                 texAttachment->isLinearTiled()) &&
            !this->vkCaps().isFormatRenderable(VK_FORMAT_R8G8B8_UNORM, 1)) {
            return false;
        }
        mipLevelCount = 1;
    }

    SkASSERT(this->vkCaps().isVkFormatTexturable(texAttachment->imageFormat()));
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    // texels is const.
    // But we may need to adjust the fPixels ptr based on the copyRect, or fRowBytes.
    // Because of this we need to make a non-const shallow copy of texels.
    SkAutoTMalloc<GrMipLevel> texelsShallowCopy;

    texelsShallowCopy.reset(mipLevelCount);
    memcpy(texelsShallowCopy.get(), texels, mipLevelCount*sizeof(GrMipLevel));

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    individualMipOffsets.push_back(0);
    size_t combinedBufferSize = width * bpp * height;
    int currentWidth = width;
    int currentHeight = height;
    if (!texelsShallowCopy[0].fPixels) {
        combinedBufferSize = 0;
    }

    // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
    // config. This works with the assumption that the bytes in pixel config is always a power of 2.
    SkASSERT((bpp & (bpp - 1)) == 0);
    const size_t alignmentMask = 0x3 | (bpp - 1);
    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; currentMipLevel++) {
        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);

        if (texelsShallowCopy[currentMipLevel].fPixels) {
            const size_t trimmedSize = currentWidth * bpp * currentHeight;
            const size_t alignmentDiff = combinedBufferSize & alignmentMask;
            if (alignmentDiff != 0) {
                combinedBufferSize += alignmentMask - alignmentDiff + 1;
            }
            individualMipOffsets.push_back(combinedBufferSize);
            combinedBufferSize += trimmedSize;
        } else {
            individualMipOffsets.push_back(0);
        }
    }
    if (0 == combinedBufferSize) {
        // We don't actually have any data to upload so just return success
        return true;
    }

    // Get a staging buffer slice to hold our mip data.
    // Vulkan requires offsets in the buffer to be aligned to multiple of the texel size and 4
    size_t alignment = SkAlign4(bpp);
    GrStagingBufferManager::Slice slice =
            fStagingBufferManager.allocateStagingBufferSlice(combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }

    int uploadLeft = left;
    int uploadTop = top;
    GrVkAttachment* uploadTexture = texAttachment;
    // For uploading RGB_888x data to an R8G8B8_UNORM texture we must first upload the data to an
    // R8G8B8A8_UNORM image and then copy it.
    sk_sp<GrVkAttachment> copyTexAttachment;
    if (dataColorType == GrColorType::kRGB_888x &&
        texAttachment->imageFormat() == VK_FORMAT_R8G8B8_UNORM) {
        bool dstHasYcbcr = texAttachment->ycbcrConversionInfo().isValid();
        if (!this->vkCaps().canCopyAsBlit(texAttachment->imageFormat(), 1, false, dstHasYcbcr,
                                          VK_FORMAT_R8G8B8A8_UNORM, 1, false, false)) {
            return false;
        }

        copyTexAttachment = GrVkAttachment::MakeTexture(this, {width, height},
                                                        VK_FORMAT_R8G8B8A8_UNORM, /*mipLevels=*/1,
                                                        GrRenderable::kNo, /*numSamples=*/1,
                                                        SkBudgeted::kYes, GrProtected::kNo);
        if (!copyTexAttachment) {
            return false;
        }

        uploadTexture = copyTexAttachment.get();
        uploadLeft = 0;
        uploadTop = 0;
    }

    char* buffer = (char*) slice.fOffsetMapPtr;
    SkTArray<VkBufferImageCopy> regions(mipLevelCount);

    currentWidth = width;
    currentHeight = height;
    int layerHeight = uploadTexture->height();
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texelsShallowCopy[currentMipLevel].fPixels) {
            SkASSERT(1 == mipLevelCount || currentHeight == layerHeight);
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
            region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(currentMipLevel), 0, 1 };
            region.imageOffset = {uploadLeft, uploadTop, 0};
            region.imageExtent = { (uint32_t)currentWidth, (uint32_t)currentHeight, 1 };
        }
        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
        layerHeight = currentHeight;
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
                                                    regions.count(),
                                                    regions.begin());

    // If we copied the data into a temporary image first, copy that image into our main texture
    // now.
    if (copyTexAttachment) {
        SkASSERT(dataColorType == GrColorType::kRGB_888x);
        SkAssertResult(this->copySurface(texAttachment, copyTexAttachment.get(),
                                         SkIRect::MakeWH(width, height),
                                         SkIPoint::Make(left, top)));
    }

    return true;
}

// It's probably possible to roll this into uploadTexDataOptimal,
// but for now it's easier to maintain as a separate entity.
bool GrVkGpu::uploadTexDataCompressed(GrVkAttachment* uploadTexture,
                                      SkImage::CompressionType compression, VkFormat vkFormat,
                                      SkISize dimensions, GrMipmapped mipMapped,
                                      const void* data, size_t dataSize) {
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
    SkTArray<VkBufferImageCopy> regions;
    SkTArray<size_t> individualMipOffsets;
    SkDEBUGCODE(size_t combinedBufferSize =) fill_in_regions(&fStagingBufferManager,
                                                             &regions, &individualMipOffsets,
                                                             &slice, compression, vkFormat,
                                                             dimensions, mipMapped);
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
                                                    regions.count(),
                                                    regions.begin());

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// TODO: make this take a GrMipmapped
sk_sp<GrTexture> GrVkGpu::onCreateTexture(SkISize dimensions,
                                          const GrBackendFormat& format,
                                          GrRenderable renderable,
                                          int renderTargetSampleCnt,
                                          SkBudgeted budgeted,
                                          GrProtected isProtected,
                                          int mipLevelCount,
                                          uint32_t levelClearMask) {
    VkFormat pixelFormat;
    SkAssertResult(format.asVkFormat(&pixelFormat));
    SkASSERT(!GrVkFormatIsCompressed(pixelFormat));
    SkASSERT(mipLevelCount > 0);

    GrMipmapStatus mipmapStatus =
            mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;

    sk_sp<GrVkTexture> tex;
    if (renderable == GrRenderable::kYes) {
        tex = GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
                this, budgeted, dimensions, pixelFormat, mipLevelCount, renderTargetSampleCnt,
                mipmapStatus, isProtected);
    } else {
        tex = GrVkTexture::MakeNewTexture(this, budgeted, dimensions, pixelFormat,
                                          mipLevelCount, isProtected, mipmapStatus);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        if (!this->currentCommandBuffer()) {
            return nullptr;
        }
        SkSTArray<1, VkImageSubresourceRange> ranges;
        bool inRange = false;
        GrVkImage* texImage = tex->textureAttachment();
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
                                                      ranges.count(), ranges.begin());
    }
    return std::move(tex);
}

sk_sp<GrTexture> GrVkGpu::onCreateCompressedTexture(SkISize dimensions,
                                                    const GrBackendFormat& format,
                                                    SkBudgeted budgeted,
                                                    GrMipmapped mipMapped,
                                                    GrProtected isProtected,
                                                    const void* data, size_t dataSize) {
    VkFormat pixelFormat;
    SkAssertResult(format.asVkFormat(&pixelFormat));
    SkASSERT(GrVkFormatIsCompressed(pixelFormat));

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height())+1;
    }

    GrMipmapStatus mipmapStatus = (mipMapped == GrMipmapped::kYes) ? GrMipmapStatus::kValid
                                                                   : GrMipmapStatus::kNotAllocated;

    auto tex = GrVkTexture::MakeNewTexture(this, budgeted, dimensions, pixelFormat,
                                           numMipLevels, isProtected, mipmapStatus);
    if (!tex) {
        return nullptr;
    }

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);
    if (!this->uploadTexDataCompressed(tex->textureAttachment(), compression, pixelFormat,
                                       dimensions, mipMapped, data, dataSize)) {
        return nullptr;
    }

    return std::move(tex);
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpu::copyBuffer(sk_sp<GrGpuBuffer> srcBuffer,
                         sk_sp<GrGpuBuffer> dstBuffer,
                         VkDeviceSize srcOffset,
                         VkDeviceSize dstOffset,
                         VkDeviceSize size) {
    if (!this->currentCommandBuffer()) {
        return;
    }
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    this->currentCommandBuffer()->copyBuffer(this, std::move(srcBuffer), std::move(dstBuffer), 1,
                                             &copyRegion);
}

bool GrVkGpu::updateBuffer(sk_sp<GrVkBuffer> buffer, const void* src,
                           VkDeviceSize offset, VkDeviceSize size) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    // Update the buffer
    this->currentCommandBuffer()->updateBuffer(this, std::move(buffer), offset, size, src);

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
    } else {
        SkASSERT(info.fImageTiling == VK_IMAGE_TILING_LINEAR);
        if (!caps.isVkFormatTexturableLinearly(info.fFormat)) {
            return false;
        }
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
    if (!backendTex.getVkImageInfo(&imageInfo)) {
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

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState = backendTex.getMutableState();
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
    if (!backendTex.getVkImageInfo(&imageInfo)) {
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

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState = backendTex.getMutableState();
    SkASSERT(mutableState);

    return GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(this, backendTex.dimensions(),
                                                                   sampleCnt, ownership, cacheable,
                                                                   imageInfo,
                                                                   std::move(mutableState));
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    GrVkImageInfo info;
    if (!backendRT.getVkImageInfo(&info)) {
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

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState = backendRT.getMutableState();
    SkASSERT(mutableState);

    sk_sp<GrVkRenderTarget> tgt = GrVkRenderTarget::MakeWrappedRenderTarget(
            this, backendRT.dimensions(), backendRT.sampleCnt(), info, std::move(mutableState));

    // We don't allow the client to supply a premade stencil buffer. We always create one if needed.
    SkASSERT(!backendRT.stencilBits());
    if (tgt) {
        SkASSERT(tgt->canAttemptStencilAttachment());
    }

    return std::move(tgt);
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo) {
    int maxSize = this->caps()->maxTextureSize();
    if (imageInfo.width() > maxSize || imageInfo.height() > maxSize) {
        return nullptr;
    }

    GrBackendFormat backendFormat = GrBackendFormat::MakeVk(vkInfo.fFormat);
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
                                  GrSurface* dst,
                                  GrSurface* src,
                                  const SkIRect& srcRect) {
    return fMSAALoadManager.loadMSAAFromResolve(this, commandBuffer, renderPass, dst, src, srcRect);
}

bool GrVkGpu::onRegenerateMipMapLevels(GrTexture* tex) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    auto* vkTex = static_cast<GrVkTexture*>(tex)->textureAttachment();
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

sk_sp<GrAttachment> GrVkGpu::makeStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                  SkISize dimensions,
                                                                  int numStencilSamples) {
    SkASSERT(numStencilSamples == rt->numSamples() || this->caps()->mixedSamplesSupport());
    SkASSERT(dimensions.width() >= rt->width());
    SkASSERT(dimensions.height() >= rt->height());

    VkFormat sFmt = this->vkCaps().preferredStencilFormat();

    fStats.incStencilAttachmentCreates();
    return GrVkAttachment::MakeStencil(this, dimensions, numStencilSamples, sFmt);
}

sk_sp<GrAttachment> GrVkGpu::makeMSAAAttachment(SkISize dimensions,
                                                const GrBackendFormat& format,
                                                int numSamples,
                                                GrProtected isProtected) {
    VkFormat pixelFormat;
    SkAssertResult(format.asVkFormat(&pixelFormat));
    SkASSERT(!GrVkFormatIsCompressed(pixelFormat));
    SkASSERT(this->vkCaps().isFormatRenderable(pixelFormat, numSamples));

    fStats.incMSAAAttachmentCreates();
    return GrVkAttachment::MakeMSAA(this, dimensions, numSamples, pixelFormat, isProtected);
}

////////////////////////////////////////////////////////////////////////////////

bool copy_src_data(char* mapPtr,
                   VkFormat vkFormat,
                   const SkTArray<size_t>& individualMipOffsets,
                   const GrPixmap srcData[],
                   int numMipLevels) {
    SkASSERT(srcData && numMipLevels);
    SkASSERT(!GrVkFormatIsCompressed(vkFormat));
    SkASSERT(individualMipOffsets.count() == numMipLevels);
    SkASSERT(mapPtr);

    size_t bytesPerPixel = GrVkFormatBytesPerBlock(vkFormat);

    for (int level = 0; level < numMipLevels; ++level) {
        const size_t trimRB = srcData[level].info().width() * bytesPerPixel;

        SkRectMemcpy(mapPtr + individualMipOffsets[level], trimRB,
                     srcData[level].addr(), srcData[level].rowBytes(),
                     trimRB, srcData[level].height());
    }
    return true;
}

bool copy_compressed_data(GrVkGpu* gpu, char* mapPtr,
                          const void* rawData, size_t dataSize) {
    SkASSERT(mapPtr);
    memcpy(mapPtr, rawData, dataSize);
    return true;
}

bool generate_compressed_data(GrVkGpu* gpu, char* mapPtr,
                              SkImage::CompressionType compression, SkISize dimensions,
                              GrMipmapped mipMapped, const SkColor4f& color) {
    SkASSERT(mapPtr);
    GrFillInCompressedData(compression, dimensions, mipMapped, mapPtr, color);

    return true;
}

bool GrVkGpu::createVkImageForBackendSurface(VkFormat vkFormat,
                                             SkISize dimensions,
                                             int sampleCnt,
                                             GrTexturable texturable,
                                             GrRenderable renderable,
                                             GrMipmapped mipMapped,
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
    if (mipMapped == GrMipmapped::kYes) {
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

bool GrVkGpu::onUpdateBackendTexture(const GrBackendTexture& backendTexture,
                                     sk_sp<GrRefCntedCallback> finishedCallback,
                                     const BackendTextureData* data) {
    GrVkImageInfo info;
    SkAssertResult(backendTexture.getVkImageInfo(&info));

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState = backendTexture.getMutableState();
    SkASSERT(mutableState);
    sk_sp<GrVkTexture> texture =
                GrVkTexture::MakeWrappedTexture(this, backendTexture.dimensions(),
                                                kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                                                kRW_GrIOType, info, std::move(mutableState));
    if (!texture) {
        return false;
    }
    GrVkAttachment* texAttachment = texture->textureAttachment();

    GrVkPrimaryCommandBuffer* cmdBuffer = this->currentCommandBuffer();
    if (!cmdBuffer) {
        return false;
    }

    texAttachment->setImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  false);

    // Unfortunately, CmdClearColorImage doesn't work for compressed formats
    bool fastPath = data->type() == BackendTextureData::Type::kColor &&
                    !GrVkFormatIsCompressed(info.fFormat);

    if (fastPath) {
        SkASSERT(data->type() == BackendTextureData::Type::kColor);
        VkClearColorValue vkColor;
        SkColor4f color = data->color();
        // If we ever support SINT or UINT formats this needs to be updated to use the int32 and
        // uint32 union members in those cases.
        vkColor.float32[0] = color.fR;
        vkColor.float32[1] = color.fG;
        vkColor.float32[2] = color.fB;
        vkColor.float32[3] = color.fA;
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseArrayLayer = 0;
        range.baseMipLevel = 0;
        range.layerCount = 1;
        range.levelCount = info.fLevelCount;
        cmdBuffer->clearColorImage(this, texAttachment, &vkColor, 1, &range);
    } else {
        SkImage::CompressionType compression = GrBackendFormatToCompressionType(
                backendTexture.getBackendFormat());

        SkTArray<VkBufferImageCopy> regions;
        SkTArray<size_t> individualMipOffsets;
        GrStagingBufferManager::Slice slice;

        fill_in_regions(&fStagingBufferManager, &regions, &individualMipOffsets,
                        &slice, compression, info.fFormat, backendTexture.dimensions(),
                        backendTexture.fMipmapped);

        if (!slice.fBuffer) {
            return false;
        }

        bool result;
        if (data->type() == BackendTextureData::Type::kPixmaps) {
            result = copy_src_data((char*)slice.fOffsetMapPtr, info.fFormat, individualMipOffsets,
                                   data->pixmaps(), info.fLevelCount);
        } else if (data->type() == BackendTextureData::Type::kCompressed) {
            result = copy_compressed_data(this, (char*)slice.fOffsetMapPtr,
                                          data->compressedData(), data->compressedSize());
        } else {
            SkASSERT(data->type() == BackendTextureData::Type::kColor);
            result = generate_compressed_data(this, (char*)slice.fOffsetMapPtr, compression,
                                              backendTexture.dimensions(),
                                              backendTexture.fMipmapped, data->color());
        }

        cmdBuffer->addGrSurface(texture);
        // Copy the buffer to the image. This call takes the raw VkBuffer instead of a GrGpuBuffer
        // because we don't need the command buffer to ref the buffer here. The reason being is that
        // the buffer is coming from the staging manager and the staging manager will make sure the
        // command buffer has a ref on the buffer. This avoids having to add and remove a ref for
        // every upload in the frame.
        const GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(slice.fBuffer);
        cmdBuffer->copyBufferToImage(this, vkBuffer->vkBuffer(), texAttachment,
                                     texAttachment->currentLayout(), regions.count(),
                                     regions.begin());
    }

    // Change image layout to shader read since if we use this texture as a borrowed
    // texture within Ganesh we require that its layout be set to that
    texAttachment->setImageLayout(this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
                                                 GrMipmapped mipMapped,
                                                 GrProtected isProtected) {
    const GrVkCaps& caps = this->vkCaps();

    if (fProtectedContext != isProtected) {
        return {};
    }

    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return {};
    }

    // TODO: move the texturability check up to GrGpu::createBackendTexture and just assert here
    if (!caps.isVkFormatTexturable(vkFormat)) {
        return {};
    }

    if (GrVkFormatNeedsYcbcrSampler(vkFormat)) {
        return {};
    }

    GrVkImageInfo info;
    if (!this->createVkImageForBackendSurface(vkFormat, dimensions, 1, GrTexturable::kYes,
                                              renderable, mipMapped, &info, isProtected)) {
        return {};
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
}

GrBackendTexture GrVkGpu::onCreateCompressedBackendTexture(
        SkISize dimensions, const GrBackendFormat& format, GrMipmapped mipMapped,
        GrProtected isProtected) {
    return this->onCreateBackendTexture(dimensions, format, GrRenderable::kNo, mipMapped,
                                        isProtected);
}

bool GrVkGpu::onUpdateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               sk_sp<GrRefCntedCallback> finishedCallback,
                                               const BackendTextureData* data) {
    return this->onUpdateBackendTexture(backendTexture, std::move(finishedCallback), data);
}

void set_layout_and_queue_from_mutable_state(GrVkGpu* gpu, GrVkImage* image,
                                             const GrVkSharedImageInfo& newInfo) {
    // Even though internally we use this helper for getting src access flags and stages they
    // can also be used for general dst flags since we don't know exactly what the client
    // plans on using the image for.
    VkImageLayout newLayout = newInfo.getImageLayout();
    if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        newLayout = image->currentLayout();
    }
    VkPipelineStageFlags dstStage = GrVkImage::LayoutToPipelineSrcStageFlags(newLayout);
    VkAccessFlags dstAccess = GrVkImage::LayoutToSrcAccessMask(newLayout);

    uint32_t currentQueueFamilyIndex = image->currentQueueFamilyIndex();
    uint32_t newQueueFamilyIndex = newInfo.getQueueFamilyIndex();
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
                                     sk_sp<GrBackendSurfaceMutableStateImpl> currentState,
                                     SkISize dimensions,
                                     const GrVkSharedImageInfo& newInfo,
                                     GrBackendSurfaceMutableState* previousState) {
    sk_sp<GrVkAttachment> texture = GrVkAttachment::MakeWrapped(
            this, dimensions, info, std::move(currentState),
           GrVkAttachment::UsageFlags::kColorAttachment, kBorrow_GrWrapOwnership,
           GrWrapCacheable::kNo, /*forSecondaryCB=*/false);
    SkASSERT(texture);
    if (!texture) {
        return false;
    }
    if (previousState) {
        previousState->setVulkanState(texture->currentLayout(),
                                      texture->currentQueueFamilyIndex());
    }
    set_layout_and_queue_from_mutable_state(this, texture.get(), newInfo);
    return true;
}

bool GrVkGpu::setBackendTextureState(const GrBackendTexture& backendTeture,
                                     const GrBackendSurfaceMutableState& newState,
                                     GrBackendSurfaceMutableState* previousState,
                                     sk_sp<GrRefCntedCallback> finishedCallback) {
    GrVkImageInfo info;
    SkAssertResult(backendTeture.getVkImageInfo(&info));
    sk_sp<GrBackendSurfaceMutableStateImpl> currentState = backendTeture.getMutableState();
    SkASSERT(currentState);
    SkASSERT(newState.isValid() && newState.fBackend == GrBackend::kVulkan);
    return this->setBackendSurfaceState(info, std::move(currentState), backendTeture.dimensions(),
                                        newState.fVkState, previousState);
}

bool GrVkGpu::setBackendRenderTargetState(const GrBackendRenderTarget& backendRenderTarget,
                                          const GrBackendSurfaceMutableState& newState,
                                          GrBackendSurfaceMutableState* previousState,
                                          sk_sp<GrRefCntedCallback> finishedCallback) {
    GrVkImageInfo info;
    SkAssertResult(backendRenderTarget.getVkImageInfo(&info));
    sk_sp<GrBackendSurfaceMutableStateImpl> currentState = backendRenderTarget.getMutableState();
    SkASSERT(currentState);
    SkASSERT(newState.fBackend == GrBackend::kVulkan);
    return this->setBackendSurfaceState(info, std::move(currentState),
                                        backendRenderTarget.dimensions(), newState.fVkState,
                                        previousState);
}

void GrVkGpu::querySampleLocations(GrRenderTarget* renderTarget,
                                   SkTArray<SkPoint>* sampleLocations) {
    // In Vulkan, sampleLocationsSupport() means that the platform uses the standard sample
    // locations defined by the spec.
    SkASSERT(this->caps()->sampleLocationsSupport());
    static constexpr SkPoint kStandardSampleLocations_1[1] = {
        {0.5f, 0.5f}};
    static constexpr SkPoint kStandardSampleLocations_2[2] = {
        {0.75f, 0.75f}, {0.25f, 0.25f}};
    static constexpr SkPoint kStandardSampleLocations_4[4] = {
        {0.375f, 0.125f}, {0.875f, 0.375f}, {0.125f, 0.625f}, {0.625f, 0.875f}};
    static constexpr SkPoint kStandardSampleLocations_8[8] = {
        {0.5625f, 0.3125f}, {0.4375f, 0.6875f}, {0.8125f, 0.5625f}, {0.3125f, 0.1875f},
        {0.1875f, 0.8125f}, {0.0625f, 0.4375f}, {0.6875f, 0.9375f}, {0.9375f, 0.0625f}};
    static constexpr SkPoint kStandardSampleLocations_16[16] = {
        {0.5625f, 0.5625f}, {0.4375f, 0.3125f}, {0.3125f, 0.625f}, {0.75f, 0.4375f},
        {0.1875f, 0.375f}, {0.625f, 0.8125f}, {0.8125f, 0.6875f}, {0.6875f, 0.1875f},
        {0.375f, 0.875f}, {0.5f, 0.0625f}, {0.25f, 0.125f}, {0.125f, 0.75f},
        {0.0f, 0.5f}, {0.9375f, 0.25f}, {0.875f, 0.9375f}, {0.0625f, 0.0f}};

    int numSamples = renderTarget->numSamples();
    if (1 == numSamples) {
        SkASSERT(this->caps()->mixedSamplesSupport());
        if (auto* stencil = renderTarget->getStencilAttachment()) {
            numSamples = stencil->numSamples();
        }
    }
    SkASSERT(numSamples > 1);
    SkASSERT(!renderTarget->getStencilAttachment() ||
             numSamples == renderTarget->getStencilAttachment()->numSamples());

    switch (numSamples) {
        case 1:
            sampleLocations->push_back_n(1, kStandardSampleLocations_1);
            break;
        case 2:
            sampleLocations->push_back_n(2, kStandardSampleLocations_2);
            break;
        case 4:
            sampleLocations->push_back_n(4, kStandardSampleLocations_4);
            break;
        case 8:
            sampleLocations->push_back_n(8, kStandardSampleLocations_8);
            break;
        case 16:
            sampleLocations->push_back_n(16, kStandardSampleLocations_16);
            break;
        default:
            SK_ABORT("Invalid vulkan sample count.");
            break;
    }
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
    GrVkAttachment* colorAttachment = vkRT->colorAttachment();
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = dstAccess;
    barrier.oldLayout = colorAttachment->currentLayout();
    barrier.newLayout = barrier.oldLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = colorAttachment->image();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, colorAttachment->mipLevels(), 0, 1};
    this->addImageMemoryBarrier(colorAttachment->resource(),
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                dstStage, true, &barrier);
}

void GrVkGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kVulkan == tex.fBackend);

    GrVkImageInfo info;
    if (tex.getVkImageInfo(&info)) {
        GrVkImage::DestroyImageInfo(this, const_cast<GrVkImageInfo*>(&info));
    }
}

bool GrVkGpu::compile(const GrProgramDesc& desc, const GrProgramInfo& programInfo) {
    SkASSERT(!(GrProcessor::CustomFeatures::kSampleLocations & programInfo.requestedFeatures()));

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
    if (programInfo.targetSupportsVkResolveLoad() && programInfo.colorLoadOp() == GrLoadOp::kLoad &&
        this->vkCaps().preferDiscardableMSAAAttachment()) {
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

#if GR_TEST_UTILS
bool GrVkGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kVulkan == tex.fBackend);

    GrVkImageInfo backend;
    if (!tex.getVkImageInfo(&backend)) {
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
    if (!this->createVkImageForBackendSurface(vkFormat, dimensions, sampleCnt, GrTexturable::kNo,
                                              GrRenderable::kYes, GrMipmapped::kNo, &info,
                                              isProtected)) {
        return {};
    }
    return GrBackendRenderTarget(dimensions.width(), dimensions.height(), 0, info);
}

void GrVkGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kVulkan == rt.fBackend);

    GrVkImageInfo info;
    if (rt.getVkImageInfo(&info)) {
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
        SkSurface::BackendSurfaceAccess access,
        const GrBackendSurfaceMutableState* newState) {
    // Submit the current command buffer to the Queue. Whether we inserted semaphores or not does
    // not effect what we do here.
    if (!proxies.empty() && (access == SkSurface::BackendSurfaceAccess::kPresent || newState)) {
        // We currently don't support passing in new surface state for multiple proxies here. The
        // only time we have multiple proxies is if we are flushing a yuv SkImage which won't have
        // state updates anyways. Additionally if we have a newState than we must not have any
        // BackendSurfaceAccess.
        SkASSERT(!newState || proxies.count() == 1);
        SkASSERT(!newState || access == SkSurface::BackendSurfaceAccess::kNoAccess);
        GrVkImage* image;
        for (GrSurfaceProxy* proxy : proxies) {
            SkASSERT(proxy->isInstantiated());
            if (GrTexture* tex = proxy->peekTexture()) {
                image = static_cast<GrVkTexture*>(tex)->textureAttachment();
            } else {
                GrRenderTarget* rt = proxy->peekRenderTarget();
                SkASSERT(rt);
                GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
                image = vkRT->externalAttachment();
            }
            if (newState) {
                const GrVkSharedImageInfo& newInfo = newState->fVkState;
                set_layout_and_queue_from_mutable_state(this, image, newInfo);
            } else {
                SkASSERT(access == SkSurface::BackendSurfaceAccess::kPresent);
                image->prepareForPresent(this);
            }
        }
    }
}

void GrVkGpu::addFinishedProc(GrGpuFinishedProc finishedProc,
                              GrGpuFinishedContext finishedContext) {
    SkASSERT(finishedProc);
    this->addFinishedCallback(GrRefCntedCallback::Make(finishedProc, finishedContext));
}

void GrVkGpu::addFinishedCallback(sk_sp<GrRefCntedCallback> finishedCallback) {
    SkASSERT(finishedCallback);
    fResourceProvider.addFinishedProcToActiveCommandBuffers(std::move(finishedCallback));
}

void GrVkGpu::takeOwnershipOfBuffer(sk_sp<GrGpuBuffer> buffer) {
    this->currentCommandBuffer()->addGrBuffer(std::move(buffer));
}

bool GrVkGpu::onSubmitToGpu(bool syncCpu) {
    if (syncCpu) {
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
    uint64_t allocatedMemory = fMemoryAllocator->totalAllocatedMemory();
    uint64_t usedMemory = fMemoryAllocator->totalUsedMemory();
    SkASSERT(usedMemory <= allocatedMemory);
    if (allocatedMemory > 0) {
        SK_HISTOGRAM_PERCENTAGE("VulkanMemoryAllocator.PercentUsed",
                                (usedMemory * 100) / allocatedMemory);
    }
    // allocatedMemory is in bytes and need to be reported it in kilobytes. SK_HISTOGRAM_MEMORY_KB
    // supports samples up to around 500MB which should support the amounts of memory we allocate.
    SK_HISTOGRAM_MEMORY_KB("VulkanMemoryAllocator.AmountAllocated", allocatedMemory >> 10);
#endif
}

void GrVkGpu::copySurfaceAsCopyImage(GrSurface* dst, GrSurface* src, GrVkImage* dstImage,
                                     GrVkImage* srcImage, const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    if (!this->currentCommandBuffer()) {
        return;
    }

#ifdef SK_DEBUG
    int dstSampleCnt = dstImage->vkImageInfo().fSampleCount;
    int srcSampleCnt = srcImage->vkImageInfo().fSampleCount;
    bool dstHasYcbcr = dstImage->ycbcrConversionInfo().isValid();
    bool srcHasYcbcr = srcImage->ycbcrConversionInfo().isValid();
    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat;
    SkAssertResult(dst->backendFormat().asVkFormat(&srcFormat));
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

void GrVkGpu::copySurfaceAsBlit(GrSurface* dst, GrSurface* src, GrVkImage* dstImage,
                                GrVkImage* srcImage, const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    if (!this->currentCommandBuffer()) {
        return;
    }

#ifdef SK_DEBUG
    int dstSampleCnt = dstImage->vkImageInfo().fSampleCount;
    int srcSampleCnt = srcImage->vkImageInfo().fSampleCount;
    bool dstHasYcbcr = dstImage->ycbcrConversionInfo().isValid();
    bool srcHasYcbcr = srcImage->ycbcrConversionInfo().isValid();
    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat;
    SkAssertResult(dst->backendFormat().asVkFormat(&srcFormat));
    SkASSERT(this->vkCaps().canCopyAsBlit(dstFormat, dstSampleCnt, dstImage->isLinearTiled(),
                                          dstHasYcbcr, srcFormat, srcSampleCnt,
                                          srcImage->isLinearTiled(), srcHasYcbcr));

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

    // Flip rect if necessary
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY, srcRect.width(),
                                        srcRect.height());

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
                                            VK_FILTER_NEAREST); // We never scale so any filter works here

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

bool GrVkGpu::onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
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

    bool useDiscardableMSAA = this->vkCaps().preferDiscardableMSAAAttachment();

    GrVkImage* dstImage;
    GrVkImage* srcImage;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(dstRT);
        if (vkRT->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        if (useDiscardableMSAA && vkRT->resolveAttachment() &&
            vkRT->resolveAttachment()->supportsInputAttachmentUsage()) {
            dstImage = vkRT->resolveAttachment();
        } else {
            dstImage = vkRT->colorAttachment();
        }
    } else if (dst->asTexture()) {
        dstImage = static_cast<GrVkTexture*>(dst->asTexture())->textureAttachment();
    } else {
        // The surface in a GrAttachment already
        dstImage = static_cast<GrVkAttachment*>(dst);
    }
    GrRenderTarget* srcRT = src->asRenderTarget();
    if (srcRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(srcRT);
        if (useDiscardableMSAA && vkRT->resolveAttachment() &&
            vkRT->resolveAttachment()->supportsInputAttachmentUsage()) {
            srcImage = vkRT->resolveAttachment();
        } else {
            srcImage = vkRT->colorAttachment();
        }
    } else if (src->asTexture()) {
        SkASSERT(src->asTexture());
        srcImage = static_cast<GrVkTexture*>(src->asTexture())->textureAttachment();
    } else {
        // The surface in a GrAttachment already
        srcImage = static_cast<GrVkAttachment*>(src);
    }

    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat = srcImage->imageFormat();

    int dstSampleCnt = dstImage->vkImageInfo().fSampleCount;
    int srcSampleCnt = srcImage->vkImageInfo().fSampleCount;

    bool dstHasYcbcr = dstImage->ycbcrConversionInfo().isValid();
    bool srcHasYcbcr = srcImage->ycbcrConversionInfo().isValid();

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

    if (this->vkCaps().canCopyAsBlit(dstFormat, dstSampleCnt, dstImage->isLinearTiled(),
                                     dstHasYcbcr, srcFormat, srcSampleCnt,
                                     srcImage->isLinearTiled(), srcHasYcbcr)) {
        this->copySurfaceAsBlit(dst, src, dstImage, srcImage, srcRect, dstPoint);
        return true;
    }

    return false;
}

bool GrVkGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                           GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                           size_t rowBytes) {
    if (surface->isProtected()) {
        return false;
    }

    if (surfaceColorType != dstColorType) {
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
        image = static_cast<GrVkTexture*>(surface->asTexture())->textureAttachment();
    }

    if (!image) {
        return false;
    }

    // Skia's RGB_888x color type, which we map to the vulkan R8G8B8_UNORM, expects the data to be
    // 32 bits, but the Vulkan format is only 24. So we first copy the surface into an R8G8B8A8
    // image and then do the read pixels from that.
    sk_sp<GrVkAttachment> copySurface;
    if (dstColorType == GrColorType::kRGB_888x && image->imageFormat() == VK_FORMAT_R8G8B8_UNORM) {
        int srcSampleCount = 0;
        if (rt) {
            srcSampleCount = rt->numSamples();
        }
        bool srcHasYcbcr = image->ycbcrConversionInfo().isValid();
        if (!this->vkCaps().canCopyAsBlit(VK_FORMAT_R8G8B8A8_UNORM, 1, false, false,
                                          image->imageFormat(), srcSampleCount,
                                          image->isLinearTiled(), srcHasYcbcr)) {
            return false;
        }

        // Make a new surface that is RGBA to copy the RGB surface into.
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        GrVkImage::ImageDesc imageDesc;
        imageDesc.fImageType = VK_IMAGE_TYPE_2D;
        imageDesc.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
        imageDesc.fWidth = width;
        imageDesc.fHeight = height;
        imageDesc.fLevels = 1;
        imageDesc.fSamples = 1;
        imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        imageDesc.fUsageFlags = usageFlags;
        imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        copySurface =
                GrVkAttachment::MakeTexture(this, {width, height}, VK_FORMAT_R8G8B8A8_UNORM,
                                            /*mipLevels=*/1, GrRenderable::kYes, /*numSamples=*/1,
                                            SkBudgeted::kYes, GrProtected::kNo);
        if (!copySurface) {
            return false;
        }

        SkIRect srcRect = SkIRect::MakeXYWH(left, top, width, height);
        SkAssertResult(this->copySurface(copySurface.get(), surface, srcRect, SkIPoint::Make(0,0)));

        top = 0;
        left = 0;
        dstColorType = GrColorType::kRGBA_8888;
        image = copySurface.get();
    }

    // Change layout of our target so it can be used as copy
    image->setImageLayout(this,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_ACCESS_TRANSFER_READ_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          false);

    size_t bpp = GrColorTypeBytesPerPixel(dstColorType);
    if (GrVkFormatBytesPerBlock(image->imageFormat()) != bpp) {
        return false;
    }
    size_t tightRowBytes = bpp * width;

    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    VkOffset3D offset = { left, top, 0 };
    region.imageOffset = offset;
    region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

    size_t transBufferRowBytes = bpp * region.imageExtent.width;
    size_t imageRows = region.imageExtent.height;
    GrResourceProvider* resourceProvider = this->getContext()->priv().resourceProvider();
    sk_sp<GrGpuBuffer> transferBuffer = resourceProvider->createBuffer(
            transBufferRowBytes * imageRows, GrGpuBufferType::kXferGpuToCpu,
            kDynamic_GrAccessPattern);

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

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, tightRowBytes, height);

    transferBuffer->unmap();
    return true;
}

bool GrVkGpu::beginRenderPass(const GrVkRenderPass* renderPass,
                              const VkClearValue* colorClear,
                              GrVkRenderTarget* target,
                              const SkIRect& renderPassBounds,
                              bool forSecondaryCB) {
    if (!this->currentCommandBuffer()) {
        return false;
    }
    SkASSERT (!target->wrapsSecondaryCommandBuffer());

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

   return this->currentCommandBuffer()->beginRenderPass(this, renderPass, clears, target,
                                                        renderPassBounds, forSecondaryCB);
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
            fDeviceIsLost = true;
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

GrFence SK_WARN_UNUSED_RESULT GrVkGpu::insertFence() {
    VkFenceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkFenceCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    VkFence fence = VK_NULL_HANDLE;
    VkResult result;

    VK_CALL_RET(result, CreateFence(this->device(), &createInfo, nullptr, &fence));
    if (result != VK_SUCCESS) {
        return 0;
    }
    VK_CALL_RET(result, QueueSubmit(this->queue(), 0, nullptr, fence));
    if (result != VK_SUCCESS) {
        VK_CALL(DestroyFence(this->device(), fence, nullptr));
        return 0;
    }

    static_assert(sizeof(GrFence) >= sizeof(VkFence));
    return (GrFence)fence;
}

bool GrVkGpu::waitFence(GrFence fence) {
    SkASSERT(VK_NULL_HANDLE != (VkFence)fence);

    VkResult result;
    VK_CALL_RET(result, WaitForFences(this->device(), 1, (VkFence*)&fence, VK_TRUE, 0));
    return (VK_SUCCESS == result);
}

void GrVkGpu::deleteFence(GrFence fence) const {
    VK_CALL(DestroyFence(this->device(), (VkFence)fence, nullptr));
}

std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT GrVkGpu::makeSemaphore(bool isOwned) {
    return GrVkSemaphore::Make(this, isOwned);
}

std::unique_ptr<GrSemaphore> GrVkGpu::wrapBackendSemaphore(
        const GrBackendSemaphore& semaphore,
        GrResourceProvider::SemaphoreWrapType wrapType,
        GrWrapOwnership ownership) {
    return GrVkSemaphore::MakeWrapped(this, semaphore.vkSemaphore(), wrapType, ownership);
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
    GrVkAttachment* vkTexture = static_cast<GrVkTexture*>(texture)->textureAttachment();
    vkTexture->setImageLayout(this,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT,
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              false);
    // TODO: should we have a way to notify the caller that this has failed? Currently if the submit
    // fails (caused by DEVICE_LOST) this will just cause us to fail the next use of the gpu.
    // Eventually we will abandon the whole GPU if this fails.
    this->submitToGpu(false);

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
