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
#include "include/private/SkTo.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrNativeRect.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/vk/GrVkAMDMemoryAllocator.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkCommandPool.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkIndexBuffer.h"
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
#include "src/gpu/vk/GrVkTransferBuffer.h"
#include "src/gpu/vk/GrVkVertexBuffer.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkSurface_Gpu.h"
#include "src/sksl/SkSLCompiler.h"

#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkTypes.h"

#include <utility>

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

#if defined(SK_BUILD_FOR_WIN) && defined(SK_DEBUG)
#include "src/core/SkLeanWindows.h"
#endif

#define VK_CALL(X) GR_VK_CALL(this->vkInterface(), X)
#define VK_CALL_RET(RET, X) GR_VK_CALL_RESULT(this, RET, X)

sk_sp<GrGpu> GrVkGpu::Make(const GrVkBackendContext& backendContext,
                           const GrContextOptions& options, GrContext* context) {
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

    instanceVersion = SkTMin(instanceVersion, apiVersion);
    physDevVersion = SkTMin(physDevVersion, apiVersion);

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

     sk_sp<GrVkGpu> vkGpu(new GrVkGpu(context, options, backendContext, interface,
                                       instanceVersion, physDevVersion));
     if (backendContext.fProtectedContext == GrProtected::kYes &&
         !vkGpu->vkCaps().supportsProtectedMemory()) {
         return nullptr;
     }
     return vkGpu;
}

////////////////////////////////////////////////////////////////////////////////

GrVkGpu::GrVkGpu(GrContext* context, const GrContextOptions& options,
                 const GrVkBackendContext& backendContext, sk_sp<const GrVkInterface> interface,
                 uint32_t instanceVersion, uint32_t physicalDeviceVersion)
        : INHERITED(context)
        , fInterface(std::move(interface))
        , fMemoryAllocator(backendContext.fMemoryAllocator)
        , fPhysicalDevice(backendContext.fPhysicalDevice)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fQueueIndex(backendContext.fGraphicsQueueIndex)
        , fResourceProvider(this)
        , fDisconnected(false)
        , fProtectedContext(backendContext.fProtectedContext) {
    SkASSERT(!backendContext.fOwnsInstanceAndDevice);

    if (!fMemoryAllocator) {
        // We were not given a memory allocator at creation
        fMemoryAllocator.reset(new GrVkAMDMemoryAllocator(backendContext.fPhysicalDevice,
                                                          fDevice, fInterface));
    }

    fCompiler = new SkSL::Compiler();

    if (backendContext.fDeviceFeatures2) {
        fVkCaps.reset(new GrVkCaps(options, this->vkInterface(), backendContext.fPhysicalDevice,
                                   *backendContext.fDeviceFeatures2, instanceVersion,
                                   physicalDeviceVersion,
                                   *backendContext.fVkExtensions, fProtectedContext));
    } else if (backendContext.fDeviceFeatures) {
        VkPhysicalDeviceFeatures2 features2;
        features2.pNext = nullptr;
        features2.features = *backendContext.fDeviceFeatures;
        fVkCaps.reset(new GrVkCaps(options, this->vkInterface(), backendContext.fPhysicalDevice,
                                   features2, instanceVersion, physicalDeviceVersion,
                                   *backendContext.fVkExtensions, fProtectedContext));
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
        fVkCaps.reset(new GrVkCaps(options, this->vkInterface(), backendContext.fPhysicalDevice,
                                   features, instanceVersion, physicalDeviceVersion, extensions,
                                   fProtectedContext));
    }
    fCaps.reset(SkRef(fVkCaps.get()));

    VK_CALL(GetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &fPhysDevProps));
    VK_CALL(GetPhysicalDeviceMemoryProperties(backendContext.fPhysicalDevice, &fPhysDevMemProps));

    fResourceProvider.init();

    fCmdPool = fResourceProvider.findOrCreateCommandPool();
    if (fCmdPool) {
        fCurrentCmdBuffer = fCmdPool->getPrimaryCommandBuffer();
        SkASSERT(fCurrentCmdBuffer);
        fCurrentCmdBuffer->begin(this);
    }
}

void GrVkGpu::destroyResources() {
    if (fCmdPool) {
        fCmdPool->getPrimaryCommandBuffer()->end(this);
        fCmdPool->close();
    }

    // wait for all commands to finish
    VkResult res = VK_CALL(QueueWaitIdle(fQueue));

    // On windows, sometimes calls to QueueWaitIdle return before actually signalling the fences
    // on the command buffers even though they have completed. This causes an assert to fire when
    // destroying the command buffers. Currently this ony seems to happen on windows, so we add a
    // sleep to make sure the fence signals.
#ifdef SK_DEBUG
    if (this->vkCaps().mustSleepOnTearDown()) {
#if defined(SK_BUILD_FOR_WIN)
        Sleep(10); // In milliseconds
#else
        sleep(1);  // In seconds
#endif
    }
#endif

#ifdef SK_DEBUG
    SkASSERT(VK_SUCCESS == res || VK_ERROR_DEVICE_LOST == res);
#endif

    if (fCmdPool) {
        fCmdPool->unref(this);
        fCmdPool = nullptr;
    }

    for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
        fSemaphoresToWaitOn[i]->unref(this);
    }
    fSemaphoresToWaitOn.reset();

    for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
        fSemaphoresToSignal[i]->unref(this);
    }
    fSemaphoresToSignal.reset();

    // must call this just before we destroy the command pool and VkDevice
    fResourceProvider.destroyResources(VK_ERROR_DEVICE_LOST == res);
}

GrVkGpu::~GrVkGpu() {
    if (!fDisconnected) {
        this->destroyResources();
        fMemoryAllocator.reset();
    }
    delete fCompiler;
}


void GrVkGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);
    if (!fDisconnected) {
        this->destroyResources();
        fMemoryAllocator.reset();

        fSemaphoresToWaitOn.reset();
        fSemaphoresToSignal.reset();
        fCurrentCmdBuffer = nullptr;
        fDisconnected = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

GrOpsRenderPass* GrVkGpu::getOpsRenderPass(
            GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    if (!fCachedOpsRenderPass) {
        fCachedOpsRenderPass.reset(new GrVkOpsRenderPass(this));
    }

    if (!fCachedOpsRenderPass->set(rt, origin, bounds, colorInfo, stencilInfo, sampledProxies)) {
        return nullptr;
    }
    return fCachedOpsRenderPass.get();
}

bool GrVkGpu::submitCommandBuffer(SyncQueue sync, GrGpuFinishedProc finishedProc,
                                  GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(fCurrentCmdBuffer);
    SkASSERT(!fCachedOpsRenderPass || !fCachedOpsRenderPass->isActive());

    if (!fCurrentCmdBuffer->hasWork() && kForce_SyncQueue != sync &&
        !fSemaphoresToSignal.count() && !fSemaphoresToWaitOn.count()) {
        SkASSERT(fDrawables.empty());
        fResourceProvider.checkCommandBuffers();
        if (finishedProc) {
            fResourceProvider.addFinishedProcToActiveCommandBuffers(finishedProc, finishedContext);
        }
        return true;
    }

    fCurrentCmdBuffer->end(this);
    SkASSERT(fCmdPool);
    fCmdPool->close();
    bool didSubmit = fCurrentCmdBuffer->submitToQueue(this, fQueue, fSemaphoresToSignal,
                                                      fSemaphoresToWaitOn);

    if (didSubmit && sync == kForce_SyncQueue) {
        fCurrentCmdBuffer->forceSync(this);
    }

    if (finishedProc) {
        // Make sure this is called after closing the current command pool
        fResourceProvider.addFinishedProcToActiveCommandBuffers(finishedProc, finishedContext);
    }

    // We must delete any drawables that had to wait until submit to destroy.
    fDrawables.reset();

    // If we didn't submit the command buffer then we did not wait on any semaphores. We will
    // continue to hold onto these semaphores and wait on them during the next command buffer
    // submission.
    if (didSubmit) {
        for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
            fSemaphoresToWaitOn[i]->unref(this);
        }
        fSemaphoresToWaitOn.reset();
    }

    // Even if we did not submit the command buffer, we drop all the signal semaphores since we will
    // not try to recover the work that wasn't submitted and instead just drop it all. The client
    // will be notified that the semaphores were not submit so that they will not try to wait on
    // them.
    for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
        fSemaphoresToSignal[i]->unref(this);
    }
    fSemaphoresToSignal.reset();

    // Release old command pool and create a new one
    fCmdPool->unref(this);
    fResourceProvider.checkCommandBuffers();
    fCmdPool = fResourceProvider.findOrCreateCommandPool();
    if (fCmdPool) {
        fCurrentCmdBuffer = fCmdPool->getPrimaryCommandBuffer();
        fCurrentCmdBuffer->begin(this);
    }
    return didSubmit;
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<GrGpuBuffer> GrVkGpu::onCreateBuffer(size_t size, GrGpuBufferType type,
                                           GrAccessPattern accessPattern, const void* data) {
    sk_sp<GrGpuBuffer> buff;
    switch (type) {
        case GrGpuBufferType::kVertex:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            buff = GrVkVertexBuffer::Make(this, size, kDynamic_GrAccessPattern == accessPattern);
            break;
        case GrGpuBufferType::kIndex:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            buff = GrVkIndexBuffer::Make(this, size, kDynamic_GrAccessPattern == accessPattern);
            break;
        case GrGpuBufferType::kXferCpuToGpu:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStream_GrAccessPattern == accessPattern);
            buff = GrVkTransferBuffer::Make(this, size, GrVkBuffer::kCopyRead_Type);
            break;
        case GrGpuBufferType::kXferGpuToCpu:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStream_GrAccessPattern == accessPattern);
            buff = GrVkTransferBuffer::Make(this, size, GrVkBuffer::kCopyWrite_Type);
            break;
        default:
            SK_ABORT("Unknown buffer type.");
    }
    if (data && buff) {
        buff->updateData(data, size);
    }
    return buff;
}

bool GrVkGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType srcColorType,
                            const GrMipLevel texels[], int mipLevelCount,
                            bool prepForTexSampling) {
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(surface->asTexture());
    if (!vkTex) {
        return false;
    }

    // Make sure we have at least the base level
    if (!mipLevelCount || !texels[0].fPixels) {
        return false;
    }

    SkASSERT(!GrVkFormatIsCompressed(vkTex->imageFormat()));
    bool success = false;
    bool linearTiling = vkTex->isLinearTiled();
    if (linearTiling) {
        if (mipLevelCount > 1) {
            SkDebugf("Can't upload mipmap data to linear tiled texture");
            return false;
        }
        if (VK_IMAGE_LAYOUT_PREINITIALIZED != vkTex->currentLayout()) {
            // Need to change the layout to general in order to perform a host write
            vkTex->setImageLayout(this,
                                  VK_IMAGE_LAYOUT_GENERAL,
                                  VK_ACCESS_HOST_WRITE_BIT,
                                  VK_PIPELINE_STAGE_HOST_BIT,
                                  false);
            if (!this->submitCommandBuffer(kForce_SyncQueue)) {
                return false;
            }
        }
        success = this->uploadTexDataLinear(vkTex, left, top, width, height, srcColorType,
                                            texels[0].fPixels, texels[0].fRowBytes);
    } else {
        SkASSERT(mipLevelCount <= vkTex->texturePriv().maxMipMapLevel() + 1);
        success = this->uploadTexDataOptimal(vkTex, left, top, width, height, srcColorType, texels,
                                             mipLevelCount);
    }

    if (prepForTexSampling) {
        vkTex->setImageLayout(this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              false);
    }

    return success;
}

bool GrVkGpu::onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                                 GrColorType surfaceColorType, GrColorType bufferColorType,
                                 GrGpuBuffer* transferBuffer, size_t bufferOffset,
                                 size_t rowBytes) {
    // Vulkan only supports offsets that are both 4-byte aligned and aligned to a pixel.
    if ((bufferOffset & 0x3) || (bufferOffset % GrColorTypeBytesPerPixel(bufferColorType))) {
        return false;
    }
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(texture);
    if (!vkTex) {
        return false;
    }

    // Can't transfer compressed data
    SkASSERT(!GrVkFormatIsCompressed(vkTex->imageFormat()));

    GrVkTransferBuffer* vkBuffer = static_cast<GrVkTransferBuffer*>(transferBuffer);
    if (!vkBuffer) {
        return false;
    }

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
        SkASSERT(bounds.contains(subRect));
    )
    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);

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

    // Copy the buffer to the image
    fCurrentCmdBuffer->copyBufferToImage(this,
                                         vkBuffer,
                                         vkTex,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         1,
                                         &region);

    vkTex->texturePriv().markMipMapsDirty();
    return true;
}

bool GrVkGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                   GrColorType surfaceColorType, GrColorType bufferColorType,
                                   GrGpuBuffer* transferBuffer, size_t offset) {
    SkASSERT(surface);
    SkASSERT(transferBuffer);
    if (fProtectedContext == GrProtected::kYes) {
        return false;
    }

    GrVkTransferBuffer* vkBuffer = static_cast<GrVkTransferBuffer*>(transferBuffer);

    GrVkImage* srcImage;
    if (GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(surface->asRenderTarget())) {
        // Reading from render targets that wrap a secondary command buffer is not allowed since
        // it would require us to know the VkImage, which we don't have, as well as need us to
        // stop and start the VkRenderPass which we don't have access to.
        if (rt->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        srcImage = rt;
    } else {
        srcImage = static_cast<GrVkTexture*>(surface->asTexture());
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

    fCurrentCmdBuffer->copyImageToBuffer(this, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         vkBuffer, 1, &region);

    // Make sure the copy to buffer has finished.
    vkBuffer->addMemoryBarrier(this,
                               VK_ACCESS_TRANSFER_WRITE_BIT,
                               VK_ACCESS_HOST_READ_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_HOST_BIT,
                               false);
    return true;
}

void GrVkGpu::resolveImage(GrSurface* dst, GrVkRenderTarget* src, const SkIRect& srcRect,
                           const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src && src->numSamples() > 1 && src->msaaImage());

    VkImageResolve resolveInfo;
    resolveInfo.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveInfo.srcOffset = {srcRect.fLeft, srcRect.fTop, 0};
    resolveInfo.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveInfo.dstOffset = {dstPoint.fX, dstPoint.fY, 0};
    resolveInfo.extent = {(uint32_t)srcRect.width(), (uint32_t)srcRect.height(), 1};

    GrVkImage* dstImage;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(dstRT);
        dstImage = vkRT;
    } else {
        SkASSERT(dst->asTexture());
        dstImage = static_cast<GrVkTexture*>(dst->asTexture());
    }
    dstImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             false);

    src->msaaImage()->setImageLayout(this,
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     VK_ACCESS_TRANSFER_READ_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     false);

    fCurrentCmdBuffer->resolveImage(this, *src->msaaImage(), *dstImage, 1, &resolveInfo);
}

void GrVkGpu::onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect,
                                    GrSurfaceOrigin resolveOrigin, ForExternalIO forExternalIO) {
    SkASSERT(target->numSamples() > 1);
    GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(target);
    SkASSERT(rt->msaaImage());

    auto nativeResolveRect = GrNativeRect::MakeRelativeTo(
            resolveOrigin, target->height(), resolveRect);
    this->resolveImage(target, rt, nativeResolveRect.asSkIRect(),
                       SkIPoint::Make(nativeResolveRect.fX, nativeResolveRect.fY));

    if (ForExternalIO::kYes == forExternalIO) {
        // This resolve is called when we are preparing an msaa surface for external I/O. It is
        // called after flushing, so we need to make sure we submit the command buffer after doing
        // the resolve so that the resolve actually happens.
        this->submitCommandBuffer(kSkip_SyncQueue);
    }
}

bool GrVkGpu::uploadTexDataLinear(GrVkTexture* tex, int left, int top, int width, int height,
                                  GrColorType dataColorType, const void* data, size_t rowBytes) {
    SkASSERT(data);
    SkASSERT(tex->isLinearTiled());

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(tex->width(), tex->height());
        SkASSERT(bounds.contains(subRect));
    )
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);
    size_t trimRowBytes = width * bpp;

    SkASSERT(VK_IMAGE_LAYOUT_PREINITIALIZED == tex->currentLayout() ||
             VK_IMAGE_LAYOUT_GENERAL == tex->currentLayout());
    const VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,  // mipLevel
        0,  // arraySlice
    };
    VkSubresourceLayout layout;

    const GrVkInterface* interface = this->vkInterface();

    GR_VK_CALL(interface, GetImageSubresourceLayout(fDevice,
                                                    tex->image(),
                                                    &subres,
                                                    &layout));

    const GrVkAlloc& alloc = tex->alloc();
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

bool GrVkGpu::uploadTexDataOptimal(GrVkTexture* tex, int left, int top, int width, int height,
                                   GrColorType dataColorType, const GrMipLevel texels[],
                                   int mipLevelCount) {
    SkASSERT(!tex->isLinearTiled());
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == tex->width() && height == tex->height()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(1 == mipLevelCount || mipLevelCount == (tex->texturePriv().maxMipMapLevel() + 1));

    if (width == 0 || height == 0) {
        return false;
    }

    if (GrPixelConfigToColorType(tex->config()) != dataColorType) {
        return false;
    }

    // For RGB_888x src data we are uploading it first to an RGBA texture and then copying it to the
    // dst RGB texture. Thus we do not upload mip levels for that.
    if (dataColorType == GrColorType::kRGB_888x && tex->imageFormat() == VK_FORMAT_R8G8B8_UNORM) {
        SkASSERT(tex->config() == kRGB_888_GrPixelConfig);
        // First check that we'll be able to do the copy to the to the R8G8B8 image in the end via a
        // blit or draw.
        if (!this->vkCaps().formatCanBeDstofBlit(VK_FORMAT_R8G8B8_UNORM, tex->isLinearTiled()) &&
            !this->vkCaps().isFormatRenderable(VK_FORMAT_R8G8B8_UNORM, 1)) {
            return false;
        }
        mipLevelCount = 1;
    }

    SkASSERT(this->vkCaps().isVkFormatTexturable(tex->imageFormat()));
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
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);

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

    // allocate buffer to hold our mip data
    sk_sp<GrVkTransferBuffer> transferBuffer =
            GrVkTransferBuffer::Make(this, combinedBufferSize, GrVkBuffer::kCopyRead_Type);
    if (!transferBuffer) {
        return false;
    }

    int uploadLeft = left;
    int uploadTop = top;
    GrVkTexture* uploadTexture = tex;
    // For uploading RGB_888x data to an R8G8B8_UNORM texture we must first upload the data to an
    // R8G8B8A8_UNORM image and then copy it.
    sk_sp<GrVkTexture> copyTexture;
    if (dataColorType == GrColorType::kRGB_888x && tex->imageFormat() == VK_FORMAT_R8G8B8_UNORM) {
        bool dstHasYcbcr = tex->ycbcrConversionInfo().isValid();
        if (!this->vkCaps().canCopyAsBlit(tex->imageFormat(), 1, false, dstHasYcbcr,
                                          VK_FORMAT_R8G8B8A8_UNORM, 1, false, false)) {
            return false;
        }
        GrSurfaceDesc surfDesc;
        surfDesc.fWidth = width;
        surfDesc.fHeight = height;
        surfDesc.fConfig = kRGBA_8888_GrPixelConfig;

        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
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

        copyTexture = GrVkTexture::MakeNewTexture(this, SkBudgeted::kYes, surfDesc, imageDesc,
                                                  GrMipMapsStatus::kNotAllocated);
        if (!copyTexture) {
            return false;
        }

        uploadTexture = copyTexture.get();
        uploadLeft = 0;
        uploadTop = 0;
    }

    char* buffer = (char*) transferBuffer->map();
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
            region.bufferOffset = transferBuffer->offset() + individualMipOffsets[currentMipLevel];
            region.bufferRowLength = currentWidth;
            region.bufferImageHeight = currentHeight;
            region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(currentMipLevel), 0, 1 };
            region.imageOffset = {uploadLeft, uploadTop, 0};
            region.imageExtent = { (uint32_t)currentWidth, (uint32_t)currentHeight, 1 };
        }
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
        layerHeight = currentHeight;
    }

    // no need to flush non-coherent memory, unmap will do that for us
    transferBuffer->unmap();

    // Change layout of our target so it can be copied to
    uploadTexture->setImageLayout(this,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  false);

    // Copy the buffer to the image
    fCurrentCmdBuffer->copyBufferToImage(this,
                                         transferBuffer.get(),
                                         uploadTexture,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         regions.count(),
                                         regions.begin());

    // If we copied the data into a temporary image first, copy that image into our main texture
    // now.
    if (copyTexture.get()) {
        SkASSERT(dataColorType == GrColorType::kRGB_888x);
        SkAssertResult(this->copySurface(tex, copyTexture.get(), SkIRect::MakeWH(width, height),
                                         SkIPoint::Make(left, top)));
    }
    if (1 == mipLevelCount) {
        tex->texturePriv().markMipMapsDirty();
    }

    return true;
}

// It's probably possible to roll this into uploadTexDataOptimal,
// but for now it's easier to maintain as a separate entity.
bool GrVkGpu::uploadTexDataCompressed(GrVkTexture* tex, int left, int top, int width, int height,
                                      SkImage::CompressionType compressionType, const void* data) {
    SkASSERT(data);
    SkASSERT(!tex->isLinearTiled());
    // For now the assumption is that our rect is the entire texture.
    // Compressed textures are read-only so this should be a reasonable assumption.
    SkASSERT(0 == left && 0 == top && width == tex->width() && height == tex->height());

    if (width == 0 || height == 0) {
        return false;
    }

    SkImage::CompressionType textureCompressionType;
    if (!GrVkFormatToCompressionType(tex->imageFormat(), &textureCompressionType) ||
        textureCompressionType != compressionType) {
        return false;
    }

    SkASSERT(this->vkCaps().isVkFormatTexturable(tex->imageFormat()));

    size_t dataSize = GrCompressedDataSize(compressionType, width, height);

    // allocate buffer to hold our mip data
    sk_sp<GrVkTransferBuffer> transferBuffer =
            GrVkTransferBuffer::Make(this, dataSize, GrVkBuffer::kCopyRead_Type);
    if (!transferBuffer) {
        return false;
    }

    int uploadLeft = left;
    int uploadTop = top;
    GrVkTexture* uploadTexture = tex;

    char* buffer = (char*)transferBuffer->map();

    memcpy(buffer, data, dataSize);

    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = transferBuffer->offset();
    region.bufferRowLength = width;
    region.bufferImageHeight = height;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { uploadLeft, uploadTop, 0 };
    region.imageExtent = { SkToU32(width), SkToU32(height), 1 };

    // no need to flush non-coherent memory, unmap will do that for us
    transferBuffer->unmap();

    // Change layout of our target so it can be copied to
    uploadTexture->setImageLayout(this,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  false);

    // Copy the buffer to the image
    fCurrentCmdBuffer->copyBufferToImage(this,
                                         transferBuffer.get(),
                                         uploadTexture,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         1,
                                         &region);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> GrVkGpu::onCreateTexture(const GrSurfaceDesc& desc,
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

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (renderable == GrRenderable::kYes) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    // For now we will set the VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT and
    // VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT on every texture since we do not know whether or not we
    // will be using this texture in some copy or not. Also this assumes, as is the current case,
    // that all render targets in vulkan are also textures. If we change this practice of setting
    // both bits, we must make sure to set the destination bit if we are uploading srcData to the
    // texture.
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // This ImageDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this ImageDesc describes the resolved texture. Therefore we always have samples set
    // to 1.
    SkASSERT(mipLevelCount > 0);
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = pixelFormat;
    imageDesc.fWidth = desc.fWidth;
    imageDesc.fHeight = desc.fHeight;
    imageDesc.fLevels = mipLevelCount;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fIsProtected = isProtected;

    GrMipMapsStatus mipMapsStatus =
            mipLevelCount > 1 ? GrMipMapsStatus::kDirty : GrMipMapsStatus::kNotAllocated;

    sk_sp<GrVkTexture> tex;
    if (renderable == GrRenderable::kYes) {
        tex = GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
                this, budgeted, desc, renderTargetSampleCnt, imageDesc, mipMapsStatus);
    } else {
        tex = GrVkTexture::MakeNewTexture(this, budgeted, desc, imageDesc, mipMapsStatus);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        SkSTArray<1, VkImageSubresourceRange> ranges;
        bool inRange = false;
        for (uint32_t i = 0; i < tex->mipLevels(); ++i) {
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
        tex->setImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);
        this->currentCommandBuffer()->clearColorImage(this, tex.get(), &kZeroClearColor,
                                                      ranges.count(), ranges.begin());
    }
    return tex;
}

sk_sp<GrTexture> GrVkGpu::onCreateCompressedTexture(int width, int height,
                                                    const GrBackendFormat& format,
                                                    SkImage::CompressionType compressionType,
                                                    SkBudgeted budgeted, const void* data) {
    VkFormat pixelFormat;
    if (!format.asVkFormat(&pixelFormat)) {
        return nullptr;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;

    // For now we will set the VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT and
    // VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT on every texture since we do not know whether or not we
    // will be using this texture in some copy or not. Also this assumes, as is the current case,
    // that all render targets in vulkan are also textures. If we change this practice of setting
    // both bits, we must make sure to set the destination bit if we are uploading srcData to the
    // texture.
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // Compressed textures with MIP levels or multiple samples are not supported as of now.
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = pixelFormat;
    imageDesc.fWidth = width;
    imageDesc.fHeight = height;
    imageDesc.fLevels = 1;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fIsProtected = GrProtected::kNo;

    GrSurfaceDesc desc;
    desc.fConfig = GrCompressionTypeToPixelConfig(compressionType);
    desc.fWidth = width;
    desc.fHeight = height;
    auto tex = GrVkTexture::MakeNewTexture(this, budgeted, desc, imageDesc,
                                           GrMipMapsStatus::kNotAllocated);
    if (!tex) {
        return nullptr;
    }

    if (!this->uploadTexDataCompressed(tex.get(), 0, 0, desc.fWidth, desc.fHeight, compressionType,
                                       data)) {
        return nullptr;
    }

    return tex;
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpu::copyBuffer(GrVkBuffer* srcBuffer, GrVkBuffer* dstBuffer, VkDeviceSize srcOffset,
                         VkDeviceSize dstOffset, VkDeviceSize size) {
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    fCurrentCmdBuffer->copyBuffer(this, srcBuffer, dstBuffer, 1, &copyRegion);
}

bool GrVkGpu::updateBuffer(GrVkBuffer* buffer, const void* src,
                           VkDeviceSize offset, VkDeviceSize size) {
    // Update the buffer
    fCurrentCmdBuffer->updateBuffer(this, buffer, offset, size, src);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool check_image_info(const GrVkCaps& caps,
                             const GrVkImageInfo& info,
                             GrColorType colorType,
                             bool needsAllocation) {
    if (VK_NULL_HANDLE == info.fImage) {
        return false;
    }

    if (VK_NULL_HANDLE == info.fAlloc.fMemory && needsAllocation) {
        return false;
    }

    if (info.fImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && !caps.supportsSwapchain()) {
        return false;
    }

    if (info.fYcbcrConversionInfo.isValid()) {
        if (!caps.supportsYcbcrConversion()) {
            return false;
        }
        if (info.fYcbcrConversionInfo.fExternalFormat != 0) {
            return true;
        }
    }

    SkASSERT(GrVkFormatColorTypePairIsValid(info.fFormat, colorType));
    return true;
}

static bool check_tex_image_info(const GrVkCaps& caps, const GrVkImageInfo& info) {
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
    return true;
}

static bool check_rt_image_info(const GrVkCaps& caps, const GrVkImageInfo& info, int sampleCnt) {
    if (!caps.isFormatRenderable(info.fFormat, sampleCnt)) {
        return false;
    }
    return true;
}

sk_sp<GrTexture> GrVkGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                               GrColorType colorType, GrWrapOwnership ownership,
                                               GrWrapCacheable cacheable, GrIOType ioType) {
    GrVkImageInfo imageInfo;
    if (!backendTex.getVkImageInfo(&imageInfo)) {
        return nullptr;
    }

    if (!check_image_info(this->vkCaps(), imageInfo, colorType,
                          kAdopt_GrWrapOwnership == ownership)) {
        return nullptr;
    }
    if (!check_tex_image_info(this->vkCaps(), imageInfo)) {
        return nullptr;
    }

    if (backendTex.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                    colorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc surfDesc;
    surfDesc.fWidth = backendTex.width();
    surfDesc.fHeight = backendTex.height();
    surfDesc.fConfig = config;

    sk_sp<GrVkImageLayout> layout = backendTex.getGrVkImageLayout();
    SkASSERT(layout);
    return GrVkTexture::MakeWrappedTexture(this, surfDesc, ownership, cacheable, ioType, imageInfo,
                                           std::move(layout));
}

sk_sp<GrTexture> GrVkGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         int sampleCnt,
                                                         GrColorType colorType,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    GrVkImageInfo imageInfo;
    if (!backendTex.getVkImageInfo(&imageInfo)) {
        return nullptr;
    }

    if (!check_image_info(this->vkCaps(), imageInfo, colorType,
                          kAdopt_GrWrapOwnership == ownership)) {
        return nullptr;
    }
    if (!check_tex_image_info(this->vkCaps(), imageInfo)) {
        return nullptr;
    }
    if (!check_rt_image_info(this->vkCaps(), imageInfo, sampleCnt)) {
        return nullptr;
    }

    if (backendTex.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }


    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                    colorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc surfDesc;
    surfDesc.fWidth = backendTex.width();
    surfDesc.fHeight = backendTex.height();
    surfDesc.fConfig = config;
    sampleCnt = this->vkCaps().getRenderTargetSampleCount(sampleCnt, imageInfo.fFormat);

    sk_sp<GrVkImageLayout> layout = backendTex.getGrVkImageLayout();
    SkASSERT(layout);

    return GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(
            this, surfDesc, sampleCnt, ownership, cacheable, imageInfo, std::move(layout));
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT,
                                                         GrColorType colorType) {
    // Currently the Vulkan backend does not support wrapping of msaa render targets directly. In
    // general this is not an issue since swapchain images in vulkan are never multisampled. Thus if
    // you want a multisampled RT it is best to wrap the swapchain images and then let Skia handle
    // creating and owning the MSAA images.
    if (backendRT.sampleCnt() > 1) {
        return nullptr;
    }

    GrVkImageInfo info;
    if (!backendRT.getVkImageInfo(&info)) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendRT.getBackendFormat(),
                                                                    colorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    if (!check_image_info(this->vkCaps(), info, colorType, false)) {
        return nullptr;
    }
    if (!check_rt_image_info(this->vkCaps(), info, backendRT.sampleCnt())) {
        return nullptr;
    }

    if (backendRT.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = backendRT.width();
    desc.fHeight = backendRT.height();
    desc.fConfig = config;

    sk_sp<GrVkImageLayout> layout = backendRT.getGrVkImageLayout();

    sk_sp<GrVkRenderTarget> tgt =
            GrVkRenderTarget::MakeWrappedRenderTarget(this, desc, 1, info, std::move(layout));

    // We don't allow the client to supply a premade stencil buffer. We always create one if needed.
    SkASSERT(!backendRT.stencilBits());
    if (tgt) {
        SkASSERT(tgt->canAttemptStencilAttachment());
    }

    return tgt;
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                  int sampleCnt,
                                                                  GrColorType grColorType) {

    GrVkImageInfo imageInfo;
    if (!tex.getVkImageInfo(&imageInfo)) {
        return nullptr;
    }
    if (!check_image_info(this->vkCaps(), imageInfo, grColorType, false)) {
        return nullptr;
    }
    if (!check_rt_image_info(this->vkCaps(), imageInfo, sampleCnt)) {
        return nullptr;
    }

    if (tex.isProtected() && (fProtectedContext == GrProtected::kNo)) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(tex.getBackendFormat(),
                                                                    grColorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = config;

    sampleCnt = this->vkCaps().getRenderTargetSampleCount(sampleCnt, imageInfo.fFormat);
    if (!sampleCnt) {
        return nullptr;
    }

    sk_sp<GrVkImageLayout> layout = tex.getGrVkImageLayout();
    SkASSERT(layout);

    return GrVkRenderTarget::MakeWrappedRenderTarget(this, desc, sampleCnt, imageInfo,
                                                     std::move(layout));
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

    GrColorType grColorType = SkColorTypeToGrColorType(imageInfo.colorType());
    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendFormat, grColorType);
    if (config == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = imageInfo.width();
    desc.fHeight = imageInfo.height();
    desc.fConfig = config;

    return GrVkRenderTarget::MakeSecondaryCBRenderTarget(this, desc, vkInfo);
}

bool GrVkGpu::onRegenerateMipMapLevels(GrTexture* tex) {
    auto* vkTex = static_cast<GrVkTexture*>(tex);
    // don't do anything for linearly tiled textures (can't have mipmaps)
    if (vkTex->isLinearTiled()) {
        SkDebugf("Trying to create mipmap for linear tiled texture");
        return false;
    }
    SkASSERT(tex->texturePriv().textureType() == GrTextureType::k2D);

    // determine if we can blit to and from this format
    const GrVkCaps& caps = this->vkCaps();
    if (!caps.formatCanBeDstofBlit(vkTex->imageFormat(), false) ||
        !caps.formatCanBeSrcofBlit(vkTex->imageFormat(), false) ||
        !caps.mipMapSupport()) {
        return false;
    }

    int width = tex->width();
    int height = tex->height();
    VkImageBlit blitRegion;
    memset(&blitRegion, 0, sizeof(VkImageBlit));

    // SkMipMap doesn't include the base level in the level count so we have to add 1
    uint32_t levelCount = SkMipMap::ComputeLevelCount(tex->width(), tex->height()) + 1;
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
        width = SkTMax(1, width / 2);
        height = SkTMax(1, height / 2);

        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel - 1;
        this->addImageMemoryBarrier(vkTex->resource(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, false, &imageMemoryBarrier);

        blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel - 1, 0, 1 };
        blitRegion.srcOffsets[0] = { 0, 0, 0 };
        blitRegion.srcOffsets[1] = { prevWidth, prevHeight, 1 };
        blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, 0, 1 };
        blitRegion.dstOffsets[0] = { 0, 0, 0 };
        blitRegion.dstOffsets[1] = { width, height, 1 };
        fCurrentCmdBuffer->blitImage(this,
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

GrStencilAttachment* GrVkGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    SkASSERT(numStencilSamples == rt->numSamples() || this->caps()->mixedSamplesSupport());
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    const GrVkCaps::StencilFormat& sFmt = this->vkCaps().preferredStencilFormat();

    GrVkStencilAttachment* stencil(GrVkStencilAttachment::Create(this,
                                                                 width,
                                                                 height,
                                                                 numStencilSamples,
                                                                 sFmt));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

////////////////////////////////////////////////////////////////////////////////

bool copy_src_data(GrVkGpu* gpu, const GrVkAlloc& alloc, VkFormat vkFormat,
                   const SkTArray<size_t>& individualMipOffsets,
                   const SkPixmap srcData[], int numMipLevels) {
    SkASSERT(srcData && numMipLevels);
    SkASSERT(!GrVkFormatIsCompressed(vkFormat));
    SkASSERT(individualMipOffsets.count() == numMipLevels);

    char* mapPtr = (char*) GrVkMemory::MapAlloc(gpu, alloc);
    if (!mapPtr) {
        return false;
    }
    size_t bytesPerPixel = gpu->vkCaps().bytesPerPixel(vkFormat);

    for (int level = 0; level < numMipLevels; ++level) {
        const size_t trimRB = srcData[level].width() * bytesPerPixel;
        SkASSERT(individualMipOffsets[level] + trimRB * srcData[level].height() <= alloc.fSize);

        SkRectMemcpy(mapPtr + individualMipOffsets[level], trimRB,
                     srcData[level].addr(), srcData[level].rowBytes(),
                     trimRB, srcData[level].height());
    }

    GrVkMemory::FlushMappedAlloc(gpu, alloc, 0, alloc.fSize);
    GrVkMemory::UnmapAlloc(gpu, alloc);
    return true;
}

static void set_image_layout(const GrVkInterface* vkInterface, VkCommandBuffer cmdBuffer,
                             GrVkImageInfo* info, VkImageLayout newLayout, uint32_t mipLevels,
                             VkAccessFlags dstAccessMask, VkPipelineStageFlagBits dstStageMask) {
    VkAccessFlags srcAccessMask = GrVkImage::LayoutToSrcAccessMask(info->fImageLayout);
    VkPipelineStageFlags srcStageMask = GrVkImage::LayoutToPipelineSrcStageFlags(
                                                                              info->fImageLayout);

    VkImageMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(VkImageMemoryBarrier));
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = info->fImageLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = info->fImage;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1};
    GR_VK_CALL(vkInterface, CmdPipelineBarrier(cmdBuffer,
                                               srcStageMask,
                                               dstStageMask,
                                               0,
                                               0, nullptr,
                                               0, nullptr,
                                               1, &barrier));
    info->fImageLayout = newLayout;
}

bool GrVkGpu::createVkImageForBackendSurface(VkFormat vkFormat,
                                             SkISize dimensions,
                                             bool texturable,
                                             bool renderable,
                                             const BackendTextureData* data,
                                             int numMipLevels,
                                             GrVkImageInfo* info,
                                             GrProtected isProtected) {
    if (!fCmdPool) {
        return false;
    }
    SkASSERT(texturable || renderable);
    if (!texturable) {
        SkASSERT(!data && numMipLevels == 1);
    }

    // Compressed formats go through onCreateCompressedBackendTexture
    SkASSERT(!GrVkFormatIsCompressed(vkFormat));

    if (fProtectedContext != isProtected) {
        return false;
    }

    if (texturable && !fVkCaps->isVkFormatTexturable(vkFormat)) {
        return false;
    }

    if (renderable && !fVkCaps->isFormatRenderable(vkFormat, 1)) {
        return false;
    }

    VkImageUsageFlags usageFlags = 0;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texturable) {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (renderable) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = vkFormat;
    imageDesc.fWidth = dimensions.width();
    imageDesc.fHeight = dimensions.height();
    imageDesc.fLevels = numMipLevels;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    imageDesc.fIsProtected = fProtectedContext;

    if (!GrVkImage::InitImageInfo(this, imageDesc, info)) {
        SkDebugf("Failed to init image info\n");
        return false;
    }

    if (!data) {
        return true;
    }

    // We need to declare these early so that we can delete them at the end outside of
    // the if block.
    GrVkAlloc bufferAlloc;
    VkBuffer buffer = VK_NULL_HANDLE;

    VkResult err;
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        fCmdPool->vkCommandPool(),                        // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer cmdBuffer;
    VK_CALL_RET(err, AllocateCommandBuffers(fDevice, &cmdInfo, &cmdBuffer));
    if (err) {
        GrVkImage::DestroyImageInfo(this, info);
        return false;
    }

    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    VK_CALL_RET(err, BeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
    if (err) {
        VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
        GrVkImage::DestroyImageInfo(this, info);
        return false;
    }

    // Set image layout and add barrier
    set_image_layout(this->vkInterface(), cmdBuffer, info, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     numMipLevels, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    if (data->type() == BackendTextureData::Type::kPixmaps) {
        size_t bytesPerPixel = fVkCaps->bytesPerPixel(vkFormat);
        SkASSERT(!dimensions.isEmpty());

        SkTArray<size_t> individualMipOffsets(numMipLevels);

        size_t combinedBufferSize = GrComputeTightCombinedBufferSize(
                bytesPerPixel, dimensions, &individualMipOffsets, numMipLevels);

        VkBufferCreateInfo bufInfo;
        memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.flags = fProtectedContext == GrProtected::kYes ? VK_BUFFER_CREATE_PROTECTED_BIT : 0;
        bufInfo.size = combinedBufferSize;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufInfo.queueFamilyIndexCount = 0;
        bufInfo.pQueueFamilyIndices = nullptr;
        VK_CALL_RET(err, CreateBuffer(fDevice, &bufInfo, nullptr, &buffer));

        if (err) {
            GrVkImage::DestroyImageInfo(this, info);
            VK_CALL(EndCommandBuffer(cmdBuffer));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
            return false;
        }

        if (!GrVkMemory::AllocAndBindBufferMemory(this, buffer, GrVkBuffer::kCopyRead_Type, true,
                                                  &bufferAlloc)) {
            GrVkImage::DestroyImageInfo(this, info);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            VK_CALL(EndCommandBuffer(cmdBuffer));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
            return false;
        }

        bool result = copy_src_data(this, bufferAlloc, vkFormat, individualMipOffsets,
                                    data->pixmaps(), numMipLevels);
        if (!result) {
            GrVkImage::DestroyImageInfo(this, info);
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            VK_CALL(EndCommandBuffer(cmdBuffer));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
            return false;
        }

        SkTArray<VkBufferImageCopy> regions(numMipLevels);

        SkISize levelDimensions = dimensions;
        for (int i = 0; i < numMipLevels; ++i) {
            // Submit copy command
            VkBufferImageCopy& region = regions.push_back();
            memset(&region, 0, sizeof(VkBufferImageCopy));
            region.bufferOffset = individualMipOffsets[i];
            region.bufferRowLength = levelDimensions.width();
            region.bufferImageHeight = levelDimensions.height();
            region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(i), 0, 1};
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {SkToU32(levelDimensions.width()),
                                  SkToU32(levelDimensions.height()), 1};

            levelDimensions = {SkTMax(1, levelDimensions.width() /2),
                               SkTMax(1, levelDimensions.height()/2)};
        }

        VK_CALL(CmdCopyBufferToImage(cmdBuffer, buffer, info->fImage, info->fImageLayout,
                                     regions.count(), regions.begin()));
    } else {
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
        range.levelCount = numMipLevels;
        VK_CALL(CmdClearColorImage(cmdBuffer, info->fImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   &vkColor, 1, &range));
    }

    if (data->type() == BackendTextureData::Type::kColor && renderable) {
        // Change image layout to color-attachment-optimal since if we use this texture as a
        // borrowed texture within Ganesh we are probably going to render to it
        set_image_layout(this->vkInterface(), cmdBuffer, info,
                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, numMipLevels,
                         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    } else if (texturable) {
        // Change image layout to shader read since if we use this texture as a borrowed
        // texture within Ganesh we require that its layout be set to that
        set_image_layout(this->vkInterface(), cmdBuffer, info,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, numMipLevels,
                         VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    // End CommandBuffer
    VK_CALL_RET(err, EndCommandBuffer(cmdBuffer));
    if (err) {
        GrVkImage::DestroyImageInfo(this, info);
        if (buffer != VK_NULL_HANDLE) { // workaround for an older NVidia driver crash
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
        }
        VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
        return false;
    }

    // Create Fence for queue
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = 0;
    VkFence fence = VK_NULL_HANDLE;

    VK_CALL_RET(err, CreateFence(fDevice, &fenceInfo, nullptr, &fence));
    if (err) {
        GrVkImage::DestroyImageInfo(this, info);
        if (buffer != VK_NULL_HANDLE) { // workaround for an older NVidia driver crash
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
        }
        VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
        return false;
    }

    VkProtectedSubmitInfo protectedSubmitInfo;
    if (fProtectedContext == GrProtected::kYes) {
        memset(&protectedSubmitInfo, 0, sizeof(VkProtectedSubmitInfo));
        protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext = nullptr;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = fProtectedContext == GrProtected::kYes ? &protectedSubmitInfo : nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    VK_CALL_RET(err, QueueSubmit(this->queue(), 1, &submitInfo, fence));

    if (!err) {
        VK_CALL_RET(err, WaitForFences(this->device(), 1, &fence, VK_TRUE, UINT64_MAX));
    }
    if (VK_SUCCESS != err) {
        GrVkImage::DestroyImageInfo(this, info);
        if (buffer != VK_NULL_HANDLE) { // workaround for an older NVidia driver crash
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
        }
        VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
        VK_CALL(DestroyFence(this->device(), fence, nullptr));
        if (VK_TIMEOUT == err) {
            SkDebugf("Fence failed to signal: %d\n", err);
            SK_ABORT("failing");
        } else {
            SkDebugf("Queue submit or fence wait failed: %d\n", err);
            return false;
        }
    }

    // Clean up transfer resources
    if (buffer != VK_NULL_HANDLE) { // workaround for an older NVidia driver crash
        GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
        VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
    }
    VK_CALL(FreeCommandBuffers(fDevice, fCmdPool->vkCommandPool(), 1, &cmdBuffer));
    VK_CALL(DestroyFence(this->device(), fence, nullptr));

    return true;
}

GrBackendTexture GrVkGpu::onCreateBackendTexture(SkISize dimensions,
                                                 const GrBackendFormat& format,
                                                 GrRenderable renderable,
                                                 const BackendTextureData* data,
                                                 int numMipLevels,
                                                 GrProtected isProtected) {
    this->handleDirtyContext();

    const GrVkCaps& caps = this->vkCaps();

    if (fProtectedContext != isProtected) {
        return GrBackendTexture();
    }

    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        SkDebugf("Could net get vkformat\n");
        return GrBackendTexture();
    }

    // TODO: move the texturability check up to GrGpu::createBackendTexture and just assert here
    if (!caps.isVkFormatTexturable(vkFormat)) {
        SkDebugf("Config is not texturable\n");
        return GrBackendTexture();
    }

    if (GrVkFormatNeedsYcbcrSampler(vkFormat)) {
        SkDebugf("Can't create BackendTexture that requires Ycbcb sampler.\n");
        return GrBackendTexture();
    }

    GrVkImageInfo info;
    if (!this->createVkImageForBackendSurface(vkFormat, dimensions, true,
                                              GrRenderable::kYes == renderable, data, numMipLevels,
                                              &info, isProtected)) {
        SkDebugf("Failed to create testing only image\n");
        return GrBackendTexture();
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
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
        if (auto* stencil = renderTarget->renderTargetPriv().getStencilAttachment()) {
            numSamples = stencil->numSamples();
        }
    }
    SkASSERT(numSamples > 1);
    SkASSERT(!renderTarget->renderTargetPriv().getStencilAttachment() ||
             numSamples == renderTarget->renderTargetPriv().getStencilAttachment()->numSamples());

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

void GrVkGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kVulkan == tex.fBackend);

    GrVkImageInfo info;
    if (tex.getVkImageInfo(&info)) {
        GrVkImage::DestroyImageInfo(this, const_cast<GrVkImageInfo*>(&info));
    }
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

GrBackendRenderTarget GrVkGpu::createTestingOnlyBackendRenderTarget(int w, int h, GrColorType ct) {
    this->handleDirtyContext();

    if (w > this->caps()->maxRenderTargetSize() || h > this->caps()->maxRenderTargetSize()) {
        return GrBackendRenderTarget();
    }

    VkFormat vkFormat = this->vkCaps().getFormatFromColorType(ct);

    GrVkImageInfo info;
    if (!this->createVkImageForBackendSurface(vkFormat, {w, h}, false, true, nullptr, 1, &info,
                                              GrProtected::kNo)) {
        return {};
    }

    return GrBackendRenderTarget(w, h, 1, 0, info);
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

void GrVkGpu::testingOnly_flushGpuAndSync() {
    SkAssertResult(this->submitCommandBuffer(kForce_SyncQueue));
}
#endif

////////////////////////////////////////////////////////////////////////////////

void GrVkGpu::addBufferMemoryBarrier(const GrVkResource* resource,
                                     VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     VkBufferMemoryBarrier* barrier) const {
    SkASSERT(fCurrentCmdBuffer);
    SkASSERT(resource);
    fCurrentCmdBuffer->pipelineBarrier(this,
                                       resource,
                                       srcStageMask,
                                       dstStageMask,
                                       byRegion,
                                       GrVkCommandBuffer::kBufferMemory_BarrierType,
                                       barrier);
}

void GrVkGpu::addImageMemoryBarrier(const GrVkResource* resource,
                                    VkPipelineStageFlags srcStageMask,
                                    VkPipelineStageFlags dstStageMask,
                                    bool byRegion,
                                    VkImageMemoryBarrier* barrier) const {
    SkASSERT(fCurrentCmdBuffer);
    SkASSERT(resource);
    fCurrentCmdBuffer->pipelineBarrier(this,
                                       resource,
                                       srcStageMask,
                                       dstStageMask,
                                       byRegion,
                                       GrVkCommandBuffer::kImageMemory_BarrierType,
                                       barrier);
}

bool GrVkGpu::onFinishFlush(GrSurfaceProxy* proxies[], int n,
                            SkSurface::BackendSurfaceAccess access, const GrFlushInfo& info,
                            const GrPrepareForExternalIORequests& externalRequests) {
    SkASSERT(n >= 0);
    SkASSERT(!n || proxies);
    // Submit the current command buffer to the Queue. Whether we inserted semaphores or not does
    // not effect what we do here.
    if (n && access == SkSurface::BackendSurfaceAccess::kPresent) {
        GrVkImage* image;
        for (int i = 0; i < n; ++i) {
            SkASSERT(proxies[i]->isInstantiated());
            if (GrTexture* tex = proxies[i]->peekTexture()) {
                image = static_cast<GrVkTexture*>(tex);
            } else {
                GrRenderTarget* rt = proxies[i]->peekRenderTarget();
                SkASSERT(rt);
                image = static_cast<GrVkRenderTarget*>(rt);
            }
            image->prepareForPresent(this);
        }
    }

    // Handle requests for preparing for external IO
    for (int i = 0; i < externalRequests.fNumImages; ++i) {
        SkImage* image = externalRequests.fImages[i];
        if (!image->isTextureBacked()) {
            continue;
        }
        SkImage_GpuBase* gpuImage = static_cast<SkImage_GpuBase*>(as_IB(image));
        sk_sp<GrTextureProxy> proxy = gpuImage->asTextureProxyRef(this->getContext());
        SkASSERT(proxy);

        if (!proxy->isInstantiated()) {
            auto resourceProvider = this->getContext()->priv().resourceProvider();
            if (!proxy->instantiate(resourceProvider)) {
                continue;
            }
        }

        GrTexture* tex = proxy->peekTexture();
        if (!tex) {
            continue;
        }
        GrVkTexture* vkTex = static_cast<GrVkTexture*>(tex);
        vkTex->prepareForExternal(this);
    }
    for (int i = 0; i < externalRequests.fNumSurfaces; ++i) {
        SkSurface* surface = externalRequests.fSurfaces[i];
        if (!surface->getCanvas()->getGrContext()) {
            continue;
        }
        SkSurface_Gpu* gpuSurface = static_cast<SkSurface_Gpu*>(surface);
        auto* rtc = gpuSurface->getDevice()->accessRenderTargetContext();
        sk_sp<GrRenderTargetProxy> proxy = rtc->asRenderTargetProxyRef();
        if (!proxy->isInstantiated()) {
            auto resourceProvider = this->getContext()->priv().resourceProvider();
            if (!proxy->instantiate(resourceProvider)) {
                continue;
            }
        }

        GrRenderTarget* rt = proxy->peekRenderTarget();
        SkASSERT(rt);
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
        if (externalRequests.fPrepareSurfaceForPresent &&
            externalRequests.fPrepareSurfaceForPresent[i]) {
            vkRT->prepareForPresent(this);
        } else {
            vkRT->prepareForExternal(this);
        }
    }

    if (info.fFlags & kSyncCpu_GrFlushFlag) {
        return this->submitCommandBuffer(kForce_SyncQueue, info.fFinishedProc,
                                         info.fFinishedContext);
    } else {
        return this->submitCommandBuffer(kSkip_SyncQueue, info.fFinishedProc,
                                         info.fFinishedContext);
    }
}

static int get_surface_sample_cnt(GrSurface* surf) {
    if (const GrRenderTarget* rt = surf->asRenderTarget()) {
        return rt->numSamples();
    }
    return 0;
}

void GrVkGpu::copySurfaceAsCopyImage(GrSurface* dst, GrSurface* src, GrVkImage* dstImage,
                                     GrVkImage* srcImage, const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
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

    fCurrentCmdBuffer->copyImage(this,
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
#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
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

    fCurrentCmdBuffer->blitImage(this,
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

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);

    GrVkImage* dstImage;
    GrVkImage* srcImage;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(dstRT);
        if (vkRT->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        dstImage = vkRT->numSamples() > 1 ? vkRT->msaaImage() : vkRT;
    } else {
        SkASSERT(dst->asTexture());
        dstImage = static_cast<GrVkTexture*>(dst->asTexture());
    }
    GrRenderTarget* srcRT = src->asRenderTarget();
    if (srcRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(srcRT);
        srcImage = vkRT->numSamples() > 1 ? vkRT->msaaImage() : vkRT;
    } else {
        SkASSERT(src->asTexture());
        srcImage = static_cast<GrVkTexture*>(src->asTexture());
    }

    VkFormat dstFormat = dstImage->imageFormat();
    VkFormat srcFormat = srcImage->imageFormat();

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

    GrVkImage* image = nullptr;
    GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(surface->asRenderTarget());
    if (rt) {
        // Reading from render targets that wrap a secondary command buffer is not allowed since
        // it would require us to know the VkImage, which we don't have, as well as need us to
        // stop and start the VkRenderPass which we don't have access to.
        if (rt->wrapsSecondaryCommandBuffer()) {
            return false;
        }
        image = rt;
    } else {
        image = static_cast<GrVkTexture*>(surface->asTexture());
    }

    if (!image) {
        return false;
    }

    // Skia's RGB_888x color type, which we map to the vulkan R8G8B8_UNORM, expects the data to be
    // 32 bits, but the Vulkan format is only 24. So we first copy the surface into an R8G8B8A8
    // image and then do the read pixels from that.
    sk_sp<GrVkTextureRenderTarget> copySurface;
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
        GrSurfaceDesc surfDesc;
        surfDesc.fWidth = width;
        surfDesc.fHeight = height;
        surfDesc.fConfig = kRGBA_8888_GrPixelConfig;

        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
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

        copySurface = GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
                this, SkBudgeted::kYes, surfDesc, 1, imageDesc, GrMipMapsStatus::kNotAllocated);
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
    size_t tightRowBytes = bpp * width;

    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));

    bool copyFromOrigin = this->vkCaps().mustDoCopiesFromOrigin();
    if (copyFromOrigin) {
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { (uint32_t)(left + width), (uint32_t)(top + height), 1 };
    } else {
        VkOffset3D offset = { left, top, 0 };
        region.imageOffset = offset;
        region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };
    }

    size_t transBufferRowBytes = bpp * region.imageExtent.width;
    size_t imageRows = region.imageExtent.height;
    auto transferBuffer = sk_sp<GrVkTransferBuffer>(
            static_cast<GrVkTransferBuffer*>(this->createBuffer(transBufferRowBytes * imageRows,
                                                                GrGpuBufferType::kXferGpuToCpu,
                                                                kStream_GrAccessPattern)
                                                     .release()));

    // Copy the image to a buffer so we can map it to cpu memory
    region.bufferOffset = transferBuffer->offset();
    region.bufferRowLength = 0; // Forces RowLength to be width. We handle the rowBytes below.
    region.bufferImageHeight = 0; // Forces height to be tightly packed. Only useful for 3d images.
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

    fCurrentCmdBuffer->copyImageToBuffer(this,
                                         image,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         transferBuffer.get(),
                                         1,
                                         &region);

    // make sure the copy to buffer has finished
    transferBuffer->addMemoryBarrier(this,
                                     VK_ACCESS_TRANSFER_WRITE_BIT,
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
    const GrVkAlloc& transAlloc = transferBuffer->alloc();
    GrVkMemory::InvalidateMappedAlloc(this, transAlloc, 0, transAlloc.fSize);

    if (copyFromOrigin) {
        uint32_t skipRows = region.imageExtent.height - height;
        mappedMemory = (char*)mappedMemory + transBufferRowBytes * skipRows + bpp * left;
    }

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, tightRowBytes, height);

    transferBuffer->unmap();
    return true;
}

// The RenderArea bounds we pass into BeginRenderPass must have a start x value that is a multiple
// of the granularity. The width must also be a multiple of the granularity or eaqual to the width
// the the entire attachment. Similar requirements for the y and height components.
void adjust_bounds_to_granularity(SkIRect* dstBounds, const SkIRect& srcBounds,
                                  const VkExtent2D& granularity, int maxWidth, int maxHeight) {
    // Adjust Width
    if ((0 != granularity.width && 1 != granularity.width)) {
        // Start with the right side of rect so we know if we end up going pass the maxWidth.
        int rightAdj = srcBounds.fRight % granularity.width;
        if (rightAdj != 0) {
            rightAdj = granularity.width - rightAdj;
        }
        dstBounds->fRight = srcBounds.fRight + rightAdj;
        if (dstBounds->fRight > maxWidth) {
            dstBounds->fRight = maxWidth;
            dstBounds->fLeft = 0;
        } else {
            dstBounds->fLeft = srcBounds.fLeft - srcBounds.fLeft % granularity.width;
        }
    } else {
        dstBounds->fLeft = srcBounds.fLeft;
        dstBounds->fRight = srcBounds.fRight;
    }

    // Adjust height
    if ((0 != granularity.height && 1 != granularity.height)) {
        // Start with the bottom side of rect so we know if we end up going pass the maxHeight.
        int bottomAdj = srcBounds.fBottom % granularity.height;
        if (bottomAdj != 0) {
            bottomAdj = granularity.height - bottomAdj;
        }
        dstBounds->fBottom = srcBounds.fBottom + bottomAdj;
        if (dstBounds->fBottom > maxHeight) {
            dstBounds->fBottom = maxHeight;
            dstBounds->fTop = 0;
        } else {
            dstBounds->fTop = srcBounds.fTop - srcBounds.fTop % granularity.height;
        }
    } else {
        dstBounds->fTop = srcBounds.fTop;
        dstBounds->fBottom = srcBounds.fBottom;
    }
}

bool GrVkGpu::beginRenderPass(const GrVkRenderPass* renderPass,
                              const VkClearValue* colorClear,
                              GrVkRenderTarget* target, GrSurfaceOrigin origin,
                              const SkIRect& bounds, bool forSecondaryCB) {
    SkASSERT (!target->wrapsSecondaryCommandBuffer());
    auto nativeBounds = GrNativeRect::MakeRelativeTo(origin, target->height(), bounds);

    // The bounds we use for the render pass should be of the granularity supported
    // by the device.
    const VkExtent2D& granularity = renderPass->granularity();
    SkIRect adjustedBounds;
    if ((0 != granularity.width && 1 != granularity.width) ||
        (0 != granularity.height && 1 != granularity.height)) {
        adjust_bounds_to_granularity(&adjustedBounds, nativeBounds.asSkIRect(), granularity,
                                     target->width(), target->height());
    } else {
        adjustedBounds = nativeBounds.asSkIRect();
    }

#ifdef SK_DEBUG
    uint32_t index;
    bool result = renderPass->colorAttachmentIndex(&index);
    SkASSERT(result && 0 == index);
    result = renderPass->stencilAttachmentIndex(&index);
    if (result) {
        SkASSERT(1 == index);
    }
#endif
    VkClearValue clears[2];
    clears[0].color = colorClear->color;
    clears[1].depthStencil.depth = 0.0f;
    clears[1].depthStencil.stencil = 0;

   return fCurrentCmdBuffer->beginRenderPass(this, renderPass, clears, target, adjustedBounds,
                                             forSecondaryCB);
}

void GrVkGpu::endRenderPass(GrRenderTarget* target, GrSurfaceOrigin origin,
                            const SkIRect& bounds) {
    fCurrentCmdBuffer->endRenderPass(this);
    this->didWriteToSurface(target, origin, &bounds);
}

void GrVkGpu::submitSecondaryCommandBuffer(std::unique_ptr<GrVkSecondaryCommandBuffer> buffer) {
    fCurrentCmdBuffer->executeCommands(this, std::move(buffer));
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

    GR_STATIC_ASSERT(sizeof(GrFence) >= sizeof(VkFence));
    return (GrFence)fence;
}

bool GrVkGpu::waitFence(GrFence fence, uint64_t timeout) {
    SkASSERT(VK_NULL_HANDLE != (VkFence)fence);

    VkResult result;
    VK_CALL_RET(result, WaitForFences(this->device(), 1, (VkFence*)&fence, VK_TRUE, timeout));
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
    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore);

    GrVkSemaphore::Resource* resource = vkSem->getResource();
    if (resource->shouldSignal()) {
        resource->ref();
        fSemaphoresToSignal.push_back(resource);
    }
}

void GrVkGpu::waitSemaphore(GrSemaphore* semaphore) {
    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore);

    GrVkSemaphore::Resource* resource = vkSem->getResource();
    if (resource->shouldWait()) {
        resource->ref();
        fSemaphoresToWaitOn.push_back(resource);
    }
}

std::unique_ptr<GrSemaphore> GrVkGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(texture);
    GrVkTexture* vkTexture = static_cast<GrVkTexture*>(texture);
    vkTexture->setImageLayout(this,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT,
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              false);
    // TODO: should we have a way to notify the caller that this has failed? Currently if the submit
    // fails (caused by DEVICE_LOST) this will just cause us to fail the next use of the gpu.
    // Eventually we will abandon the whole GPU if this fails.
    this->submitCommandBuffer(kSkip_SyncQueue);

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
