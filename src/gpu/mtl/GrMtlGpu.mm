/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"

#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlOpsRenderPass.h"
#include "src/gpu/mtl/GrMtlSemaphore.h"
#include "src/gpu/mtl/GrMtlTexture.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/mtl/GrMtlUtil.h"
#include "src/sksl/SkSLCompiler.h"

#import <simd/simd.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

static bool get_feature_set(id<MTLDevice> device, MTLFeatureSet* featureSet) {
    // Mac OSX
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.12, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v2]) {
            *featureSet = MTLFeatureSet_OSX_GPUFamily1_v2;
            return true;
        }
    }
    if ([device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v1]) {
        *featureSet = MTLFeatureSet_OSX_GPUFamily1_v1;
        return true;
    }
#endif

    // iOS Family group 3
#ifdef SK_BUILD_FOR_IOS
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily3_v2;
            return true;
        }
    }
    if (@available(iOS 9.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily3_v1;
            return true;
        }
    }

    // iOS Family group 2
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v3]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily2_v3;
            return true;
        }
    }
    if (@available(iOS 9.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily2_v2;
            return true;
        }
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily2_v1;
        return true;
    }

    // iOS Family group 1
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v3]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily1_v3;
            return true;
        }
    }
    if (@available(iOS 9.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily1_v2;
            return true;
        }
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily1_v1;
        return true;
    }
#endif
    // No supported feature sets were found
    return false;
}

sk_sp<GrGpu> GrMtlGpu::Make(GrContext* context, const GrContextOptions& options,
                            id<MTLDevice> device, id<MTLCommandQueue> queue) {
    if (!device || !queue) {
        return nullptr;
    }
    MTLFeatureSet featureSet;
    if (!get_feature_set(device, &featureSet)) {
        return nullptr;
    }
    return sk_sp<GrGpu>(new GrMtlGpu(context, options, device, queue, featureSet));
}

GrMtlGpu::GrMtlGpu(GrContext* context, const GrContextOptions& options,
                   id<MTLDevice> device, id<MTLCommandQueue> queue, MTLFeatureSet featureSet)
        : INHERITED(context)
        , fDevice(device)
        , fQueue(queue)
        , fCmdBuffer(nullptr)
        , fCompiler(new SkSL::Compiler())
        , fResourceProvider(this)
        , fDisconnected(false) {
    fMtlCaps.reset(new GrMtlCaps(options, fDevice, featureSet));
    fCaps = fMtlCaps;
}

GrMtlGpu::~GrMtlGpu() {
    if (!fDisconnected) {
        this->destroyResources();
    }
}

void GrMtlGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);

    if (DisconnectType::kCleanup == type) {
        this->destroyResources();
    } else {
        delete fCmdBuffer;
        fCmdBuffer = nullptr;

        fResourceProvider.destroyResources();

        fQueue = nil;
        fDevice = nil;

        fDisconnected = true;
    }
}

void GrMtlGpu::destroyResources() {
    // Will implicitly delete the command buffer
    this->submitCommandBuffer(SyncQueue::kForce_SyncQueue);
    fResourceProvider.destroyResources();

    fQueue = nil;
    fDevice = nil;
}

GrOpsRenderPass* GrMtlGpu::getOpsRenderPass(
            GrRenderTarget* renderTarget, GrSurfaceOrigin origin, const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    return new GrMtlOpsRenderPass(this, renderTarget, origin, colorInfo, stencilInfo);
}

void GrMtlGpu::submit(GrOpsRenderPass* renderPass) {
    GrMtlOpsRenderPass* mtlRenderPass = reinterpret_cast<GrMtlOpsRenderPass*>(renderPass);
    mtlRenderPass->submit();
    delete renderPass;
}

GrMtlCommandBuffer* GrMtlGpu::commandBuffer() {
    if (!fCmdBuffer) {
        fCmdBuffer = GrMtlCommandBuffer::Create(fQueue);
    }
    return fCmdBuffer;
}

void GrMtlGpu::submitCommandBuffer(SyncQueue sync) {
    if (fCmdBuffer) {
        fResourceProvider.addBufferCompletionHandler(fCmdBuffer);
        fCmdBuffer->commit(SyncQueue::kForce_SyncQueue == sync);
        delete fCmdBuffer;
        fCmdBuffer = nullptr;
    }
}

bool GrMtlGpu::onFinishFlush(GrSurfaceProxy*[], int, SkSurface::BackendSurfaceAccess,
                             const GrFlushInfo& info, const GrPrepareForExternalIORequests&) {
    bool forceSync = SkToBool(info.fFlags & kSyncCpu_GrFlushFlag) ||
                     (info.fFinishedProc && !this->mtlCaps().fenceSyncSupport());
    // TODO: do we care about info.fSemaphore?
    if (forceSync) {
        this->submitCommandBuffer(kForce_SyncQueue);
        // After a forced sync everything previously sent to the GPU is done.
        for (const auto& cb : fFinishCallbacks) {
            cb.fCallback(cb.fContext);
            this->deleteFence(cb.fFence);
        }
        fFinishCallbacks.clear();
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
    } else {
        if (info.fFinishedProc) {
            FinishCallback callback;
            callback.fCallback = info.fFinishedProc;
            callback.fContext = info.fFinishedContext;
            callback.fFence = this->insertFence();
            fFinishCallbacks.push_back(callback);
        }
        this->submitCommandBuffer(kSkip_SyncQueue);
    }
    return true;
}

void GrMtlGpu::checkFinishProcs() {
    // Bail after the first unfinished sync since we expect they signal in the order inserted.
    while (!fFinishCallbacks.empty() && this->waitFence(fFinishCallbacks.front().fFence,
                                                       /* timeout = */ 0)) {
        fFinishCallbacks.front().fCallback(fFinishCallbacks.front().fContext);
        this->deleteFence(fFinishCallbacks.front().fFence);
        fFinishCallbacks.pop_front();
    }
}

sk_sp<GrGpuBuffer> GrMtlGpu::onCreateBuffer(size_t size, GrGpuBufferType type,
                                            GrAccessPattern accessPattern, const void* data) {
    return GrMtlBuffer::Make(this, size, type, accessPattern, data);
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

bool GrMtlGpu::uploadToTexture(GrMtlTexture* tex, int left, int top, int width, int height,
                               GrColorType dataColorType, const GrMipLevel texels[],
                               int mipLevelCount) {
    SkASSERT(this->caps()->isFormatTexturable(tex->backendFormat()));
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == tex->width() && height == tex->height()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(1 == mipLevelCount || mipLevelCount == (tex->texturePriv().maxMipMapLevel() + 1));

    if (!check_max_blit_width(width)) {
        return false;
    }
    if (width == 0 || height == 0) {
        return false;
    }
    if (GrPixelConfigToColorType(tex->config()) != dataColorType) {
        return false;
    }

    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    SkASSERT(1 == mipLevelCount || mipLevelCount == (int)mtlTexture.mipmapLevelCount);

    if (1 == mipLevelCount && !texels[0].fPixels) {
        return true;   // no data to upload
    }

    for (int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!texels[i].fPixels) {
            return false;
        }
    }

    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = GrComputeTightCombinedBufferSize(
            bpp, {width, height}, &individualMipOffsets, mipLevelCount);
    SkASSERT(combinedBufferSize);

    size_t bufferOffset;
    id<MTLBuffer> transferBuffer = this->resourceProvider().getDynamicBuffer(combinedBufferSize,
                                                                             &bufferOffset);
    if (!transferBuffer) {
        return false;
    }
    char* buffer = (char*) transferBuffer.contents + bufferOffset;

    int currentWidth = width;
    int currentHeight = height;
    int layerHeight = tex->height();
    MTLOrigin origin = MTLOriginMake(left, top, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texels[currentMipLevel].fPixels) {
            SkASSERT(1 == mipLevelCount || currentHeight == layerHeight);
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t rowBytes = texels[currentMipLevel].fRowBytes;

            // copy data into the buffer, skipping any trailing bytes
            char* dst = buffer + individualMipOffsets[currentMipLevel];
            const char* src = (const char*)texels[currentMipLevel].fPixels;
            SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);

            [blitCmdEncoder copyFromBuffer: transferBuffer
                              sourceOffset: bufferOffset + individualMipOffsets[currentMipLevel]
                         sourceBytesPerRow: trimRowBytes
                       sourceBytesPerImage: trimRowBytes*currentHeight
                                sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                                 toTexture: mtlTexture
                          destinationSlice: 0
                          destinationLevel: currentMipLevel
                         destinationOrigin: origin];
        }
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
        layerHeight = currentHeight;
    }
#ifdef SK_BUILD_FOR_MAC
    [transferBuffer didModifyRange: NSMakeRange(bufferOffset, combinedBufferSize)];
#endif

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->texturePriv().markMipMapsDirty();
    }

    return true;
}

bool GrMtlGpu::clearTexture(GrMtlTexture* tex, GrColorType dataColorType, uint32_t levelMask) {
    SkASSERT(this->caps()->isFormatTexturableAndUploadable(dataColorType, tex->backendFormat()));

    if (!levelMask) {
        return true;
    }

    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    int mipLevelCount = (int)mtlTexture.mipmapLevelCount;

    // TODO: implement some way of reusing transfer buffers?
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = 0;
    int currentWidth = tex->width();
    int currentHeight = tex->height();

    // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
    // config. This works with the assumption that the bytes in pixel config is always a power of 2.
    // TODO: can we just copy from a single buffer the size of the largest cleared level w/o a perf
    // penalty?
    SkASSERT((bpp & (bpp - 1)) == 0);
    const size_t alignmentMask = 0x3 | (bpp - 1);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (levelMask & (1 << currentMipLevel)) {
            const size_t trimmedSize = currentWidth * bpp * currentHeight;
            const size_t alignmentDiff = combinedBufferSize & alignmentMask;
            if (alignmentDiff != 0) {
                combinedBufferSize += alignmentMask - alignmentDiff + 1;
            }
            individualMipOffsets.push_back(combinedBufferSize);
            combinedBufferSize += trimmedSize;
        }
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
    }
    SkASSERT(combinedBufferSize > 0 && !individualMipOffsets.empty());

    // TODO: Create GrMtlTransferBuffer
    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        options |= MTLResourceStorageModePrivate;
    }
    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: combinedBufferSize
                                                        options: options];
    if (nil == transferBuffer) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    // clear the buffer to transparent black
    NSRange clearRange;
    clearRange.location = 0;
    clearRange.length = combinedBufferSize;
    [blitCmdEncoder fillBuffer: transferBuffer
                         range: clearRange
                         value: 0];

    // now copy buffer to texture
    currentWidth = tex->width();
    currentHeight = tex->height();
    MTLOrigin origin = MTLOriginMake(0, 0, 0);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (levelMask & (1 << currentMipLevel)) {
            const size_t rowBytes = currentWidth * bpp;

            [blitCmdEncoder copyFromBuffer: transferBuffer
                              sourceOffset: individualMipOffsets[currentMipLevel]
                         sourceBytesPerRow: rowBytes
                       sourceBytesPerImage: rowBytes * currentHeight
                                sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                                 toTexture: mtlTexture
                          destinationSlice: 0
                          destinationLevel: currentMipLevel
                         destinationOrigin: origin];
        }
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
    }

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->texturePriv().markMipMapsDirty();
    }

    return true;
}

GrStencilAttachment* GrMtlGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    SkASSERT(numStencilSamples == rt->numSamples());
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numSamples();

    const GrMtlCaps::StencilFormat& sFmt = this->mtlCaps().preferredStencilFormat();

    GrMtlStencilAttachment* stencil(GrMtlStencilAttachment::Create(this,
                                                                   width,
                                                                   height,
                                                                   samples,
                                                                   sFmt));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

sk_sp<GrTexture> GrMtlGpu::onCreateTexture(const GrSurfaceDesc& desc,
                                           const GrBackendFormat& format,
                                           GrRenderable renderable,
                                           int renderTargetSampleCnt,
                                           SkBudgeted budgeted,
                                           GrProtected isProtected,
                                           int mipLevelCount,
                                           uint32_t levelClearMask) {
    // We don't support protected textures in Metal.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    SkASSERT(mipLevelCount > 0);

    MTLPixelFormat mtlPixelFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlPixelFormat != MTLPixelFormatInvalid);
    SkASSERT(!this->caps()->isFormatCompressed(format));

    sk_sp<GrMtlTexture> tex;
    // This TexDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this TexDesc describes the resolved texture. Therefore we always have samples
    // set to 1.
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = MTLTextureType2D;
    texDesc.pixelFormat = mtlPixelFormat;
    texDesc.width = desc.fWidth;
    texDesc.height = desc.fHeight;
    texDesc.depth = 1;
    texDesc.mipmapLevelCount = mipLevelCount;
    texDesc.sampleCount = 1;
    texDesc.arrayLength = 1;
    // Make all textures have private gpu only access. We can use transfer buffers or textures
    // to copy to them.
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead;
        texDesc.usage |= (renderable == GrRenderable::kYes) ? MTLTextureUsageRenderTarget : 0;
    }

    GrMipMapsStatus mipMapsStatus =
            mipLevelCount > 1 ? GrMipMapsStatus::kDirty : GrMipMapsStatus::kNotAllocated;
    if (renderable == GrRenderable::kYes) {
        tex = GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(this, budgeted,
                                                                   desc, renderTargetSampleCnt,
                                                                   texDesc, mipMapsStatus);
    } else {
        tex = GrMtlTexture::MakeNewTexture(this, budgeted, desc, texDesc, mipMapsStatus);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        auto colorType = GrPixelConfigToColorType(desc.fConfig);
        this->clearTexture(tex.get(), colorType, levelClearMask);
    }

    return tex;
}

sk_sp<GrTexture> GrMtlGpu::onCreateCompressedTexture(int width, int height,
                                                     const GrBackendFormat& format,
                                                     SkImage::CompressionType compressionType,
                                                     SkBudgeted budgeted, const void* data) {
    SkASSERT(this->caps()->isFormatTexturable(format));
    SkASSERT(data);

    if (!check_max_blit_width(width)) {
        return nullptr;
    }

    MTLPixelFormat mtlPixelFormat = GrBackendFormatAsMTLPixelFormat(format);

    // This TexDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this TexDesc describes the resolved texture. Therefore we always have samples
    // set to 1.
    // Compressed textures with MIP levels or multiple samples are not supported as of now.
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = MTLTextureType2D;
    texDesc.pixelFormat = mtlPixelFormat;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.depth = 1;
    texDesc.mipmapLevelCount = 1;
    texDesc.sampleCount = 1;
    texDesc.arrayLength = 1;
    // Make all textures have private gpu only access. We can use transfer buffers or textures
    // to copy to them.
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead;
    }

    GrSurfaceDesc desc;
    desc.fConfig = GrCompressionTypeToPixelConfig(compressionType);
    desc.fWidth = width;
    desc.fHeight = height;
    auto tex = GrMtlTexture::MakeNewTexture(this, budgeted, desc, texDesc,
                                            GrMipMapsStatus::kNotAllocated);
    if (!tex) {
        return nullptr;
    }

    // Upload to texture
    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);

    SkImage::CompressionType textureCompressionType;
    if (!GrMtlFormatToCompressionType(mtlTexture.pixelFormat, &textureCompressionType) ||
        textureCompressionType != compressionType) {
        return nullptr;
    }

    size_t dataSize = GrCompressedDataSize(compressionType, width, height);
    SkASSERT(dataSize);

    size_t bufferOffset;
    id<MTLBuffer> transferBuffer = this->resourceProvider().getDynamicBuffer(dataSize,
                                                                             &bufferOffset);
    if (!transferBuffer) {
        return nullptr;
    }
    char* buffer = (char*) transferBuffer.contents + bufferOffset;

    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    const size_t rowBytes = GrCompressedRowBytes(compressionType, width);

    // copy data into the buffer, skipping any trailing bytes
    memcpy(buffer, data, dataSize);
    [blitCmdEncoder copyFromBuffer: transferBuffer
                      sourceOffset: bufferOffset
                 sourceBytesPerRow: rowBytes
               sourceBytesPerImage: dataSize
                        sourceSize: MTLSizeMake(width, height, 1)
                         toTexture: mtlTexture
                  destinationSlice: 0
                  destinationLevel: 0
                 destinationOrigin: origin];
#ifdef SK_BUILD_FOR_MAC
    [transferBuffer didModifyRange: NSMakeRange(bufferOffset, dataSize)];
#endif

    return tex;
}

static id<MTLTexture> get_texture_from_backend(const GrBackendTexture& backendTex) {
    GrMtlTextureInfo textureInfo;
    if (!backendTex.getMtlTextureInfo(&textureInfo)) {
        return nil;
    }
    return GrGetMTLTexture(textureInfo.fTexture.get());
}

static id<MTLTexture> get_texture_from_backend(const GrBackendRenderTarget& backendRT) {
    GrMtlTextureInfo textureInfo;
    if (!backendRT.getMtlTextureInfo(&textureInfo)) {
        return nil;
    }
    return GrGetMTLTexture(textureInfo.fTexture.get());
}

static inline void init_surface_desc(GrSurfaceDesc* surfaceDesc, id<MTLTexture> mtlTexture,
                                     GrRenderable renderable, GrPixelConfig config) {
    if (@available(macOS 10.11, iOS 9.0, *)) {
        if (renderable == GrRenderable::kYes) {
            SkASSERT(MTLTextureUsageRenderTarget & mtlTexture.usage);
        }
    }
    surfaceDesc->fWidth = mtlTexture.width;
    surfaceDesc->fHeight = mtlTexture.height;
    surfaceDesc->fConfig = config;
}

sk_sp<GrTexture> GrMtlGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                GrColorType grColorType,
                                                GrWrapOwnership,
                                                GrWrapCacheable cacheable, GrIOType ioType) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                    grColorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, GrRenderable::kNo, config);

    return GrMtlTexture::MakeWrappedTexture(this, surfDesc, mtlTexture, cacheable, ioType);
}

sk_sp<GrTexture> GrMtlGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                          int sampleCnt,
                                                          GrColorType colorType,
                                                          GrWrapOwnership,
                                                          GrWrapCacheable cacheable) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }

    const GrMtlCaps& caps = this->mtlCaps();

    MTLPixelFormat format = mtlTexture.pixelFormat;
    if (!caps.isFormatRenderable(format, sampleCnt)) {
        return nullptr;
    }

    GrPixelConfig config = caps.getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                           colorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, GrRenderable::kYes, config);

    sampleCnt = caps.getRenderTargetSampleCount(sampleCnt, format);
    SkASSERT(sampleCnt);

    return GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(this, surfDesc, sampleCnt,
                                                                    mtlTexture, cacheable);
}

sk_sp<GrRenderTarget> GrMtlGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT,
                                                          GrColorType grColorType) {
    // TODO: Revisit this when the Metal backend is completed. It may support MSAA render targets.
    if (backendRT.sampleCnt() > 1) {
        return nullptr;
    }
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendRT);
    if (!mtlTexture) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendRT.getBackendFormat(),
                                                                    grColorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, GrRenderable::kYes, config);

    return GrMtlRenderTarget::MakeWrappedRenderTarget(this, surfDesc, backendRT.sampleCnt(),
                                                      mtlTexture);
}

sk_sp<GrRenderTarget> GrMtlGpu::onWrapBackendTextureAsRenderTarget(
        const GrBackendTexture& backendTex, int sampleCnt, GrColorType grColorType) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }

    MTLPixelFormat format = mtlTexture.pixelFormat;
    if (!this->mtlCaps().isFormatRenderable(format, sampleCnt)) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                    grColorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, GrRenderable::kYes, config);
    sampleCnt = this->mtlCaps().getRenderTargetSampleCount(sampleCnt, format);
    if (!sampleCnt) {
        return nullptr;
    }

    return GrMtlRenderTarget::MakeWrappedRenderTarget(this, surfDesc, sampleCnt, mtlTexture);
}

bool GrMtlGpu::onRegenerateMipMapLevels(GrTexture* texture) {
    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> mtlTexture = grMtlTexture->mtlTexture();

    // Automatic mipmap generation is only supported by color-renderable formats
    if (!fMtlCaps->isFormatRenderable(mtlTexture.pixelFormat, 1) &&
        // We have pixel configs marked as textureable-only that use RGBA8 as the internal format
        MTLPixelFormatRGBA8Unorm != mtlTexture.pixelFormat) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder generateMipmapsForTexture: mtlTexture];

    return true;
}

// Used to "clear" a backend texture to a constant color by transferring.
static GrColorType mtl_format_to_backend_tex_clear_colortype(MTLPixelFormat format) {
    switch(format) {
        case MTLPixelFormatA8Unorm:         return GrColorType::kAlpha_8;
        case MTLPixelFormatR8Unorm:         return GrColorType::kR_8;

#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:     return GrColorType::kBGR_565;
        case MTLPixelFormatABGR4Unorm:      return GrColorType::kABGR_4444;
#endif
        case MTLPixelFormatRGBA8Unorm:      return GrColorType::kRGBA_8888;
        case MTLPixelFormatRGBA8Unorm_sRGB: return GrColorType::kRGBA_8888_SRGB;

        case MTLPixelFormatRG8Unorm:        return GrColorType::kRG_88;
        case MTLPixelFormatBGRA8Unorm:      return GrColorType::kBGRA_8888;
        case MTLPixelFormatRGB10A2Unorm:    return GrColorType::kRGBA_1010102;
        case MTLPixelFormatR16Float:        return GrColorType::kR_F16;
        case MTLPixelFormatRGBA16Float:     return GrColorType::kRGBA_F16;
        case MTLPixelFormatR16Unorm:        return GrColorType::kR_16;
        case MTLPixelFormatRG16Unorm:       return GrColorType::kRG_1616;
        case MTLPixelFormatRGBA16Unorm:     return GrColorType::kRGBA_16161616;
        case MTLPixelFormatRG16Float:       return GrColorType::kRG_F16;
        default:                            return GrColorType::kUnknown;
    }

    SkUNREACHABLE;
}

void copy_src_data(char* dst, size_t bytesPerPixel, const SkTArray<size_t>& individualMipOffsets,
                   const SkPixmap srcData[], int numMipLevels, size_t bufferSize) {
    SkASSERT(srcData && numMipLevels);
    SkASSERT(individualMipOffsets.count() == numMipLevels);

    for (int level = 0; level < numMipLevels; ++level) {
        const size_t trimRB = srcData[level].width() * bytesPerPixel;
        SkASSERT(individualMipOffsets[level] + trimRB * srcData[level].height() <= bufferSize);
        SkRectMemcpy(dst + individualMipOffsets[level], trimRB,
                     srcData[level].addr(), srcData[level].rowBytes(),
                     trimRB, srcData[level].height());
    }
}

bool GrMtlGpu::createMtlTextureForBackendSurface(MTLPixelFormat format,
                                                 SkISize dimensions,
                                                 bool texturable,
                                                 bool renderable,
                                                 const BackendTextureData* data,
                                                 int numMipLevels,
                                                 GrMtlTextureInfo* info) {
    SkASSERT(texturable || renderable);
    if (!texturable) {
        SkASSERT(!data && numMipLevels == 1);
    }

#ifdef SK_BUILD_FOR_IOS
    // Compressed formats go through onCreateCompressedBackendTexture
    SkASSERT(!GrMtlFormatIsCompressed(format));
#endif

    if (texturable && !fMtlCaps->isFormatTexturable(format)) {
        return false;
    }
    if (renderable && !fMtlCaps->isFormatRenderable(format, 1)) {
        return false;
    }

    if (!check_max_blit_width(dimensions.width())) {
        return false;
    }

    bool mipmapped = numMipLevels > 1;
    MTLTextureDescriptor* desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: format
                                                               width: dimensions.width()
                                                              height: dimensions.height()
                                                           mipmapped: mipmapped];
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.storageMode = MTLStorageModePrivate;
        desc.usage = texturable ? MTLTextureUsageShaderRead : 0;
        desc.usage |= renderable ? MTLTextureUsageRenderTarget : 0;
    }
    id<MTLTexture> testTexture = [fDevice newTextureWithDescriptor: desc];

    if (!data) {
        info->fTexture.reset(GrRetainPtrFromId(testTexture));

        return true;
    }

    // Create the transfer buffer
    size_t bytesPerPixel = fMtlCaps->bytesPerPixel(format);

    NSUInteger options = 0;  // TODO: consider other options here
    if (@available(macOS 10.11, iOS 9.0, *)) {
#ifdef SK_BUILD_FOR_MAC
        options |= MTLResourceStorageModeManaged;
#else
        options |= MTLResourceStorageModeShared;
#endif
    }

    // Create a transfer buffer and fill with data.
    SkSTArray<16, size_t> individualMipOffsets;
    id<MTLBuffer> transferBuffer;
    size_t transferBufferSize;
    if (data->type() == BackendTextureData::Type::kPixmaps) {
        transferBufferSize = GrComputeTightCombinedBufferSize(bytesPerPixel, dimensions,
                                                              &individualMipOffsets, numMipLevels);
        transferBuffer = [fDevice newBufferWithLength: transferBufferSize
                                              options: options];
        if (nil == transferBuffer) {
            return false;
        }
        char* buffer = (char*)transferBuffer.contents;
        copy_src_data(buffer, bytesPerPixel, individualMipOffsets, data->pixmaps(), numMipLevels,
                      transferBufferSize);
    } else {
        SkASSERT(data->type() == BackendTextureData::Type::kColor);
        auto colorType = mtl_format_to_backend_tex_clear_colortype(format);
        if (colorType == GrColorType::kUnknown) {
            return false;
        }
        GrImageInfo ii(colorType, kUnpremul_SkAlphaType, nullptr, dimensions);
        auto rb = ii.minRowBytes();
        transferBufferSize = rb*dimensions.height();
        transferBuffer = [fDevice newBufferWithLength: transferBufferSize
                                              options: options];
        if (nil == transferBuffer) {
            return false;
        }
        if (!GrClearImage(ii, transferBuffer.contents, rb, data->color())) {
            return false;
        }
        // Reuse the same buffer for all levels. Should be ok since we made the row bytes tight.
        individualMipOffsets.push_back_n(numMipLevels, (size_t)0);
    }

    // Transfer buffer contents to texture
    int currentWidth = dimensions.width();
    int currentHeight = dimensions.height();
    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    id<MTLCommandBuffer> cmdBuffer = [fQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitCmdEncoder = [cmdBuffer blitCommandEncoder];

    for (int currentMipLevel = 0; currentMipLevel < numMipLevels; currentMipLevel++) {
        size_t trimRowBytes = currentWidth * bytesPerPixel;
        size_t levelSize = trimRowBytes*currentHeight;

        // TODO: can this all be done in one go?
        [blitCmdEncoder copyFromBuffer: transferBuffer
                          sourceOffset: individualMipOffsets[currentMipLevel]
                     sourceBytesPerRow: trimRowBytes
                   sourceBytesPerImage: levelSize
                            sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                             toTexture: testTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];

        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
    }
#ifdef SK_BUILD_FOR_MAC
    [transferBuffer didModifyRange: NSMakeRange(0, transferBufferSize)];
#endif

    [blitCmdEncoder endEncoding];
    [cmdBuffer commit];
    [cmdBuffer waitUntilCompleted];
    transferBuffer = nil;

    info->fTexture.reset(GrRetainPtrFromId(testTexture));

    return true;
}

GrBackendTexture GrMtlGpu::onCreateBackendTexture(SkISize dimensions,
                                                  const GrBackendFormat& format,
                                                  GrRenderable renderable,
                                                  const BackendTextureData* data,
                                                  int numMipLevels,
                                                  GrProtected isProtected) {
    // GrGpu::createBackendTexture should've ensured these conditions
    const MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(mtlFormat, dimensions, true,
                                                 GrRenderable::kYes == renderable, data,
                                                 numMipLevels, &info)) {
        return {};
    }
    GrMipMapped mipMapped = numMipLevels > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
    GrBackendTexture backendTex(dimensions.width(), dimensions.height(), mipMapped, info);
    return backendTex;
}

void GrMtlGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kMetal == tex.backend());
    // Nothing to do here, will get cleaned up when the GrBackendTexture object goes away
}

#if GR_TEST_UTILS
bool GrMtlGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kMetal == tex.backend());

    GrMtlTextureInfo info;
    if (!tex.getMtlTextureInfo(&info)) {
        return false;
    }
    id<MTLTexture> mtlTexture = GrGetMTLTexture(info.fTexture.get());
    if (!mtlTexture) {
        return false;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        return mtlTexture.usage & MTLTextureUsageShaderRead;
    } else {
        return true; // best we can do
    }
}

GrBackendRenderTarget GrMtlGpu::createTestingOnlyBackendRenderTarget(int w, int h, GrColorType ct) {
    if (w > this->caps()->maxRenderTargetSize() || h > this->caps()->maxRenderTargetSize()) {
        return GrBackendRenderTarget();
    }

    GrPixelConfig config = GrColorTypeToPixelConfig(ct);

    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return GrBackendRenderTarget();
    }

    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(format, {w, h}, false, true, nullptr, 1, &info)) {
        return {};
    }

    GrBackendRenderTarget backendRT(w, h, 1, info);
    return backendRT;
}

void GrMtlGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kMetal == rt.backend());

    GrMtlTextureInfo info;
    if (rt.getMtlTextureInfo(&info)) {
        this->testingOnly_flushGpuAndSync();
        // Nothing else to do here, will get cleaned up when the GrBackendRenderTarget
        // is deleted.
    }
}

void GrMtlGpu::testingOnly_flushGpuAndSync() {
    this->submitCommandBuffer(kForce_SyncQueue);
}
#endif // GR_TEST_UTILS

static int get_surface_sample_cnt(GrSurface* surf) {
    if (const GrRenderTarget* rt = surf->asRenderTarget()) {
        return rt->numSamples();
    }
    return 0;
}

void GrMtlGpu::copySurfaceAsResolve(GrSurface* dst, GrSurface* src) {
    // TODO: Add support for subrectangles
    GrMtlRenderTarget* srcRT = static_cast<GrMtlRenderTarget*>(src->asRenderTarget());
    GrRenderTarget* dstRT = dst->asRenderTarget();
    id<MTLTexture> dstTexture;
    if (dstRT) {
        GrMtlRenderTarget* mtlRT = static_cast<GrMtlRenderTarget*>(dstRT);
        dstTexture = mtlRT->mtlColorTexture();
    } else {
        SkASSERT(dst->asTexture());
        dstTexture = static_cast<GrMtlTexture*>(dst->asTexture())->mtlTexture();
    }

    this->resolveTexture(dstTexture, srcRT->mtlColorTexture());
}

void GrMtlGpu::copySurfaceAsBlit(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                 const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
    SkASSERT(this->mtlCaps().canCopyAsBlit(dst, dstSampleCnt, src, srcSampleCnt,
                                           srcRect, dstPoint, dst == src));
#endif
    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromTexture: srcTex
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(srcRect.x(), srcRect.y(), 0)
                         sourceSize: MTLSizeMake(srcRect.width(), srcRect.height(), 1)
                          toTexture: dstTex
                   destinationSlice: 0
                   destinationLevel: 0
                  destinationOrigin: MTLOriginMake(dstPoint.fX, dstPoint.fY, 0)];
}

bool GrMtlGpu::onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                             const SkIPoint& dstPoint) {
    SkASSERT(!src->isProtected() && !dst->isProtected());

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);

    bool success = false;
    if (this->mtlCaps().canCopyAsResolve(dst, dstSampleCnt, src, srcSampleCnt, srcRect, dstPoint)) {
        this->copySurfaceAsResolve(dst, src);
        success = true;
    } else if (this->mtlCaps().canCopyAsBlit(dst, dstSampleCnt, src, srcSampleCnt, srcRect,
                                             dstPoint, dst == src)) {
        this->copySurfaceAsBlit(dst, src, srcRect, dstPoint);
        success = true;
    }
    if (success) {
        SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.x(), dstPoint.y(),
                                            srcRect.width(), srcRect.height());
        // The rect is already in device space so we pass in kTopLeft so no flip is done.
        this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
    }
    return success;
}

bool GrMtlGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType srcColorType,
                             const GrMipLevel texels[], int mipLevelCount,
                             bool prepForTexSampling) {
    GrMtlTexture* mtlTexture = static_cast<GrMtlTexture*>(surface->asTexture());
    // TODO: In principle we should be able to support pure rendertargets as well, but
    // until we find a use case we'll only support texture rendertargets.
    if (!mtlTexture) {
        return false;
    }
    if (!mipLevelCount) {
        return false;
    }
#ifdef SK_DEBUG
    for (int i = 0; i < mipLevelCount; i++) {
        SkASSERT(texels[i].fPixels);
    }
#endif
    return this->uploadToTexture(mtlTexture, left, top, width, height, srcColorType, texels,
                                 mipLevelCount);
}

bool GrMtlGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                            size_t rowBytes) {
    SkASSERT(surface);

    if (surfaceColorType != dstColorType) {
        return false;
    }

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    size_t transBufferRowBytes = bpp * width;
    size_t transBufferImageBytes = transBufferRowBytes * height;

    // TODO: implement some way of reusing buffers instead of making a new one every time.
    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
#ifdef SK_BUILD_FOR_MAC
        options |= MTLResourceStorageModeManaged;
#else
        options |= MTLResourceStorageModeShared;
#endif
    }

    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: transBufferImageBytes
                                                        options: options];

    if (!this->readOrTransferPixels(surface, left, top, width, height, dstColorType, transferBuffer,
                                    0, transBufferImageBytes, transBufferRowBytes)) {
        return false;
    }
    this->submitCommandBuffer(kForce_SyncQueue);

    const void* mappedMemory = transferBuffer.contents;

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, transBufferRowBytes, height);

    return true;
}

bool GrMtlGpu::onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                                  GrColorType textureColorType, GrColorType bufferColorType,
                                  GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) {
    SkASSERT(texture);
    SkASSERT(transferBuffer);
    if (textureColorType != bufferColorType ||
        GrPixelConfigToColorType(texture->config()) != bufferColorType) {
        return false;
    }

    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> mtlTexture = grMtlTexture->mtlTexture();
    SkASSERT(mtlTexture);

    GrMtlBuffer* grMtlBuffer = static_cast<GrMtlBuffer*>(transferBuffer);
    id<MTLBuffer> mtlBuffer = grMtlBuffer->mtlBuffer();
    SkASSERT(mtlBuffer);

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (offset % bpp) {
        return false;
    }
    MTLOrigin origin = MTLOriginMake(left, top, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromBuffer: mtlBuffer
                      sourceOffset: offset + grMtlBuffer->offset()
                 sourceBytesPerRow: rowBytes
               sourceBytesPerImage: rowBytes*height
                        sourceSize: MTLSizeMake(width, height, 1)
                         toTexture: mtlTexture
                  destinationSlice: 0
                  destinationLevel: 0
                 destinationOrigin: origin];

    return true;
}

bool GrMtlGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                    GrColorType surfaceColorType, GrColorType bufferColorType,
                                    GrGpuBuffer* transferBuffer, size_t offset) {
    SkASSERT(surface);
    SkASSERT(transferBuffer);

    if (surfaceColorType != bufferColorType) {
        return false;
    }

    // Metal only supports offsets that are aligned to a pixel.
    int bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (offset % bpp) {
        return false;
    }

    GrMtlBuffer* grMtlBuffer = static_cast<GrMtlBuffer*>(transferBuffer);
    grMtlBuffer->bind();

    size_t transBufferRowBytes = bpp * width;
    size_t transBufferImageBytes = transBufferRowBytes * height;

    return this->readOrTransferPixels(surface, left, top, width, height, bufferColorType,
                                      grMtlBuffer->mtlBuffer(), offset + grMtlBuffer->offset(),
                                      transBufferImageBytes, transBufferRowBytes);
}

bool GrMtlGpu::readOrTransferPixels(GrSurface* surface, int left, int top, int width, int height,
                                    GrColorType dstColorType, id<MTLBuffer> transferBuffer,
                                    size_t offset, size_t imageBytes, size_t rowBytes) {
    if (!check_max_blit_width(width)) {
        return false;
    }
    if (GrPixelConfigToColorType(surface->config()) != dstColorType) {
        return false;
    }

    id<MTLTexture> mtlTexture;
    if (GrMtlRenderTarget* rt = static_cast<GrMtlRenderTarget*>(surface->asRenderTarget())) {
        if (rt->numSamples() > 1) {
            SkASSERT(rt->requiresManualMSAAResolve());  // msaa-render-to-texture not yet supported.
            mtlTexture = rt->mtlResolveTexture();
        } else {
            SkASSERT(!rt->requiresManualMSAAResolve());
            mtlTexture = rt->mtlColorTexture();
        }
    } else if (GrMtlTexture* texture = static_cast<GrMtlTexture*>(surface->asTexture())) {
        mtlTexture = texture->mtlTexture();
    }
    if (!mtlTexture) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromTexture: mtlTexture
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(left, top, 0)
                         sourceSize: MTLSizeMake(width, height, 1)
                           toBuffer: transferBuffer
                  destinationOffset: offset
             destinationBytesPerRow: rowBytes
           destinationBytesPerImage: imageBytes];
#ifdef SK_BUILD_FOR_MAC
    // Sync GPU data back to the CPU
    [blitCmdEncoder synchronizeResource: transferBuffer];
#endif

    return true;
}

GrFence SK_WARN_UNUSED_RESULT GrMtlGpu::insertFence() {
    GrMtlCommandBuffer* cmdBuffer = this->commandBuffer();
    // We create a semaphore and signal it within the current
    // command buffer's completion handler.
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    cmdBuffer->addCompletedHandler(^(id <MTLCommandBuffer>commandBuffer) {
        dispatch_semaphore_signal(semaphore);
    });

    const void* cfFence = (__bridge_retained const void*) semaphore;
    return (GrFence) cfFence;
}

bool GrMtlGpu::waitFence(GrFence fence, uint64_t timeout) {
    const void* cfFence = (const void*) fence;
    dispatch_semaphore_t semaphore = (__bridge dispatch_semaphore_t)cfFence;

    long result = dispatch_semaphore_wait(semaphore, timeout);

    return !result;
}

void GrMtlGpu::deleteFence(GrFence fence) const {
    const void* cfFence = (const void*) fence;
    // In this case it's easier to release in CoreFoundation than depend on ARC
    CFRelease(cfFence);
}

std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT GrMtlGpu::makeSemaphore(bool /*isOwned*/) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrMtlSemaphore::Make(this);
}

std::unique_ptr<GrSemaphore> GrMtlGpu::wrapBackendSemaphore(
        const GrBackendSemaphore& semaphore,
        GrResourceProvider::SemaphoreWrapType wrapType,
        GrWrapOwnership /*ownership*/) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrMtlSemaphore::MakeWrapped(semaphore.mtlSemaphore(), semaphore.mtlValue());
}

void GrMtlGpu::insertSemaphore(GrSemaphore* semaphore) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        GrMtlSemaphore* mtlSem = static_cast<GrMtlSemaphore*>(semaphore);

        this->commandBuffer()->encodeSignalEvent(mtlSem->event(), mtlSem->value());
    }
}

void GrMtlGpu::waitSemaphore(GrSemaphore* semaphore) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        GrMtlSemaphore* mtlSem = static_cast<GrMtlSemaphore*>(semaphore);

        this->commandBuffer()->encodeWaitForEvent(mtlSem->event(), mtlSem->value());
    }
}

void GrMtlGpu::onResolveRenderTarget(GrRenderTarget* target, const SkIRect&, GrSurfaceOrigin,
                                     ForExternalIO forExternalIO) {
    this->resolveTexture(static_cast<GrMtlRenderTarget*>(target)->mtlResolveTexture(),
                         static_cast<GrMtlRenderTarget*>(target)->mtlColorTexture());

    if (ForExternalIO::kYes == forExternalIO) {
        // This resolve is called when we are preparing an msaa surface for external I/O. It is
        // called after flushing, so we need to make sure we submit the command buffer after
        // doing the resolve so that the resolve actually happens.
        this->submitCommandBuffer(kSkip_SyncQueue);
    }
}

void GrMtlGpu::resolveTexture(id<MTLTexture> resolveTexture, id<MTLTexture> colorTexture) {
    auto renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = colorTexture;
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    renderPassDesc.colorAttachments[0].resolveTexture = resolveTexture;
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;

    id<MTLRenderCommandEncoder> cmdEncoder =
            this->commandBuffer()->getRenderCommandEncoder(renderPassDesc, nullptr, nullptr);
    SkASSERT(nil != cmdEncoder);
    cmdEncoder.label = @"resolveTexture";
}

#if GR_TEST_UTILS
void GrMtlGpu::testingOnly_startCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        // TODO: add Metal 3 interface as well
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        [captureManager startCaptureWithDevice: fDevice];
    }
}

void GrMtlGpu::testingOnly_endCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        [captureManager stopCapture];
    }
}
#endif
