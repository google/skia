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
#include "src/gpu/mtl/GrMtlGpuCommandBuffer.h"
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
    if ([device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v2]) {
        *featureSet = MTLFeatureSet_OSX_GPUFamily1_v2;
        return true;
    }
    if ([device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v1]) {
        *featureSet = MTLFeatureSet_OSX_GPUFamily1_v1;
        return true;
    }
#endif

    // iOS Family group 3
#ifdef SK_BUILD_FOR_IOS
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily3_v2;
        return true;
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily3_v1;
        return true;
    }

    // iOS Family group 2
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v3]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily2_v3;
        return true;
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily2_v2;
        return true;
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily2_v1;
        return true;
    }

    // iOS Family group 1
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v3]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily1_v3;
        return true;
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily1_v2;
        return true;
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

GrGpuRTCommandBuffer* GrMtlGpu::getCommandBuffer(
            GrRenderTarget* renderTarget, GrSurfaceOrigin origin, const SkRect& bounds,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    return new GrMtlGpuRTCommandBuffer(this, renderTarget, origin, bounds, colorInfo, stencilInfo);
}

GrGpuTextureCommandBuffer* GrMtlGpu::getCommandBuffer(GrTexture* texture,
                                                      GrSurfaceOrigin origin) {
    return new GrMtlGpuTextureCommandBuffer(this, texture, origin);
}

void GrMtlGpu::submit(GrGpuCommandBuffer* buffer) {
    GrMtlGpuRTCommandBuffer* mtlRTCmdBuffer =
            reinterpret_cast<GrMtlGpuRTCommandBuffer*>(buffer->asRTCommandBuffer());
    if (mtlRTCmdBuffer) {
        mtlRTCmdBuffer->submit();
    }
    delete buffer;
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

    // TODO: implement some way of reusing transfer buffers?
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = GrComputeTightCombinedBufferSize(bpp, width, height,
                                                                 &individualMipOffsets,
                                                                 mipLevelCount);
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
    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: combinedBufferSize
                                                        options: MTLResourceStorageModePrivate];
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
                                           const GrMipLevel texels[],
                                           int mipLevelCount) {
    // We don't support protected textures in Metal.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    int mipLevels = !mipLevelCount ? 1 : mipLevelCount;

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
    texDesc.mipmapLevelCount = mipLevels;
    texDesc.sampleCount = 1;
    texDesc.arrayLength = 1;
    // Make all textures have private gpu only access. We can use transfer buffers or textures
    // to copy to them.
    texDesc.storageMode = MTLStorageModePrivate;
    texDesc.usage = MTLTextureUsageShaderRead;
    texDesc.usage |= (renderable == GrRenderable::kYes) ? MTLTextureUsageRenderTarget : 0;

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

    auto colorType = GrPixelConfigToColorType(desc.fConfig);
    if (mipLevelCount && texels[0].fPixels) {
        if (!this->uploadToTexture(tex.get(), 0, 0, desc.fWidth, desc.fHeight, colorType, texels,
                                   mipLevelCount)) {
            tex->unref();
            return nullptr;
        }
    }

    if (this->caps()->shouldInitializeTextures()) {
        uint32_t levelMask = ~0;
        SkASSERT(mipLevelCount < 32);
        for (int i = 0; i < mipLevelCount; ++i) {
            if (!texels[i].fPixels) {
                levelMask &= ~(1 << i);
            }
        }
        this->clearTexture(tex.get(), colorType, levelMask);
    }

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
    if (renderable == GrRenderable::kYes) {
        SkASSERT(MTLTextureUsageRenderTarget & mtlTexture.usage);
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

static GrPixelConfig mtl_format_to_pixelconfig(MTLPixelFormat format) {
    switch(format) {
        case MTLPixelFormatA8Unorm:         return kAlpha_8_GrPixelConfig;
        case MTLPixelFormatR8Unorm:         return kAlpha_8_GrPixelConfig;

#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:     return kRGB_565_GrPixelConfig;
        case MTLPixelFormatABGR4Unorm:      return kRGBA_4444_GrPixelConfig;
#endif
        case MTLPixelFormatRGBA8Unorm:      return kRGBA_8888_GrPixelConfig;
        case MTLPixelFormatRGBA8Unorm_sRGB: return kSRGBA_8888_GrPixelConfig;

#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:       return kRGB_ETC1_GrPixelConfig;
#endif
        case MTLPixelFormatRG8Unorm:        return kRG_88_GrPixelConfig;
        case MTLPixelFormatBGRA8Unorm:      return kBGRA_8888_GrPixelConfig;
        case MTLPixelFormatRGB10A2Unorm:    return kRGBA_1010102_GrPixelConfig;
        case MTLPixelFormatR16Float:        return kAlpha_half_GrPixelConfig;
        case MTLPixelFormatRGBA16Float:     return kRGBA_half_GrPixelConfig;
        case MTLPixelFormatRGBA32Float:     return kRGBA_float_GrPixelConfig;
        case MTLPixelFormatR16Unorm:        return kR_16_GrPixelConfig;
        case MTLPixelFormatRG16Unorm:       return kRG_1616_GrPixelConfig;
        case MTLPixelFormatRGBA16Unorm:     return kRGBA_16161616_GrPixelConfig;
        case MTLPixelFormatRG16Float:       return kRG_half_GrPixelConfig;
        default:                            return kUnknown_GrPixelConfig;
    }

    SkUNREACHABLE;
}

bool GrMtlGpu::createTestingOnlyMtlTextureInfo(MTLPixelFormat format,
                                               int w, int h, bool texturable,
                                               bool renderable, GrMipMapped mipMapped,
                                               const void* srcData, size_t srcRowBytes,
                                               const SkColor4f* color, GrMtlTextureInfo* info) {
    SkASSERT(texturable || renderable);
    if (!texturable) {
        SkASSERT(GrMipMapped::kNo == mipMapped);
        SkASSERT(!srcData);
    }

    if (texturable && !fMtlCaps->isFormatTexturable(format)) {
        return false;
    }
    if (renderable && !fMtlCaps->isFormatRenderable(format, 1)) {
        return false;
    }
    // Currently we don't support uploading pixel data when mipped.
    if (srcData && GrMipMapped::kYes == mipMapped) {
        return false;
    }
    if(!check_max_blit_width(w)) {
        return false;
    }

    // TODO: allow uninitialized textures to be truly uninitialized
    if (!color) {
        color = &SkColors::kTransparent;
    }

    int mipLevelCount = 1;
    if (GrMipMapped::kYes == mipMapped) {
        mipLevelCount = SkMipMap::ComputeLevelCount(w, h) + 1;
    }

    bool mipmapped = mipMapped == GrMipMapped::kYes ? true : false;
    MTLTextureDescriptor* desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: format
                                                               width: w
                                                              height: h
                                                           mipmapped: mipmapped];
    desc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
    desc.storageMode = MTLStorageModePrivate;
    desc.usage = texturable ? MTLTextureUsageShaderRead : 0;
    desc.usage |= renderable ? MTLTextureUsageRenderTarget : 0;
    id<MTLTexture> testTexture = [fDevice newTextureWithDescriptor: desc];

    size_t bytesPerPixel = GrMtlBytesPerFormat(format);

    if (!srcRowBytes) {
        srcRowBytes = w * bytesPerPixel;
#ifdef SK_BUILD_FOR_MAC
        if (!srcData) {
            // On MacOS, the fillBuffer command needs a range with a multiple of 4 bytes
            srcRowBytes = ((srcRowBytes + 3) & (~3));
        }
#endif
    }

    size_t combinedBufferSize = 0;
    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    if (srcData) {
        SkASSERT(1 == mipLevelCount);
        individualMipOffsets.push_back(0);

        combinedBufferSize = srcRowBytes * h;
    } else if (color) {
        combinedBufferSize = GrComputeTightCombinedBufferSize(bytesPerPixel, w, h,
                                                              &individualMipOffsets,
                                                              mipLevelCount);
    }

    NSUInteger options = 0;  // TODO: consider other options here
#ifdef SK_BUILD_FOR_MAC
    options |= MTLResourceStorageModeManaged;
#else
    options |= MTLResourceStorageModeShared;
#endif

    GrPixelConfig config = mtl_format_to_pixelconfig(format);
    SkASSERT(kUnknown_GrPixelConfig != config);

    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: combinedBufferSize
                                                        options: options];
    if (nil == transferBuffer) {
        return false;
    }

    char* buffer = (char*) transferBuffer.contents;

    if (srcData) {
        const size_t trimRowBytes = w * bytesPerPixel;

        SkASSERT(1 == mipLevelCount);
        if (!srcRowBytes) {
            srcRowBytes = trimRowBytes;
        }

        // copy data into the buffer, skipping the trailing bytes
        const char* src = (const char*) srcData;
        SkRectMemcpy(buffer, trimRowBytes, src, srcRowBytes, trimRowBytes, h);
    } else if (color) {
        GrFillInData(config, w, h, individualMipOffsets, buffer, *color);
    }

    int currentWidth = w;
    int currentHeight = h;
    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    id<MTLCommandBuffer> cmdBuffer = [fQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitCmdEncoder = [cmdBuffer blitCommandEncoder];

    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        const size_t trimRowBytes = currentWidth * bytesPerPixel;

        // TODO: can this all be done in one go?
        [blitCmdEncoder copyFromBuffer: transferBuffer
                          sourceOffset: individualMipOffsets[currentMipLevel]
                     sourceBytesPerRow: trimRowBytes
                   sourceBytesPerImage: trimRowBytes*currentHeight
                            sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                             toTexture: testTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];

        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
    }
#ifdef SK_BUILD_FOR_MAC
    [transferBuffer didModifyRange: NSMakeRange(0, combinedBufferSize)];
#endif

    [blitCmdEncoder endEncoding];
    [cmdBuffer commit];
    [cmdBuffer waitUntilCompleted];
    transferBuffer = nil;

    info->fTexture.reset(GrRetainPtrFromId(testTexture));

    return true;
}

GrBackendTexture GrMtlGpu::createBackendTexture(int w, int h,
                                                const GrBackendFormat& format,
                                                GrMipMapped mipMapped,
                                                GrRenderable renderable,
                                                const void* pixels, size_t rowBytes,
                                                const SkColor4f* color, GrProtected isProtected) {
    if (w > this->caps()->maxTextureSize() || h > this->caps()->maxTextureSize()) {
        return GrBackendTexture();
    }

    const MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    GrMtlTextureInfo info;
    if (!this->createTestingOnlyMtlTextureInfo(mtlFormat,
                                               w, h, true,
                                               GrRenderable::kYes == renderable, mipMapped,
                                               pixels, rowBytes, color, &info)) {
        return {};
    }

    GrBackendTexture backendTex(w, h, mipMapped, info);
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
    return mtlTexture.usage & MTLTextureUsageShaderRead;
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
    if (!this->createTestingOnlyMtlTextureInfo(format, w, h, false, true,
                                               GrMipMapped::kNo, nullptr, 0, nullptr, &info)) {
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
    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src);

#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
    SkASSERT(this->mtlCaps().canCopyAsBlit(dstTex.pixelFormat, dstSampleCnt, srcTex.pixelFormat,
                                           srcSampleCnt, srcRect, dstPoint, dst == src));
#endif
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
                             const SkIPoint& dstPoint, bool canDiscardOutsideDstRect) {
    SkASSERT(!src->isProtected() && !dst->isProtected());

    MTLPixelFormat dstFormat = GrBackendFormatAsMTLPixelFormat(dst->backendFormat());
    MTLPixelFormat srcFormat = GrBackendFormatAsMTLPixelFormat(src->backendFormat());

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);

    bool success = false;
    if (this->mtlCaps().canCopyAsResolve(dst, dstSampleCnt, src, srcSampleCnt, srcRect, dstPoint)) {
        this->copySurfaceAsResolve(dst, src);
        success = true;
    } else if (this->mtlCaps().canCopyAsBlit(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt,
                                             srcRect, dstPoint, dst == src)) {
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
                             const GrMipLevel texels[], int mipLevelCount) {
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
    if (!check_max_blit_width(width)) {
        return false;
    }
    if (GrPixelConfigToColorType(surface->config()) != dstColorType) {
        return false;
    }

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    size_t transBufferRowBytes = bpp * width;

    id<MTLTexture> mtlTexture;
    GrMtlRenderTarget* rt = static_cast<GrMtlRenderTarget*>(surface->asRenderTarget());
    if (rt) {
        // resolve the render target if necessary
        switch (rt->getResolveType()) {
            case GrMtlRenderTarget::kCantResolve_ResolveType:
                return false;
            case GrMtlRenderTarget::kAutoResolves_ResolveType:
                mtlTexture = rt->mtlColorTexture();
                break;
            case GrMtlRenderTarget::kCanResolve_ResolveType:
                this->resolveRenderTargetNoFlush(rt);
                mtlTexture = rt->mtlResolveTexture();
                break;
            default:
                SK_ABORT("Unknown resolve type");
        }
    } else {
        GrMtlTexture* texture = static_cast<GrMtlTexture*>(surface->asTexture());
        if (texture) {
            mtlTexture = texture->mtlTexture();
        }
    }

    if (!mtlTexture) {
        return false;
    }

    size_t transBufferImageBytes = transBufferRowBytes * height;

    // TODO: implement some way of reusing buffers instead of making a new one every time.
    NSUInteger options = 0;
#ifdef SK_BUILD_FOR_MAC
    options |= MTLResourceStorageModeManaged;
#else
    options |= MTLResourceStorageModeShared;
#endif
    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: transBufferImageBytes
                                                        options: options];

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromTexture: mtlTexture
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(left, top, 0)
                         sourceSize: MTLSizeMake(width, height, 1)
                           toBuffer: transferBuffer
                  destinationOffset: 0
             destinationBytesPerRow: transBufferRowBytes
           destinationBytesPerImage: transBufferImageBytes];
#ifdef SK_BUILD_FOR_MAC
    // Sync GPU data back to the CPU
    [blitCmdEncoder synchronizeResource: transferBuffer];
#endif

    this->submitCommandBuffer(kForce_SyncQueue);
    const void* mappedMemory = transferBuffer.contents;

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, transBufferRowBytes, height);

    return true;
}

void GrMtlGpu::internalResolveRenderTarget(GrRenderTarget* target, bool requiresSubmit) {
    if (target->needsResolve()) {
        this->resolveTexture(static_cast<GrMtlRenderTarget*>(target)->mtlResolveTexture(),
                             static_cast<GrMtlRenderTarget*>(target)->mtlColorTexture());
        target->flagAsResolved();

        if (requiresSubmit) {
            this->submitCommandBuffer(kSkip_SyncQueue);
        }
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
