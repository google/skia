/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkGpu.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrGeometryProcessor.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrMesh.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrTexturePriv.h"

#include "GrVkCommandBuffer.h"
#include "GrVkGpuCommandBuffer.h"
#include "GrVkImage.h"
#include "GrVkIndexBuffer.h"
#include "GrVkMemory.h"
#include "GrVkPipeline.h"
#include "GrVkPipelineState.h"
#include "GrVkRenderPass.h"
#include "GrVkResourceProvider.h"
#include "GrVkSemaphore.h"
#include "GrVkTexelBuffer.h"
#include "GrVkTexture.h"
#include "GrVkTextureRenderTarget.h"
#include "GrVkTransferBuffer.h"
#include "GrVkVertexBuffer.h"

#include "SkConvertPixels.h"
#include "SkMipMap.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkTypes.h"

#include "SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

#define VK_CALL(X) GR_VK_CALL(this->vkInterface(), X)
#define VK_CALL_RET(RET, X) GR_VK_CALL_RET(this->vkInterface(), RET, X)
#define VK_CALL_ERRCHECK(X) GR_VK_CALL_ERRCHECK(this->vkInterface(), X)

#ifdef SK_ENABLE_VK_LAYERS
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 pLayerPrefix,
    const char*                 pMessage,
    void*                       pUserData) {
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        SkDebugf("Vulkan error [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
        return VK_TRUE; // skip further layers
    } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        SkDebugf("Vulkan warning [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        SkDebugf("Vulkan perf warning [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    } else {
        SkDebugf("Vulkan info/debug [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    }
    return VK_FALSE;
}
#endif

sk_sp<GrGpu> GrVkGpu::Make(GrBackendContext backendContext, const GrContextOptions& options,
                           GrContext* context) {
    const auto* backend = reinterpret_cast<const GrVkBackendContext*>(backendContext);
    return Make(sk_ref_sp(backend), options, context);
}

sk_sp<GrGpu> GrVkGpu::Make(sk_sp<const GrVkBackendContext> backendContext,
                           const GrContextOptions& options, GrContext* context) {
    if (!backendContext) {
        return nullptr;
    }

    if (!backendContext->fInterface->validate(backendContext->fExtensions)) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrVkGpu(context, options, std::move(backendContext)));
}

////////////////////////////////////////////////////////////////////////////////

GrVkGpu::GrVkGpu(GrContext* context, const GrContextOptions& options,
                 sk_sp<const GrVkBackendContext> backendCtx)
        : INHERITED(context)
        , fBackendContext(std::move(backendCtx))
        , fDevice(fBackendContext->fDevice)
        , fQueue(fBackendContext->fQueue)
        , fResourceProvider(this)
        , fDisconnected(false) {
#ifdef SK_ENABLE_VK_LAYERS
    fCallback = VK_NULL_HANDLE;
    if (fBackendContext->fExtensions & kEXT_debug_report_GrVkExtensionFlag) {
        // Setup callback creation information
        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
        callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        callbackCreateInfo.pNext = nullptr;
        callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                   VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                   //VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                   //VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                   VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        callbackCreateInfo.pfnCallback = &DebugReportCallback;
        callbackCreateInfo.pUserData = nullptr;

        // Register the callback
        GR_VK_CALL_ERRCHECK(this->vkInterface(),
                            CreateDebugReportCallbackEXT(fBackendContext->fInstance,
                                                         &callbackCreateInfo, nullptr, &fCallback));
    }
#endif

    fCompiler = new SkSL::Compiler();

    fVkCaps.reset(new GrVkCaps(options, this->vkInterface(), fBackendContext->fPhysicalDevice,
                               fBackendContext->fFeatures, fBackendContext->fExtensions));
    fCaps.reset(SkRef(fVkCaps.get()));

    VK_CALL(GetPhysicalDeviceProperties(fBackendContext->fPhysicalDevice, &fPhysDevProps));
    VK_CALL(GetPhysicalDeviceMemoryProperties(fBackendContext->fPhysicalDevice, &fPhysDevMemProps));

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,      // sType
        nullptr,                                         // pNext
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // CmdPoolCreateFlags
        fBackendContext->fGraphicsQueueIndex,            // queueFamilyIndex
    };
    GR_VK_CALL_ERRCHECK(this->vkInterface(), CreateCommandPool(fDevice, &cmdPoolInfo, nullptr,
                                                               &fCmdPool));

    // must call this after creating the CommandPool
    fResourceProvider.init();
    fCurrentCmdBuffer = fResourceProvider.findOrCreatePrimaryCommandBuffer();
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->begin(this);

    // set up our heaps
    fHeaps[kLinearImage_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 16*1024*1024));
    fHeaps[kOptimalImage_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 64*1024*1024));
    fHeaps[kSmallOptimalImage_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 2*1024*1024));
    fHeaps[kVertexBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kIndexBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kUniformBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 256*1024));
    fHeaps[kTexelBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kCopyReadBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kCopyWriteBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 16*1024*1024));
}

void GrVkGpu::destroyResources() {
    if (fCurrentCmdBuffer) {
        fCurrentCmdBuffer->end(this);
        fCurrentCmdBuffer->unref(this);
    }

    // wait for all commands to finish
    fResourceProvider.checkCommandBuffers();
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

    for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
        fSemaphoresToWaitOn[i]->unref(this);
    }
    fSemaphoresToWaitOn.reset();

    for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
        fSemaphoresToSignal[i]->unref(this);
    }
    fSemaphoresToSignal.reset();


    fCopyManager.destroyResources(this);

    // must call this just before we destroy the command pool and VkDevice
    fResourceProvider.destroyResources(VK_ERROR_DEVICE_LOST == res);

    if (fCmdPool != VK_NULL_HANDLE) {
        VK_CALL(DestroyCommandPool(fDevice, fCmdPool, nullptr));
    }

#ifdef SK_ENABLE_VK_LAYERS
    if (fCallback) {
        VK_CALL(DestroyDebugReportCallbackEXT(fBackendContext->fInstance, fCallback, nullptr));
    }
#endif

}

GrVkGpu::~GrVkGpu() {
    if (!fDisconnected) {
        this->destroyResources();
    }
    delete fCompiler;
}


void GrVkGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);
    if (!fDisconnected) {
        if (DisconnectType::kCleanup == type) {
            this->destroyResources();
        } else {
            fCurrentCmdBuffer->unrefAndAbandon();
            for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
                fSemaphoresToWaitOn[i]->unrefAndAbandon();
            }
            for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
                fSemaphoresToSignal[i]->unrefAndAbandon();
            }
            fCopyManager.abandonResources();

            // must call this just before we destroy the command pool and VkDevice
            fResourceProvider.abandonResources();
        }
        fSemaphoresToWaitOn.reset();
        fSemaphoresToSignal.reset();
#ifdef SK_ENABLE_VK_LAYERS
        fCallback = VK_NULL_HANDLE;
#endif
        fCurrentCmdBuffer = nullptr;
        fCmdPool = VK_NULL_HANDLE;
        fDisconnected = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

GrGpuRTCommandBuffer* GrVkGpu::createCommandBuffer(
            GrRenderTarget* rt, GrSurfaceOrigin origin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    return new GrVkGpuRTCommandBuffer(this, rt, origin, colorInfo, stencilInfo);
}

GrGpuTextureCommandBuffer* GrVkGpu::createCommandBuffer(GrTexture* texture,
                                                        GrSurfaceOrigin origin) {
    return new GrVkGpuTextureCommandBuffer(this, texture, origin);
}

void GrVkGpu::submitCommandBuffer(SyncQueue sync) {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->end(this);

    fCurrentCmdBuffer->submitToQueue(this, fQueue, sync, fSemaphoresToSignal, fSemaphoresToWaitOn);

    for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
        fSemaphoresToWaitOn[i]->unref(this);
    }
    fSemaphoresToWaitOn.reset();
    for (int i = 0; i < fSemaphoresToSignal.count(); ++i) {
        fSemaphoresToSignal[i]->unref(this);
    }
    fSemaphoresToSignal.reset();

    fResourceProvider.checkCommandBuffers();

    // Release old command buffer and create a new one
    fCurrentCmdBuffer->unref(this);
    fCurrentCmdBuffer = fResourceProvider.findOrCreatePrimaryCommandBuffer();
    SkASSERT(fCurrentCmdBuffer);

    fCurrentCmdBuffer->begin(this);
}

///////////////////////////////////////////////////////////////////////////////
GrBuffer* GrVkGpu::onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern accessPattern,
                                  const void* data) {
    GrBuffer* buff;
    switch (type) {
        case kVertex_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            buff = GrVkVertexBuffer::Create(this, size, kDynamic_GrAccessPattern == accessPattern);
            break;
        case kIndex_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            buff = GrVkIndexBuffer::Create(this, size, kDynamic_GrAccessPattern == accessPattern);
            break;
        case kXferCpuToGpu_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStream_GrAccessPattern == accessPattern);
            buff = GrVkTransferBuffer::Create(this, size, GrVkBuffer::kCopyRead_Type);
            break;
        case kXferGpuToCpu_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStream_GrAccessPattern == accessPattern);
            buff = GrVkTransferBuffer::Create(this, size, GrVkBuffer::kCopyWrite_Type);
            break;
        case kTexel_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            buff = GrVkTexelBuffer::Create(this, size, kDynamic_GrAccessPattern == accessPattern);
            break;
        case kDrawIndirect_GrBufferType:
            SK_ABORT("DrawIndirect Buffers not supported  in vulkan backend.");
            return nullptr;
        default:
            SK_ABORT("Unknown buffer type.");
            return nullptr;
    }
    if (data && buff) {
        buff->updateData(data, size);
    }
    return buff;
}

////////////////////////////////////////////////////////////////////////////////
bool GrVkGpu::onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                                   int width, int height,
                                   GrPixelConfig srcConfig, DrawPreference* drawPreference,
                                   WritePixelTempDrawInfo* tempDrawInfo) {
    GrRenderTarget* renderTarget = dstSurface->asRenderTarget();

    // Start off assuming no swizzling
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();
    tempDrawInfo->fWriteConfig = srcConfig;

    // These settings we will always want if a temp draw is performed. Initially set the config
    // to srcConfig, though that may be modified if we decide to do a R/B swap
    tempDrawInfo->fTempSurfaceDesc.fFlags = kNone_GrSurfaceFlags;
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcConfig;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 1;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin;

    if (dstSurface->config() == srcConfig) {
        // We only support writing pixels to textures. Forcing a draw lets us write to pure RTs.
        if (!dstSurface->asTexture()) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
        // If the dst is MSAA, we have to draw, or we'll just be writing to the resolve target.
        if (renderTarget && renderTarget->numColorSamples() > 1) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
        return true;
    }

    // Any config change requires a draw
    ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);

    bool configsAreRBSwaps = GrPixelConfigSwapRAndB(srcConfig) == dstSurface->config();

    if (!this->vkCaps().isConfigTexturable(srcConfig) && configsAreRBSwaps) {
        tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
        tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
        tempDrawInfo->fWriteConfig = dstSurface->config();
    }
    return true;
}

bool GrVkGpu::onWritePixels(GrSurface* surface, GrSurfaceOrigin origin,
                            int left, int top, int width, int height,
                            GrPixelConfig config,
                            const GrMipLevel texels[], int mipLevelCount) {
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(surface->asTexture());
    if (!vkTex) {
        return false;
    }

    // Make sure we have at least the base level
    if (!mipLevelCount || !texels[0].fPixels) {
        return false;
    }

    // We assume Vulkan doesn't do sRGB <-> linear conversions when reading and writing pixels.
    if (GrPixelConfigIsSRGB(surface->config()) != GrPixelConfigIsSRGB(config)) {
        return false;
    }

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
            this->submitCommandBuffer(kForce_SyncQueue);
        }
        success = this->uploadTexDataLinear(vkTex, origin, left, top, width, height, config,
                                            texels[0].fPixels, texels[0].fRowBytes);
    } else {
        int currentMipLevels = vkTex->texturePriv().maxMipMapLevel() + 1;
        if (mipLevelCount > currentMipLevels) {
            if (!vkTex->reallocForMipmap(this, mipLevelCount)) {
                return false;
            }
        }
        success = this->uploadTexDataOptimal(vkTex, origin, left, top, width, height, config,
                                             texels, mipLevelCount);
    }

    return success;
}

bool GrVkGpu::onTransferPixels(GrTexture* texture,
                               int left, int top, int width, int height,
                               GrPixelConfig config, GrBuffer* transferBuffer,
                               size_t bufferOffset, size_t rowBytes) {
    // Vulkan only supports 4-byte aligned offsets
    if (SkToBool(bufferOffset & 0x2)) {
        return false;
    }
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(texture);
    if (!vkTex) {
        return false;
    }
    GrVkTransferBuffer* vkBuffer = static_cast<GrVkTransferBuffer*>(transferBuffer);
    if (!vkBuffer) {
        return false;
    }

    // We assume Vulkan doesn't do sRGB <-> linear conversions when reading and writing pixels.
    if (GrPixelConfigIsSRGB(texture->config()) != GrPixelConfigIsSRGB(config)) {
        return false;
    }

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
        SkASSERT(bounds.contains(subRect));
    )
    size_t bpp = GrBytesPerPixel(config);
    if (rowBytes == 0) {
        rowBytes = bpp*width;
    }

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

void GrVkGpu::resolveImage(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                           GrVkRenderTarget* src, GrSurfaceOrigin srcOrigin,
                           const SkIRect& srcRect, const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src && src->numColorSamples() > 1 && src->msaaImage());

    if (this->vkCaps().mustSubmitCommandsBeforeCopyOp()) {
        this->submitCommandBuffer(GrVkGpu::kSkip_SyncQueue);
    }

    // Flip rect if necessary
    SkIRect srcVkRect = srcRect;
    int32_t dstY = dstPoint.fY;

    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == dstOrigin);
        srcVkRect.fTop = src->height() - srcRect.fBottom;
        srcVkRect.fBottom = src->height() - srcRect.fTop;
        dstY = dst->height() - dstPoint.fY - srcVkRect.height();
    }

    VkImageResolve resolveInfo;
    resolveInfo.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    resolveInfo.srcOffset = { srcVkRect.fLeft, srcVkRect.fTop, 0 };
    resolveInfo.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    resolveInfo.dstOffset = { dstPoint.fX, dstY, 0 };
    resolveInfo.extent = { (uint32_t)srcVkRect.width(), (uint32_t)srcVkRect.height(), 1 };

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

void GrVkGpu::internalResolveRenderTarget(GrRenderTarget* target, GrSurfaceOrigin origin,
                                          bool requiresSubmit) {
    if (target->needsResolve()) {
        SkASSERT(target->numColorSamples() > 1);
        GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(target);
        SkASSERT(rt->msaaImage());

        const SkIRect& srcRect = rt->getResolveRect();

        this->resolveImage(target, origin, rt, origin, srcRect,
                           SkIPoint::Make(srcRect.fLeft, srcRect.fTop));

        rt->flagAsResolved();

        if (requiresSubmit) {
            this->submitCommandBuffer(kSkip_SyncQueue);
        }
    }
}

bool GrVkGpu::uploadTexDataLinear(GrVkTexture* tex, GrSurfaceOrigin texOrigin,
                                  int left, int top, int width, int height,
                                  GrPixelConfig dataConfig,
                                  const void* data,
                                  size_t rowBytes) {
    SkASSERT(data);
    SkASSERT(tex->isLinearTiled());

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(tex->width(), tex->height());
        SkASSERT(bounds.contains(subRect));
    )
    size_t bpp = GrBytesPerPixel(dataConfig);
    size_t trimRowBytes = width * bpp;
    if (!rowBytes) {
        rowBytes = trimRowBytes;
    }

    SkASSERT(VK_IMAGE_LAYOUT_PREINITIALIZED == tex->currentLayout() ||
             VK_IMAGE_LAYOUT_GENERAL == tex->currentLayout());
    const VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,  // mipLevel
        0,  // arraySlice
    };
    VkSubresourceLayout layout;
    VkResult err;

    const GrVkInterface* interface = this->vkInterface();

    GR_VK_CALL(interface, GetImageSubresourceLayout(fDevice,
                                                    tex->image(),
                                                    &subres,
                                                    &layout));

    int texTop = kBottomLeft_GrSurfaceOrigin == texOrigin ? tex->height() - top - height : top;
    const GrVkAlloc& alloc = tex->alloc();
    VkDeviceSize offset = alloc.fOffset + texTop*layout.rowPitch + left*bpp;
    VkDeviceSize offsetDiff = 0;
    VkDeviceSize size = height*layout.rowPitch;
    // For Noncoherent buffers we want to make sure the range that we map, both offset and size,
    // are aligned to the nonCoherentAtomSize limit. We may have to move the initial offset back to
    // meet the alignment requirements. So we track how far we move back and then adjust the mapped
    // ptr back up so that this is opaque to the caller.
    if (SkToBool(alloc.fFlags & GrVkAlloc::kNoncoherent_Flag)) {
        VkDeviceSize alignment = this->physicalDeviceProperties().limits.nonCoherentAtomSize;
        offsetDiff = offset & (alignment - 1);
        offset = offset - offsetDiff;
        // Make size of the map aligned to nonCoherentAtomSize
        size = (size + alignment - 1) & ~(alignment - 1);
    }
    SkASSERT(offset >= alloc.fOffset);
    SkASSERT(size <= alloc.fOffset + alloc.fSize);
    void* mapPtr;
    err = GR_VK_CALL(interface, MapMemory(fDevice, alloc.fMemory, offset, size, 0, &mapPtr));
    if (err) {
        return false;
    }
    mapPtr = reinterpret_cast<char*>(mapPtr) + offsetDiff;

    if (kBottomLeft_GrSurfaceOrigin == texOrigin) {
        // copy into buffer by rows
        const char* srcRow = reinterpret_cast<const char*>(data);
        char* dstRow = reinterpret_cast<char*>(mapPtr)+(height - 1)*layout.rowPitch;
        for (int y = 0; y < height; y++) {
            memcpy(dstRow, srcRow, trimRowBytes);
            srcRow += rowBytes;
            dstRow -= layout.rowPitch;
        }
    } else {
        SkRectMemcpy(mapPtr, static_cast<size_t>(layout.rowPitch), data, rowBytes, trimRowBytes,
                     height);
    }

    GrVkMemory::FlushMappedAlloc(this, alloc, offset, size);
    GR_VK_CALL(interface, UnmapMemory(fDevice, alloc.fMemory));

    return true;
}

bool GrVkGpu::uploadTexDataOptimal(GrVkTexture* tex, GrSurfaceOrigin texOrigin,
                                   int left, int top, int width, int height,
                                   GrPixelConfig dataConfig,
                                   const GrMipLevel texels[], int mipLevelCount) {
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

    SkASSERT(this->caps()->isConfigTexturable(tex->config()));
    size_t bpp = GrBytesPerPixel(dataConfig);

    // texels is const.
    // But we may need to adjust the fPixels ptr based on the copyRect, or fRowBytes.
    // Because of this we need to make a non-const shallow copy of texels.
    SkAutoTMalloc<GrMipLevel> texelsShallowCopy;

    if (mipLevelCount) {
        texelsShallowCopy.reset(mipLevelCount);
        memcpy(texelsShallowCopy.get(), texels, mipLevelCount*sizeof(GrMipLevel));
    }

    // Determine whether we need to flip when we copy into the buffer
    bool flipY = (kBottomLeft_GrSurfaceOrigin == texOrigin && mipLevelCount);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    individualMipOffsets.push_back(0);
    size_t combinedBufferSize = width * bpp * height;
    int currentWidth = width;
    int currentHeight = height;
    if (mipLevelCount > 0  && !texelsShallowCopy[0].fPixels) {
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
    GrVkTransferBuffer* transferBuffer =
                   GrVkTransferBuffer::Create(this, combinedBufferSize, GrVkBuffer::kCopyRead_Type);
    if(!transferBuffer) {
        return false;
    }

    char* buffer = (char*) transferBuffer->map();
    SkTArray<VkBufferImageCopy> regions(mipLevelCount);

    currentWidth = width;
    currentHeight = height;
    int layerHeight = tex->height();
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texelsShallowCopy[currentMipLevel].fPixels) {
            SkASSERT(1 == mipLevelCount || currentHeight == layerHeight);
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t rowBytes = texelsShallowCopy[currentMipLevel].fRowBytes
                                    ? texelsShallowCopy[currentMipLevel].fRowBytes
                                    : trimRowBytes;

            // copy data into the buffer, skipping the trailing bytes
            char* dst = buffer + individualMipOffsets[currentMipLevel];
            const char* src = (const char*)texelsShallowCopy[currentMipLevel].fPixels;
            if (flipY) {
                src += (currentHeight - 1) * rowBytes;
                for (int y = 0; y < currentHeight; y++) {
                    memcpy(dst, src, trimRowBytes);
                    src -= rowBytes;
                    dst += trimRowBytes;
                }
            } else {
                SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);
            }

            VkBufferImageCopy& region = regions.push_back();
            memset(&region, 0, sizeof(VkBufferImageCopy));
            region.bufferOffset = transferBuffer->offset() + individualMipOffsets[currentMipLevel];
            region.bufferRowLength = currentWidth;
            region.bufferImageHeight = currentHeight;
            region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, SkToU32(currentMipLevel), 0, 1 };
            region.imageOffset = { left, flipY ? layerHeight - top - currentHeight : top, 0 };
            region.imageExtent = { (uint32_t)currentWidth, (uint32_t)currentHeight, 1 };
        }
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
        layerHeight = currentHeight;
    }

    // no need to flush non-coherent memory, unmap will do that for us
    transferBuffer->unmap();

    // Change layout of our target so it can be copied to
    tex->setImageLayout(this,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        false);

    // Copy the buffer to the image
    fCurrentCmdBuffer->copyBufferToImage(this,
                                         transferBuffer,
                                         tex,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         regions.count(),
                                         regions.begin());
    transferBuffer->unref();
    if (1 == mipLevelCount) {
        tex->texturePriv().markMipMapsDirty();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> GrVkGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                          const GrMipLevel texels[], int mipLevelCount) {
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    VkFormat pixelFormat;
    if (!GrPixelConfigToVkFormat(desc.fConfig, &pixelFormat)) {
        return nullptr;
    }

    if (!fVkCaps->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    if (renderTarget && !fVkCaps->isConfigRenderable(desc.fConfig, false)) {
        return nullptr;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (renderTarget) {
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
    int mipLevels = !mipLevelCount ? 1 : mipLevelCount;
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = pixelFormat;
    imageDesc.fWidth = desc.fWidth;
    imageDesc.fHeight = desc.fHeight;
    imageDesc.fLevels = mipLevels;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    GrMipMapsStatus mipMapsStatus = GrMipMapsStatus::kNotAllocated;
    if (mipLevels > 1) {
        mipMapsStatus = GrMipMapsStatus::kValid;
        for (int i = 0; i < mipLevels; ++i) {
            if (!texels[i].fPixels) {
                mipMapsStatus = GrMipMapsStatus::kDirty;
                break;
            }
        }
    }

    sk_sp<GrVkTexture> tex;
    if (renderTarget) {
        tex = GrVkTextureRenderTarget::CreateNewTextureRenderTarget(this, budgeted, desc,
                                                                    imageDesc,
                                                                    mipMapsStatus);
    } else {
        tex = GrVkTexture::CreateNewTexture(this, budgeted, desc, imageDesc,
                                            mipMapsStatus);
    }

    if (!tex) {
        return nullptr;
    }

    if (mipLevelCount) {
        if (!this->uploadTexDataOptimal(tex.get(), desc.fOrigin, 0, 0, desc.fWidth, desc.fHeight,
                                        desc.fConfig, texels, mipLevelCount)) {
            tex->unref();
            return nullptr;
        }
    }

    if (desc.fFlags & kPerformInitialClear_GrSurfaceFlag) {
        VkClearColorValue zeroClearColor;
        memset(&zeroClearColor, 0, sizeof(zeroClearColor));
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseArrayLayer = 0;
        range.baseMipLevel = 0;
        range.layerCount = 1;
        range.levelCount = 1;
        tex->setImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);
        this->currentCommandBuffer()->clearColorImage(this, tex.get(), &zeroClearColor, 1, &range);
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

static bool check_backend_texture(const GrBackendTexture& backendTex,
                                  GrPixelConfig config) {
    const GrVkImageInfo* info = backendTex.getVkImageInfo();
    if (!info) {
        return false;
    }

    if (VK_NULL_HANDLE == info->fImage || VK_NULL_HANDLE == info->fAlloc.fMemory) {
        return false;
    }

    SkASSERT(GrVkFormatPixelConfigPairIsValid(info->fFormat, config));
    return true;
}

sk_sp<GrTexture> GrVkGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                               GrWrapOwnership ownership) {
    if (!check_backend_texture(backendTex, backendTex.config())) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kNone_GrSurfaceFlags;
    surfDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // Not actually used in the following
    surfDesc.fWidth = backendTex.width();
    surfDesc.fHeight = backendTex.height();
    surfDesc.fConfig = backendTex.config();
    surfDesc.fSampleCnt = 1;

    return GrVkTexture::MakeWrappedTexture(this, surfDesc, ownership, backendTex.getVkImageInfo());
}

sk_sp<GrTexture> GrVkGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         int sampleCnt,
                                                         GrWrapOwnership ownership) {
    if (!check_backend_texture(backendTex, backendTex.config())) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin; // Not actually used in the following
    surfDesc.fWidth = backendTex.width();
    surfDesc.fHeight = backendTex.height();
    surfDesc.fConfig = backendTex.config();
    surfDesc.fSampleCnt = this->caps()->getSampleCount(sampleCnt, backendTex.config());

    return GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(this, surfDesc, ownership,
                                                                   backendTex.getVkImageInfo());
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT){
    // Currently the Vulkan backend does not support wrapping of msaa render targets directly. In
    // general this is not an issue since swapchain images in vulkan are never multisampled. Thus if
    // you want a multisampled RT it is best to wrap the swapchain images and then let Skia handle
    // creating and owning the MSAA images.
    if (backendRT.sampleCnt() > 1) {
        return nullptr;
    }

    const GrVkImageInfo* info = backendRT.getVkImageInfo();
    if (!info) {
        return nullptr;
    }
    if (VK_NULL_HANDLE == info->fImage) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin; // Not actually used in the following
    desc.fWidth = backendRT.width();
    desc.fHeight = backendRT.height();
    desc.fConfig = backendRT.config();
    desc.fSampleCnt = 1;

    sk_sp<GrVkRenderTarget> tgt = GrVkRenderTarget::MakeWrappedRenderTarget(this, desc, info);
    if (tgt && backendRT.stencilBits()) {
        if (!createStencilAttachmentForRenderTarget(tgt.get(), desc.fWidth, desc.fHeight)) {
            return nullptr;
        }
    }
    return tgt;
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                  int sampleCnt) {

    const GrVkImageInfo* info = tex.getVkImageInfo();
    if (!info) {
        return nullptr;
    }
    if (VK_NULL_HANDLE == info->fImage) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin; // Not actually used in the following
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = tex.config();
    desc.fSampleCnt = this->caps()->getSampleCount(sampleCnt, tex.config());
    if (!desc.fSampleCnt) {
        return nullptr;
    }

    sk_sp<GrVkRenderTarget> tgt = GrVkRenderTarget::MakeWrappedRenderTarget(this, desc, info);
    return tgt;
}

void GrVkGpu::generateMipmap(GrVkTexture* tex, GrSurfaceOrigin texOrigin) {
    // don't do anything for linearly tiled textures (can't have mipmaps)
    if (tex->isLinearTiled()) {
        SkDebugf("Trying to create mipmap for linear tiled texture");
        return;
    }

    // determine if we can blit to and from this format
    const GrVkCaps& caps = this->vkCaps();
    if (!caps.configCanBeDstofBlit(tex->config(), false) ||
        !caps.configCanBeSrcofBlit(tex->config(), false) ||
        !caps.mipMapSupport()) {
        return;
    }

    if (this->vkCaps().mustSubmitCommandsBeforeCopyOp()) {
        this->submitCommandBuffer(kSkip_SyncQueue);
    }

    // We may need to resolve the texture first if it is also a render target
    GrVkRenderTarget* texRT = static_cast<GrVkRenderTarget*>(tex->asRenderTarget());
    if (texRT) {
        this->internalResolveRenderTarget(texRT, texOrigin, false);
    }

    int width = tex->width();
    int height = tex->height();
    VkImageBlit blitRegion;
    memset(&blitRegion, 0, sizeof(VkImageBlit));

    // SkMipMap doesn't include the base level in the level count so we have to add 1
    uint32_t levelCount = SkMipMap::ComputeLevelCount(tex->width(), tex->height()) + 1;
    if (levelCount != tex->mipLevels()) {
        const GrVkResource* oldResource = tex->resource();
        oldResource->ref();
        // grab handle to the original image resource
        VkImage oldImage = tex->image();

        // change the original image's layout so we can copy from it
        tex->setImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);

        if (!tex->reallocForMipmap(this, levelCount)) {
            oldResource->unref(this);
            return;
        }
        // change the new image's layout so we can blit to it
        tex->setImageLayout(this, VK_IMAGE_LAYOUT_GENERAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);

        // Blit original image to top level of new image
        blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        blitRegion.srcOffsets[0] = { 0, 0, 0 };
        blitRegion.srcOffsets[1] = { width, height, 1 };
        blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        blitRegion.dstOffsets[0] = { 0, 0, 0 };
        blitRegion.dstOffsets[1] = { width, height, 1 };

        fCurrentCmdBuffer->blitImage(this,
                                     oldResource,
                                     oldImage,
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     tex->resource(),
                                     tex->image(),
                                     VK_IMAGE_LAYOUT_GENERAL,
                                     1,
                                     &blitRegion,
                                     VK_FILTER_LINEAR);

        oldResource->unref(this);
    } else {
        // change layout of the layers so we can write to them.
        tex->setImageLayout(this, VK_IMAGE_LAYOUT_GENERAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);
    }

    // setup memory barrier
    SkASSERT(GrVkFormatIsSupported(tex->imageFormat()));
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        nullptr,                                         // pNext
        VK_ACCESS_TRANSFER_WRITE_BIT,                    // srcAccessMask
        VK_ACCESS_TRANSFER_READ_BIT,                     // dstAccessMask
        VK_IMAGE_LAYOUT_GENERAL,                         // oldLayout
        VK_IMAGE_LAYOUT_GENERAL,                         // newLayout
        VK_QUEUE_FAMILY_IGNORED,                         // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                         // dstQueueFamilyIndex
        tex->image(),                                    // image
        { aspectFlags, 0, 1, 0, 1 }                      // subresourceRange
    };

    // Blit the miplevels
    uint32_t mipLevel = 1;
    while (mipLevel < levelCount) {
        int prevWidth = width;
        int prevHeight = height;
        width = SkTMax(1, width / 2);
        height = SkTMax(1, height / 2);

        imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel - 1;
        this->addImageMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    false, &imageMemoryBarrier);

        blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel - 1, 0, 1 };
        blitRegion.srcOffsets[0] = { 0, 0, 0 };
        blitRegion.srcOffsets[1] = { prevWidth, prevHeight, 1 };
        blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, 0, 1 };
        blitRegion.dstOffsets[0] = { 0, 0, 0 };
        blitRegion.dstOffsets[1] = { width, height, 1 };
        fCurrentCmdBuffer->blitImage(this,
                                     *tex,
                                     *tex,
                                     1,
                                     &blitRegion,
                                     VK_FILTER_LINEAR);
        ++mipLevel;
    }
}

////////////////////////////////////////////////////////////////////////////////

GrStencilAttachment* GrVkGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                     int width,
                                                                     int height) {
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numStencilSamples();

    const GrVkCaps::StencilFormat& sFmt = this->vkCaps().preferedStencilFormat();

    GrVkStencilAttachment* stencil(GrVkStencilAttachment::Create(this,
                                                                 width,
                                                                 height,
                                                                 samples,
                                                                 sFmt));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

////////////////////////////////////////////////////////////////////////////////

bool copy_testing_data(GrVkGpu* gpu, void* srcData, const GrVkAlloc& alloc, size_t bufferOffset,
                       size_t srcRowBytes, size_t dstRowBytes, int h) {
    // For Noncoherent buffers we want to make sure the range that we map, both offset and size,
    // are aligned to the nonCoherentAtomSize limit. We may have to move the initial offset back to
    // meet the alignment requirements. So we track how far we move back and then adjust the mapped
    // ptr back up so that this is opaque to the caller.
    VkDeviceSize mapSize = dstRowBytes * h;
    VkDeviceSize mapOffset = alloc.fOffset + bufferOffset;
    VkDeviceSize offsetDiff = 0;
    if (SkToBool(alloc.fFlags & GrVkAlloc::kNoncoherent_Flag)) {
        VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
        offsetDiff = mapOffset & (alignment - 1);
        mapOffset = mapOffset - offsetDiff;
        // Make size of the map aligned to nonCoherentAtomSize
        mapSize = (mapSize + alignment - 1) & ~(alignment - 1);
    }
    SkASSERT(mapOffset >= alloc.fOffset);
    SkASSERT(mapSize + mapOffset <= alloc.fOffset + alloc.fSize);
    void* mapPtr;
    VkResult err = GR_VK_CALL(gpu->vkInterface(), MapMemory(gpu->device(),
                                                            alloc.fMemory,
                                                            mapOffset,
                                                            mapSize,
                                                            0,
                                                            &mapPtr));
    mapPtr = reinterpret_cast<char*>(mapPtr) + offsetDiff;
    if (err) {
        return false;
    }

    if (srcData) {
        // If there is no padding on dst we can do a single memcopy.
        // This assumes the srcData comes in with no padding.
        SkRectMemcpy(mapPtr, static_cast<size_t>(dstRowBytes),
                     srcData, srcRowBytes, srcRowBytes, h);
    } else {
        // If there is no srcdata we always copy 0's into the textures so that it is initialized
        // with some data.
        if (srcRowBytes == static_cast<size_t>(dstRowBytes)) {
            memset(mapPtr, 0, srcRowBytes * h);
        } else {
            for (int i = 0; i < h; ++i) {
                memset(mapPtr, 0, srcRowBytes);
                mapPtr = SkTAddOffset<void>(mapPtr, static_cast<size_t>(dstRowBytes));
            }
        }
    }
    GrVkMemory::FlushMappedAlloc(gpu, alloc, mapOffset, mapSize);
    GR_VK_CALL(gpu->vkInterface(), UnmapMemory(gpu->device(), alloc.fMemory));
    return true;
}

GrBackendTexture GrVkGpu::createTestingOnlyBackendTexture(void* srcData, int w, int h,
                                                          GrPixelConfig config,
                                                          bool isRenderTarget,
                                                          GrMipMapped mipMapped) {

    VkFormat pixelFormat;
    if (!GrPixelConfigToVkFormat(config, &pixelFormat)) {
        return GrBackendTexture(); // invalid
    }

    bool linearTiling = false;
    if (!fVkCaps->isConfigTexturable(config)) {
        return GrBackendTexture(); // invalid
    }

    if (isRenderTarget && !fVkCaps->isConfigRenderable(config, false)) {
        return GrBackendTexture(); // invalid
    }

    // Currently we don't support uploading pixel data when mipped.
    if (srcData && GrMipMapped::kYes == mipMapped) {
        return GrBackendTexture(); // invalid
    }

    if (fVkCaps->isConfigTexturableLinearly(config) &&
        (!isRenderTarget || fVkCaps->isConfigRenderableLinearly(config, false)) &&
        GrMipMapped::kNo == mipMapped) {
        linearTiling = true;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (isRenderTarget) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    VkImage image = VK_NULL_HANDLE;
    GrVkAlloc alloc;

    VkImageTiling imageTiling = linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    VkImageLayout initialLayout = (VK_IMAGE_TILING_LINEAR == imageTiling)
                                ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!GrSampleCountToVkSampleCount(1, &vkSamples)) {
        return GrBackendTexture(); // invalid
    }

    // Figure out the number of mip levels.
    uint32_t mipLevels = 1;
    if (GrMipMapped::kYes == mipMapped) {
        mipLevels = SkMipMap::ComputeLevelCount(w, h) + 1;
    }

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        nullptr,                                     // pNext
        0,                                           // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        pixelFormat,                                 // VkFormat
        { (uint32_t) w, (uint32_t) h, 1 },           // VkExtent3D
        mipLevels,                                   // mipLevels
        1,                                           // arrayLayers
        vkSamples,                                   // samples
        imageTiling,                                 // VkImageTiling
        usageFlags,                                  // VkImageUsageFlags
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
        0,                                           // queueFamilyCount
        0,                                           // pQueueFamilyIndices
        initialLayout                                // initialLayout
    };

    GR_VK_CALL_ERRCHECK(this->vkInterface(), CreateImage(this->device(), &imageCreateInfo, nullptr, &image));

    if (!GrVkMemory::AllocAndBindImageMemory(this, image, linearTiling, &alloc)) {
        VK_CALL(DestroyImage(this->device(), image, nullptr));
        return GrBackendTexture(); // invalid
    }

    // We need to declare these early so that we can delete them at the end outside of the if block.
    GrVkAlloc bufferAlloc;
    VkBuffer buffer = VK_NULL_HANDLE;

    VkResult err;
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        fCmdPool,                                         // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer cmdBuffer;
    err = VK_CALL(AllocateCommandBuffers(fDevice, &cmdInfo, &cmdBuffer));
    if (err) {
        GrVkMemory::FreeImageMemory(this, false, alloc);
        VK_CALL(DestroyImage(fDevice, image, nullptr));
        return GrBackendTexture(); // invalid
    }

    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    err = VK_CALL(BeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
    SkASSERT(!err);

    size_t bpp = GrBytesPerPixel(config);
    size_t rowCopyBytes = bpp * w;
    if (linearTiling) {
        const VkImageSubresource subres = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,  // mipLevel
            0,  // arraySlice
        };
        VkSubresourceLayout layout;

        VK_CALL(GetImageSubresourceLayout(fDevice, image, &subres, &layout));

        if (!copy_testing_data(this, srcData, alloc, 0, rowCopyBytes,
                               static_cast<size_t>(layout.rowPitch), h)) {
            GrVkMemory::FreeImageMemory(this, true, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            VK_CALL(EndCommandBuffer(cmdBuffer));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
            return GrBackendTexture(); // invalid
        }
    } else {
        SkASSERT(w && h);

        SkTArray<size_t> individualMipOffsets(mipLevels);
        individualMipOffsets.push_back(0);
        size_t combinedBufferSize = w * bpp * h;
        int currentWidth = w;
        int currentHeight = h;
        // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
        // config. This works with the assumption that the bytes in pixel config is always a power
        // of 2.
        SkASSERT((bpp & (bpp - 1)) == 0);
        const size_t alignmentMask = 0x3 | (bpp - 1);
        for (uint32_t currentMipLevel = 1; currentMipLevel < mipLevels; currentMipLevel++) {
            currentWidth = SkTMax(1, currentWidth/2);
            currentHeight = SkTMax(1, currentHeight/2);

            const size_t trimmedSize = currentWidth * bpp * currentHeight;
            const size_t alignmentDiff = combinedBufferSize & alignmentMask;
            if (alignmentDiff != 0) {
                combinedBufferSize += alignmentMask - alignmentDiff + 1;
            }
            individualMipOffsets.push_back(combinedBufferSize);
            combinedBufferSize += trimmedSize;
        }

        VkBufferCreateInfo bufInfo;
        memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.flags = 0;
        bufInfo.size = combinedBufferSize;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufInfo.queueFamilyIndexCount = 0;
        bufInfo.pQueueFamilyIndices = nullptr;
        err = VK_CALL(CreateBuffer(fDevice, &bufInfo, nullptr, &buffer));

        if (err) {
            GrVkMemory::FreeImageMemory(this, false, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            VK_CALL(EndCommandBuffer(cmdBuffer));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
            return GrBackendTexture(); // invalid
        }

        if (!GrVkMemory::AllocAndBindBufferMemory(this, buffer, GrVkBuffer::kCopyRead_Type,
                                                  true, &bufferAlloc)) {
            GrVkMemory::FreeImageMemory(this, false, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            VK_CALL(EndCommandBuffer(cmdBuffer));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
            return GrBackendTexture(); // invalid
        }

        currentWidth = w;
        currentHeight = h;
        for (uint32_t currentMipLevel = 0; currentMipLevel < mipLevels; currentMipLevel++) {
            SkASSERT(0 == currentMipLevel || !srcData);
            size_t currentRowBytes = bpp * currentWidth;
            size_t bufferOffset = individualMipOffsets[currentMipLevel];
            if (!copy_testing_data(this, srcData, bufferAlloc, bufferOffset,
                                   currentRowBytes, currentRowBytes, currentHeight)) {
                GrVkMemory::FreeImageMemory(this, false, alloc);
                VK_CALL(DestroyImage(fDevice, image, nullptr));
                GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
                VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
                VK_CALL(EndCommandBuffer(cmdBuffer));
                VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
                return GrBackendTexture(); // invalid
            }
            currentWidth = SkTMax(1, currentWidth/2);
            currentHeight = SkTMax(1, currentHeight/2);
        }

        // Set image layout and add barrier
        VkImageMemoryBarrier barrier;
        memset(&barrier, 0, sizeof(VkImageMemoryBarrier));
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(initialLayout);
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = initialLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0 , 1};

        VK_CALL(CmdPipelineBarrier(cmdBuffer,
                                   GrVkMemory::LayoutToPipelineStageFlags(initialLayout),
                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   0,
                                   0, nullptr,
                                   0, nullptr,
                                   1, &barrier));
        initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        SkTArray<VkBufferImageCopy> regions(mipLevels);

        currentWidth = w;
        currentHeight = h;
        for (uint32_t currentMipLevel = 0; currentMipLevel < mipLevels; currentMipLevel++) {
            // Submit copy command
            VkBufferImageCopy& region = regions.push_back();
            memset(&region, 0, sizeof(VkBufferImageCopy));
            region.bufferOffset = individualMipOffsets[currentMipLevel];
            region.bufferRowLength = currentWidth;
            region.bufferImageHeight = currentHeight;
            region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { (uint32_t)currentWidth, (uint32_t)currentHeight, 1 };
            currentWidth = SkTMax(1, currentWidth/2);
            currentHeight = SkTMax(1, currentHeight/2);
        }

        VK_CALL(CmdCopyBufferToImage(cmdBuffer, buffer, image, initialLayout, regions.count(),
                                     regions.begin()));
    }
    // Change Image layout to shader read since if we use this texture as a borrowed textures within
    // Ganesh we require that its layout be set to that
    VkImageMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(VkImageMemoryBarrier));
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(initialLayout);
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = initialLayout;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0 , 1};

    VK_CALL(CmdPipelineBarrier(cmdBuffer,
                               GrVkMemory::LayoutToPipelineStageFlags(initialLayout),
                               VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                               0,
                               0, nullptr,
                               0, nullptr,
                               1, &barrier));

    // End CommandBuffer
    err = VK_CALL(EndCommandBuffer(cmdBuffer));
    SkASSERT(!err);

    // Create Fence for queue
    VkFence fence;
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    err = VK_CALL(CreateFence(fDevice, &fenceInfo, nullptr, &fence));
    SkASSERT(!err);

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    err = VK_CALL(QueueSubmit(this->queue(), 1, &submitInfo, fence));
    SkASSERT(!err);

    err = VK_CALL(WaitForFences(fDevice, 1, &fence, true, UINT64_MAX));
    if (VK_TIMEOUT == err) {
        GrVkMemory::FreeImageMemory(this, false, alloc);
        VK_CALL(DestroyImage(fDevice, image, nullptr));
        GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
        VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
        VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
        VK_CALL(DestroyFence(fDevice, fence, nullptr));
        SkDebugf("Fence failed to signal: %d\n", err);
        SK_ABORT("failing");
    }
    SkASSERT(!err);

    // Clean up transfer resources
    if (buffer != VK_NULL_HANDLE) { // workaround for an older NVidia driver crash
        GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
        VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
    }
    VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
    VK_CALL(DestroyFence(fDevice, fence, nullptr));


    GrVkImageInfo info;
    info.fImage = image;
    info.fAlloc = alloc;
    info.fImageTiling = imageTiling;
    info.fImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.fFormat = pixelFormat;
    info.fLevelCount = mipLevels;

    return GrBackendTexture(w, h, info);
}

bool GrVkGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(kVulkan_GrBackend == tex.fBackend);

    const GrVkImageInfo* backend = tex.getVkImageInfo();

    if (backend && backend->fImage && backend->fAlloc.fMemory) {
        VkMemoryRequirements req;
        memset(&req, 0, sizeof(req));
        GR_VK_CALL(this->vkInterface(), GetImageMemoryRequirements(fDevice,
                                                                   backend->fImage,
                                                                   &req));
        // TODO: find a better check
        // This will probably fail with a different driver
        return (req.size > 0) && (req.size <= 8192 * 8192);
    }

    return false;
}

void GrVkGpu::deleteTestingOnlyBackendTexture(GrBackendTexture* tex, bool abandon) {
    SkASSERT(kVulkan_GrBackend == tex->fBackend);

    const GrVkImageInfo* info = tex->getVkImageInfo();

    if (info && !abandon) {
        // something in the command buffer may still be using this, so force submit
        this->submitCommandBuffer(kForce_SyncQueue);
        GrVkImage::DestroyImageInfo(this, const_cast<GrVkImageInfo*>(info));
    }
}

void GrVkGpu::testingOnly_flushGpuAndSync() {
    this->submitCommandBuffer(kForce_SyncQueue);
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpu::addMemoryBarrier(VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion,
                               VkMemoryBarrier* barrier) const {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->pipelineBarrier(this,
                                       srcStageMask,
                                       dstStageMask,
                                       byRegion,
                                       GrVkCommandBuffer::kMemory_BarrierType,
                                       barrier);
}

void GrVkGpu::addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     VkBufferMemoryBarrier* barrier) const {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->pipelineBarrier(this,
                                       srcStageMask,
                                       dstStageMask,
                                       byRegion,
                                       GrVkCommandBuffer::kBufferMemory_BarrierType,
                                       barrier);
}

void GrVkGpu::addImageMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                    VkPipelineStageFlags dstStageMask,
                                    bool byRegion,
                                    VkImageMemoryBarrier* barrier) const {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->pipelineBarrier(this,
                                       srcStageMask,
                                       dstStageMask,
                                       byRegion,
                                       GrVkCommandBuffer::kImageMemory_BarrierType,
                                       barrier);
}

void GrVkGpu::onFinishFlush(bool insertedSemaphore) {
    // Submit the current command buffer to the Queue. Whether we inserted semaphores or not does
    // not effect what we do here.
    this->submitCommandBuffer(kSkip_SyncQueue);
}

void GrVkGpu::clearStencil(GrRenderTarget* target, int clearValue) {
    if (!target) {
        return;
    }
    GrStencilAttachment* stencil = target->renderTargetPriv().getStencilAttachment();
    GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)stencil;


    VkClearDepthStencilValue vkStencilColor;
    vkStencilColor.depth = 0.0f;
    vkStencilColor.stencil = clearValue;

    vkStencil->setImageLayout(this,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_ACCESS_TRANSFER_WRITE_BIT,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              false);

    VkImageSubresourceRange subRange;
    memset(&subRange, 0, sizeof(VkImageSubresourceRange));
    subRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    subRange.baseMipLevel = 0;
    subRange.levelCount = 1;
    subRange.baseArrayLayer = 0;
    subRange.layerCount = 1;

    // TODO: I imagine that most times we want to clear a stencil it will be at the beginning of a
    // draw. Thus we should look into using the load op functions on the render pass to clear out
    // the stencil there.
    fCurrentCmdBuffer->clearDepthStencilImage(this, vkStencil, &vkStencilColor, 1, &subRange);
}

inline bool can_copy_image(const GrSurface* dst, GrSurfaceOrigin dstOrigin,
                           const GrSurface* src, GrSurfaceOrigin srcOrigin,
                           const GrVkGpu* gpu) {
    const GrRenderTarget* dstRT = dst->asRenderTarget();
    const GrRenderTarget* srcRT = src->asRenderTarget();
    if (dstRT && srcRT) {
        if (srcRT->numColorSamples() != dstRT->numColorSamples()) {
            return false;
        }
    } else if (dstRT) {
        if (dstRT->numColorSamples() > 1) {
            return false;
        }
    } else if (srcRT) {
        if (srcRT->numColorSamples() > 1) {
            return false;
        }
    }

    // We require that all vulkan GrSurfaces have been created with transfer_dst and transfer_src
    // as image usage flags.
    if (srcOrigin == dstOrigin &&
        GrBytesPerPixel(src->config()) == GrBytesPerPixel(dst->config())) {
        return true;
    }

    return false;
}

void GrVkGpu::copySurfaceAsCopyImage(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                     GrSurface* src, GrSurfaceOrigin srcOrigin,
                                     GrVkImage* dstImage,
                                     GrVkImage* srcImage,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    SkASSERT(can_copy_image(dst, dstOrigin, src, srcOrigin, this));

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

    // Flip rect if necessary
    SkIRect srcVkRect = srcRect;
    int32_t dstY = dstPoint.fY;

    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == dstOrigin);
        srcVkRect.fTop = src->height() - srcRect.fBottom;
        srcVkRect.fBottom =  src->height() - srcRect.fTop;
        dstY = dst->height() - dstPoint.fY - srcVkRect.height();
    }

    VkImageCopy copyRegion;
    memset(&copyRegion, 0, sizeof(VkImageCopy));
    copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.srcOffset = { srcVkRect.fLeft, srcVkRect.fTop, 0 };
    copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.dstOffset = { dstPoint.fX, dstY, 0 };
    copyRegion.extent = { (uint32_t)srcVkRect.width(), (uint32_t)srcVkRect.height(), 1 };

    fCurrentCmdBuffer->copyImage(this,
                                 srcImage,
                                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 dstImage,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 1,
                                 &copyRegion);

    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    this->didWriteToSurface(dst, &dstRect);
}

inline bool can_copy_as_blit(const GrSurface* dst,
                             const GrSurface* src,
                             const GrVkImage* dstImage,
                             const GrVkImage* srcImage,
                             const GrVkGpu* gpu) {
    // We require that all vulkan GrSurfaces have been created with transfer_dst and transfer_src
    // as image usage flags.
    const GrVkCaps& caps = gpu->vkCaps();
    if (!caps.configCanBeDstofBlit(dst->config(), dstImage->isLinearTiled()) ||
        !caps.configCanBeSrcofBlit(src->config(), srcImage->isLinearTiled())) {
        return false;
    }

    // We cannot blit images that are multisampled. Will need to figure out if we can blit the
    // resolved msaa though.
    if ((dst->asRenderTarget() && dst->asRenderTarget()->numColorSamples() > 1) ||
        (src->asRenderTarget() && src->asRenderTarget()->numColorSamples() > 1)) {
        return false;
    }

    return true;
}

void GrVkGpu::copySurfaceAsBlit(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                GrSurface* src, GrSurfaceOrigin srcOrigin,
                                GrVkImage* dstImage,
                                GrVkImage* srcImage,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    SkASSERT(can_copy_as_blit(dst, src, dstImage, srcImage, this));

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
    SkIRect srcVkRect;
    srcVkRect.fLeft = srcRect.fLeft;
    srcVkRect.fRight = srcRect.fRight;
    SkIRect dstRect;
    dstRect.fLeft = dstPoint.fX;
    dstRect.fRight = dstPoint.fX + srcRect.width();

    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        srcVkRect.fTop = src->height() - srcRect.fBottom;
        srcVkRect.fBottom = src->height() - srcRect.fTop;
    } else {
        srcVkRect.fTop = srcRect.fTop;
        srcVkRect.fBottom = srcRect.fBottom;
    }

    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
        dstRect.fTop = dst->height() - dstPoint.fY - srcVkRect.height();
    } else {
        dstRect.fTop = dstPoint.fY;
    }
    dstRect.fBottom = dstRect.fTop + srcVkRect.height();

    // If we have different origins, we need to flip the top and bottom of the dst rect so that we
    // get the correct origintation of the copied data.
    if (srcOrigin != dstOrigin) {
        SkTSwap(dstRect.fTop, dstRect.fBottom);
    }

    VkImageBlit blitRegion;
    memset(&blitRegion, 0, sizeof(VkImageBlit));
    blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    blitRegion.srcOffsets[0] = { srcVkRect.fLeft, srcVkRect.fTop, 0 };
    blitRegion.srcOffsets[1] = { srcVkRect.fRight, srcVkRect.fBottom, 1 };
    blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    blitRegion.dstOffsets[0] = { dstRect.fLeft, dstRect.fTop, 0 };
    blitRegion.dstOffsets[1] = { dstRect.fRight, dstRect.fBottom, 1 };

    fCurrentCmdBuffer->blitImage(this,
                                 *srcImage,
                                 *dstImage,
                                 1,
                                 &blitRegion,
                                 VK_FILTER_NEAREST); // We never scale so any filter works here

    this->didWriteToSurface(dst, &dstRect);
}

inline bool can_copy_as_resolve(const GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                const GrSurface* src, GrSurfaceOrigin srcOrigin,
                                const GrVkGpu* gpu) {
    // Our src must be a multisampled render target
    if (!src->asRenderTarget() || 1 == src->asRenderTarget()->numColorSamples()) {
        return false;
    }

    // The dst must not be a multisampled render target, expect in the case where the dst is the
    // resolve texture connected to the msaa src. We check for this in case we are copying a part of
    // a surface to a different region in the same surface.
    if (dst->asRenderTarget() && dst->asRenderTarget()->numColorSamples() > 1 && dst != src) {
        return false;
    }

    // Surfaces must have the same origin.
    if (srcOrigin != dstOrigin) {
        return false;
    }

    return true;
}

void GrVkGpu::copySurfaceAsResolve(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                   GrSurface* src, GrSurfaceOrigin srcOrigin,
                                   const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    GrVkRenderTarget* srcRT = static_cast<GrVkRenderTarget*>(src->asRenderTarget());
    this->resolveImage(dst, dstOrigin, srcRT, srcOrigin, srcRect, dstPoint);
}

bool GrVkGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                            GrSurface* src, GrSurfaceOrigin srcOrigin,
                            const SkIRect& srcRect, const SkIPoint& dstPoint,
                            bool canDiscardOutsideDstRect) {
    if (can_copy_as_resolve(dst, dstOrigin, src, srcOrigin, this)) {
        this->copySurfaceAsResolve(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint);
        return true;
    }

    if (this->vkCaps().mustSubmitCommandsBeforeCopyOp()) {
        this->submitCommandBuffer(GrVkGpu::kSkip_SyncQueue);
    }

    if (fCopyManager.copySurfaceAsDraw(this, dst, dstOrigin, src, srcOrigin, srcRect, dstPoint,
                                       canDiscardOutsideDstRect)) {
        auto dstRect = srcRect.makeOffset(dstPoint.fX, dstPoint.fY);
        this->didWriteToSurface(dst, &dstRect);
        return true;
    }

    GrVkImage* dstImage;
    GrVkImage* srcImage;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(dstRT);
        dstImage = vkRT->numColorSamples() > 1 ? vkRT->msaaImage() : vkRT;
    } else {
        SkASSERT(dst->asTexture());
        dstImage = static_cast<GrVkTexture*>(dst->asTexture());
    }
    GrRenderTarget* srcRT = src->asRenderTarget();
    if (srcRT) {
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(srcRT);
        srcImage = vkRT->numColorSamples() > 1 ? vkRT->msaaImage() : vkRT;
    } else {
        SkASSERT(src->asTexture());
        srcImage = static_cast<GrVkTexture*>(src->asTexture());
    }

    // For borrowed textures, we *only* want to copy using draws (to avoid layout changes)
    if (srcImage->isBorrowed()) {
        return false;
    }

    if (can_copy_image(dst, dstOrigin, src, srcOrigin, this)) {
        this->copySurfaceAsCopyImage(dst, dstOrigin, src, srcOrigin, dstImage, srcImage,
                                     srcRect, dstPoint);
        return true;
    }

    if (can_copy_as_blit(dst, src, dstImage, srcImage, this)) {
        this->copySurfaceAsBlit(dst, dstOrigin, src, srcOrigin, dstImage, srcImage,
                                srcRect, dstPoint);
        return true;
    }

    return false;
}

void GrVkGpu::onQueryMultisampleSpecs(GrRenderTarget* rt, GrSurfaceOrigin, const GrStencilSettings&,
                                      int* effectiveSampleCnt, SamplePattern*) {
    // TODO: stub.
    SkASSERT(!this->caps()->sampleLocationsSupport());
    *effectiveSampleCnt = rt->numStencilSamples();
}

bool GrVkGpu::onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                                  int width, int height, size_t rowBytes,
                                  GrPixelConfig readConfig, DrawPreference* drawPreference,
                                  ReadPixelTempDrawInfo* tempDrawInfo) {
    // These settings we will always want if a temp draw is performed.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 1;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.
    tempDrawInfo->fTempSurfaceFit = SkBackingFit::kApprox;

    // For now assume no swizzling, we may change that below.
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();

    // Depends on why we need/want a temp draw. Start off assuming no change, the surface we read
    // from will be srcConfig and we will read readConfig pixels from it.
    // Note that if we require a draw and return a non-renderable format for the temp surface the
    // base class will fail for us.
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcSurface->config();
    tempDrawInfo->fReadConfig = readConfig;

    if (srcSurface->config() == readConfig) {
        return true;
    }

    // Any config change requires a draw
    ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    tempDrawInfo->fTempSurfaceDesc.fConfig = readConfig;
    tempDrawInfo->fReadConfig = readConfig;

    return true;
}

bool GrVkGpu::onReadPixels(GrSurface* surface, GrSurfaceOrigin origin,
                           int left, int top, int width, int height,
                           GrPixelConfig config,
                           void* buffer,
                           size_t rowBytes) {
    VkFormat pixelFormat;
    if (!GrPixelConfigToVkFormat(config, &pixelFormat)) {
        return false;
    }

    GrVkImage* image = nullptr;
    GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(surface->asRenderTarget());
    if (rt) {
        // resolve the render target if necessary
        switch (rt->getResolveType()) {
            case GrVkRenderTarget::kCantResolve_ResolveType:
                return false;
            case GrVkRenderTarget::kAutoResolves_ResolveType:
                break;
            case GrVkRenderTarget::kCanResolve_ResolveType:
                this->internalResolveRenderTarget(rt, origin, false);
                break;
            default:
                SK_ABORT("Unknown resolve type");
        }
        image = rt;
    } else {
        image = static_cast<GrVkTexture*>(surface->asTexture());
    }

    if (!image) {
        return false;
    }

    // Change layout of our target so it can be used as copy
    image->setImageLayout(this,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_ACCESS_TRANSFER_READ_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          false);

    size_t bpp = GrBytesPerPixel(config);
    size_t tightRowBytes = bpp * width;
    bool flipY = kBottomLeft_GrSurfaceOrigin == origin;

    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));

    bool copyFromOrigin = this->vkCaps().mustDoCopiesFromOrigin();
    if (copyFromOrigin) {
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { (uint32_t)(left + width),
                               (uint32_t)(flipY ? surface->height() - top : top + height),
                               1
                             };
    } else {
        VkOffset3D offset = {
            left,
            flipY ? surface->height() - top - height : top,
            0
        };
        region.imageOffset = offset;
        region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };
    }

    size_t transBufferRowBytes = bpp * region.imageExtent.width;
    size_t imageRows = bpp * region.imageExtent.height;
    GrVkTransferBuffer* transferBuffer =
            static_cast<GrVkTransferBuffer*>(this->createBuffer(transBufferRowBytes * imageRows,
                                                                kXferGpuToCpu_GrBufferType,
                                                                kStream_GrAccessPattern));

    // Copy the image to a buffer so we can map it to cpu memory
    region.bufferOffset = transferBuffer->offset();
    region.bufferRowLength = 0; // Forces RowLength to be width. We handle the rowBytes below.
    region.bufferImageHeight = 0; // Forces height to be tightly packed. Only useful for 3d images.
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

    fCurrentCmdBuffer->copyImageToBuffer(this,
                                         image,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         transferBuffer,
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
    this->submitCommandBuffer(kForce_SyncQueue);
    void* mappedMemory = transferBuffer->map();
    const GrVkAlloc& transAlloc = transferBuffer->alloc();
    GrVkMemory::InvalidateMappedAlloc(this, transAlloc, transAlloc.fOffset, VK_WHOLE_SIZE);

    if (copyFromOrigin) {
        uint32_t skipRows = region.imageExtent.height - height;
        mappedMemory = (char*)mappedMemory + transBufferRowBytes * skipRows + bpp * left;
    }

    if (flipY) {
        const char* srcRow = reinterpret_cast<const char*>(mappedMemory);
        char* dstRow = reinterpret_cast<char*>(buffer)+(height - 1) * rowBytes;
        for (int y = 0; y < height; y++) {
            memcpy(dstRow, srcRow, tightRowBytes);
            srcRow += transBufferRowBytes;
            dstRow -= rowBytes;
        }
    } else {
        SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, tightRowBytes, height);
    }

    transferBuffer->unmap();
    transferBuffer->unref();
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

void GrVkGpu::submitSecondaryCommandBuffer(const SkTArray<GrVkSecondaryCommandBuffer*>& buffers,
                                           const GrVkRenderPass* renderPass,
                                           const VkClearValue* colorClear,
                                           GrVkRenderTarget* target, GrSurfaceOrigin origin,
                                           const SkIRect& bounds) {
    const SkIRect* pBounds = &bounds;
    SkIRect flippedBounds;
    if (kBottomLeft_GrSurfaceOrigin == origin) {
        flippedBounds = bounds;
        flippedBounds.fTop = target->height() - bounds.fBottom;
        flippedBounds.fBottom = target->height() - bounds.fTop;
        pBounds = &flippedBounds;
    }

    // The bounds we use for the render pass should be of the granularity supported
    // by the device.
    const VkExtent2D& granularity = renderPass->granularity();
    SkIRect adjustedBounds;
    if ((0 != granularity.width && 1 != granularity.width) ||
        (0 != granularity.height && 1 != granularity.height)) {
        adjust_bounds_to_granularity(&adjustedBounds, *pBounds, granularity,
                                     target->width(), target->height());
        pBounds = &adjustedBounds;
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

    fCurrentCmdBuffer->beginRenderPass(this, renderPass, clears, *target, *pBounds, true);
    for (int i = 0; i < buffers.count(); ++i) {
        fCurrentCmdBuffer->executeCommands(this, buffers[i]);
    }
    fCurrentCmdBuffer->endRenderPass(this);

    this->didWriteToSurface(target, &bounds);
}

GrFence SK_WARN_UNUSED_RESULT GrVkGpu::insertFence() {
    VkFenceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkFenceCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    VkFence fence = VK_NULL_HANDLE;

    VK_CALL_ERRCHECK(CreateFence(this->device(), &createInfo, nullptr, &fence));
    VK_CALL(QueueSubmit(this->queue(), 0, nullptr, fence));

    GR_STATIC_ASSERT(sizeof(GrFence) >= sizeof(VkFence));
    return (GrFence)fence;
}

bool GrVkGpu::waitFence(GrFence fence, uint64_t timeout) {
    SkASSERT(VK_NULL_HANDLE != (VkFence)fence);

    VkResult result = VK_CALL(WaitForFences(this->device(), 1, (VkFence*)&fence, VK_TRUE, timeout));
    return (VK_SUCCESS == result);
}

void GrVkGpu::deleteFence(GrFence fence) const {
    VK_CALL(DestroyFence(this->device(), (VkFence)fence, nullptr));
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrVkGpu::makeSemaphore(bool isOwned) {
    return GrVkSemaphore::Make(this, isOwned);
}

sk_sp<GrSemaphore> GrVkGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                 GrResourceProvider::SemaphoreWrapType wrapType,
                                                 GrWrapOwnership ownership) {
    return GrVkSemaphore::MakeWrapped(this, semaphore.vkSemaphore(), wrapType, ownership);
}

void GrVkGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) {
    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore.get());

    GrVkSemaphore::Resource* resource = vkSem->getResource();
    if (resource->shouldSignal()) {
        resource->ref();
        fSemaphoresToSignal.push_back(resource);
    }

    if (flush) {
        this->submitCommandBuffer(kSkip_SyncQueue);
    }
}

void GrVkGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore.get());

    GrVkSemaphore::Resource* resource = vkSem->getResource();
    if (resource->shouldWait()) {
        resource->ref();
        fSemaphoresToWaitOn.push_back(resource);
    }
}

sk_sp<GrSemaphore> GrVkGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(texture);
    GrVkTexture* vkTexture = static_cast<GrVkTexture*>(texture);
    vkTexture->setImageLayout(this,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT,
                              VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                              false);
    this->submitCommandBuffer(kSkip_SyncQueue);

    // The image layout change serves as a barrier, so no semaphore is needed
    return nullptr;
}

