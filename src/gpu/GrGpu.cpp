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
#include "src/core/SkMathPriv.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrMesh.h"
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

bool GrGpu::IsACopyNeededForRepeatWrapMode(const GrCaps* caps, GrTextureProxy* texProxy,
                                           int width, int height,
                                           GrSamplerState::Filter filter,
                                           GrTextureProducer::CopyParams* copyParams,
                                           SkScalar scaleAdjust[2]) {
    if (!caps->npotTextureTileSupport() &&
        (!SkIsPow2(width) || !SkIsPow2(height))) {
        SkASSERT(scaleAdjust);
        copyParams->fWidth = GrNextPow2(width);
        copyParams->fHeight = GrNextPow2(height);
        SkASSERT(scaleAdjust);
        scaleAdjust[0] = ((SkScalar)copyParams->fWidth) / width;
        scaleAdjust[1] = ((SkScalar)copyParams->fHeight) / height;
        switch (filter) {
        case GrSamplerState::Filter::kNearest:
            copyParams->fFilter = GrSamplerState::Filter::kNearest;
            break;
        case GrSamplerState::Filter::kBilerp:
        case GrSamplerState::Filter::kMipMap:
            // We are only ever scaling up so no reason to ever indicate kMipMap.
            copyParams->fFilter = GrSamplerState::Filter::kBilerp;
            break;
        }
        return true;
    }

    if (texProxy) {
        // If the texture format itself doesn't support repeat wrap mode or mipmapping (and
        // those capabilities are required) force a copy.
        if (texProxy->hasRestrictedSampling()) {
            copyParams->fFilter = GrSamplerState::Filter::kNearest;
            copyParams->fWidth = texProxy->width();
            copyParams->fHeight = texProxy->height();
            return true;
        }
    }

    return false;
}

bool GrGpu::IsACopyNeededForMips(const GrCaps* caps, const GrTextureProxy* texProxy,
                                 GrSamplerState::Filter filter,
                                 GrTextureProducer::CopyParams* copyParams) {
    SkASSERT(texProxy);
    bool willNeedMips = GrSamplerState::Filter::kMipMap == filter && caps->mipMapSupport();
    // If the texture format itself doesn't support mipmapping (and those capabilities are required)
    // force a copy.
    if (willNeedMips && texProxy->mipMapped() == GrMipMapped::kNo) {
        copyParams->fFilter = GrSamplerState::Filter::kNearest;
        copyParams->fWidth = texProxy->width();
        copyParams->fHeight = texProxy->height();
        return true;
    }

    return false;
}

static bool validate_texel_levels(int w, int h, GrColorType texelColorType,
                                  const GrMipLevel* texels, int mipLevelCount, const GrCaps* caps) {
    SkASSERT(mipLevelCount > 0);
    bool hasBasePixels = texels[0].fPixels;
    int levelsWithPixelsCnt = 0;
    auto bpp = GrColorTypeBytesPerPixel(texelColorType);
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

sk_sp<GrTexture> GrGpu::createTextureCommon(const GrSurfaceDesc& desc,
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
    if (!this->caps()->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig,
                                             renderable, renderTargetSampleCnt, mipMapped)) {
        return nullptr;
    }

    if (renderable == GrRenderable::kYes) {
        renderTargetSampleCnt =
                this->caps()->getRenderTargetSampleCount(renderTargetSampleCnt, format);
    }
    // Attempt to catch un- or wrongly initialized sample counts.
    SkASSERT(renderTargetSampleCnt > 0 && renderTargetSampleCnt <= 64);
    this->handleDirtyContext();
    auto tex = this->onCreateTexture(desc,
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

sk_sp<GrTexture> GrGpu::createTexture(const GrSurfaceDesc& desc,
                                      const GrBackendFormat& format,
                                      GrRenderable renderable,
                                      int renderTargetSampleCnt,
                                      GrMipMapped mipMapped,
                                      SkBudgeted budgeted,
                                      GrProtected isProtected) {
    int mipLevelCount = 1;
    if (mipMapped == GrMipMapped::kYes) {
        mipLevelCount = 32 - SkCLZ(static_cast<uint32_t>(SkTMax(desc.fWidth, desc.fHeight)));
    }
    uint32_t levelClearMask =
            this->caps()->shouldInitializeTextures() ? (1 << mipLevelCount) - 1 : 0;
    auto tex = this->createTextureCommon(desc, format, renderable, renderTargetSampleCnt, budgeted,
                                         isProtected, mipLevelCount, levelClearMask);
    if (tex && mipMapped == GrMipMapped::kYes && levelClearMask) {
        tex->texturePriv().markMipMapsClean();
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::createTexture(const GrSurfaceDesc& desc,
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
        if (!validate_texel_levels(desc.fWidth, desc.fHeight, srcColorType, texels, texelLevelCount,
                                   this->caps())) {
            return nullptr;
        }
    }

    int mipLevelCount = SkTMax(1, texelLevelCount);
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

    auto tex = this->createTextureCommon(desc, format, renderable, renderTargetSampleCnt, budgeted,
                                         isProtected, texelLevelCount, levelClearMask);
    if (tex) {
        bool markMipLevelsClean = false;
        // Currently if level 0 does not have pixels then no other level may, as enforced by
        // validate_texel_levels.
        if (texelLevelCount && texels[0].fPixels) {
            if (!this->writePixels(tex.get(), 0, 0, desc.fWidth, desc.fHeight, textureColorType,
                                   srcColorType, texels, texelLevelCount)) {
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

sk_sp<GrTexture> GrGpu::createCompressedTexture(int width, int height,
                                                const GrBackendFormat& format,
                                                SkImage::CompressionType compressionType,
                                                SkBudgeted budgeted, const void* data,
                                                size_t dataSize) {
    // If we ever add a new CompressionType, we should add a check here to make sure the
    // GrBackendFormat and CompressionType are compatible with eachother.
    SkASSERT(compressionType == SkImage::kETC1_CompressionType);

    this->handleDirtyContext();
    if (width  < 1 || width  > this->caps()->maxTextureSize() ||
        height < 1 || height > this->caps()->maxTextureSize()) {
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
    if (dataSize < GrCompressedDataSize(compressionType, width, height)) {
        return nullptr;
    }
    return this->onCreateCompressedTexture(width, height, format, compressionType, budgeted, data);
}

sk_sp<GrTexture> GrGpu::wrapBackendTexture(const GrBackendTexture& backendTex,
                                           GrColorType colorType,
                                           GrWrapOwnership ownership, GrWrapCacheable cacheable,
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

    return this->onWrapBackendTexture(backendTex, colorType, ownership, cacheable, ioType);
}

sk_sp<GrTexture> GrGpu::wrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                     int sampleCnt, GrColorType colorType,
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
    sk_sp<GrTexture> tex = this->onWrapRenderableBackendTexture(backendTex, sampleCnt, colorType,
                                                                ownership, cacheable);
    SkASSERT(!tex || tex->asRenderTarget());
    if (tex && sampleCnt > 1 && !caps->msaaResolvesAutomatically()) {
        tex->asRenderTarget()->setRequiresManualMSAAResolve();
    }
    return tex;
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendRenderTarget(const GrBackendRenderTarget& backendRT,
                                                     GrColorType colorType) {
    this->handleDirtyContext();

    const GrCaps* caps = this->caps();

    if (!caps->isFormatRenderable(backendRT.getBackendFormat(), backendRT.sampleCnt())) {
        return nullptr;
    }

    return this->onWrapBackendRenderTarget(backendRT, colorType);
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendTextureAsRenderTarget(const GrBackendTexture& backendTex,
                                                              int sampleCnt,
                                                              GrColorType colorType) {
    this->handleDirtyContext();

    const GrCaps* caps = this->caps();

    int maxSize = caps->maxTextureSize();
    if (backendTex.width() > maxSize || backendTex.height() > maxSize) {
        return nullptr;
    }

    if (!caps->isFormatRenderable(backendTex.getBackendFormat(), sampleCnt)) {
        return nullptr;
    }

    auto rt = this->onWrapBackendTextureAsRenderTarget(backendTex, sampleCnt, colorType);
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

    if (this->caps()->isFormatCompressed(surface->backendFormat())) {
        return false;
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
    SkASSERT(this->caps()->isFormatTexturableAndUploadable(surfaceColorType,
                                                           surface->backendFormat()));

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

    if (!validate_texel_levels(width, height, srcColorType, texels, mipLevelCount, this->caps())) {
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
    SkASSERT(this->caps()->isFormatTexturableAndUploadable(textureColorType,
                                                           texture->backendFormat()));

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
    SkASSERT(texture->texturePriv().mipMapsAreDirty());
    SkASSERT(!texture->asRenderTarget() || !texture->asRenderTarget()->needsResolve());
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

void GrGpu::resolveRenderTarget(GrRenderTarget* target, ForExternalIO forExternalIO) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target, forExternalIO);
}

void GrGpu::didWriteToSurface(GrSurface* surface, GrSurfaceOrigin origin, const SkIRect* bounds,
                              uint32_t mipLevels) const {
    SkASSERT(surface);
    SkASSERT(!surface->readOnly());
    // Mark any MIP chain and resolve buffer as dirty if and only if there is a non-empty bounds.
    if (nullptr == bounds || !bounds->isEmpty()) {
        if (GrRenderTarget* target = surface->asRenderTarget()) {
            SkIRect flippedBounds;
            if (kBottomLeft_GrSurfaceOrigin == origin && bounds) {
                flippedBounds = {bounds->fLeft, surface->height() - bounds->fBottom,
                                 bounds->fRight, surface->height() - bounds->fTop};
                bounds = &flippedBounds;
            }
            target->flagAsNeedingResolve(bounds);
        }
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

    if (this->caps()->semaphoreSupport()) {
        for (int i = 0; i < info.fNumSemaphores; ++i) {
            sk_sp<GrSemaphore> semaphore;
            if (info.fSignalSemaphores[i].isInitialized()) {
                semaphore = resourceProvider->wrapBackendSemaphore(
                        info.fSignalSemaphores[i],
                        GrResourceProvider::SemaphoreWrapType::kWillSignal,
                        kBorrow_GrWrapOwnership);
            } else {
                semaphore = resourceProvider->makeSemaphore(false);
            }
            this->insertSemaphore(semaphore);

            if (!info.fSignalSemaphores[i].isInitialized()) {
                info.fSignalSemaphores[i] = semaphore->backendSemaphore();
            }
        }
    }
    this->onFinishFlush(proxies, n, access, info, externalRequests);
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
}

void GrGpu::Stats::dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) {
    keys->push_back(SkString("render_target_binds")); values->push_back(fRenderTargetBinds);
    keys->push_back(SkString("shader_compilations")); values->push_back(fShaderCompilations);
}

#endif // GR_GPU_STATS
#endif // GR_TEST_UTILS


bool GrGpu::MipMapsAreCorrect(int baseWidth, int baseHeight, GrMipMapped mipMapped,
                              const SkPixmap srcData[], int numMipLevels) {
    if (!srcData) {
        return true;
    }

    if (baseWidth != srcData[0].width() || baseHeight != srcData[0].height()) {
        return false;
    }

    if (mipMapped == GrMipMapped::kYes) {
        if (numMipLevels != SkMipMap::ComputeLevelCount(baseWidth, baseHeight) + 1) {
            return false;
        }

        SkColorType colorType = srcData[0].colorType();

        int currentWidth = baseWidth;
        int currentHeight = baseHeight;
        for (int i = 1; i < numMipLevels; ++i) {
            currentWidth = SkTMax(1, currentWidth / 2);
            currentHeight = SkTMax(1, currentHeight / 2);

            if (srcData[i].colorType() != colorType) { // all levels must have same colorType
                return false;
            }

            if (srcData[i].width() != currentWidth || srcData[i].height() != currentHeight) {
                return false;
            }
        }
    } else if (numMipLevels != 1) {
        return false;
    }

    return true;
}

GrBackendTexture GrGpu::createBackendTexture(int w, int h, const GrBackendFormat& format,
                                             GrMipMapped mipMapped, GrRenderable renderable,
                                             const SkPixmap srcData[], int numMipLevels,
                                             const SkColor4f* color, GrProtected isProtected) {
    const GrCaps* caps = this->caps();

    if (!format.isValid()) {
        return {};
    }

    if (caps->isFormatCompressed(format)) {
        // Compressed formats must go through the createCompressedBackendTexture API
        return {};
    }

    if (w < 1 || w > caps->maxTextureSize() || h < 1 || h > caps->maxTextureSize()) {
        return {};
    }

    // TODO: maybe just ignore the mipMapped parameter in this case
    if (mipMapped == GrMipMapped::kYes && !this->caps()->mipMapSupport()) {
        return {};
    }

    if (!MipMapsAreCorrect(w, h, mipMapped, srcData, numMipLevels)) {
        return {};
    }

    return this->onCreateBackendTexture(w, h, format, mipMapped, renderable,
                                        srcData, numMipLevels, color, isProtected);
}
