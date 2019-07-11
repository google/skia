/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"

#include "src/core/SkConvertPixels.h"
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
    SkASSERT(this->caps()->isConfigTexturable(tex->config()));
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

    // TODO: implement some way of reusing transfer buffers?
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    individualMipOffsets.push_back(0);
    size_t combinedBufferSize = width * bpp * height;
    int currentWidth = width;
    int currentHeight = height;
    if (!texels[0].fPixels) {
        combinedBufferSize = 0;
    }

    // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
    // config. This works with the assumption that the bytes in pixel config is always a power of 2.
    SkASSERT((bpp & (bpp - 1)) == 0);
    const size_t alignmentMask = 0x3 | (bpp - 1);
    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; currentMipLevel++) {
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);

        if (texels[currentMipLevel].fPixels) {
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

    sk_sp<GrMtlBuffer> transferBuffer = GrMtlBuffer::Make(this, combinedBufferSize,
                                                          GrGpuBufferType::kXferCpuToGpu,
                                                          kStream_GrAccessPattern);
    if (!transferBuffer) {
        return false;
    }

    char* buffer = (char*) transferBuffer->map();
    size_t bufferOffset = transferBuffer->offset();

    currentWidth = width;
    currentHeight = height;
    int layerHeight = tex->height();
    MTLOrigin origin = MTLOriginMake(left, top, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texels[currentMipLevel].fPixels) {
            SkASSERT(1 == mipLevelCount || currentHeight == layerHeight);
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t rowBytes = texels[currentMipLevel].fRowBytes;

            // copy data into the buffer, skipping the trailing bytes
            char* dst = buffer + individualMipOffsets[currentMipLevel];
            const char* src = (const char*)texels[currentMipLevel].fPixels;
            SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);

            [blitCmdEncoder copyFromBuffer: transferBuffer->mtlBuffer()
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
    transferBuffer->unmap();

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->texturePriv().markMipMapsDirty();
    }

    return true;
}

bool GrMtlGpu::clearTexture(GrMtlTexture* tex, GrColorType dataColorType) {
    SkASSERT(this->caps()->isConfigTexturable(tex->config()));

    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    int mipLevelCount = (int)mtlTexture.mipmapLevelCount;

    // TODO: implement some way of reusing transfer buffers?
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    individualMipOffsets.push_back(0);
    int width = tex->width();
    int height = tex->height();
    size_t combinedBufferSize = width * bpp * height;
    int currentWidth = width;
    int currentHeight = height;

    // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
    // config. This works with the assumption that the bytes in pixel config is always a power of 2.
    // TODO: can we just copy from a single buffer the size of the top level w/o a perf penalty?
    SkASSERT((bpp & (bpp - 1)) == 0);
    const size_t alignmentMask = 0x3 | (bpp - 1);
    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; currentMipLevel++) {
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
    if (0 == combinedBufferSize) {
        // We don't actually have any data to upload so just return success
        return true;
    }

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
    currentWidth = width;
    currentHeight = height;
    MTLOrigin origin = MTLOriginMake(0, 0, 0);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        const size_t rowBytes = currentWidth * bpp;

        [blitCmdEncoder copyFromBuffer: transferBuffer
                          sourceOffset: individualMipOffsets[currentMipLevel]
                     sourceBytesPerRow: rowBytes
                   sourceBytesPerImage: rowBytes*currentHeight
                            sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                             toTexture: mtlTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];
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

sk_sp<GrTexture> GrMtlGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                           const GrMipLevel texels[], int mipLevelCount) {
    int mipLevels = !mipLevelCount ? 1 : mipLevelCount;

    if (!fMtlCaps->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        return nullptr; // TODO: add compressed texture support
    }

    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    sk_sp<GrMtlTexture> tex;
    // This TexDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this TexDesc describes the resolved texture. Therefore we always have samples
    // set to 1.
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = MTLTextureType2D;
    texDesc.pixelFormat = format;
    texDesc.width = desc.fWidth;
    texDesc.height = desc.fHeight;
    texDesc.depth = 1;
    texDesc.mipmapLevelCount = mipLevels;
    texDesc.sampleCount = 1;
    texDesc.arrayLength = 1;
    texDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
    // Make all textures have private gpu only access. We can use transfer buffers or textures
    // to copy to them.
    texDesc.storageMode = MTLStorageModePrivate;
    texDesc.usage = MTLTextureUsageShaderRead;
    texDesc.usage |= renderTarget ? MTLTextureUsageRenderTarget : 0;

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

    if (renderTarget) {
        tex = GrMtlTextureRenderTarget::CreateNewTextureRenderTarget(this, budgeted,
                                                                     desc, texDesc, mipMapsStatus);
    } else {
        tex = GrMtlTexture::CreateNewTexture(this, budgeted, desc, texDesc, mipMapsStatus);
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

    if (desc.fFlags & kPerformInitialClear_GrSurfaceFlag) {
        this->clearTexture(tex.get(), colorType);
    }

    return std::move(tex);
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
                                     bool isRenderTarget, GrPixelConfig config) {
    if (isRenderTarget) {
        SkASSERT(MTLTextureUsageRenderTarget & mtlTexture.usage);
    }
    surfaceDesc->fFlags = isRenderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    surfaceDesc->fWidth = mtlTexture.width;
    surfaceDesc->fHeight = mtlTexture.height;
    surfaceDesc->fConfig = config;
    surfaceDesc->fSampleCnt = 1;
}

sk_sp<GrTexture> GrMtlGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                GrWrapOwnership,
                                                GrWrapCacheable cacheable, GrIOType ioType) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, false, backendTex.config());

    return GrMtlTexture::MakeWrappedTexture(this, surfDesc, mtlTexture, cacheable, ioType);
}

sk_sp<GrTexture> GrMtlGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                          int sampleCnt,
                                                          GrWrapOwnership,
                                                          GrWrapCacheable cacheable) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, true, backendTex.config());
    surfDesc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, surfDesc.fConfig);
    if (!surfDesc.fSampleCnt) {
        return nullptr;
    }

    return GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(this, surfDesc, mtlTexture,
                                                                    cacheable);
}

sk_sp<GrRenderTarget> GrMtlGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    // TODO: Revisit this when the Metal backend is completed. It may support MSAA render targets.
    if (backendRT.sampleCnt() > 1) {
        return nullptr;
    }
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendRT);
    if (!mtlTexture) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, true, backendRT.config());

    return GrMtlRenderTarget::MakeWrappedRenderTarget(this, surfDesc, mtlTexture);
}

sk_sp<GrRenderTarget> GrMtlGpu::onWrapBackendTextureAsRenderTarget(
        const GrBackendTexture& backendTex, int sampleCnt) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, true, backendTex.config());
    surfDesc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, surfDesc.fConfig);
    if (!surfDesc.fSampleCnt) {
        return nullptr;
    }

    return GrMtlRenderTarget::MakeWrappedRenderTarget(this, surfDesc, mtlTexture);
}

bool GrMtlGpu::onRegenerateMipMapLevels(GrTexture* texture) {
    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> mtlTexture = grMtlTexture->mtlTexture();

    // Automatic mipmap generation is only supported by color-renderable formats
    if (!fMtlCaps->isConfigRenderable(texture->config()) &&
        // We have pixel configs marked as textureable-only that use RGBA8 as the internal format
        MTLPixelFormatRGBA8Unorm != mtlTexture.pixelFormat) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder generateMipmapsForTexture: mtlTexture];

    return true;
}

bool GrMtlGpu::createTestingOnlyMtlTextureInfo(GrPixelConfig config, MTLPixelFormat format,
                                               int w, int h, bool texturable,
                                               bool renderable, GrMipMapped mipMapped,
                                               const void* srcData, size_t srcRowBytes,
                                               GrMtlTextureInfo* info) {
    SkASSERT(texturable || renderable);
    if (!texturable) {
        SkASSERT(GrMipMapped::kNo == mipMapped);
        SkASSERT(!srcData);
    }

    if (texturable && !fMtlCaps->isConfigTexturable(config)) {
        return false;
    }
    if (renderable && !fMtlCaps->isConfigRenderable(config)) {
        return false;
    }
    // Currently we don't support uploading pixel data when mipped.
    if (srcData && GrMipMapped::kYes == mipMapped) {
        return false;
    }
    if(!check_max_blit_width(w)) {
        return false;
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

    size_t bpp = GrBytesPerPixel(config);
    if (!srcRowBytes) {
        srcRowBytes = w * bpp;
#ifdef SK_BUILD_FOR_MAC
        if (!srcData) {
            // On MacOS, the fillBuffer command needs a range with a multiple of 4 bytes
            srcRowBytes = ((srcRowBytes + 3) & (~3));
        }
#endif
    }
    size_t bufferSize = srcRowBytes * h;
    NSUInteger options = 0;  // TODO: consider other options here
#ifdef SK_BUILD_FOR_MAC
    options |= MTLResourceStorageModeManaged;
#else
    options |= MTLResourceStorageModeShared;
#endif

    // TODO: Create GrMtlTransferBuffer
    id<MTLBuffer> transferBuffer;
    if (srcData) {
        transferBuffer = [fDevice newBufferWithBytes: srcData
                                              length: bufferSize
                                             options: options];
    } else {
        transferBuffer = [fDevice newBufferWithLength: bufferSize
                                              options: options];
    }
    if (nil == transferBuffer) {
        return false;
    }

    id<MTLCommandBuffer> cmdBuffer = [fQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitCmdEncoder = [cmdBuffer blitCommandEncoder];
    if (!srcData) {
        [blitCmdEncoder fillBuffer: transferBuffer
                             range: NSMakeRange(0, bufferSize)
                             value: 0];
    }
    [blitCmdEncoder copyFromBuffer: transferBuffer
                      sourceOffset: 0
                 sourceBytesPerRow: srcRowBytes
               sourceBytesPerImage: bufferSize
                        sourceSize: MTLSizeMake(w, h, 1)
                         toTexture: testTexture
                  destinationSlice: 0
                  destinationLevel: 0
                 destinationOrigin: MTLOriginMake(0, 0, 0)];
    [blitCmdEncoder endEncoding];
    [cmdBuffer commit];
    [cmdBuffer waitUntilCompleted];
    transferBuffer = nil;

    info->fTexture.reset(GrRetainPtrFromId(testTexture));

    return true;
}

static bool mtl_format_to_pixel_config(MTLPixelFormat format, GrPixelConfig* config) {
    GrPixelConfig dontCare;
    if (!config) {
        config = &dontCare;
    }

    switch (format) {
        case MTLPixelFormatInvalid:
            *config = kUnknown_GrPixelConfig;
            return false;
        case MTLPixelFormatRGBA8Unorm:
            *config = kRGBA_8888_GrPixelConfig;
            return true;
        case MTLPixelFormatRG8Unorm:
            *config = kRG_88_GrPixelConfig;
            return true;
        case MTLPixelFormatBGRA8Unorm:
            *config = kBGRA_8888_GrPixelConfig;
            return true;
        case MTLPixelFormatRGBA8Unorm_sRGB:
            *config = kSRGBA_8888_GrPixelConfig;
            return true;
        case MTLPixelFormatRGB10A2Unorm:
            *config = kRGBA_1010102_GrPixelConfig;
            return true;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:
            *config = kRGB_565_GrPixelConfig;
            return true;
        case MTLPixelFormatABGR4Unorm:
            *config = kRGBA_4444_GrPixelConfig;
            return true;
#endif
        case MTLPixelFormatR8Unorm:
            *config = kAlpha_8_GrPixelConfig;
            return true;
        case MTLPixelFormatRGBA32Float:
            *config = kRGBA_float_GrPixelConfig;
            return true;
        case MTLPixelFormatRG32Float:
            *config = kRG_float_GrPixelConfig;
            return true;
        case MTLPixelFormatRGBA16Float:
            *config = kRGBA_half_GrPixelConfig;
            return true;
        case MTLPixelFormatR16Float:
            *config = kAlpha_half_GrPixelConfig;
            return true;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:
            *config = kRGB_ETC1_GrPixelConfig;
            return true;
#endif
        case MTLPixelFormatR16Unorm:
            *config = kR_16_GrPixelConfig;
            return true;
        case MTLPixelFormatRG16Unorm:
            *config = kRG_1616_GrPixelConfig;
            return true;

        // Experimental (for Y416 and mutant P016/P010)
        case MTLPixelFormatRGBA16Unorm:
            *config = kRGBA_16161616_GrPixelConfig;
            return true;
        case MTLPixelFormatRG16Float:
            *config = kRG_half_GrPixelConfig;
            return true;
        default:
            return false;
    }

    SK_ABORT("Unexpected config");
    return false;
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

    const GrMTLPixelFormat* mtlFormat = format.getMtlFormat();
    if (!mtlFormat) {
        return GrBackendTexture();
    }

    GrPixelConfig config;

    if (!mtl_format_to_pixel_config(static_cast<MTLPixelFormat>(*mtlFormat), &config)) {
        return GrBackendTexture();
    }

    GrMtlTextureInfo info;
    if (!this->createTestingOnlyMtlTextureInfo(config, static_cast<MTLPixelFormat>(*mtlFormat),
                                               w, h, true,
                                               GrRenderable::kYes == renderable, mipMapped,
                                               pixels, rowBytes, &info)) {
        return {};
    }

    GrBackendTexture backendTex(w, h, mipMapped, info);
    backendTex.fConfig = config;
    return backendTex;
}

void GrMtlGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kMetal == tex.fBackend);
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
    if (!this->createTestingOnlyMtlTextureInfo(config, format, w, h, false, true,
                                               GrMipMapped::kNo, nullptr, 0, &info)) {
        return {};
    }

    GrBackendRenderTarget backendRT(w, h, 1, info);
    backendRT.fConfig = config;
    return backendRT;
}

void GrMtlGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kMetal == rt.fBackend);

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

bool GrMtlGpu::copySurfaceAsBlit(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                 const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
    SkASSERT(this->mtlCaps().canCopyAsBlit(dst->config(), dstSampleCnt, src->config(), srcSampleCnt,
                                           srcRect, dstPoint, dst == src));
#endif
    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst, false);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src, false);

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

    return true;
}

bool GrMtlGpu::onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                             const SkIPoint& dstPoint, bool canDiscardOutsideDstRect) {

    GrPixelConfig dstConfig = dst->config();
    GrPixelConfig srcConfig = src->config();

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);

    if (dstSampleCnt > 1 || srcSampleCnt > 1) {
        SkASSERT(false); // Currently dont support MSAA. TODO: add copySurfaceAsResolve().
        return false;
    }

    bool success = false;
    if (this->mtlCaps().canCopyAsBlit(dstConfig, dstSampleCnt, srcConfig, srcSampleCnt, srcRect,
                                      dstPoint, dst == src)) {
        success = this->copySurfaceAsBlit(dst, src, srcRect, dstPoint);
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
                             GrColorType srcColorType, const GrMipLevel texels[],
                             int mipLevelCount) {
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
                            GrColorType dstColorType, void* buffer, size_t rowBytes) {
    SkASSERT(surface);
    if (!check_max_blit_width(width)) {
        return false;
    }
    if (GrPixelConfigToColorType(surface->config()) != dstColorType) {
        return false;
    }

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    size_t transBufferRowBytes = bpp * width;
    bool doResolve = get_surface_sample_cnt(surface) > 1;
    id<MTLTexture> mtlTexture = GrGetMTLTextureFromSurface(surface, doResolve);
    if (!mtlTexture || [mtlTexture isFramebufferOnly]) {
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

