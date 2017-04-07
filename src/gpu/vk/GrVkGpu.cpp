/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkGpu.h"

#include "GrContextOptions.h"
#include "GrGeometryProcessor.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrMesh.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrSurfacePriv.h"
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

GrGpu* GrVkGpu::Create(GrBackendContext backendContext, const GrContextOptions& options,
                       GrContext* context) {
    const GrVkBackendContext* vkBackendContext =
        reinterpret_cast<const GrVkBackendContext*>(backendContext);
    if (!vkBackendContext) {
        vkBackendContext = GrVkBackendContext::Create();
        if (!vkBackendContext) {
            return nullptr;
        }
    } else {
        vkBackendContext->ref();
    }

    if (!vkBackendContext->fInterface->validate(vkBackendContext->fExtensions)) {
        return nullptr;
    }

    return new GrVkGpu(context, options, vkBackendContext);
}

////////////////////////////////////////////////////////////////////////////////

GrVkGpu::GrVkGpu(GrContext* context, const GrContextOptions& options,
                 const GrVkBackendContext* backendCtx)
    : INHERITED(context)
    , fDevice(backendCtx->fDevice)
    , fQueue(backendCtx->fQueue)
    , fResourceProvider(this) {
    fBackendContext.reset(backendCtx);

#ifdef SK_ENABLE_VK_LAYERS
    fCallback = VK_NULL_HANDLE;
    if (backendCtx->fExtensions & kEXT_debug_report_GrVkExtensionFlag) {
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
        GR_VK_CALL_ERRCHECK(this->vkInterface(), CreateDebugReportCallbackEXT(
                            backendCtx->fInstance, &callbackCreateInfo, nullptr, &fCallback));
    }
#endif

    fCompiler = new SkSL::Compiler();

    fVkCaps.reset(new GrVkCaps(options, this->vkInterface(), backendCtx->fPhysicalDevice,
                               backendCtx->fFeatures, backendCtx->fExtensions));
    fCaps.reset(SkRef(fVkCaps.get()));

    VK_CALL(GetPhysicalDeviceMemoryProperties(backendCtx->fPhysicalDevice, &fPhysDevMemProps));

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,      // sType
        nullptr,                                         // pNext
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // CmdPoolCreateFlags
        backendCtx->fGraphicsQueueIndex,                 // queueFamilyIndex
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
    // We want the OptimalImage_Heap to use a SubAlloc_strategy but it occasionally causes the
    // device to run out of memory. Most likely this is caused by fragmentation in the device heap
    // and we can't allocate more. Until we get a fix moving this to SingleAlloc.
    fHeaps[kOptimalImage_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 64*1024*1024));
    fHeaps[kSmallOptimalImage_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 2*1024*1024));
    fHeaps[kVertexBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kIndexBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kUniformBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 256*1024));
    fHeaps[kCopyReadBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSingleAlloc_Strategy, 0));
    fHeaps[kCopyWriteBuffer_Heap].reset(new GrVkHeap(this, GrVkHeap::kSubAlloc_Strategy, 16*1024*1024));
}

GrVkGpu::~GrVkGpu() {
    fCurrentCmdBuffer->end(this);
    fCurrentCmdBuffer->unref(this);

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

    fCopyManager.destroyResources(this);

    // must call this just before we destroy the command pool and VkDevice
    fResourceProvider.destroyResources(VK_ERROR_DEVICE_LOST == res);

    VK_CALL(DestroyCommandPool(fDevice, fCmdPool, nullptr));

    delete fCompiler;

#ifdef SK_ENABLE_VK_LAYERS
    if (fCallback) {
        VK_CALL(DestroyDebugReportCallbackEXT(fBackendContext->fInstance, fCallback, nullptr));
        fCallback = VK_NULL_HANDLE;
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

GrGpuCommandBuffer* GrVkGpu::createCommandBuffer(
            const GrGpuCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuCommandBuffer::LoadAndStoreInfo& stencilInfo) {
    return new GrVkGpuCommandBuffer(this, colorInfo, stencilInfo);
}

void GrVkGpu::submitCommandBuffer(SyncQueue sync,
                                  const GrVkSemaphore::Resource* signalSemaphore) {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->end(this);

    fCurrentCmdBuffer->submitToQueue(this, fQueue, sync, signalSemaphore, fSemaphoresToWaitOn);

    for (int i = 0; i < fSemaphoresToWaitOn.count(); ++i) {
        fSemaphoresToWaitOn[i]->unref(this);
    }
    fSemaphoresToWaitOn.reset();

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
            SkASSERT(kStream_GrAccessPattern == accessPattern);
            buff = GrVkTransferBuffer::Create(this, size, GrVkBuffer::kCopyRead_Type);
            break;
        case kXferGpuToCpu_GrBufferType:
            SkASSERT(kStream_GrAccessPattern == accessPattern);
            buff = GrVkTransferBuffer::Create(this, size, GrVkBuffer::kCopyWrite_Type);
            break;
        default:
            SkFAIL("Unknown buffer type.");
            return nullptr;
    }
    if (data && buff) {
        buff->updateData(data, size);
    }
    return buff;
}

////////////////////////////////////////////////////////////////////////////////
bool GrVkGpu::onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                                   GrPixelConfig srcConfig, DrawPreference* drawPreference,
                                   WritePixelTempDrawInfo* tempDrawInfo) {
    if (GrPixelConfigIsCompressed(dstSurface->config())) {
        return false;
    }

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
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 0;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin;

    if (dstSurface->config() == srcConfig) {
        return true;
    }

    if (renderTarget && this->vkCaps().isConfigRenderable(renderTarget->config(),
                                                          renderTarget->numColorSamples() > 1)) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);

        bool configsAreRBSwaps = GrPixelConfigSwapRAndB(srcConfig) == dstSurface->config();

        if (!this->vkCaps().isConfigTexturable(srcConfig) && configsAreRBSwaps) {
            if (!this->vkCaps().isConfigTexturable(dstSurface->config())) {
                return false;
            }
            tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
            tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
            tempDrawInfo->fWriteConfig = dstSurface->config();
        }
        return true;
    }

    return false;
}

bool GrVkGpu::onWritePixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrPixelConfig config,
                            const SkTArray<GrMipLevel>& texels) {
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(surface->asTexture());
    if (!vkTex) {
        return false;
    }

    // Make sure we have at least the base level
    if (texels.empty() || !texels.begin()->fPixels) {
        return false;
    }

    // We assume Vulkan doesn't do sRGB <-> linear conversions when reading and writing pixels.
    if (GrPixelConfigIsSRGB(surface->config()) != GrPixelConfigIsSRGB(config)) {
        return false;
    }

    bool success = false;
    if (GrPixelConfigIsCompressed(vkTex->desc().fConfig)) {
        // We check that config == desc.fConfig in GrGpu::getWritePixelsInfo()
        SkASSERT(config == vkTex->desc().fConfig);
        // TODO: add compressed texture support
        // delete the following two lines and uncomment the two after that when ready
        vkTex->unref();
        return false;
        //success = this->uploadCompressedTexData(vkTex->desc(), buffer, false, left, top, width,
        //                                       height);
    } else {
        bool linearTiling = vkTex->isLinearTiled();
        if (linearTiling) {
            if (texels.count() > 1) {
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
            success = this->uploadTexDataLinear(vkTex, left, top, width, height, config,
                                                texels.begin()->fPixels, texels.begin()->fRowBytes);
        } else {
            int newMipLevels = texels.count();
            int currentMipLevels = vkTex->texturePriv().maxMipMapLevel() + 1;
            if (newMipLevels > currentMipLevels) {
                if (!vkTex->reallocForMipmap(this, newMipLevels)) {
                    return false;
                }
            }
            success = this->uploadTexDataOptimal(vkTex, left, top, width, height, config, texels);
        }
    }

    return success;
}

void GrVkGpu::resolveImage(GrVkRenderTarget* dst, GrVkRenderTarget* src, const SkIRect& srcRect,
                           const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src && src->numColorSamples() > 1 && src->msaaImage());

    if (this->vkCaps().mustSubmitCommandsBeforeCopyOp()) {
        this->submitCommandBuffer(GrVkGpu::kSkip_SyncQueue);
    }

    // Flip rect if necessary
    SkIRect srcVkRect = srcRect;
    int32_t dstY = dstPoint.fY;

    if (kBottomLeft_GrSurfaceOrigin == src->origin()) {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == dst->origin());
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

    dst->setImageLayout(this,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        false);

    src->msaaImage()->setImageLayout(this,
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     VK_ACCESS_TRANSFER_READ_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     false);

    fCurrentCmdBuffer->resolveImage(this, *src->msaaImage(), *dst, 1, &resolveInfo);
}

void GrVkGpu::internalResolveRenderTarget(GrRenderTarget* target, bool requiresSubmit) {
    if (target->needsResolve()) {
        SkASSERT(target->numColorSamples() > 1);
        GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(target);
        SkASSERT(rt->msaaImage());

        const SkIRect& srcRect = rt->getResolveRect();

        this->resolveImage(rt, rt, srcRect, SkIPoint::Make(srcRect.fLeft, srcRect.fTop));

        rt->flagAsResolved();

        if (requiresSubmit) {
            this->submitCommandBuffer(kSkip_SyncQueue);
        }
    }
}

bool GrVkGpu::uploadTexDataLinear(GrVkTexture* tex,
                                  int left, int top, int width, int height,
                                  GrPixelConfig dataConfig,
                                  const void* data,
                                  size_t rowBytes) {
    SkASSERT(data);
    SkASSERT(tex->isLinearTiled());

    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrPixelConfigIsCompressed(dataConfig));

    size_t bpp = GrBytesPerPixel(dataConfig);

    const GrSurfaceDesc& desc = tex->desc();

    if (!GrSurfacePriv::AdjustWritePixelParams(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                               &width, &height, &data, &rowBytes)) {
        return false;
    }
    size_t trimRowBytes = width * bpp;

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

    int texTop = kBottomLeft_GrSurfaceOrigin == desc.fOrigin ? tex->height() - top - height : top;
    const GrVkAlloc& alloc = tex->alloc();
    VkDeviceSize offset = alloc.fOffset + texTop*layout.rowPitch + left*bpp;
    VkDeviceSize size = height*layout.rowPitch;
    void* mapPtr;
    err = GR_VK_CALL(interface, MapMemory(fDevice, alloc.fMemory, offset, size, 0, &mapPtr));
    if (err) {
        return false;
    }

    if (kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
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

    GrVkMemory::FlushMappedAlloc(this, alloc);
    GR_VK_CALL(interface, UnmapMemory(fDevice, alloc.fMemory));

    return true;
}

bool GrVkGpu::uploadTexDataOptimal(GrVkTexture* tex,
                                   int left, int top, int width, int height,
                                   GrPixelConfig dataConfig,
                                   const SkTArray<GrMipLevel>& texels) {
    SkASSERT(!tex->isLinearTiled());
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(1 == texels.count() ||
             (0 == left && 0 == top && width == tex->width() && height == tex->height()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(1 == texels.count() || texels.count() == (tex->texturePriv().maxMipMapLevel() + 1));

    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrPixelConfigIsCompressed(dataConfig));

    if (width == 0 || height == 0) {
        return false;
    }

    const GrSurfaceDesc& desc = tex->desc();
    SkASSERT(this->caps()->isConfigTexturable(desc.fConfig));
    size_t bpp = GrBytesPerPixel(dataConfig);

    // texels is const.
    // But we may need to adjust the fPixels ptr based on the copyRect, or fRowBytes.
    // Because of this we need to make a non-const shallow copy of texels.
    SkTArray<GrMipLevel> texelsShallowCopy(texels);

    for (int currentMipLevel = texelsShallowCopy.count() - 1; currentMipLevel >= 0;
         currentMipLevel--) {
        SkASSERT(texelsShallowCopy[currentMipLevel].fPixels);
    }

    // Determine whether we need to flip when we copy into the buffer
    bool flipY = (kBottomLeft_GrSurfaceOrigin == desc.fOrigin && !texelsShallowCopy.empty());

    // adjust any params (left, top, currentWidth, currentHeight
    // find the combined size of all the mip levels and the relative offset of
    // each into the collective buffer
    // Do the first level separately because we may need to adjust width and height
    // (for the non-mipped case).
    if (!GrSurfacePriv::AdjustWritePixelParams(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                               &width,
                                               &height,
                                               &texelsShallowCopy[0].fPixels,
                                               &texelsShallowCopy[0].fRowBytes)) {
        return false;
    }
    SkTArray<size_t> individualMipOffsets(texelsShallowCopy.count());
    individualMipOffsets.push_back(0);
    size_t combinedBufferSize = width * bpp * height;
    int currentWidth = width;
    int currentHeight = height;
    // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
    // config. This works with the assumption that the bytes in pixel config is always a power of 2.
    SkASSERT((bpp & (bpp - 1)) == 0);
    const size_t alignmentMask = 0x3 | (bpp - 1);
    for (int currentMipLevel = 1; currentMipLevel < texelsShallowCopy.count(); currentMipLevel++) {
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
        if (!GrSurfacePriv::AdjustWritePixelParams(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                                   &currentWidth,
                                                   &currentHeight,
                                                   &texelsShallowCopy[currentMipLevel].fPixels,
                                                   &texelsShallowCopy[currentMipLevel].fRowBytes)) {
            return false;
        }
        const size_t trimmedSize = currentWidth * bpp * currentHeight;
        const size_t alignmentDiff = combinedBufferSize & alignmentMask;
        if (alignmentDiff != 0) {
           combinedBufferSize += alignmentMask - alignmentDiff + 1;
        }
        individualMipOffsets.push_back(combinedBufferSize);
        combinedBufferSize += trimmedSize;
    }

    // allocate buffer to hold our mip data
    GrVkTransferBuffer* transferBuffer =
                   GrVkTransferBuffer::Create(this, combinedBufferSize, GrVkBuffer::kCopyRead_Type);

    char* buffer = (char*) transferBuffer->map();
    SkTArray<VkBufferImageCopy> regions(texelsShallowCopy.count());

    currentWidth = width;
    currentHeight = height;
    int layerHeight = tex->height();
    for (int currentMipLevel = 0; currentMipLevel < texelsShallowCopy.count(); currentMipLevel++) {
        SkASSERT(1 == texelsShallowCopy.count() || currentHeight == layerHeight);
        const size_t trimRowBytes = currentWidth * bpp;
        const size_t rowBytes = texelsShallowCopy[currentMipLevel].fRowBytes;

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
    if (1 == texelsShallowCopy.count()) {
       tex->texturePriv().dirtyMipMaps(true);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
GrTexture* GrVkGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                    const SkTArray<GrMipLevel>& texels) {
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

    bool linearTiling = false;
    if (SkToBool(desc.fFlags & kZeroCopy_GrSurfaceFlag)) {
        // we can't have a linear texture with a mipmap
        if (texels.count() > 1) {
            SkDebugf("Trying to create linear tiled texture with mipmap");
            return nullptr;
        }
        if (fVkCaps->isConfigTexturableLinearly(desc.fConfig) &&
            (!renderTarget || fVkCaps->isConfigRenderableLinearly(desc.fConfig, false))) {
            linearTiling = true;
        } else {
            return nullptr;
        }
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

    VkFlags memProps = (!texels.empty() && linearTiling) ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT :
                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // This ImageDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this ImageDesc describes the resolved texture. Therefore we always have samples set
    // to 1.
    int mipLevels = texels.empty() ? 1 : texels.count();
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = pixelFormat;
    imageDesc.fWidth = desc.fWidth;
    imageDesc.fHeight = desc.fHeight;
    imageDesc.fLevels = linearTiling ? 1 : mipLevels;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = memProps;

    GrVkTexture* tex;
    if (renderTarget) {
        tex = GrVkTextureRenderTarget::CreateNewTextureRenderTarget(this, budgeted, desc,
                                                                    imageDesc);
    } else {
        tex = GrVkTexture::CreateNewTexture(this, budgeted, desc, imageDesc);
    }

    if (!tex) {
        return nullptr;
    }

    if (!texels.empty()) {
        SkASSERT(texels.begin()->fPixels);
        bool success;
        if (linearTiling) {
            success = this->uploadTexDataLinear(tex, 0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                                texels.begin()->fPixels, texels.begin()->fRowBytes);
        } else {
            success = this->uploadTexDataOptimal(tex, 0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                                 texels);
        }
        if (!success) {
            tex->unref();
            return nullptr;
        }
    }

    return tex;
}

////////////////////////////////////////////////////////////////////////////////

bool GrVkGpu::updateBuffer(GrVkBuffer* buffer, const void* src,
                           VkDeviceSize offset, VkDeviceSize size) {

    // Update the buffer
    fCurrentCmdBuffer->updateBuffer(this, buffer, offset, size, src);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

static GrSurfaceOrigin resolve_origin(GrSurfaceOrigin origin) {
    // By default, all textures in Vk use TopLeft
    if (kDefault_GrSurfaceOrigin == origin) {
        return kTopLeft_GrSurfaceOrigin;
    } else {
        return origin;
    }
}

sk_sp<GrTexture> GrVkGpu::onWrapBackendTexture(const GrBackendTextureDesc& desc,
                                               GrWrapOwnership ownership) {
    if (0 == desc.fTextureHandle) {
        return nullptr;
    }

    int maxSize = this->caps()->maxTextureSize();
    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }

    const GrVkImageInfo* info = reinterpret_cast<const GrVkImageInfo*>(desc.fTextureHandle);
    if (VK_NULL_HANDLE == info->fImage || VK_NULL_HANDLE == info->fAlloc.fMemory) {
        return nullptr;
    }
#ifdef SK_DEBUG
    VkFormat format;
    if (!GrPixelConfigToVkFormat(desc.fConfig, &format)) {
        return nullptr;
    }
    SkASSERT(format == info->fFormat);
#endif

    GrSurfaceDesc surfDesc;
    // next line relies on GrBackendTextureDesc's flags matching GrTexture's
    surfDesc.fFlags = (GrSurfaceFlags)desc.fFlags;
    surfDesc.fWidth = desc.fWidth;
    surfDesc.fHeight = desc.fHeight;
    surfDesc.fConfig = desc.fConfig;
    surfDesc.fSampleCnt = SkTMin(desc.fSampleCnt, this->caps()->maxSampleCount());
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrBackendTextureFlag);
    SkASSERT(!renderTarget || kAdoptAndCache_GrWrapOwnership != ownership);  // Not supported
    // In GL, Chrome assumes all textures are BottomLeft
    // In VK, we don't have this restriction
    surfDesc.fOrigin = resolve_origin(desc.fOrigin);

    if (!renderTarget) {
        return GrVkTexture::MakeWrappedTexture(this, surfDesc, ownership, info);
    }
    return GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(this, surfDesc, ownership, info);
}

sk_sp<GrRenderTarget> GrVkGpu::onWrapBackendRenderTarget(const GrBackendRenderTargetDesc& wrapDesc){

    const GrVkImageInfo* info =
        reinterpret_cast<const GrVkImageInfo*>(wrapDesc.fRenderTargetHandle);
    if (VK_NULL_HANDLE == info->fImage) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fConfig = wrapDesc.fConfig;
    desc.fFlags = kCheckAllocation_GrSurfaceFlag | kRenderTarget_GrSurfaceFlag;
    desc.fWidth = wrapDesc.fWidth;
    desc.fHeight = wrapDesc.fHeight;
    desc.fSampleCnt = SkTMin(wrapDesc.fSampleCnt, this->caps()->maxSampleCount());

    desc.fOrigin = resolve_origin(wrapDesc.fOrigin);

    sk_sp<GrVkRenderTarget> tgt = GrVkRenderTarget::MakeWrappedRenderTarget(this, desc, info);
    if (tgt && wrapDesc.fStencilBits) {
        if (!createStencilAttachmentForRenderTarget(tgt.get(), desc.fWidth, desc.fHeight)) {
            return nullptr;
        }
    }
    return tgt;
}

void GrVkGpu::generateMipmap(GrVkTexture* tex) {
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
        this->internalResolveRenderTarget(texRT, false);
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
    SkASSERT(GrVkFormatToPixelConfig(tex->imageFormat(), nullptr));
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        NULL,                                            // pNext
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

bool copy_testing_data(GrVkGpu* gpu, void* srcData, const GrVkAlloc& alloc,
                       size_t srcRowBytes, size_t dstRowBytes, int h) {
    void* mapPtr;
    VkResult err = GR_VK_CALL(gpu->vkInterface(), MapMemory(gpu->device(),
                                                            alloc.fMemory,
                                                            alloc.fOffset,
                                                            dstRowBytes * h,
                                                            0,
                                                            &mapPtr));
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
    GrVkMemory::FlushMappedAlloc(gpu, alloc);
    GR_VK_CALL(gpu->vkInterface(), UnmapMemory(gpu->device(), alloc.fMemory));
    return true;
}

GrBackendObject GrVkGpu::createTestingOnlyBackendTexture(void* srcData, int w, int h,
                                                         GrPixelConfig config,
                                                         bool isRenderTarget) {

    VkFormat pixelFormat;
    if (!GrPixelConfigToVkFormat(config, &pixelFormat)) {
        return 0;
    }

    bool linearTiling = false;
    if (!fVkCaps->isConfigTexturable(config)) {
        return 0;
    }

    if (isRenderTarget && !fVkCaps->isConfigRenderable(config, false)) {
        return 0;
    }

    if (fVkCaps->isConfigTexturableLinearly(config) &&
        (!isRenderTarget || fVkCaps->isConfigRenderableLinearly(config, false))) {
        linearTiling = true;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (isRenderTarget) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    VkImage image = VK_NULL_HANDLE;
    GrVkAlloc alloc = { VK_NULL_HANDLE, 0, 0, 0 };

    VkImageTiling imageTiling = linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    VkImageLayout initialLayout = (VK_IMAGE_TILING_LINEAR == imageTiling)
                                ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!GrSampleCountToVkSampleCount(1, &vkSamples)) {
        return 0;
    }

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        NULL,                                        // pNext
        0,                                           // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        pixelFormat,                                 // VkFormat
        { (uint32_t) w, (uint32_t) h, 1 },           // VkExtent3D
        1,                                           // mipLevels
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
        return 0;
    }

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

        if (!copy_testing_data(this, srcData, alloc, rowCopyBytes,
                               static_cast<size_t>(layout.rowPitch), h)) {
            GrVkMemory::FreeImageMemory(this, linearTiling, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            return 0;
        }
    } else {
        SkASSERT(w && h);

        VkBuffer buffer;
        VkBufferCreateInfo bufInfo;
        memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.flags = 0;
        bufInfo.size = rowCopyBytes * h;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufInfo.queueFamilyIndexCount = 0;
        bufInfo.pQueueFamilyIndices = nullptr;
        VkResult err;
        err = VK_CALL(CreateBuffer(fDevice, &bufInfo, nullptr, &buffer));

        if (err) {
            GrVkMemory::FreeImageMemory(this, linearTiling, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            return 0;
        }

        GrVkAlloc bufferAlloc = { VK_NULL_HANDLE, 0, 0, 0 };
        if (!GrVkMemory::AllocAndBindBufferMemory(this, buffer, GrVkBuffer::kCopyRead_Type,
                                                  true, &bufferAlloc)) {
            GrVkMemory::FreeImageMemory(this, linearTiling, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            return 0;
        }

        if (!copy_testing_data(this, srcData, bufferAlloc, rowCopyBytes, rowCopyBytes, h)) {
            GrVkMemory::FreeImageMemory(this, linearTiling, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            return 0;
        }

        const VkCommandBufferAllocateInfo cmdInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
            NULL,                                             // pNext
            fCmdPool,                                         // commandPool
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
            1                                                 // bufferCount
        };

        VkCommandBuffer cmdBuffer;
        err = VK_CALL(AllocateCommandBuffers(fDevice, &cmdInfo, &cmdBuffer));
        if (err) {
            GrVkMemory::FreeImageMemory(this, linearTiling, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            return 0;
        }

        VkCommandBufferBeginInfo cmdBufferBeginInfo;
        memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.pNext = nullptr;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmdBufferBeginInfo.pInheritanceInfo = nullptr;

        err = VK_CALL(BeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
        SkASSERT(!err);

        // Set image layout and add barrier
        VkImageMemoryBarrier barrier;
        memset(&barrier, 0, sizeof(VkImageMemoryBarrier));
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(initialLayout);
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0 , 1};

        VK_CALL(CmdPipelineBarrier(cmdBuffer,
                                   GrVkMemory::LayoutToPipelineStageFlags(initialLayout),
                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   0,
                                   0, nullptr,
                                   0, nullptr,
                                   1, &barrier));
        initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        // Submit copy command
        VkBufferImageCopy region;
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = 0;
        region.bufferRowLength = w;
        region.bufferImageHeight = h;
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { (uint32_t)w, (uint32_t)h, 1 };

        VK_CALL(CmdCopyBufferToImage(cmdBuffer, buffer, image, initialLayout, 1, &region));

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
            GrVkMemory::FreeImageMemory(this, linearTiling, alloc);
            VK_CALL(DestroyImage(fDevice, image, nullptr));
            GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
            VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
            VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
            VK_CALL(DestroyFence(fDevice, fence, nullptr));
            SkDebugf("Fence failed to signal: %d\n", err);
            SkFAIL("failing");
        }
        SkASSERT(!err);

        // Clean up transfer resources
        GrVkMemory::FreeBufferMemory(this, GrVkBuffer::kCopyRead_Type, bufferAlloc);
        VK_CALL(DestroyBuffer(fDevice, buffer, nullptr));
        VK_CALL(FreeCommandBuffers(fDevice, fCmdPool, 1, &cmdBuffer));
        VK_CALL(DestroyFence(fDevice, fence, nullptr));
    }

    GrVkImageInfo* info = new GrVkImageInfo;
    info->fImage = image;
    info->fAlloc = alloc;
    info->fImageTiling = imageTiling;
    info->fImageLayout = initialLayout;
    info->fFormat = pixelFormat;
    info->fLevelCount = 1;

    return (GrBackendObject)info;
}

bool GrVkGpu::isTestingOnlyBackendTexture(GrBackendObject id) const {
    const GrVkImageInfo* backend = reinterpret_cast<const GrVkImageInfo*>(id);

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

void GrVkGpu::deleteTestingOnlyBackendTexture(GrBackendObject id, bool abandon) {
    GrVkImageInfo* backend = reinterpret_cast<GrVkImageInfo*>(id);
    if (backend) {
        if (!abandon) {
            // something in the command buffer may still be using this, so force submit
            this->submitCommandBuffer(kForce_SyncQueue);
            GrVkImage::DestroyImageInfo(this, backend);
        }
        delete backend;
    }
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

void GrVkGpu::finishOpList() {
    // Submit the current command buffer to the Queue
    this->submitCommandBuffer(kSkip_SyncQueue);
}

void GrVkGpu::clearStencil(GrRenderTarget* target) {
    if (nullptr == target) {
        return;
    }
    GrStencilAttachment* stencil = target->renderTargetPriv().getStencilAttachment();
    GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)stencil;


    VkClearDepthStencilValue vkStencilColor;
    memset(&vkStencilColor, 0, sizeof(VkClearDepthStencilValue));

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

inline bool can_copy_image(const GrSurface* dst,
                           const GrSurface* src,
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
    if (src->origin() == dst->origin() &&
        GrBytesPerPixel(src->config()) == GrBytesPerPixel(dst->config())) {
        return true;
    }

    return false;
}

void GrVkGpu::copySurfaceAsCopyImage(GrSurface* dst,
                                     GrSurface* src,
                                     GrVkImage* dstImage,
                                     GrVkImage* srcImage,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    SkASSERT(can_copy_image(dst, src, this));

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

    if (kBottomLeft_GrSurfaceOrigin == src->origin()) {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == dst->origin());
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

void GrVkGpu::copySurfaceAsBlit(GrSurface* dst,
                                GrSurface* src,
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

    if (kBottomLeft_GrSurfaceOrigin == src->origin()) {
        srcVkRect.fTop = src->height() - srcRect.fBottom;
        srcVkRect.fBottom = src->height() - srcRect.fTop;
    } else {
        srcVkRect.fTop = srcRect.fTop;
        srcVkRect.fBottom = srcRect.fBottom;
    }

    if (kBottomLeft_GrSurfaceOrigin == dst->origin()) {
        dstRect.fTop = dst->height() - dstPoint.fY - srcVkRect.height();
    } else {
        dstRect.fTop = dstPoint.fY;
    }
    dstRect.fBottom = dstRect.fTop + srcVkRect.height();

    // If we have different origins, we need to flip the top and bottom of the dst rect so that we
    // get the correct origintation of the copied data.
    if (src->origin() != dst->origin()) {
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

inline bool can_copy_as_resolve(const GrSurface* dst,
                                const GrSurface* src,
                                const GrVkGpu* gpu) {
    // Our src must be a multisampled render target
    if (!src->asRenderTarget() || src->asRenderTarget()->numColorSamples() <= 1) {
        return false;
    }

    // The dst must be a render target but not multisampled
    if (!dst->asRenderTarget() || dst->asRenderTarget()->numColorSamples() > 1) {
        return false;
    }

    // Surfaces must have the same origin.
    if (src->origin() != dst->origin()) {
        return false;
    }

    return true;
}

void GrVkGpu::copySurfaceAsResolve(GrSurface* dst,
                                   GrSurface* src,
                                   const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    GrVkRenderTarget* dstRT = static_cast<GrVkRenderTarget*>(dst->asRenderTarget());
    GrVkRenderTarget* srcRT = static_cast<GrVkRenderTarget*>(src->asRenderTarget());
    SkASSERT(dstRT && dstRT->numColorSamples() <= 1);
    this->resolveImage(dstRT, srcRT, srcRect, dstPoint);
}

bool GrVkGpu::onCopySurface(GrSurface* dst,
                            GrSurface* src,
                            const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    if (can_copy_as_resolve(dst, src, this)) {
        this->copySurfaceAsResolve(dst, src, srcRect, dstPoint);
        return true;
    }

    if (this->vkCaps().mustSubmitCommandsBeforeCopyOp()) {
        this->submitCommandBuffer(GrVkGpu::kSkip_SyncQueue);
    }

    if (fCopyManager.copySurfaceAsDraw(this, dst, src, srcRect, dstPoint)) {
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

    if (can_copy_image(dst, src, this)) {
        this->copySurfaceAsCopyImage(dst, src, dstImage, srcImage, srcRect, dstPoint);
        return true;
    }

    if (can_copy_as_blit(dst, src, dstImage, srcImage, this)) {
        this->copySurfaceAsBlit(dst, src, dstImage, srcImage, srcRect, dstPoint);
        return true;
    }

    return false;
}

void GrVkGpu::onQueryMultisampleSpecs(GrRenderTarget* rt, const GrStencilSettings&,
                                      int* effectiveSampleCnt, SamplePattern*) {
    // TODO: stub.
    SkASSERT(!this->caps()->sampleLocationsSupport());
    *effectiveSampleCnt = rt->desc().fSampleCnt;
}

bool GrVkGpu::onGetReadPixelsInfo(GrSurface* srcSurface, int width, int height, size_t rowBytes,
                                  GrPixelConfig readConfig, DrawPreference* drawPreference,
                                  ReadPixelTempDrawInfo* tempDrawInfo) {
    // These settings we will always want if a temp draw is performed.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 0;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.
    tempDrawInfo->fTempSurfaceFit = SkBackingFit::kApprox;

    // For now assume no swizzling, we may change that below.
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();

    // Depends on why we need/want a temp draw. Start off assuming no change, the surface we read
    // from will be srcConfig and we will read readConfig pixels from it.
    // Not that if we require a draw and return a non-renderable format for the temp surface the
    // base class will fail for us.
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcSurface->config();
    tempDrawInfo->fReadConfig = readConfig;

    if (srcSurface->config() == readConfig) {
        return true;
    }

    if (this->vkCaps().isConfigRenderable(readConfig, srcSurface->desc().fSampleCnt > 1)) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        tempDrawInfo->fTempSurfaceDesc.fConfig = readConfig;
        tempDrawInfo->fReadConfig = readConfig;
        return true;
    }

    return false;
}

bool GrVkGpu::onReadPixels(GrSurface* surface,
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
                this->internalResolveRenderTarget(rt, false);
                break;
            default:
                SkFAIL("Unknown resolve type");
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
    bool flipY = kBottomLeft_GrSurfaceOrigin == surface->origin();

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
    GrVkTransferBuffer* transferBuffer =
            static_cast<GrVkTransferBuffer*>(this->createBuffer(transBufferRowBytes * height,
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
    GrVkMemory::InvalidateMappedAlloc(this, transferBuffer->alloc());
    void* mappedMemory = transferBuffer->map();

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
                                           GrVkRenderTarget* target,
                                           const SkIRect& bounds) {
    const SkIRect* pBounds = &bounds;
    SkIRect flippedBounds;
    if (kBottomLeft_GrSurfaceOrigin == target->origin()) {
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

    fCurrentCmdBuffer->beginRenderPass(this, renderPass, colorClear, *target, *pBounds, true);
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

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrVkGpu::makeSemaphore() {
    return GrVkSemaphore::Make(this);
}

void GrVkGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore) {
    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore.get());

    this->submitCommandBuffer(kSkip_SyncQueue, vkSem->getResource());
}

void GrVkGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    GrVkSemaphore* vkSem = static_cast<GrVkSemaphore*>(semaphore.get());

    const GrVkSemaphore::Resource* resource = vkSem->getResource();
    resource->ref();
    fSemaphoresToWaitOn.push_back(resource);
}

void GrVkGpu::flush() {
    // We submit the command buffer to the queue whenever Ganesh is flushed, so nothing is needed
}
