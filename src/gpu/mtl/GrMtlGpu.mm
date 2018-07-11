/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlGpu.h"

#include "GrMtlTexture.h"
#include "GrMtlTextureRenderTarget.h"
#include "GrMtlUtil.h"
#include "GrTexturePriv.h"
#include "SkConvertPixels.h"

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
        , fQueue(queue) {

    fMtlCaps.reset(new GrMtlCaps(options, fDevice, featureSet));
    fCaps = fMtlCaps;

    fCmdBuffer = [fQueue commandBuffer];

    MTLTextureDescriptor* txDesc = [[MTLTextureDescriptor alloc] init];
    txDesc.textureType = MTLTextureType3D;
    txDesc.height = 64;
    txDesc.width = 64;
    txDesc.depth = 64;
    txDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    txDesc.arrayLength = 1;
    txDesc.mipmapLevelCount = 1;
    id<MTLTexture> testTexture = [fDevice newTextureWithDescriptor:txDesc];
    // To get ride of unused var warning
    int width = [testTexture width];
    SkDebugf("width: %d\n", width);
    // Unused queue warning fix
    SkDebugf("ptr to queue: %p\n", fQueue);
}

void GrMtlGpu::submitCommandBuffer(SyncQueue sync) {
    SkASSERT(fCmdBuffer);
    [fCmdBuffer commit];
    if (SyncQueue::kForce_SyncQueue == sync) {
        [fCmdBuffer waitUntilCompleted];
    }
    fCmdBuffer = [fQueue commandBuffer];
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

    MTLTextureDescriptor* transferDesc = GrGetMTLTextureDescriptor(mtlTexture);
    transferDesc.mipmapLevelCount = mipLevelCount;
    transferDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
    transferDesc.storageMode = MTLStorageModeManaged;
    // TODO: implement some way of reusing transfer textures
    id<MTLTexture> transferTexture = [fDevice newTextureWithDescriptor:transferDesc];
    SkASSERT(transferTexture);

    int currentWidth = width;
    int currentHeight = height;
    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);
    MTLOrigin origin = MTLOriginMake(left, top, 0);

    SkASSERT(mtlTexture.pixelFormat == transferTexture.pixelFormat);
    SkASSERT(mtlTexture.sampleCount == transferTexture.sampleCount);

    id<MTLBlitCommandEncoder> blitCmdEncoder = [fCmdBuffer blitCommandEncoder];
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        size_t rowBytes = texels[currentMipLevel].fRowBytes ? texels[currentMipLevel].fRowBytes
                                                            : bpp * currentWidth;
        SkASSERT(texels[currentMipLevel].fPixels);
        if (rowBytes < bpp * currentWidth || rowBytes % bpp) {
            return false;
        }
        [transferTexture replaceRegion: MTLRegionMake2D(left, top, width, height)
                           mipmapLevel: currentMipLevel
                             withBytes: texels[currentMipLevel].fPixels
                           bytesPerRow: rowBytes];

        [blitCmdEncoder copyFromTexture: transferTexture
                            sourceSlice: 0
                            sourceLevel: currentMipLevel
                           sourceOrigin: origin
                             sourceSize: MTLSizeMake(width, height, 1)
                              toTexture: mtlTexture
                       destinationSlice: 0
                       destinationLevel: currentMipLevel
                      destinationOrigin: origin];
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
    }
    [blitCmdEncoder endEncoding];

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->texturePriv().markMipMapsDirty();
    }
    return true;
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

    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    // This TexDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this TexDesc describes the resolved texture. Therefore we always have samples set
    // to 1.
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
        mipMapsStatus = texels[0].fPixels ? GrMipMapsStatus::kValid : GrMipMapsStatus::kDirty;
#ifdef SK_DEBUG
        for (int i = 1; i < mipLevels; ++i) {
            if (mipMapsStatus == GrMipMapsStatus::kValid) {
                SkASSERT(texels[i].fPixels);
            } else {
                SkASSERT(!texels[i].fPixels);
            }
        }
#endif
    }
    sk_sp<GrMtlTexture> tex;
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
        // Do initial clear of the texture
    }
    return std::move(tex);
}

static id<MTLTexture> get_texture_from_backend(const GrBackendTexture& backendTex,
                                               GrWrapOwnership ownership) {
    GrMtlTextureInfo textureInfo;
    if (!backendTex.getMtlTextureInfo(&textureInfo)) {
        return nil;
    }
    return GrGetMTLTexture(textureInfo.fTexture, ownership);
}

static id<MTLTexture> get_texture_from_backend(const GrBackendRenderTarget& backendRT) {
    GrMtlTextureInfo textureInfo;
    if (!backendRT.getMtlTextureInfo(&textureInfo)) {
        return nil;
    }
    return GrGetMTLTexture(textureInfo.fTexture, GrWrapOwnership::kBorrow_GrWrapOwnership);
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
                                                GrWrapOwnership ownership) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex, ownership);
    if (!mtlTexture) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, false, backendTex.config());

    return GrMtlTexture::MakeWrappedTexture(this, surfDesc, mtlTexture);
}

sk_sp<GrTexture> GrMtlGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                          int sampleCnt,
                                                          GrWrapOwnership ownership) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex, ownership);
    if (!mtlTexture) {
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    init_surface_desc(&surfDesc, mtlTexture, true, backendTex.config());
    surfDesc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, surfDesc.fConfig);
    if (!surfDesc.fSampleCnt) {
        return nullptr;
    }

    return GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(this, surfDesc, mtlTexture);
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
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex,
                                                         GrWrapOwnership::kBorrow_GrWrapOwnership);
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

bool GrMtlGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType dstColorType, void* buffer, size_t rowBytes) {
    SkASSERT(surface);
    if (!check_max_blit_width(width)) {
        return false;
    }
    if (GrPixelConfigToColorType(surface->config()) != dstColorType) {
        return false;
    }

    id<MTLTexture> mtlTexture = nil;
    GrMtlRenderTarget* renderTarget = static_cast<GrMtlRenderTarget*>(surface->asRenderTarget());
    GrMtlTexture* texture;
    if (renderTarget) {
        // TODO: Handle resolving rt here when MSAA is supported. Right now we just grab the render
        // texture since we cannot currently have a multi-sampled rt.
        mtlTexture = renderTarget->mtlRenderTexture();
    } else {
        texture = static_cast<GrMtlTexture*>(surface->asTexture());
        if (texture) {
            mtlTexture = texture->mtlTexture();
        }
    }

    if (!mtlTexture) {
        return false;
    }

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    size_t transBufferRowBytes = bpp * width;
    size_t transBufferImageBytes = transBufferRowBytes * height;

    // TODO: implement some way of reusing buffers instead of making a new one every time.
    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: transBufferImageBytes
                                                        options: MTLResourceStorageModeShared];

    id<MTLBlitCommandEncoder> blitCmdEncoder = [fCmdBuffer blitCommandEncoder];
    [blitCmdEncoder copyFromTexture: mtlTexture
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(left, top, 0)
                         sourceSize: MTLSizeMake(width, height, 1)
                           toBuffer: transferBuffer
                  destinationOffset: 0
             destinationBytesPerRow: transBufferRowBytes
           destinationBytesPerImage: transBufferImageBytes];
    [blitCmdEncoder endEncoding];

    this->submitCommandBuffer(kForce_SyncQueue);
    const void* mappedMemory = transferBuffer.contents;

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, transBufferRowBytes, height);
    return true;
}
