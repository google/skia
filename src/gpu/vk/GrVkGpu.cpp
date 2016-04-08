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
#include "GrVkImage.h"
#include "GrVkIndexBuffer.h"
#include "GrVkMemory.h"
#include "GrVkPipeline.h"
#include "GrVkPipelineState.h"
#include "GrVkRenderPass.h"
#include "GrVkResourceProvider.h"
#include "GrVkTexture.h"
#include "GrVkTextureRenderTarget.h"
#include "GrVkTransferBuffer.h"
#include "GrVkVertexBuffer.h"

#include "SkConfig8888.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkTypes.h"

#define VK_CALL(X) GR_VK_CALL(this->vkInterface(), X)
#define VK_CALL_RET(RET, X) GR_VK_CALL_RET(this->vkInterface(), RET, X)
#define VK_CALL_ERRCHECK(X) GR_VK_CALL_ERRCHECK(this->vkInterface(), X)

#ifdef ENABLE_VK_LAYERS
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
    SkAutoTUnref<const GrVkBackendContext> vkBackendContext(
                                       reinterpret_cast<const GrVkBackendContext*>(backendContext));
    if (!vkBackendContext) {
        vkBackendContext.reset(GrVkBackendContext::Create());
        if (!vkBackendContext) {
            return nullptr;
        }
    } else {
        vkBackendContext->ref();
    }

    return new GrVkGpu(context, options, vkBackendContext);
}

////////////////////////////////////////////////////////////////////////////////

GrVkGpu::GrVkGpu(GrContext* context, const GrContextOptions& options,
                 const GrVkBackendContext* backendCtx)
    : INHERITED(context)
    , fVkInstance(backendCtx->fInstance)
    , fDevice(backendCtx->fDevice)
    , fQueue(backendCtx->fQueue)
    , fResourceProvider(this) {
    fBackendContext.reset(backendCtx);

#ifdef ENABLE_VK_LAYERS
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
        GR_VK_CALL_ERRCHECK(this->vkInterface(), CreateDebugReportCallbackEXT(fVkInstance,
                            &callbackCreateInfo, nullptr, &fCallback));
    }
#endif

    fCompiler = shaderc_compiler_initialize();

    fVkCaps.reset(new GrVkCaps(options, this->vkInterface(), backendCtx->fPhysicalDevice,
                               backendCtx->fFeatures, backendCtx->fExtensions));
    fCaps.reset(SkRef(fVkCaps.get()));

    VK_CALL(GetPhysicalDeviceMemoryProperties(backendCtx->fPhysicalDevice, &fPhysDevMemProps));

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // sType
        nullptr,                                    // pNext
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,       // CmdPoolCreateFlags
        backendCtx->fQueueFamilyIndex,              // queueFamilyIndex
    };
    GR_VK_CALL_ERRCHECK(this->vkInterface(), CreateCommandPool(fDevice, &cmdPoolInfo, nullptr,
                                                               &fCmdPool));

    // must call this after creating the CommandPool
    fResourceProvider.init();
    fCurrentCmdBuffer = fResourceProvider.createCommandBuffer();
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->begin(this);
}

GrVkGpu::~GrVkGpu() {
    fCurrentCmdBuffer->end(this);
    fCurrentCmdBuffer->unref(this);

    // wait for all commands to finish
    fResourceProvider.checkCommandBuffers();
    SkDEBUGCODE(VkResult res =) VK_CALL(QueueWaitIdle(fQueue));
    // VK_ERROR_DEVICE_LOST is acceptable when tearing down (see 4.2.4 in spec)
    SkASSERT(VK_SUCCESS == res || VK_ERROR_DEVICE_LOST == res);

    // must call this just before we destroy the VkDevice
    fResourceProvider.destroyResources();

    VK_CALL(DestroyCommandPool(fDevice, fCmdPool, nullptr));

    shaderc_compiler_release(fCompiler);

#ifdef ENABLE_VK_LAYERS
    VK_CALL(DestroyDebugReportCallbackEXT(fVkInstance, fCallback, nullptr));
#endif
}

///////////////////////////////////////////////////////////////////////////////

void GrVkGpu::submitCommandBuffer(SyncQueue sync) {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->end(this);

    fCurrentCmdBuffer->submitToQueue(this, fQueue, sync);
    fResourceProvider.checkCommandBuffers();

    // Release old command buffer and create a new one
    fCurrentCmdBuffer->unref(this);
    fCurrentCmdBuffer = fResourceProvider.createCommandBuffer();
    SkASSERT(fCurrentCmdBuffer);

    fCurrentCmdBuffer->begin(this);
}

///////////////////////////////////////////////////////////////////////////////
GrBuffer* GrVkGpu::onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern accessPattern) {
    switch (type) {
        case kVertex_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            return GrVkVertexBuffer::Create(this, size, kDynamic_GrAccessPattern == accessPattern);
        case kIndex_GrBufferType:
            SkASSERT(kDynamic_GrAccessPattern == accessPattern ||
                     kStatic_GrAccessPattern == accessPattern);
            return GrVkIndexBuffer::Create(this, size, kDynamic_GrAccessPattern == accessPattern);
        case kXferCpuToGpu_GrBufferType:
            SkASSERT(kStream_GrAccessPattern == accessPattern);
            return GrVkTransferBuffer::Create(this, size, GrVkBuffer::kCopyRead_Type);
        case kXferGpuToCpu_GrBufferType:
            SkASSERT(kStream_GrAccessPattern == accessPattern);
            return GrVkTransferBuffer::Create(this, size, GrVkBuffer::kCopyWrite_Type);
        default:
            SkFAIL("Unknown buffer type.");
            return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool GrVkGpu::onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                                   GrPixelConfig srcConfig, DrawPreference* drawPreference,
                                   WritePixelTempDrawInfo* tempDrawInfo) {
    if (kIndex_8_GrPixelConfig == srcConfig || GrPixelConfigIsCompressed(dstSurface->config())) {
        return false;
    }

    // Currently we don't handle draws, so if the caller wants/needs to do a draw we need to fail
    if (kNoDraw_DrawPreference != *drawPreference) {
        return false;
    }

    if (dstSurface->config() != srcConfig) {
        // TODO: This should fall back to drawing or copying to change config of dstSurface to
        // match that of srcConfig.
        return false;
    }

    return true;
}

bool GrVkGpu::onWritePixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrPixelConfig config,
                            const SkTArray<GrMipLevel>& texels) {
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(surface->asTexture());
    if (!vkTex) {
        return false;
    }

    // TODO: We're ignoring MIP levels here.
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
        if (linearTiling && VK_IMAGE_LAYOUT_PREINITIALIZED != vkTex->currentLayout()) {
            // Need to change the layout to general in order to perform a host write
            VkImageLayout layout = vkTex->currentLayout();
            VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(layout);
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
            VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(layout);
            VkAccessFlags dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            vkTex->setImageLayout(this,
                                  VK_IMAGE_LAYOUT_GENERAL,
                                  srcAccessMask,
                                  dstAccessMask,
                                  srcStageMask,
                                  dstStageMask,
                                  false);
        }
        success = this->uploadTexData(vkTex, left, top, width, height, config,
                                      texels.begin()->fPixels, texels.begin()->fRowBytes);
    }

    if (success) {
        vkTex->texturePriv().dirtyMipMaps(true);
        return true;
    }

    return false;
}

bool GrVkGpu::uploadTexData(GrVkTexture* tex,
                            int left, int top, int width, int height,
                            GrPixelConfig dataConfig,
                            const void* data,
                            size_t rowBytes) {
    SkASSERT(data);

    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrPixelConfigIsCompressed(dataConfig));

    bool linearTiling = tex->isLinearTiled();

    size_t bpp = GrBytesPerPixel(dataConfig);

    const GrSurfaceDesc& desc = tex->desc();

    if (!GrSurfacePriv::AdjustWritePixelParams(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                               &width, &height, &data, &rowBytes)) {
        return false;
    }
    size_t trimRowBytes = width * bpp;

    if (linearTiling) {
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
                                                        tex->textureImage(),
                                                        &subres,
                                                        &layout));

        int texTop = kBottomLeft_GrSurfaceOrigin == desc.fOrigin ? tex->height() - top - height
                                                                    : top;
        VkDeviceSize offset = texTop*layout.rowPitch + left*bpp;
        VkDeviceSize size = height*layout.rowPitch;
        void* mapPtr;
        err = GR_VK_CALL(interface, MapMemory(fDevice, tex->textureMemory(), offset, size, 0,
                                                &mapPtr));
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
            // If there is no padding on the src (rowBytes) or dst (layout.rowPitch) we can memcpy
            if (trimRowBytes == rowBytes && trimRowBytes == layout.rowPitch) {
                memcpy(mapPtr, data, trimRowBytes * height);
            } else {
                SkRectMemcpy(mapPtr, static_cast<size_t>(layout.rowPitch), data, rowBytes,
                             trimRowBytes, height);
            }
        }

        GR_VK_CALL(interface, UnmapMemory(fDevice, tex->textureMemory()));
    } else {
        GrVkTransferBuffer* transferBuffer =
            GrVkTransferBuffer::Create(this, trimRowBytes * height, GrVkBuffer::kCopyRead_Type);

        void* mapPtr = transferBuffer->map();

        if (kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
            // copy into buffer by rows
            const char* srcRow = reinterpret_cast<const char*>(data);
            char* dstRow = reinterpret_cast<char*>(mapPtr)+(height - 1)*trimRowBytes;
            for (int y = 0; y < height; y++) {
                memcpy(dstRow, srcRow, trimRowBytes);
                srcRow += rowBytes;
                dstRow -= trimRowBytes;
            }
        } else {
            // If there is no padding on the src data rows, we can do a single memcpy
            if (trimRowBytes == rowBytes) {
                memcpy(mapPtr, data, trimRowBytes * height);
            } else {
                SkRectMemcpy(mapPtr, trimRowBytes, data, rowBytes, trimRowBytes, height);
            }
        }

        transferBuffer->unmap();

        // make sure the unmap has finished
        transferBuffer->addMemoryBarrier(this,
                                         VK_ACCESS_HOST_WRITE_BIT,
                                         VK_ACCESS_TRANSFER_READ_BIT,
                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         false);

        // Set up copy region
        bool flipY = kBottomLeft_GrSurfaceOrigin == tex->origin();
        VkOffset3D offset = {
            left,
            flipY ? tex->height() - top - height : top,
            0
        };

        VkBufferImageCopy region;
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = 0;
        region.bufferRowLength = width;
        region.bufferImageHeight = height;
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageOffset = offset;
        region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

        // Change layout of our target so it can be copied to
        VkImageLayout layout = tex->currentLayout();
        VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(layout);
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(layout);
        VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        tex->setImageLayout(this,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            srcAccessMask,
                            dstAccessMask,
                            srcStageMask,
                            dstStageMask,
                            false);

        // Copy the buffer to the image
        fCurrentCmdBuffer->copyBufferToImage(this,
                                             transferBuffer,
                                             tex,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             1,
                                             &region);

        // Submit the current command buffer to the Queue
        this->submitCommandBuffer(kSkip_SyncQueue);

        transferBuffer->unref();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
GrTexture* GrVkGpu::onCreateTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle lifeCycle,
                                    const SkTArray<GrMipLevel>& texels) {
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    VkFormat pixelFormat;
    if (!GrPixelConfigToVkFormat(desc.fConfig, &pixelFormat)) {
        return nullptr;
    }

    if (!fVkCaps->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    bool linearTiling = false;
    if (SkToBool(desc.fFlags & kZeroCopy_GrSurfaceFlag)) {
        if (fVkCaps->isConfigTexurableLinearly(desc.fConfig) &&
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
    // that all render targets in vulkan are also texutres. If we change this practice of setting
    // both bits, we must make sure to set the destination bit if we are uploading srcData to the
    // texture.
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkFlags memProps = (!texels.empty() && linearTiling) ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT :
                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // This ImageDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this ImageDesc describes the resolved texutre. Therefore we always have samples set
    // to 1.
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = pixelFormat;
    imageDesc.fWidth = desc.fWidth;
    imageDesc.fHeight = desc.fHeight;
    imageDesc.fLevels = 1;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = memProps;

    GrVkTexture* tex;
    if (renderTarget) {
        tex = GrVkTextureRenderTarget::CreateNewTextureRenderTarget(this, desc, lifeCycle,
                                                                    imageDesc);
    } else {
        tex = GrVkTexture::CreateNewTexture(this, desc, lifeCycle, imageDesc);
    }

    if (!tex) {
        return nullptr;
    }

    // TODO: We're ignoring MIP levels here.
    if (!texels.empty()) {
        SkASSERT(texels.begin()->fPixels);
        if (!this->uploadTexData(tex, 0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                 texels.begin()->fPixels, texels.begin()->fRowBytes)) {
            tex->unref();
            return nullptr;
        }
    }

    return tex;
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

GrTexture* GrVkGpu::onWrapBackendTexture(const GrBackendTextureDesc& desc,
                                         GrWrapOwnership ownership) {
    VkFormat format;
    if (!GrPixelConfigToVkFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    if (0 == desc.fTextureHandle) {
        return nullptr;
    }

    int maxSize = this->caps()->maxTextureSize();
    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }

    const GrVkTextureInfo* info = reinterpret_cast<const GrVkTextureInfo*>(desc.fTextureHandle);
    if (VK_NULL_HANDLE == info->fImage || VK_NULL_HANDLE == info->fAlloc) {
        return nullptr;
    }

    GrGpuResource::LifeCycle lifeCycle = (kAdopt_GrWrapOwnership == ownership)
                                         ? GrGpuResource::kAdopted_LifeCycle
                                         : GrGpuResource::kBorrowed_LifeCycle;

    GrSurfaceDesc surfDesc;
    // next line relies on GrBackendTextureDesc's flags matching GrTexture's
    surfDesc.fFlags = (GrSurfaceFlags)desc.fFlags;
    surfDesc.fWidth = desc.fWidth;
    surfDesc.fHeight = desc.fHeight;
    surfDesc.fConfig = desc.fConfig;
    surfDesc.fSampleCnt = SkTMin(desc.fSampleCnt, this->caps()->maxSampleCount());
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrBackendTextureFlag);
    // In GL, Chrome assumes all textures are BottomLeft
    // In VK, we don't have this restriction
    surfDesc.fOrigin = resolve_origin(desc.fOrigin);

    GrVkTexture* texture = nullptr;
    if (renderTarget) {
        texture = GrVkTextureRenderTarget::CreateWrappedTextureRenderTarget(this, surfDesc,
                                                                            lifeCycle, format,
                                                                            info);
    } else {
        texture = GrVkTexture::CreateWrappedTexture(this, surfDesc, lifeCycle, format,
                                                    info);
    }
    if (!texture) {
        return nullptr;
    }

    return texture;
}

GrRenderTarget* GrVkGpu::onWrapBackendRenderTarget(const GrBackendRenderTargetDesc& wrapDesc,
                                                   GrWrapOwnership ownership) {

    const GrVkTextureInfo* info =
        reinterpret_cast<const GrVkTextureInfo*>(wrapDesc.fRenderTargetHandle);
    if (VK_NULL_HANDLE == info->fImage ||
        (VK_NULL_HANDLE == info->fAlloc && kAdopt_GrWrapOwnership == ownership)) {
        return nullptr;
    }

    GrGpuResource::LifeCycle lifeCycle = (kAdopt_GrWrapOwnership == ownership)
                                         ? GrGpuResource::kAdopted_LifeCycle
                                         : GrGpuResource::kBorrowed_LifeCycle;

    GrSurfaceDesc desc;
    desc.fConfig = wrapDesc.fConfig;
    desc.fFlags = kCheckAllocation_GrSurfaceFlag;
    desc.fWidth = wrapDesc.fWidth;
    desc.fHeight = wrapDesc.fHeight;
    desc.fSampleCnt = SkTMin(wrapDesc.fSampleCnt, this->caps()->maxSampleCount());

    desc.fOrigin = resolve_origin(wrapDesc.fOrigin);

    GrVkRenderTarget* tgt = GrVkRenderTarget::CreateWrappedRenderTarget(this, desc,
                                                                        lifeCycle,
                                                                        info);
    if (tgt && wrapDesc.fStencilBits) {
        if (!createStencilAttachmentForRenderTarget(tgt, desc.fWidth, desc.fHeight)) {
            tgt->unref();
            return nullptr;
        }
    }
    return tgt;
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpu::bindGeometry(const GrPrimitiveProcessor& primProc,
                           const GrNonInstancedMesh& mesh) {
    GrVkVertexBuffer* vbuf;
    vbuf = (GrVkVertexBuffer*)mesh.vertexBuffer();
    SkASSERT(vbuf);
    SkASSERT(!vbuf->isMapped());

    vbuf->addMemoryBarrier(this,
                           VK_ACCESS_HOST_WRITE_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                           VK_PIPELINE_STAGE_HOST_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           false);

    fCurrentCmdBuffer->bindVertexBuffer(this, vbuf);

    if (mesh.isIndexed()) {
        GrVkIndexBuffer* ibuf = (GrVkIndexBuffer*)mesh.indexBuffer();
        SkASSERT(ibuf);
        SkASSERT(!ibuf->isMapped());

        ibuf->addMemoryBarrier(this,
                               VK_ACCESS_HOST_WRITE_BIT,
                               VK_ACCESS_INDEX_READ_BIT,
                               VK_PIPELINE_STAGE_HOST_BIT,
                               VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                               false);

        fCurrentCmdBuffer->bindIndexBuffer(this, ibuf);
    }
}

////////////////////////////////////////////////////////////////////////////////

GrStencilAttachment* GrVkGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                     int width,
                                                                     int height) {
    SkASSERT(rt->asTexture());
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numStencilSamples();

    const GrVkCaps::StencilFormat& sFmt = this->vkCaps().preferedStencilFormat();

    GrVkStencilAttachment* stencil(GrVkStencilAttachment::Create(this,
                                                                 GrGpuResource::kCached_LifeCycle,
                                                                 width,
                                                                 height,
                                                                 samples,
                                                                 sFmt));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

////////////////////////////////////////////////////////////////////////////////

GrBackendObject GrVkGpu::createTestingOnlyBackendTexture(void* srcData, int w, int h,
                                                         GrPixelConfig config) {

    VkFormat pixelFormat;
    if (!GrPixelConfigToVkFormat(config, &pixelFormat)) {
        return 0;
    }

    bool linearTiling = false;
    if (!fVkCaps->isConfigTexturable(config)) {
        return 0;
    }

    if (fVkCaps->isConfigTexurableLinearly(config)) {
        linearTiling = true;
    }

    // Currently this is not supported since it requires a copy which has not yet been implemented.
    if (srcData && !linearTiling) {
        return 0;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkFlags memProps = (srcData && linearTiling) ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT :
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory alloc = VK_NULL_HANDLE;

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

    if (!GrVkMemory::AllocAndBindImageMemory(this, image, memProps, &alloc)) {
        VK_CALL(DestroyImage(this->device(), image, nullptr));
        return 0;
    }

    if (srcData) {
        if (linearTiling) {
            const VkImageSubresource subres = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,  // mipLevel
                0,  // arraySlice
            };
            VkSubresourceLayout layout;
            VkResult err;

            VK_CALL(GetImageSubresourceLayout(fDevice, image, &subres, &layout));

            void* mapPtr;
            err = VK_CALL(MapMemory(fDevice, alloc, 0, layout.rowPitch * h, 0, &mapPtr));
            if (err) {
                VK_CALL(FreeMemory(this->device(), alloc, nullptr));
                VK_CALL(DestroyImage(this->device(), image, nullptr));
                return 0;
            }

            size_t bpp = GrBytesPerPixel(config);
            size_t rowCopyBytes = bpp * w;
            // If there is no padding on dst (layout.rowPitch) we can do a single memcopy.
            // This assumes the srcData comes in with no padding.
            if (rowCopyBytes == layout.rowPitch) {
                memcpy(mapPtr, srcData, rowCopyBytes * h);
            } else {
                SkRectMemcpy(mapPtr, static_cast<size_t>(layout.rowPitch), srcData, rowCopyBytes,
                             rowCopyBytes, h);
            }
            VK_CALL(UnmapMemory(fDevice, alloc));
        } else {
            // TODO: Add support for copying to optimal tiling
            SkASSERT(false);
        }
    }

    GrVkTextureInfo* info = new GrVkTextureInfo;
    info->fImage = image;
    info->fAlloc = alloc;
    info->fImageTiling = imageTiling;
    info->fImageLayout = initialLayout;

    return (GrBackendObject)info;
}

bool GrVkGpu::isTestingOnlyBackendTexture(GrBackendObject id) const {
    const GrVkTextureInfo* backend = reinterpret_cast<const GrVkTextureInfo*>(id);

    if (backend && backend->fImage && backend->fAlloc) {
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
    const GrVkTextureInfo* backend = reinterpret_cast<const GrVkTextureInfo*>(id);

    if (backend) {
        if (!abandon) {
            // something in the command buffer may still be using this, so force submit
            this->submitCommandBuffer(kForce_SyncQueue);

            VK_CALL(FreeMemory(this->device(), backend->fAlloc, nullptr));
            VK_CALL(DestroyImage(this->device(), backend->fImage, nullptr));
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

void GrVkGpu::finishDrawTarget() {
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

    VkImageLayout origDstLayout = vkStencil->currentLayout();

    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);;
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkStencil->setImageLayout(this,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              srcAccessMask,
                              dstAccessMask,
                              srcStageMask,
                              dstStageMask,
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

void GrVkGpu::onClearStencilClip(GrRenderTarget* target, const SkIRect& rect, bool insideClip) {
    SkASSERT(target);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(target);
    GrStencilAttachment* sb = target->renderTargetPriv().getStencilAttachment();
    GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)sb;

    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = sb->bits();

    // The contract with the callers does not guarantee that we preserve all bits in the stencil
    // during this clear. Thus we will clear the entire stencil to the desired value.

    VkClearDepthStencilValue vkStencilColor;
    memset(&vkStencilColor, 0, sizeof(VkClearDepthStencilValue));
    if (insideClip) {
        vkStencilColor.stencil = (1 << (stencilBitCount - 1));
    } else {
        vkStencilColor.stencil = 0;
    }

    VkImageLayout origDstLayout = vkStencil->currentLayout();
    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);
    VkAccessFlags dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    VkPipelineStageFlags srcStageMask =
        GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    vkStencil->setImageLayout(this,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                              srcAccessMask,
                              dstAccessMask,
                              srcStageMask,
                              dstStageMask,
                              false);

    VkClearRect clearRect;
    // Flip rect if necessary
    SkIRect vkRect = rect;

    if (kBottomLeft_GrSurfaceOrigin == vkRT->origin()) {
        vkRect.fTop = vkRT->height() - rect.fBottom;
        vkRect.fBottom = vkRT->height() - rect.fTop;
    }

    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };

    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    const GrVkRenderPass* renderPass = vkRT->simpleRenderPass();
    SkASSERT(renderPass);
    fCurrentCmdBuffer->beginRenderPass(this, renderPass, *vkRT);

    uint32_t stencilIndex;
    SkAssertResult(renderPass->stencilAttachmentIndex(&stencilIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    attachment.colorAttachment = 0; // this value shouldn't matter
    attachment.clearValue.depthStencil = vkStencilColor;

    fCurrentCmdBuffer->clearAttachments(this, 1, &attachment, 1, &clearRect);
    fCurrentCmdBuffer->endRenderPass(this);

    return;
}

void GrVkGpu::onClear(GrRenderTarget* target, const SkIRect& rect, GrColor color) {
    // parent class should never let us get here with no RT
    SkASSERT(target);

    VkClearColorValue vkColor;
    GrColorToRGBAFloat(color, vkColor.float32);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(target);
    VkImageLayout origDstLayout = vkRT->currentLayout();

    if (rect.width() != target->width() || rect.height() != target->height()) {
        VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);
        VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkPipelineStageFlags srcStageMask =
            GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkRT->setImageLayout(this,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             srcAccessMask,
                             dstAccessMask,
                             srcStageMask,
                             dstStageMask,
                             false);

        VkClearRect clearRect;
        // Flip rect if necessary
        SkIRect vkRect = rect;
        if (kBottomLeft_GrSurfaceOrigin == vkRT->origin()) {
            vkRect.fTop = vkRT->height() - rect.fBottom;
            vkRect.fBottom = vkRT->height() - rect.fTop;
        }
        clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
        clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;

        const GrVkRenderPass* renderPass = vkRT->simpleRenderPass();
        SkASSERT(renderPass);
        fCurrentCmdBuffer->beginRenderPass(this, renderPass, *vkRT);

        uint32_t colorIndex;
        SkAssertResult(renderPass->colorAttachmentIndex(&colorIndex));

        VkClearAttachment attachment;
        attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        attachment.colorAttachment = colorIndex;
        attachment.clearValue.color = vkColor;

        fCurrentCmdBuffer->clearAttachments(this, 1, &attachment, 1, &clearRect);
        fCurrentCmdBuffer->endRenderPass(this);
        return;
    }

    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);;
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkRT->setImageLayout(this,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         srcAccessMask,
                         dstAccessMask,
                         srcStageMask,
                         dstStageMask,
                         false);


    VkImageSubresourceRange subRange;
    memset(&subRange, 0, sizeof(VkImageSubresourceRange));
    subRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subRange.baseMipLevel = 0;
    subRange.levelCount = 1;
    subRange.baseArrayLayer = 0;
    subRange.layerCount = 1;

    // In the future we may not actually be doing this type of clear at all. If we are inside a
    // render pass or doing a non full clear then we will use CmdClearColorAttachment. The more
    // common use case will be clearing an attachment at the start of a render pass, in which case
    // we will use the clear load ops.
    fCurrentCmdBuffer->clearColorImage(this,
                                       vkRT,
                                       &vkColor,
                                       1, &subRange);
}

inline bool can_copy_image(const GrSurface* dst,
                           const GrSurface* src,
                           const GrVkGpu* gpu) {
    // Currently we don't support msaa
    if ((dst->asRenderTarget() && dst->asRenderTarget()->numColorSamples() > 1) ||
        (src->asRenderTarget() && src->asRenderTarget()->numColorSamples() > 1)) {
        return false;
    }

    // We require that all vulkan GrSurfaces have been created with transfer_dst and transfer_src 
    // as image usage flags.
    if (src->origin() == dst->origin() &&
        GrBytesPerPixel(src->config()) == GrBytesPerPixel(dst->config())) {
        return true;
    }

    // How does msaa play into this? If a VkTexture is multisampled, are we copying the multisampled
    // or the resolved image here? Im multisampled, Vulkan requires sample counts to be the same.

    return false;
}

void GrVkGpu::copySurfaceAsCopyImage(GrSurface* dst,
                                     GrSurface* src,
                                     GrVkImage* dstImage,
                                     GrVkImage* srcImage,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    SkASSERT(can_copy_image(dst, src, this));

    VkImageLayout origDstLayout = dstImage->currentLayout();
    VkImageLayout origSrcLayout = srcImage->currentLayout();

    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    // These flags are for flushing/invalidating caches and for the dst image it doesn't matter if
    // the cache is flushed since it is only being written to.
    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);;
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    dstImage->setImageLayout(this,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            srcAccessMask,
                            dstAccessMask,
                            srcStageMask,
                            dstStageMask,
                            false);

    srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(origSrcLayout);
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origSrcLayout);
    dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    srcImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             srcAccessMask,
                             dstAccessMask,
                             srcStageMask,
                             dstStageMask,
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
    copyRegion.extent = { (uint32_t)srcVkRect.width(), (uint32_t)srcVkRect.height(), 0 };

    fCurrentCmdBuffer->copyImage(this,
                                 srcImage,
                                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 dstImage,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 1,
                                 &copyRegion);
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

    VkImageLayout origDstLayout = dstImage->currentLayout();
    VkImageLayout origSrcLayout = srcImage->currentLayout();

    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);;
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    dstImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             srcAccessMask,
                             dstAccessMask,
                             srcStageMask,
                             dstStageMask,
                             false);

    srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(origSrcLayout);
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origSrcLayout);
    dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    srcImage->setImageLayout(this,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             srcAccessMask,
                             dstAccessMask,
                             srcStageMask,
                             dstStageMask,
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
    blitRegion.srcOffsets[1] = { srcVkRect.fRight, srcVkRect.fBottom, 0 };
    blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    blitRegion.dstOffsets[0] = { dstRect.fLeft, dstRect.fTop, 0 };
    blitRegion.dstOffsets[1] = { dstRect.fRight, dstRect.fBottom, 0 };

    fCurrentCmdBuffer->blitImage(this,
                                 srcImage,
                                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 dstImage,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 1,
                                 &blitRegion,
                                 VK_FILTER_NEAREST); // We never scale so any filter works here
}

inline bool can_copy_as_draw(const GrSurface* dst,
                             const GrSurface* src,
                             const GrVkGpu* gpu) {
    return false;
}

void GrVkGpu::copySurfaceAsDraw(GrSurface* dst,
                                GrSurface* src,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    SkASSERT(false);
}

bool GrVkGpu::onCopySurface(GrSurface* dst,
                            GrSurface* src,
                            const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    GrVkImage* dstImage;
    GrVkImage* srcImage;
    if (dst->asTexture()) {
        dstImage = static_cast<GrVkTexture*>(dst->asTexture());
    } else {
        SkASSERT(dst->asRenderTarget());
        dstImage = static_cast<GrVkRenderTarget*>(dst->asRenderTarget());
    }
    if (src->asTexture()) {
        srcImage = static_cast<GrVkTexture*>(src->asTexture());
    } else {
        SkASSERT(src->asRenderTarget());
        srcImage = static_cast<GrVkRenderTarget*>(src->asRenderTarget());
    }

    if (can_copy_image(dst, src, this)) {
        this->copySurfaceAsCopyImage(dst, src, dstImage, srcImage, srcRect, dstPoint);
        return true;
    }

    if (can_copy_as_blit(dst, src, dstImage, srcImage, this)) {
        this->copySurfaceAsBlit(dst, src, dstImage, srcImage, srcRect, dstPoint);
        return true;
    }

    if (can_copy_as_draw(dst, src, this)) {
        this->copySurfaceAsDraw(dst, src, srcRect, dstPoint);
        return true;
    }

    return false;
}

void GrVkGpu::onGetMultisampleSpecs(GrRenderTarget* rt, const GrStencilSettings&,
                                    int* effectiveSampleCnt, SkAutoTDeleteArray<SkPoint>*) {
    // TODO: stub.
    SkASSERT(!this->caps()->sampleLocationsSupport());
    *effectiveSampleCnt = rt->desc().fSampleCnt;
}

bool GrVkGpu::onGetReadPixelsInfo(GrSurface* srcSurface, int width, int height, size_t rowBytes,
                                  GrPixelConfig readConfig, DrawPreference* drawPreference,
                                  ReadPixelTempDrawInfo* tempDrawInfo) {
    // Currently we don't handle draws, so if the caller wants/needs to do a draw we need to fail
    if (kNoDraw_DrawPreference != *drawPreference) {
        return false;
    }

    if (srcSurface->config() != readConfig) {
        // TODO: This should fall back to drawing or copying to change config of srcSurface to match
        // that of readConfig.
        return false;
    }

    return true;
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

    GrVkTexture* tgt = static_cast<GrVkTexture*>(surface->asTexture());
    if (!tgt) {
        return false;
    }

    // Change layout of our target so it can be used as copy
    VkImageLayout layout = tgt->currentLayout();
    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(layout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(layout);
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    tgt->setImageLayout(this,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        srcAccessMask,
                        dstAccessMask,
                        srcStageMask,
                        dstStageMask,
                        false);

    GrVkTransferBuffer* transferBuffer =
        static_cast<GrVkTransferBuffer*>(this->createBuffer(rowBytes * height,
                                                            kXferGpuToCpu_GrBufferType,
                                                            kStream_GrAccessPattern));

    bool flipY = kBottomLeft_GrSurfaceOrigin == surface->origin();
    VkOffset3D offset = {
        left,
        flipY ? surface->height() - top - height : top,
        0
    };

    // Copy the image to a buffer so we can map it to cpu memory
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = 0;
    region.bufferRowLength = 0; // Forces RowLength to be imageExtent.width
    region.bufferImageHeight = 0; // Forces height to be tightly packed. Only useful for 3d images.
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = offset;
    region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

    fCurrentCmdBuffer->copyImageToBuffer(this,
                                         tgt,
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

    memcpy(buffer, mappedMemory, rowBytes*height);

    transferBuffer->unmap();
    transferBuffer->unref();

    if (flipY) {
        SkAutoSMalloc<32 * sizeof(GrColor)> scratch;
        size_t tightRowBytes = GrBytesPerPixel(config) * width;
        scratch.reset(tightRowBytes);
        void* tmpRow = scratch.get();
        // flip y in-place by rows
        const int halfY = height >> 1;
        char* top = reinterpret_cast<char*>(buffer);
        char* bottom = top + (height - 1) * rowBytes;
        for (int y = 0; y < halfY; y++) {
            memcpy(tmpRow, top, tightRowBytes);
            memcpy(top, bottom, tightRowBytes);
            memcpy(bottom, tmpRow, tightRowBytes);
            top += rowBytes;
            bottom -= rowBytes;
        }
    }

    return true;
}
sk_sp<GrVkPipelineState> GrVkGpu::prepareDrawState(const GrPipeline& pipeline,
                                                   const GrPrimitiveProcessor& primProc,
                                                   GrPrimitiveType primitiveType,
                                                   const GrVkRenderPass& renderPass) {
    sk_sp<GrVkPipelineState> pipelineState =
        fResourceProvider.findOrCreateCompatiblePipelineState(pipeline,
                                                              primProc,
                                                              primitiveType,
                                                              renderPass);
    if (!pipelineState) {
        return pipelineState;
    }

    pipelineState->setData(this, primProc, pipeline);

    pipelineState->bind(this, fCurrentCmdBuffer);

    GrVkPipeline::SetDynamicState(this, fCurrentCmdBuffer, pipeline);

    return pipelineState;
}

void GrVkGpu::onDraw(const GrPipeline& pipeline,
                     const GrPrimitiveProcessor& primProc,
                     const GrMesh* meshes,
                     int meshCount) {
    if (!meshCount) {
        return;
    }
    GrRenderTarget* rt = pipeline.getRenderTarget();
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
    const GrVkRenderPass* renderPass = vkRT->simpleRenderPass();
    SkASSERT(renderPass);

    fCurrentCmdBuffer->beginRenderPass(this, renderPass, *vkRT);

    GrPrimitiveType primitiveType = meshes[0].primitiveType();
    sk_sp<GrVkPipelineState> pipelineState = this->prepareDrawState(pipeline,
                                                                    primProc,
                                                                    primitiveType,
                                                                    *renderPass);
    if (!pipelineState) {
        return;
    }

    // Change layout of our render target so it can be used as the color attachment
    VkImageLayout layout = vkRT->currentLayout();
    // Our color attachment is purely a destination and won't be read so don't need to flush or
    // invalidate any caches
    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(layout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(layout);
    VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    vkRT->setImageLayout(this,
                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                         srcAccessMask,
                         dstAccessMask,
                         srcStageMask,
                         dstStageMask,
                         false);

    // If we are using a stencil attachment we also need to update its layout
    if (!pipeline.getStencil().isDisabled()) {
        GrStencilAttachment* stencil = vkRT->renderTargetPriv().getStencilAttachment();
        GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)stencil;
        VkImageLayout origDstLayout = vkStencil->currentLayout();
        VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(origDstLayout);
        VkAccessFlags dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        VkPipelineStageFlags srcStageMask =
            GrVkMemory::LayoutToPipelineStageFlags(origDstLayout);
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkStencil->setImageLayout(this,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                  srcAccessMask,
                                  dstAccessMask,
                                  srcStageMask,
                                  dstStageMask,
                                  false);
    }


    for (int i = 0; i < meshCount; ++i) {
        if (GrXferBarrierType barrierType = pipeline.xferBarrierType(*this->caps())) {
            this->xferBarrier(pipeline.getRenderTarget(), barrierType);
        }

        const GrMesh& mesh = meshes[i];
        GrMesh::Iterator iter;
        const GrNonInstancedMesh* nonIdxMesh = iter.init(mesh);
        do {
            if (nonIdxMesh->primitiveType() != primitiveType) {
                // Technically we don't have to call this here (since there is a safety check in
                // pipelineState:setData but this will allow for quicker freeing of resources if the
                // pipelineState sits in a cache for a while.
                pipelineState->freeTempResources(this);
                SkDEBUGCODE(pipelineState = nullptr);
                primitiveType = nonIdxMesh->primitiveType();
                pipelineState = this->prepareDrawState(pipeline,
                                                       primProc,
                                                       primitiveType,
                                                       *renderPass);
                if (!pipelineState) {
                    return;
                }
            }
            SkASSERT(pipelineState);
            this->bindGeometry(primProc, *nonIdxMesh);

            if (nonIdxMesh->isIndexed()) {
                fCurrentCmdBuffer->drawIndexed(this,
                                               nonIdxMesh->indexCount(),
                                               1,
                                               nonIdxMesh->startIndex(),
                                               nonIdxMesh->startVertex(),
                                               0);
            } else {
                fCurrentCmdBuffer->draw(this,
                                        nonIdxMesh->vertexCount(),
                                        1,
                                        nonIdxMesh->startVertex(),
                                        0);
            }

            fStats.incNumDraws();
        } while ((nonIdxMesh = iter.next()));
    }

    fCurrentCmdBuffer->endRenderPass(this);

    // Technically we don't have to call this here (since there is a safety check in
    // pipelineState:setData but this will allow for quicker freeing of resources if the
    // pipelineState sits in a cache for a while.
    pipelineState->freeTempResources(this);

#if SWAP_PER_DRAW
    glFlush();
#if defined(SK_BUILD_FOR_MAC)
    aglSwapBuffers(aglGetCurrentContext());
    int set_a_break_pt_here = 9;
    aglSwapBuffers(aglGetCurrentContext());
#elif defined(SK_BUILD_FOR_WIN32)
    SwapBuf();
    int set_a_break_pt_here = 9;
    SwapBuf();
#endif
#endif
}
