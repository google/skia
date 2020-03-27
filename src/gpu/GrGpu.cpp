/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrGpu.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrNativeRect.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrTracing.h"
#include "src/utils/SkJSONWriter.h"

////////////////////////////////////////////////////////////////////////////////

GrGpu::GrGpu(GrContext* context) : fResetBits(kAll_GrBackendState), fContext(context) {}

GrGpu::~GrGpu() {}

void GrGpu::disconnect(DisconnectType) {}

////////////////////////////////////////////////////////////////////////////////

bool GrGpu::IsACopyNeededForMips(const GrCaps* caps, const GrTextureProxy* texProxy,
                                 GrSamplerState::Filter filter) {
    SkASSERT(texProxy);
    if (filter != GrSamplerState::Filter::kMipMap || texProxy->mipMapped() == GrMipMapped::kYes ||
        !caps->mipMapSupport()) {
        return false;
    }
    return SkMipMap::ComputeLevelCount(texProxy->width(), texProxy->height()) > 0;
}

static bool validate_texel_levels(SkISize dimensions, GrColorType texelColorType,
                                  const GrMipLevel* texels, int mipLevelCount, const GrCaps* caps) {
    SkASSERT(mipLevelCount > 0);
    bool hasBasePixels = texels[0].fPixels;
    int levelsWithPixelsCnt = 0;
    auto bpp = GrColorTypeBytesPerPixel(texelColorType);
    int w = dimensions.fWidth;
    int h = dimensions.fHeight;
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        if (texels[currentMipLevel].fPixels) {
            const size_t minRowBytes = w * bpp;
            if (caps->writePixelsRowBytesSupport()) {
                if (texels[currentMipLevel].fRowBytes < minRowBytes) {
                    return false;
                }
                if (texels[currentMipLevel].fRowBytes % bpp) {
                    return false;
                }
            } else {
                if (texels[currentMipLevel].fRowBytes != minRowBytes) {
                    return false;
                }
            }
            ++levelsWithPixelsCnt;
        }
        if (w == 1 && h == 1) {
            if (currentMipLevel != mipLevelCount - 1) {
                return false;
            }
        } else {
            w = std::max(w / 2, 1);
            h = std::max(h / 2, 1);
        }
    }
    // Either just a base layer or a full stack is required.
    if (mipLevelCount != 1 && (w != 1 || h != 1)) {
        return false;
    }
    // Can specify just the base, all levels, or no levels.
    if (!hasBasePixels) {
        return levelsWithPixelsCnt == 0;
    }
    return levelsWithPixelsCnt == 1 || levelsWithPixelsCnt == mipLevelCount;
}

sk_sp<GrTexture> GrGpu::createTextureCommon(SkISize dimensions,
                                            const GrBackendFormat& format,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            SkBudgeted budgeted,
                                            GrProtected isProtected,
                                            int mipLevelCount,
                                            uint32_t levelClearMask) {
    if (this->caps()->isFormatCompressed(format)) {
        // Call GrGpu::createCompressedTexture.
        return nullptr;
    }

    GrMipMapped mipMapped = mipLevelCount > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
    if (!this->caps()->validateSurfaceParams(dimensions, format, renderable, renderTargetSampleCnt,
                                             mipMapped)) {
        return nullptr;
    }

    if (renderable == GrRenderable::kYes) {
        renderTargetSampleCnt =
                this->caps()->getRenderTargetSampleCount(renderTargetSampleCnt, format);
    }
    // Attempt to catch un- or wrongly initialized sample counts.
    SkASSERT(renderTargetSampleCnt > 0 && renderTargetSampleCnt <= 64);
    this->handleDirtyContext();
    auto tex = this->onCreateTexture(dimensions,
                                     format,
                                     renderable,
                                     renderTargetSampleCnt,
                                     budgeted,
                                     isProtected,
                                     mipLevelCount,
                                     levelClearMask);
    if (tex) {
        SkASSERT(tex->backendFormat() == format);
        SkASSERT(GrRenderable::kNo == renderable || tex->asRenderTarget());
        if (!this->caps()->reuseScratchTextures() && renderable == GrRenderable::kNo) {
            tex->resourcePriv().removeScratchKey();
        }
        fStats.incTextureCreates();
        if (renderTargetSampleCnt > 1 && !this->caps()->msaaResolvesAutomatically()) {
            SkASSERT(GrRenderable::kYes == renderable);
            tex->asRenderTarget()->setRequiresManualMSAAResolve();
        }
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::createTexture(SkISize dimensions,
                                      const GrBackendFormat& format,
                                      GrRenderable renderable,
                                      int renderTargetSampleCnt,
                                      GrMipMapped mipMapped,
                                      SkBudgeted budgeted,
                                      GrProtected isProtected) {
    int mipLevelCount = 1;
    if (mipMapped == GrMipMapped::kYes) {
        mipLevelCount =
                32 - SkCLZ(static_cast<uint32_t>(std::max(dimensions.fWidth, dimensions.fHeight)));
    }
    uint32_t levelClearMask =
            this->caps()->shouldInitializeTextures() ? (1 << mipLevelCount) - 1 : 0;
    auto tex = this->createTextureCommon(dimensions, format, renderable, renderTargetSampleCnt,
                                         budgeted, isProtected, mipLevelCount, levelClearMask);
    if (tex && mipMapped == GrMipMapped::kYes && levelClearMask) {
        tex->texturePriv().markMipMapsClean();
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::createTexture(SkISize dimensions,
                                      const GrBackendFormat& format,
                                      GrRenderable renderable,
                                      int renderTargetSampleCnt,
                                      SkBudgeted budgeted,
                                      GrProtected isProtected,
                                      GrColorType textureColorType,
                                      GrColorType srcColorType,
                                      const GrMipLevel texels[],
                                      int texelLevelCount) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (texelLevelCount) {
        if (!validate_texel_levels(dimensions, srcColorType, texels, texelLevelCount,
                                   this->caps())) {
            return nullptr;
        }
    }

    int mipLevelCount = std::max(1, texelLevelCount);
    uint32_t levelClearMask = 0;
    if (this->caps()->shouldInitializeTextures()) {
        if (texelLevelCount) {
            for (int i = 0; i < mipLevelCount; ++i) {
                if (!texels->fPixels) {
                    levelClearMask |= static_cast<uint32_t>(1 << i);
                }
            }
        } else {
            levelClearMask = static_cast<uint32_t>((1 << mipLevelCount) - 1);
        }
    }

    auto tex = this->createTextureCommon(dimensions, format, renderable, renderTargetSampleCnt,
                                         budgeted, isProtected, texelLevelCount, levelClearMask);
    if (tex) {
        bool markMipLevelsClean = false;
        // Currently if level 0 does not have pixels then no other level may, as enforced by
        // validate_texel_levels.
        if (texelLevelCount && texels[0].fPixels) {
            if (!this->writePixels(tex.get(), 0, 0, dimensions.fWidth, dimensions.fHeight,
                                   textureColorType, srcColorType, texels, texelLevelCount)) {
                return nullptr;
            }
            // Currently if level[1] of mip map has pixel data then so must all other levels.
            // as enforced by validate_texel_levels.
            markMipLevelsClean = (texelLevelCount > 1 && !levelClearMask && texels[1].fPixels);
            fStats.incTextureUploads();
        } else if (levelClearMask && mipLevelCount > 1) {
            markMipLevelsClean = true;
        }
        if (markMipLevelsClean) {
            tex->texturePriv().markMipMapsClean();
        }
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::createCompressedTexture(SkISize dimensions,
                                                const GrBackendFormat& format,
                                                SkBudgeted budgeted,
                                                GrMipMapped mipMapped,
                                                GrProtected isProtected,
                                                const void* data,
                                                size_t dataSize) {
    this->handleDirtyContext();
    if (dimensions.width()  < 1 || dimensions.width()  > this->caps()->maxTextureSize() ||
        dimensions.height() < 1 || dimensions.height() > this->caps()->maxTextureSize()) {
        return nullptr;
    }
    // Note if we relax the requirement that data must be provided then we must check
    // caps()->shouldInitializeTextures() here.
    if (!data) {
        return nullptr;
    }
    if (!this->caps()->isFormatTexturable(format)) {
        return nullptr;
    }

    // TODO: expand CompressedDataIsCorrect to work here too
    SkImage::CompressionType compressionType = this->caps()->compressionType(format);

    if (dataSize < SkCompressedDataSize(compressionType, dimensions, nullptr,
                                        mipMapped == GrMipMapped::kYes)) {
        return nullptr;
    }
    return this->onCreateCompressedTexture(dimensions, format, budgeted, mipMapped, isProtected,
                                           data, dataSize);
}

sk_sp<GrTexture> GrGpu::wrapBackendTexture(const GrBackendTexture& backendTex,
                                           GrWrapOwnership ownership,
                                           GrWrapCacheable cacheable,
                                           GrIOType ioType) {
    SkASSERT(ioType != kWrite_GrIOType);
    this->handleDirtyContext();

    const GrCaps* caps = this->caps();
    SkASSERT(caps);

    if (!caps->isFormatTexturable(backendTex.getBackendFormat())) {
        return nullptr;
    }
    if (backendTex.width() > caps->maxTextureSize() ||
        backendTex.height() > caps->maxTextureSize()) {
        return nullptr;
    }

    return this->onWrapBackendTexture(backendTex, ownership, cacheable, ioType);
}

sk_sp<GrTexture> GrGpu::wrapCompressedBackendTexture(const GrBackendTexture& backendTex,
                                                     GrWrapOwnership ownership,
                                                     GrWrapCacheable cacheable) {
    this->handleDirtyContext();

    const GrCaps* caps = this->caps();
    SkASSERT(caps);

    if (!caps->isFormatTexturable(backendTex.getBackendFormat())) {
        return nullptr;
    }
    if (backendTex.width() > caps->maxTextureSize() ||
        backendTex.height() > caps->maxTextureSize()) {
        return nullptr;
    }

    return this->onWrapCompressedBackendTexture(backendTex, ownership, cacheable);
}

sk_sp<GrTexture> GrGpu::wrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                     int sampleCnt,
                                                     GrWrapOwnership ownership,
                                                     GrWrapCacheable cacheable) {
    this->handleDirtyContext();
    if (sampleCnt < 1) {
        return nullptr;
    }

    const GrCaps* caps = this->caps();

    if (!caps->isFormatTexturable(backendTex.getBackendFormat()) ||
        !caps->isFormatRenderable(backendTex.getBackendFormat(), sampleCnt)) {
        return nullptr;
    }

    if (backendTex.width() > caps->maxRenderTargetSize() ||
        backendTex.height() > caps->maxRenderTargetSize()) {
        return nullptr;
    }
    sk_sp<GrTexture> tex =
            this->onWrapRenderableBackendTexture(backendTex, sampleCnt, ownership, cacheable);
    SkASSERT(!tex || tex->asRenderTarget());
    if (tex && sampleCnt > 1 && !caps->msaaResolvesAutomatically()) {
        tex->asRenderTarget()->setRequiresManualMSAAResolve();
    }
    return tex;
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    this->handleDirtyContext();

    const GrCaps* caps = this->caps();

    if (!caps->isFormatRenderable(backendRT.getBackendFormat(), backendRT.sampleCnt())) {
        return nullptr;
    }

    sk_sp<GrRenderTarget> rt = this->onWrapBackendRenderTarget(backendRT);
    if (backendRT.isFramebufferOnly()) {
        rt->setFramebufferOnly();
    }
    return rt;
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendTextureAsRenderTarget(const GrBackendTexture& backendTex,
                                                              int sampleCnt) {
    this->handleDirtyContext();

    const GrCaps* caps = this->caps();

    int maxSize = caps->maxTextureSize();
    if (backendTex.width() > maxSize || backendTex.height() > maxSize) {
        return nullptr;
    }

    if (!caps->isFormatRenderable(backendTex.getBackendFormat(), sampleCnt)) {
        return nullptr;
    }

    auto rt = this->onWrapBackendTextureAsRenderTarget(backendTex, sampleCnt);
    if (rt && sampleCnt > 1 && !this->caps()->msaaResolvesAutomatically()) {
        rt->setRequiresManualMSAAResolve();
    }
    return rt;
}

sk_sp<GrRenderTarget> GrGpu::wrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo& imageInfo,
                                                                 const GrVkDrawableInfo& vkInfo) {
    return this->onWrapVulkanSecondaryCBAsRenderTarget(imageInfo, vkInfo);
}

sk_sp<GrRenderTarget> GrGpu::onWrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo& imageInfo,
                                                                   const GrVkDrawableInfo& vkInfo) {
    // This is only supported on Vulkan so we default to returning nullptr here
    return nullptr;
}

sk_sp<GrGpuBuffer> GrGpu::createBuffer(size_t size, GrGpuBufferType intendedType,
                                       GrAccessPattern accessPattern, const void* data) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    this->handleDirtyContext();
    sk_sp<GrGpuBuffer> buffer = this->onCreateBuffer(size, intendedType, accessPattern, data);
    if (!this->caps()->reuseScratchBuffers()) {
        buffer->resourcePriv().removeScratchKey();
    }
    return buffer;
}

bool GrGpu::copySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                        const SkIPoint& dstPoint) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(dst && src);
    SkASSERT(!src->framebufferOnly());

    if (dst->readOnly()) {
        return false;
    }

    this->handleDirtyContext();

    return this->onCopySurface(dst, src, srcRect, dstPoint);
}

bool GrGpu::readPixels(GrSurface* surface, int left, int top, int width, int height,
                       GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                       size_t rowBytes) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(surface);
    SkASSERT(!surface->framebufferOnly());
    SkASSERT(this->caps()->isFormatTexturable(surface->backendFormat()));

    auto subRect = SkIRect::MakeXYWH(left, top, width, height);
    auto bounds  = SkIRect::MakeWH(surface->width(), surface->height());
    if (!bounds.contains(subRect)) {
        return false;
    }

    size_t minRowBytes = SkToSizeT(GrColorTypeBytesPerPixel(dstColorType) * width);
    if (!this->caps()->readPixelsRowBytesSupport()) {
        if (rowBytes != minRowBytes) {
            return false;
        }
    } else {
        if (rowBytes < minRowBytes) {
            return false;
        }
        if (rowBytes % GrColorTypeBytesPerPixel(dstColorType)) {
            return false;
        }
    }

    this->handleDirtyContext();

    return this->onReadPixels(surface, left, top, width, height, surfaceColorType, dstColorType,
                              buffer, rowBytes);
}

bool GrGpu::writePixels(GrSurface* surface, int left, int top, int width, int height,
                        GrColorType surfaceColorType, GrColorType srcColorType,
                        const GrMipLevel texels[], int mipLevelCount, bool prepForTexSampling) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(surface);
    SkASSERT(!surface->framebufferOnly());

    if (surface->readOnly()) {
        return false;
    }

    if (mipLevelCount == 0) {
        return false;
    } else if (mipLevelCount == 1) {
        // We require that if we are not mipped, then the write region is contained in the surface
        auto subRect = SkIRect::MakeXYWH(left, top, width, height);
        auto bounds  = SkIRect::MakeWH(surface->width(), surface->height());
        if (!bounds.contains(subRect)) {
            return false;
        }
    } else if (0 != left || 0 != top || width != surface->width() || height != surface->height()) {
        // We require that if the texels are mipped, than the write region is the entire surface
        return false;
    }

    if (!validate_texel_levels({width, height}, srcColorType, texels, mipLevelCount,
                               this->caps())) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onWritePixels(surface, left, top, width, height, surfaceColorType, srcColorType,
                            texels, mipLevelCount, prepForTexSampling)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(surface, kTopLeft_GrSurfaceOrigin, &rect, mipLevelCount);
        fStats.incTextureUploads();
        return true;
    }
    return false;
}

bool GrGpu::transferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                             GrColorType textureColorType, GrColorType bufferColorType,
                             GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(texture);
    SkASSERT(transferBuffer);

    if (texture->readOnly()) {
        return false;
    }

    // We require that the write region is contained in the texture
    SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
    SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
    if (!bounds.contains(subRect)) {
        return false;
    }

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (this->caps()->writePixelsRowBytesSupport()) {
        if (rowBytes < SkToSizeT(bpp * width)) {
            return false;
        }
        if (rowBytes % bpp) {
            return false;
        }
    } else {
        if (rowBytes != SkToSizeT(bpp * width)) {
            return false;
        }
    }

    this->handleDirtyContext();
    if (this->onTransferPixelsTo(texture, left, top, width, height, textureColorType,
                                 bufferColorType, transferBuffer, offset, rowBytes)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(texture, kTopLeft_GrSurfaceOrigin, &rect);
        fStats.incTransfersToTexture();

        return true;
    }
    return false;
}

bool GrGpu::transferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                               GrColorType surfaceColorType, GrColorType bufferColorType,
                               GrGpuBuffer* transferBuffer, size_t offset) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(surface);
    SkASSERT(transferBuffer);
    SkASSERT(this->caps()->isFormatTexturable(surface->backendFormat()));

#ifdef SK_DEBUG
    auto supportedRead = this->caps()->supportedReadPixelsColorType(
            surfaceColorType, surface->backendFormat(), bufferColorType);
    SkASSERT(supportedRead.fOffsetAlignmentForTransferBuffer);
    SkASSERT(offset % supportedRead.fOffsetAlignmentForTransferBuffer == 0);
#endif

    // We require that the write region is contained in the texture
    SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
    SkIRect bounds = SkIRect::MakeWH(surface->width(), surface->height());
    if (!bounds.contains(subRect)) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onTransferPixelsFrom(surface, left, top, width, height, surfaceColorType,
                                   bufferColorType, transferBuffer, offset)) {
        fStats.incTransfersFromSurface();
        return true;
    }
    return false;
}

bool GrGpu::regenerateMipMapLevels(GrTexture* texture) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(texture);
    SkASSERT(this->caps()->mipMapSupport());
    SkASSERT(texture->texturePriv().mipMapped() == GrMipMapped::kYes);
    if (!texture->texturePriv().mipMapsAreDirty()) {
        // This can happen when the proxy expects mipmaps to be dirty, but they are not dirty on the
        // actual target. This may be caused by things that the drawingManager could not predict,
        // i.e., ops that don't draw anything, aborting a draw for exceptional circumstances, etc.
        // NOTE: This goes away once we quit tracking mipmap state on the actual texture.
        return true;
    }
    if (texture->readOnly()) {
        return false;
    }
    if (this->onRegenerateMipMapLevels(texture)) {
        texture->texturePriv().markMipMapsClean();
        return true;
    }
    return false;
}

void GrGpu::resetTextureBindings() {
    this->handleDirtyContext();
    this->onResetTextureBindings();
}

void GrGpu::resolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect,
                                ForExternalIO forExternalIO) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target, resolveRect, forExternalIO);
}

void GrGpu::didWriteToSurface(GrSurface* surface, GrSurfaceOrigin origin, const SkIRect* bounds,
                              uint32_t mipLevels) const {
    SkASSERT(surface);
    SkASSERT(!surface->readOnly());
    // Mark any MIP chain and resolve buffer as dirty if and only if there is a non-empty bounds.
    if (nullptr == bounds || !bounds->isEmpty()) {
        GrTexture* texture = surface->asTexture();
        if (texture && 1 == mipLevels) {
            texture->texturePriv().markMipMapsDirty();
        }
    }
}

int GrGpu::findOrAssignSamplePatternKey(GrRenderTarget* renderTarget) {
    SkASSERT(this->caps()->sampleLocationsSupport());
    SkASSERT(renderTarget->numSamples() > 1 ||
             (renderTarget->renderTargetPriv().getStencilAttachment() &&
              renderTarget->renderTargetPriv().getStencilAttachment()->numSamples() > 1));

    SkSTArray<16, SkPoint> sampleLocations;
    this->querySampleLocations(renderTarget, &sampleLocations);
    return fSamplePatternDictionary.findOrAssignSamplePatternKey(sampleLocations);
}

GrSemaphoresSubmitted GrGpu::finishFlush(GrSurfaceProxy* proxies[],
                                         int n,
                                         SkSurface::BackendSurfaceAccess access,
                                         const GrFlushInfo& info,
                                         const GrPrepareForExternalIORequests& externalRequests) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    this->stats()->incNumFinishFlushes();
    GrResourceProvider* resourceProvider = fContext->priv().resourceProvider();

    struct SemaphoreInfo {
        std::unique_ptr<GrSemaphore> fSemaphore;
        bool fDidCreate = false;
    };

    bool failedSemaphoreCreation = false;
    std::unique_ptr<SemaphoreInfo[]> semaphoreInfos(new SemaphoreInfo[info.fNumSemaphores]);
    if (this->caps()->semaphoreSupport() && info.fNumSemaphores) {
        for (int i = 0; i < info.fNumSemaphores && !failedSemaphoreCreation; ++i) {
            if (info.fSignalSemaphores[i].isInitialized()) {
                semaphoreInfos[i].fSemaphore = resourceProvider->wrapBackendSemaphore(
                        info.fSignalSemaphores[i],
                        GrResourceProvider::SemaphoreWrapType::kWillSignal,
                        kBorrow_GrWrapOwnership);
            } else {
                semaphoreInfos[i].fSemaphore = resourceProvider->makeSemaphore(false);
                semaphoreInfos[i].fDidCreate = true;
            }
            if (!semaphoreInfos[i].fSemaphore) {
                semaphoreInfos[i].fDidCreate = false;
                failedSemaphoreCreation = true;
            }
        }
        if (!failedSemaphoreCreation) {
            for (int i = 0; i < info.fNumSemaphores && !failedSemaphoreCreation; ++i) {
                this->insertSemaphore(semaphoreInfos[i].fSemaphore.get());
            }
        }
    }

    // We always want to try flushing, so do that before checking if we failed semaphore creation.
    if (!this->onFinishFlush(proxies, n, access, info, externalRequests) ||
        failedSemaphoreCreation) {
        // If we didn't do the flush or failed semaphore creations then none of the semaphores were
        // submitted. Therefore the client can't wait on any of the semaphores. Additionally any
        // semaphores we created here the client is not responsible for deleting so we must make
        // sure they get deleted. We do this by changing the ownership from borrowed to owned.
        for (int i = 0; i < info.fNumSemaphores; ++i) {
            if (semaphoreInfos[i].fDidCreate) {
                SkASSERT(semaphoreInfos[i].fSemaphore);
                semaphoreInfos[i].fSemaphore->setIsOwned();
            }
        }
        return GrSemaphoresSubmitted::kNo;
    }

    for (int i = 0; i < info.fNumSemaphores; ++i) {
        if (!info.fSignalSemaphores[i].isInitialized()) {
            SkASSERT(semaphoreInfos[i].fSemaphore);
            info.fSignalSemaphores[i] = semaphoreInfos[i].fSemaphore->backendSemaphore();
        }
    }

    return this->caps()->semaphoreSupport() ? GrSemaphoresSubmitted::kYes
                                            : GrSemaphoresSubmitted::kNo;
}

#ifdef SK_ENABLE_DUMP_GPU
void GrGpu::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    // TODO: Is there anything useful in the base class to dump here?

    this->onDumpJSON(writer);

    writer->endObject();
}
#else
void GrGpu::dumpJSON(SkJSONWriter* writer) const { }
#endif

#if GR_TEST_UTILS

#if GR_GPU_STATS
static const char* cache_result_to_str(int i) {
    const char* kCacheResultStrings[GrGpu::Stats::kNumProgramCacheResults] = {
        "hits",
        "misses",
        "partials"
    };
    static_assert(0 == (int) GrGpu::Stats::ProgramCacheResult::kHit);
    static_assert(1 == (int) GrGpu::Stats::ProgramCacheResult::kMiss);
    static_assert(2 == (int) GrGpu::Stats::ProgramCacheResult::kPartial);
    static_assert(GrGpu::Stats::kNumProgramCacheResults == 3);
    return kCacheResultStrings[i];
}

void GrGpu::Stats::dump(SkString* out) {
    out->appendf("Render Target Binds: %d\n", fRenderTargetBinds);
    out->appendf("Shader Compilations: %d\n", fShaderCompilations);
    out->appendf("Textures Created: %d\n", fTextureCreates);
    out->appendf("Texture Uploads: %d\n", fTextureUploads);
    out->appendf("Transfers to Texture: %d\n", fTransfersToTexture);
    out->appendf("Transfers from Surface: %d\n", fTransfersFromSurface);
    out->appendf("Stencil Buffer Creates: %d\n", fStencilAttachmentCreates);
    out->appendf("Number of draws: %d\n", fNumDraws);
    out->appendf("Number of Scratch Textures reused %d\n", fNumScratchTexturesReused);

    SkASSERT(fNumInlineCompilationFailures == 0);
    out->appendf("Number of Inline compile failures %d\n", fNumInlineCompilationFailures);
    for (int i = 0; i < Stats::kNumProgramCacheResults-1; ++i) {
        out->appendf("Inline Program Cache %s %d\n", cache_result_to_str(i),
                     fInlineProgramCacheStats[i]);
    }

    SkASSERT(fNumPreCompilationFailures == 0);
    out->appendf("Number of precompile failures %d\n", fNumPreCompilationFailures);
    for (int i = 0; i < Stats::kNumProgramCacheResults-1; ++i) {
        out->appendf("Precompile Program Cache %s %d\n", cache_result_to_str(i),
                     fPreProgramCacheStats[i]);
    }

    SkASSERT(fNumCompilationFailures == 0);
    out->appendf("Total number of compilation failures %d\n", fNumCompilationFailures);
    out->appendf("Total number of partial compilation successes %d\n",
                 fNumPartialCompilationSuccesses);
    out->appendf("Total number of compilation successes %d\n", fNumCompilationSuccesses);

    // enable this block to output CSV-style stats for program pre-compilation
#if 0
    SkASSERT(fNumInlineCompilationFailures == 0);
    SkASSERT(fNumPreCompilationFailures == 0);
    SkASSERT(fNumCompilationFailures == 0);
    SkASSERT(fNumPartialCompilationSuccesses == 0);

    SkDebugf("%d, %d, %d, %d, %d\n",
             fInlineProgramCacheStats[(int) Stats::ProgramCacheResult::kHit],
             fInlineProgramCacheStats[(int) Stats::ProgramCacheResult::kMiss],
             fPreProgramCacheStats[(int) Stats::ProgramCacheResult::kHit],
             fPreProgramCacheStats[(int) Stats::ProgramCacheResult::kMiss],
             fNumCompilationSuccesses);
#endif
}

void GrGpu::Stats::dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) {
    keys->push_back(SkString("render_target_binds")); values->push_back(fRenderTargetBinds);
    keys->push_back(SkString("shader_compilations")); values->push_back(fShaderCompilations);
}

#endif // GR_GPU_STATS
#endif // GR_TEST_UTILS

bool GrGpu::MipMapsAreCorrect(SkISize dimensions,
                              GrMipMapped mipMapped,
                              const BackendTextureData* data) {
    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    if (!data || data->type() == BackendTextureData::Type::kColor) {
        return true;
    }

    if (data->type() == BackendTextureData::Type::kCompressed) {
        return false;  // This should be going through CompressedDataIsCorrect
    }

    SkASSERT(data->type() == BackendTextureData::Type::kPixmaps);

    if (data->pixmap(0).dimensions() != dimensions) {
        return false;
    }

    SkColorType colorType = data->pixmap(0).colorType();
    for (int i = 1; i < numMipLevels; ++i) {
        dimensions = {std::max(1, dimensions.width()/2), std::max(1, dimensions.height()/2)};
        if (dimensions != data->pixmap(i).dimensions()) {
            return false;
        }
        if (colorType != data->pixmap(i).colorType()) {
            return false;
        }
    }
    return true;
}

bool GrGpu::CompressedDataIsCorrect(SkISize dimensions, SkImage::CompressionType compressionType,
                                    GrMipMapped mipMapped, const BackendTextureData* data) {

    if (!data || data->type() == BackendTextureData::Type::kColor) {
        return true;
    }

    if (data->type() == BackendTextureData::Type::kPixmaps) {
        return false;
    }

    SkASSERT(data->type() == BackendTextureData::Type::kCompressed);

    size_t computedSize = SkCompressedDataSize(compressionType, dimensions,
                                               nullptr, mipMapped == GrMipMapped::kYes);

    return computedSize == data->compressedSize();
}

GrBackendTexture GrGpu::createBackendTexture(SkISize dimensions,
                                             const GrBackendFormat& format,
                                             GrRenderable renderable,
                                             GrMipMapped mipMapped,
                                             GrProtected isProtected,
                                             const BackendTextureData* data) {
    const GrCaps* caps = this->caps();

    if (!format.isValid()) {
        return {};
    }

    if (caps->isFormatCompressed(format)) {
        // Compressed formats must go through the createCompressedBackendTexture API
        return {};
    }

    if (data && data->type() == BackendTextureData::Type::kPixmaps) {
        auto ct = SkColorTypeToGrColorType(data->pixmap(0).colorType());
        if (!caps->areColorTypeAndFormatCompatible(ct, format)) {
            return {};
        }
    }

    if (dimensions.isEmpty() || dimensions.width()  > caps->maxTextureSize() ||
                                dimensions.height() > caps->maxTextureSize()) {
        return {};
    }

    if (mipMapped == GrMipMapped::kYes && !this->caps()->mipMapSupport()) {
        return {};
    }

    if (!MipMapsAreCorrect(dimensions, mipMapped, data)) {
        return {};
    }

    return this->onCreateBackendTexture(dimensions, format, renderable, mipMapped,
                                        isProtected, data);
}

GrBackendTexture GrGpu::createCompressedBackendTexture(SkISize dimensions,
                                                       const GrBackendFormat& format,
                                                       GrMipMapped mipMapped,
                                                       GrProtected isProtected,
                                                       const BackendTextureData* data) {
    const GrCaps* caps = this->caps();

    if (!format.isValid()) {
        return {};
    }

    SkImage::CompressionType compressionType = caps->compressionType(format);
    if (compressionType == SkImage::CompressionType::kNone) {
        // Uncompressed formats must go through the createBackendTexture API
        return {};
    }

    if (dimensions.isEmpty() ||
        dimensions.width()  > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        return {};
    }

    if (mipMapped == GrMipMapped::kYes && !this->caps()->mipMapSupport()) {
        return {};
    }

    if (!CompressedDataIsCorrect(dimensions, compressionType, mipMapped, data)) {
        return {};
    }

    return this->onCreateCompressedBackendTexture(dimensions, format, mipMapped,
                                                  isProtected, data);
}
