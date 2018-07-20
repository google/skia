/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlGpu.h"

#include "GrMtlGpuCommandBuffer.h"
#include "GrMtlTexture.h"
#include "GrMtlTextureRenderTarget.h"
#include "GrMtlUtil.h"
#include "GrTexturePriv.h"
#include "SkConvertPixels.h"
#include "SkSLCompiler.h"

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
        , fQueue(queue) {

    fMtlCaps.reset(new GrMtlCaps(options, fDevice, featureSet));
    fCaps = fMtlCaps;

    fCmdBuffer = [fQueue commandBuffer];
    fCompiler.reset(new SkSL::Compiler());
}

GrGpuRTCommandBuffer* GrMtlGpu::createCommandBuffer(
            GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    return new GrMtlGpuRTCommandBuffer(this, renderTarget, origin, colorInfo, stencilInfo);
}

GrGpuTextureCommandBuffer* GrMtlGpu::createCommandBuffer(GrTexture* texture,
                                                         GrSurfaceOrigin origin) {
    return new GrMtlGpuTextureCommandBuffer(this, texture, origin);
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

#ifdef GR_TEST_UTILS
bool GrMtlGpu::createTestingOnlyMtlTextureInfo(GrPixelConfig config, int w, int h, bool texturable,
                                                bool renderable, GrMipMapped mipMapped,
                                                const void* srcData, GrMtlTextureInfo* info) {
    SkASSERT(texturable || renderable);
    if (!texturable) {
        SkASSERT(GrMipMapped::kNo == mipMapped);
        SkASSERT(!srcData);
    }

    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return false;
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

    SkAutoTMalloc<GrColor> srcBuffer;
    if (!srcData) {
        srcBuffer.reset(w * h);
        memset(srcBuffer, 0, w * h * sizeof(GrColor));
        srcData = srcBuffer;
    }
    SkASSERT(srcData);

    desc.storageMode = MTLStorageModeManaged;
    id<MTLTexture> transferTexture = [fDevice newTextureWithDescriptor: desc];
    auto colorType = GrPixelConfigToColorType(config);
    int rowBytes = w * GrColorTypeBytesPerPixel(colorType);
    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    SkASSERT(testTexture.pixelFormat == transferTexture.pixelFormat);
    SkASSERT(testTexture.sampleCount == transferTexture.sampleCount);

    id<MTLCommandBuffer> cmdBuffer = [fQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitCmdEncoder = [cmdBuffer blitCommandEncoder];
    int currentWidth = w;
    int currentHeight = h;
    for (int mipLevel = 0; mipLevel < (int)testTexture.mipmapLevelCount; mipLevel++) {
        [transferTexture replaceRegion: MTLRegionMake2D(0, 0, currentWidth, currentHeight)
                           mipmapLevel: mipLevel
                             withBytes: srcData
                           bytesPerRow: rowBytes];

        [blitCmdEncoder copyFromTexture: transferTexture
                            sourceSlice: 0
                            sourceLevel: mipLevel
                           sourceOrigin: origin
                             sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                              toTexture: testTexture
                       destinationSlice: 0
                       destinationLevel: mipLevel
                      destinationOrigin: origin];
        currentWidth = SkTMax(1, currentWidth/2);
        currentHeight = SkTMax(1, currentHeight/2);
    }
    [blitCmdEncoder endEncoding];
    [cmdBuffer commit];
    [cmdBuffer waitUntilCompleted];

    info->fTexture = GrReleaseId(testTexture);
    return true;
}

GrBackendTexture GrMtlGpu::createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                           GrPixelConfig config, bool isRT,
                                                           GrMipMapped mipMapped) {
    if (w > this->caps()->maxTextureSize() || h > this->caps()->maxTextureSize()) {
        return GrBackendTexture();
    }
    GrMtlTextureInfo info;
    if (!this->createTestingOnlyMtlTextureInfo(config, w, h, true, isRT, mipMapped, pixels,
                                               &info)) {
        return {};
    }

    GrBackendTexture backendTex(w, h, mipMapped, info);
    backendTex.fConfig = config;
    return backendTex;
}

bool GrMtlGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(kMetal_GrBackend == tex.backend());

    GrMtlTextureInfo info;
    if (!tex.getMtlTextureInfo(&info)) {
        return false;
    }
    id<MTLTexture> mtlTexture = GrGetMTLTexture(info.fTexture,
                                                GrWrapOwnership::kBorrow_GrWrapOwnership);
    if (!mtlTexture) {
        return false;
    }
    return mtlTexture.usage & MTLTextureUsageShaderRead;
}

void GrMtlGpu::deleteTestingOnlyBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(kMetal_GrBackend == tex.fBackend);

    GrMtlTextureInfo info;
    if (tex.getMtlTextureInfo(&info)) {
        // Adopts the metal texture so that ARC will clean it up.
        GrGetMTLTexture(info.fTexture, GrWrapOwnership::kAdopt_GrWrapOwnership);
    }
}

GrBackendRenderTarget GrMtlGpu::createTestingOnlyBackendRenderTarget(int w, int h, GrColorType ct,
                                                                     GrSRGBEncoded srgbEncoded) {
    if (w > this->caps()->maxRenderTargetSize() || h > this->caps()->maxRenderTargetSize()) {
        return GrBackendRenderTarget();
    }
    auto config = GrColorTypeToPixelConfig(ct, srgbEncoded);
    if (kUnknown_GrPixelConfig == config) {
        return {};
    }
    GrMtlTextureInfo info;
    if (!this->createTestingOnlyMtlTextureInfo(config, w, h, false, true, GrMipMapped::kNo, nullptr,
                                               &info)) {
        return {};
    }

    GrBackendRenderTarget backendRT(w, h, 1, info);
    backendRT.fConfig = config;
    return backendRT;
}

void GrMtlGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(kMetal_GrBackend == rt.fBackend);

    GrMtlTextureInfo info;
    if (rt.getMtlTextureInfo(&info)) {
        this->testingOnly_flushGpuAndSync();
        // Adopts the metal texture so that ARC will clean it up.
        GrGetMTLTexture(info.fTexture, GrWrapOwnership::kAdopt_GrWrapOwnership);
    }
}

void GrMtlGpu::testingOnly_flushGpuAndSync() {
    this->submitCommandBuffer(kForce_SyncQueue);
}
#endif // GR_TEST_UTILS

id<MTLTexture> get_mtl_texture_from_surface(GrSurface* surface) {
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
    return mtlTexture;
}

static int get_surface_sample_cnt(GrSurface* surf) {
    if (const GrRenderTarget* rt = surf->asRenderTarget()) {
        return rt->numColorSamples();
    }
    return 0;
}

bool GrMtlGpu::copySurfaceAsBlit(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                 GrSurface* src, GrSurfaceOrigin srcOrigin,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
    SkASSERT(this->mtlCaps().canCopyAsBlit(dst->config(), dstSampleCnt, dstOrigin,
                                           src->config(), srcSampleCnt, srcOrigin,
                                           srcRect, dstPoint, dst == src));
#endif
    id<MTLTexture> dstTex = get_mtl_texture_from_surface(dst);
    id<MTLTexture> srcTex = get_mtl_texture_from_surface(src);

    // Flip rect if necessary
    SkIRect srcMtlRect;
    srcMtlRect.fLeft = srcRect.fLeft;
    srcMtlRect.fRight = srcRect.fRight;
    SkIRect dstRect;
    dstRect.fLeft = dstPoint.fX;
    dstRect.fRight = dstPoint.fX + srcRect.width();

    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        srcMtlRect.fTop = srcTex.height - srcRect.fBottom;
        srcMtlRect.fBottom = srcTex.height - srcRect.fTop;
    } else {
        srcMtlRect.fTop = srcRect.fTop;
        srcMtlRect.fBottom = srcRect.fBottom;
    }

    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
        dstRect.fTop = dstTex.height - dstPoint.fY - srcMtlRect.height();
    } else {
        dstRect.fTop = dstPoint.fY;
    }
    dstRect.fBottom = dstRect.fTop + srcMtlRect.height();

    id<MTLBlitCommandEncoder> blitCmdEncoder = [fCmdBuffer blitCommandEncoder];
    [blitCmdEncoder copyFromTexture: srcTex
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(srcMtlRect.x(), srcMtlRect.y(), 0)
                         sourceSize: MTLSizeMake(srcMtlRect.width(), srcMtlRect.height(), 1)
                          toTexture: dstTex
                   destinationSlice: 0
                   destinationLevel: 0
                  destinationOrigin: MTLOriginMake(dstRect.x(), dstRect.y(), 0)];
    [blitCmdEncoder endEncoding];

    return true;
}

bool GrMtlGpu::copySurfaceAsDraw(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                 GrSurface* src, GrSurfaceOrigin srcOrigin,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint,
                                 bool canDiscardOutsideDstRect) {
    SkASSERT(this->mtlCaps().canCopyAsDraw(dst->config(), SkToBool(dst->asRenderTarget()),
                                           src->config(), SkToBool(src->asTexture())));

    id<MTLTexture> dstTex = get_mtl_texture_from_surface(dst);
    id<MTLTexture> srcTex = get_mtl_texture_from_surface(src);

    if (fCopyProgramPipelineState == nil) {
        SkASSERT(fCopyProgramSamplerState == nil);
        SkASSERT(fCopyProgramVertexAttributeBuffer == nil);
        this->createCopyProgram(dstTex);
    }

    // UPDATE UNIFORM DESCRIPTOR SET
    int w = srcRect.width();
    int h = srcRect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dstTex.width;
    int dh = dstTex.height;
    float dx0 = 2.f * dstPoint.fX / dw - 1.f;
    float dx1 = 2.f * (dstPoint.fX + w) / dw - 1.f;
    float dy0 = 2.f * dstPoint.fY / dh - 1.f;
    float dy1 = 2.f * (dstPoint.fY + h) / dh - 1.f;
    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
        dy0 = -dy0;
        dy1 = -dy1;
    }

    float sx0 = (float)srcRect.fLeft;
    float sx1 = (float)(srcRect.fLeft + w);
    float sy0 = (float)srcRect.fTop;
    float sy1 = (float)(srcRect.fTop + h);
    int sh = srcTex.height;
    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }
    // src rect edges in normalized texture space (0 to 1).
    int sw = srcTex.width;
    sx0 /= sw;
    sx1 /= sw;
    sy0 /= sh;
    sy1 /= sh;

    const simd::float4 vertexUniformBuffer[2] = {
        {dx1 - dx0, dy1 - dy0, dx0, dy0}, // posXform
        {sx1 - sx0, sy1 - sy0, sx0, sy0}, // texCoordXform
    };

    MTLRenderPassDescriptor* renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    renderPassDesc.colorAttachments[0].texture = dstTex;
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    renderPassDesc.colorAttachments[0].loadAction = canDiscardOutsideDstRect ? MTLLoadActionDontCare
                                                                             : MTLLoadActionLoad;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> renderCmdEncoder =
            [fCmdBuffer renderCommandEncoderWithDescriptor: renderPassDesc];
    [renderCmdEncoder setRenderPipelineState: fCopyProgramPipelineState];
    [renderCmdEncoder setVertexBuffer: fCopyProgramVertexAttributeBuffer
                               offset: 0
                              atIndex: 0];
    [renderCmdEncoder setVertexBytes: vertexUniformBuffer
                              length: sizeof(vertexUniformBuffer)
                             atIndex: 1];
    [renderCmdEncoder setFragmentTexture: srcTex
                                 atIndex: 0];
    [renderCmdEncoder setFragmentSamplerState: fCopyProgramSamplerState
                                      atIndex: 0];
    [renderCmdEncoder drawPrimitives: MTLPrimitiveTypeTriangleStrip
                         vertexStart: 0
                         vertexCount: 4];
    [renderCmdEncoder endEncoding];
    return true;
}

bool GrMtlGpu::copySurfaceAsDrawThenBlit(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                         GrSurface* src, GrSurfaceOrigin srcOrigin,
                                         const SkIRect& srcRect, const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    // Should be the only case which requires a draw then blit.
    SkASSERT(dstOrigin != srcOrigin && SkToBool(dst->asRenderTarget()) == false);

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
    SkASSERT(!this->mtlCaps().canCopyAsBlit(dst->config(), dstSampleCnt, dstOrigin,
                                            src->config(), srcSampleCnt, srcOrigin,
                                            srcRect, dstPoint, dst == src));
    SkASSERT(!this->mtlCaps().canCopyAsDraw(dst->config(), SkToBool(dst->asRenderTarget()),
                                            src->config(), SkToBool(src->asTexture())));
    SkASSERT(this->mtlCaps().canCopyAsDrawThenBlit(dst->config(),src->config(),
                                                   SkToBool(src->asTexture())));
#endif
    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    surfDesc.fWidth = dst->width();
    surfDesc.fHeight = dst->height();
    surfDesc.fConfig = dst->config();
    surfDesc.fSampleCnt = 1;

    id<MTLTexture> dstTex = get_mtl_texture_from_surface(dst);
    MTLTextureDescriptor* textureDesc = GrGetMTLTextureDescriptor(dstTex);
    textureDesc.sampleCount = 1;
    textureDesc.mipmapLevelCount = 1;
    textureDesc.usage |= MTLTextureUsageRenderTarget;

    sk_sp<GrMtlTexture> transferTexture =
            GrMtlTextureRenderTarget::CreateNewTextureRenderTarget(this,
                                                                   SkBudgeted::kNo,
                                                                   surfDesc,
                                                                   textureDesc,
                                                                   GrMipMapsStatus::kNotAllocated);

    GrSurfaceOrigin transferOrigin = dstOrigin;
    SkASSERT(this->mtlCaps().canCopyAsDraw(transferTexture->config(),
                                           SkToBool(transferTexture->asRenderTarget()),
                                           src->config(),
                                           SkToBool(src->asTexture())));
    if (!this->copySurface(transferTexture.get(), transferOrigin,
                           src, srcOrigin,
                           srcRect, dstPoint)) {
        return false;
    }

    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    SkASSERT(this->mtlCaps().canCopyAsBlit(dst->config(),
                                           dstSampleCnt,
                                           dstOrigin,
                                           transferTexture->config(),
                                           get_surface_sample_cnt(transferTexture.get()),
                                           transferOrigin,
                                           dstRect, dstPoint, false));
    if (!this->copySurface(dst, dstOrigin,
                           transferTexture.get(), transferOrigin,
                           dstRect, dstPoint)) {
        return false;
    }
    return true;
}

bool GrMtlGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                             GrSurface* src, GrSurfaceOrigin srcOrigin,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint,
                             bool canDiscardOutsideDstRect) {

    GrPixelConfig dstConfig = dst->config();
    GrPixelConfig srcConfig = src->config();

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);

    if (dstSampleCnt > 1 || srcSampleCnt > 1) {
        SkASSERT(false); // Currently dont support MSAA
        return false;
    }

    bool success = false;
    if (this->mtlCaps().canCopyAsBlit(dstConfig, dstSampleCnt, dstOrigin,
                                      srcConfig, srcSampleCnt, srcOrigin,
                                      srcRect, dstPoint, dst == src)) {
        success = this->copySurfaceAsBlit(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint);
    } else if (this->mtlCaps().canCopyAsDraw(dst->config(), SkToBool(dst->asRenderTarget()),
                                             src->config(), SkToBool(src->asTexture()))) {
        success = this->copySurfaceAsDraw(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint,
                                          canDiscardOutsideDstRect);
    } else if (this->mtlCaps().canCopyAsDrawThenBlit(dst->config(), src->config(),
                                                     SkToBool(src->asTexture()))) {
        success = this->copySurfaceAsDrawThenBlit(dst, dstOrigin, src, srcOrigin,
                                                  srcRect, dstPoint);
    }
    if (success) {
        SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.x(), dstPoint.y(),
                                            srcRect.width(), srcRect.height());
        this->didWriteToSurface(dst, dstOrigin, &dstRect);
    }
    return success;
}

void GrMtlGpu::initCopyProgramBuffer() {
    // Create and upload  per vertex attribute data
    static const simd::float2 vdata[4] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
    };
    fCopyProgramVertexAttributeBuffer =
            [fDevice newBufferWithLength: sizeof(vdata)
                                 options: MTLResourceStorageModePrivate];
    id<MTLBuffer> transferBuffer = [fDevice newBufferWithBytes: vdata
                                                        length: sizeof(vdata)
                                                       options: MTLResourceStorageModeManaged];

    id<MTLBlitCommandEncoder> blitCmdEncoder = [fCmdBuffer blitCommandEncoder];
    [blitCmdEncoder copyFromBuffer: transferBuffer
                      sourceOffset: 0
                          toBuffer: fCopyProgramVertexAttributeBuffer
                 destinationOffset: 0
                              size: sizeof(vdata)];
    [blitCmdEncoder endEncoding];
}

void GrMtlGpu::initCopyProgramSampler() {
     // Create sampler for copy as draw
    MTLSamplerDescriptor* samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
    samplerDescriptor.rAddressMode = MTLSamplerAddressModeClampToZero;
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToZero;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToZero;
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.mipFilter = MTLSamplerMipFilterNotMipmapped;
    samplerDescriptor.lodMinClamp = 0;
    samplerDescriptor.lodMaxClamp = 0;
    samplerDescriptor.normalizedCoordinates = true;

    fCopyProgramSamplerState = [fDevice newSamplerStateWithDescriptor: samplerDescriptor];
}

bool GrMtlGpu::createCopyProgram(id<MTLTexture> dstTex) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    fCopyProgramPipelineState = nil;
    for (const auto& pipelineState: fPipelineStateCache) {
        if (pipelineState.fPixelFormat == dstTex.pixelFormat) {
            fCopyProgramPipelineState = pipelineState.fPipelineState;
        }
    }

    if (fCopyProgramPipelineState == nil) {
        // Create shader code for copy as draw
        const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
        const char* version = shaderCaps->versionDeclString();
        SkString vertShaderText(version);
        vertShaderText.append(
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"

            "layout(set = 1, binding = 0) uniform vertexUniformBuffer {"
                "float4 uPosXform;"
                "float4 uTexCoordXform;"
            "};"
            "layout(location = 0) in float2 inPosition;"
            "layout(location = 1) out float2 vTexCoord;"

            "// Copy Program VS\n"
            "void main() {"
                "vTexCoord = inPosition * uTexCoordXform.xy + uTexCoordXform.zw;"
                "sk_Position.xy = inPosition * uPosXform.xy + uPosXform.zw;"
                "sk_Position.zw = float2(0, 1);"
            "}"
        );

        SkString fragShaderText(version);
        fragShaderText.append(
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"

            "layout(set = 1, binding = 0) uniform sampler2D uTexture;"
            "layout(location = 1) in float2 vTexCoord;"

            "// Copy Program FS\n"
            "void main() {"
                "sk_FragColor = texture(uTexture, vTexCoord);"
            "}"
        );

        SkSL::Program::Settings settings;
        SkSL::Program::Inputs inputs;
        id<MTLLibrary> vertexLibrary = GrCompileMtlShaderLibrary(this, vertShaderText.c_str(),
                                                                 SkSL::Program::kVertex_Kind,
                                                                 settings, &inputs);
        SkASSERT(inputs.isEmpty());
        SkASSERT(vertexLibrary);

        id<MTLLibrary> fragmentLibrary = GrCompileMtlShaderLibrary(this, fragShaderText.c_str(),
                                                                   SkSL::Program::kFragment_Kind,
                                                                   settings, &inputs);
        SkASSERT(inputs.isEmpty());
        SkASSERT(fragmentLibrary);

        id<MTLFunction> vertexFunction = [vertexLibrary newFunctionWithName: @"vertexMain"];
        id<MTLFunction> fragmentFunction = [fragmentLibrary newFunctionWithName: @"fragmentMain"];
        SkASSERT(vertexFunction);
        SkASSERT(fragmentFunction);

        // Create vertex descriptor for pipeline
        // Expected [[stage_in]] (vertex attribute) format:
        // struct Input {
        //     float2 inPosition [[attribute(0)]];
        // };
        MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;

        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[0].stepRate = 1;
        vertexDescriptor.layouts[0].stride = sizeof(simd::float2);

        // Create pipeline state for copy as draw
        MTLRenderPipelineDescriptor* pipelineDescriptor =
                [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
        pipelineDescriptor.colorAttachments[0].pixelFormat = dstTex.pixelFormat;

        NSError* error = nil;
        fCopyProgramPipelineState =
                [fDevice newRenderPipelineStateWithDescriptor: pipelineDescriptor error: &error];
        if (error) {
            SkDebugf("Error creating pipeline: %s\n",
                      [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            return false;
        }
    }
    fPipelineStateCache.emplace_back(MtlPipelineStateInfo{fCopyProgramPipelineState,
                                                          dstTex.pixelFormat});
    if (fCopyProgramSamplerState == nil) {
        this->initCopyProgramSampler();
    }

    if (fCopyProgramVertexAttributeBuffer == nil) {
        this->initCopyProgramBuffer();
    }

    SkASSERT(fCopyProgramPipelineState);
    SkASSERT(fCopyProgramSamplerState);
    SkASSERT(fCopyProgramVertexAttributeBuffer);
    return true;
}

bool GrMtlGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType srcColorType, const GrMipLevel texels[],
                             int mipLevelCount) {
    GrMtlTexture* mtlTexture = static_cast<GrMtlTexture*>(surface->asTexture());
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

    id<MTLTexture> mtlTexture = get_mtl_texture_from_surface(surface);
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
