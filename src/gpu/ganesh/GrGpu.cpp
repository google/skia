/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/ganesh/GrGpu.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrRingBuffer.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxyPriv.h"
#include "src/gpu/ganesh/GrTracing.h"
#include "src/sksl/SkSLCompiler.h"

using namespace skia_private;

////////////////////////////////////////////////////////////////////////////////

GrGpu::GrGpu(GrDirectContext* direct) : fResetBits(kAll_GrBackendState), fContext(direct) {}

GrGpu::~GrGpu() {
    this->callSubmittedProcs(false);
}

void GrGpu::initCapsAndCompiler(sk_sp<const GrCaps> caps) {
    fCaps = std::move(caps);
    fCompiler = std::make_unique<SkSL::Compiler>(fCaps->shaderCaps());
}

void GrGpu::disconnect(DisconnectType type) {}

////////////////////////////////////////////////////////////////////////////////

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
                                            GrTextureType textureType,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            skgpu::Budgeted budgeted,
                                            GrProtected isProtected,
                                            int mipLevelCount,
                                            uint32_t levelClearMask,
                                            std::string_view label) {
    if (this->caps()->isFormatCompressed(format)) {
        // Call GrGpu::createCompressedTexture.
        return nullptr;
    }

    skgpu::Mipmapped mipmapped = mipLevelCount > 1 ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    if (!this->caps()->validateSurfaceParams(dimensions,
                                             format,
                                             renderable,
                                             renderTargetSampleCnt,
                                             mipmapped,
                                             textureType)) {
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
                                     levelClearMask,
                                     label);
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
                                      GrTextureType textureType,
                                      GrRenderable renderable,
                                      int renderTargetSampleCnt,
                                      skgpu::Mipmapped mipmapped,
                                      skgpu::Budgeted budgeted,
                                      GrProtected isProtected,
                                      std::string_view label) {
    int mipLevelCount = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        mipLevelCount =
                32 - SkCLZ(static_cast<uint32_t>(std::max(dimensions.fWidth, dimensions.fHeight)));
    }
    uint32_t levelClearMask =
            this->caps()->shouldInitializeTextures() ? (1 << mipLevelCount) - 1 : 0;
    auto tex = this->createTextureCommon(dimensions,
                                         format,
                                         textureType,
                                         renderable,
                                         renderTargetSampleCnt,
                                         budgeted,
                                         isProtected,
                                         mipLevelCount,
                                         levelClearMask,
                                         label);
    if (tex && mipmapped == skgpu::Mipmapped::kYes && levelClearMask) {
        tex->markMipmapsClean();
    }

    return tex;
}

sk_sp<GrTexture> GrGpu::createTexture(SkISize dimensions,
                                      const GrBackendFormat& format,
                                      GrTextureType textureType,
                                      GrRenderable renderable,
                                      int renderTargetSampleCnt,
                                      skgpu::Budgeted budgeted,
                                      GrProtected isProtected,
                                      GrColorType textureColorType,
                                      GrColorType srcColorType,
                                      const GrMipLevel texels[],
                                      int texelLevelCount,
                                      std::string_view label) {
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

    auto tex = this->createTextureCommon(dimensions,
                                         format,
                                         textureType,
                                         renderable,
                                         renderTargetSampleCnt,
                                         budgeted,
                                         isProtected,
                                         texelLevelCount,
                                         levelClearMask,
                                         label);
    if (tex) {
        bool markMipLevelsClean = false;
        // Currently if level 0 does not have pixels then no other level may, as enforced by
        // validate_texel_levels.
        if (texelLevelCount && texels[0].fPixels) {
            if (!this->writePixels(tex.get(),
                                   SkIRect::MakeSize(dimensions),
                                   textureColorType,
                                   srcColorType,
                                   texels,
                                   texelLevelCount)) {
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
            tex->markMipmapsClean();
        }
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::createCompressedTexture(SkISize dimensions,
                                                const GrBackendFormat& format,
                                                skgpu::Budgeted budgeted,
                                                skgpu::Mipmapped mipmapped,
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

    // TODO: expand CompressedDataIsCorrect to work here too
    SkTextureCompressionType compressionType = GrBackendFormatToCompressionType(format);
    if (compressionType == SkTextureCompressionType::kNone) {
        return nullptr;
    }

    if (!this->caps()->isFormatTexturable(format, GrTextureType::k2D)) {
        return nullptr;
    }

    if (dataSize <
        SkCompressedDataSize(
                compressionType, dimensions, nullptr, mipmapped == skgpu::Mipmapped::kYes)) {
        return nullptr;
    }
    return this->onCreateCompressedTexture(dimensions, format, budgeted, mipmapped, isProtected,
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

    if (!caps->isFormatTexturable(backendTex.getBackendFormat(), backendTex.textureType())) {
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

    if (!caps->isFormatTexturable(backendTex.getBackendFormat(), backendTex.textureType())) {
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

    if (!caps->isFormatTexturable(backendTex.getBackendFormat(), backendTex.textureType()) ||
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

sk_sp<GrRenderTarget> GrGpu::wrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo& imageInfo,
                                                                 const GrVkDrawableInfo& vkInfo) {
    return this->onWrapVulkanSecondaryCBAsRenderTarget(imageInfo, vkInfo);
}

sk_sp<GrRenderTarget> GrGpu::onWrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo& imageInfo,
                                                                   const GrVkDrawableInfo& vkInfo) {
    // This is only supported on Vulkan so we default to returning nullptr here
    return nullptr;
}

sk_sp<GrGpuBuffer> GrGpu::createBuffer(size_t size,
                                       GrGpuBufferType intendedType,
                                       GrAccessPattern accessPattern) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    this->handleDirtyContext();
    if ((intendedType == GrGpuBufferType::kXferCpuToGpu ||
         intendedType == GrGpuBufferType::kXferGpuToCpu) &&
        accessPattern == kStatic_GrAccessPattern) {
        return nullptr;
    }
    sk_sp<GrGpuBuffer> buffer = this->onCreateBuffer(size, intendedType, accessPattern);
    if (!this->caps()->reuseScratchBuffers()) {
        buffer->resourcePriv().removeScratchKey();
    }
    return buffer;
}

bool GrGpu::copySurface(GrSurface* dst, const SkIRect& dstRect,
                        GrSurface* src, const SkIRect& srcRect,
                        GrSamplerState::Filter filter) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(dst && src);
    SkASSERT(!src->framebufferOnly());

    if (dst->readOnly()) {
        return false;
    }

    this->handleDirtyContext();

    return this->onCopySurface(dst, dstRect, src, srcRect, filter);
}

bool GrGpu::readPixels(GrSurface* surface,
                       SkIRect rect,
                       GrColorType surfaceColorType,
                       GrColorType dstColorType,
                       void* buffer,
                       size_t rowBytes) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(surface);
    SkASSERT(!surface->framebufferOnly());
    SkASSERT(this->caps()->areColorTypeAndFormatCompatible(surfaceColorType,
                                                           surface->backendFormat()));

    if (!SkIRect::MakeSize(surface->dimensions()).contains(rect)) {
        return false;
    }

    size_t minRowBytes = SkToSizeT(GrColorTypeBytesPerPixel(dstColorType) * rect.width());
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

    return this->onReadPixels(surface, rect, surfaceColorType, dstColorType, buffer, rowBytes);
}

bool GrGpu::writePixels(GrSurface* surface,
                        SkIRect rect,
                        GrColorType surfaceColorType,
                        GrColorType srcColorType,
                        const GrMipLevel texels[],
                        int mipLevelCount,
                        bool prepForTexSampling) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    ATRACE_ANDROID_FRAMEWORK_ALWAYS("Texture upload(%u) %ix%i",
                                    surface->uniqueID().asUInt(), rect.width(), rect.height());
    SkASSERT(surface);
    SkASSERT(!surface->framebufferOnly());

    if (surface->readOnly()) {
        return false;
    }

    if (mipLevelCount == 0) {
        return false;
    } else if (mipLevelCount == 1) {
        // We require that if we are not mipped, then the write region is contained in the surface
        if (!SkIRect::MakeSize(surface->dimensions()).contains(rect)) {
            return false;
        }
    } else if (rect != SkIRect::MakeSize(surface->dimensions())) {
        // We require that if the texels are mipped, than the write region is the entire surface
        return false;
    }

    if (!validate_texel_levels(rect.size(), srcColorType, texels, mipLevelCount, this->caps())) {
        return false;
    }

    this->handleDirtyContext();
    if (!this->onWritePixels(surface,
                             rect,
                             surfaceColorType,
                             srcColorType,
                             texels,
                             mipLevelCount,
                             prepForTexSampling)) {
        return false;
    }

    this->didWriteToSurface(surface, kTopLeft_GrSurfaceOrigin, &rect, mipLevelCount);
    fStats.incTextureUploads();

    return true;
}

bool GrGpu::transferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                       size_t srcOffset,
                                       sk_sp<GrGpuBuffer> dst,
                                       size_t dstOffset,
                                       size_t size) {
    SkASSERT(src);
    SkASSERT(dst);
    SkASSERT(srcOffset % this->caps()->transferFromBufferToBufferAlignment() == 0);
    SkASSERT(dstOffset % this->caps()->transferFromBufferToBufferAlignment() == 0);
    SkASSERT(size      % this->caps()->transferFromBufferToBufferAlignment() == 0);
    SkASSERT(srcOffset + size <= src->size());
    SkASSERT(dstOffset + size <= dst->size());
    SkASSERT(src->intendedType() == GrGpuBufferType::kXferCpuToGpu);
    SkASSERT(dst->intendedType() != GrGpuBufferType::kXferCpuToGpu);

    this->handleDirtyContext();
    if (!this->onTransferFromBufferToBuffer(std::move(src),
                                            srcOffset,
                                            std::move(dst),
                                            dstOffset,
                                            size)) {
        return false;
    }

    fStats.incBufferTransfers();

    return true;
}

bool GrGpu::transferPixelsTo(GrTexture* texture,
                             SkIRect rect,
                             GrColorType textureColorType,
                             GrColorType bufferColorType,
                             sk_sp<GrGpuBuffer> transferBuffer,
                             size_t offset,
                             size_t rowBytes) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(texture);
    SkASSERT(transferBuffer);
    SkASSERT(transferBuffer->intendedType() == GrGpuBufferType::kXferCpuToGpu);

    if (texture->readOnly()) {
        return false;
    }

    // We require that the write region is contained in the texture
    if (!SkIRect::MakeSize(texture->dimensions()).contains(rect)) {
        return false;
    }

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (this->caps()->writePixelsRowBytesSupport()) {
        if (rowBytes < SkToSizeT(bpp*rect.width())) {
            return false;
        }
        if (rowBytes % bpp) {
            return false;
        }
    } else {
        if (rowBytes != SkToSizeT(bpp*rect.width())) {
            return false;
        }
    }

    this->handleDirtyContext();
    if (!this->onTransferPixelsTo(texture,
                                  rect,
                                  textureColorType,
                                  bufferColorType,
                                  std::move(transferBuffer),
                                  offset,
                                  rowBytes)) {
        return false;
    }

    this->didWriteToSurface(texture, kTopLeft_GrSurfaceOrigin, &rect);
    fStats.incTransfersToTexture();

    return true;
}

bool GrGpu::transferPixelsFrom(GrSurface* surface,
                               SkIRect rect,
                               GrColorType surfaceColorType,
                               GrColorType bufferColorType,
                               sk_sp<GrGpuBuffer> transferBuffer,
                               size_t offset) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(surface);
    SkASSERT(transferBuffer);
    SkASSERT(transferBuffer->intendedType() == GrGpuBufferType::kXferGpuToCpu);
    SkASSERT(this->caps()->areColorTypeAndFormatCompatible(surfaceColorType,
                                                           surface->backendFormat()));

#ifdef SK_DEBUG
    auto supportedRead = this->caps()->supportedReadPixelsColorType(
            surfaceColorType, surface->backendFormat(), bufferColorType);
    SkASSERT(supportedRead.fOffsetAlignmentForTransferBuffer);
    SkASSERT(offset % supportedRead.fOffsetAlignmentForTransferBuffer == 0);
#endif

    // We require that the write region is contained in the texture
    if (!SkIRect::MakeSize(surface->dimensions()).contains(rect)) {
        return false;
    }

    this->handleDirtyContext();
    if (!this->onTransferPixelsFrom(surface,
                                    rect,
                                    surfaceColorType,
                                    bufferColorType,
                                    std::move(transferBuffer),
                                    offset)) {
        return false;
    }

    fStats.incTransfersFromSurface();

    return true;
}

bool GrGpu::regenerateMipMapLevels(GrTexture* texture) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(texture);
    SkASSERT(this->caps()->mipmapSupport());
    SkASSERT(texture->mipmapped() == skgpu::Mipmapped::kYes);
    if (!texture->mipmapsAreDirty()) {
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
        texture->markMipmapsClean();
        return true;
    }
    return false;
}

void GrGpu::resetTextureBindings() {
    this->handleDirtyContext();
    this->onResetTextureBindings();
}

void GrGpu::resolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target, resolveRect);
}

void GrGpu::didWriteToSurface(GrSurface* surface, GrSurfaceOrigin origin, const SkIRect* bounds,
                              uint32_t mipLevels) const {
    SkASSERT(surface);
    SkASSERT(!surface->readOnly());
    // Mark any MIP chain as dirty if and only if there is a non-empty bounds.
    if (nullptr == bounds || !bounds->isEmpty()) {
        GrTexture* texture = surface->asTexture();
        if (texture) {
            if (mipLevels == 1) {
                texture->markMipmapsDirty();
            } else {
                texture->markMipmapsClean();
            }
        }
    }
}

void GrGpu::executeFlushInfo(SkSpan<GrSurfaceProxy*> proxies,
                             SkSurfaces::BackendSurfaceAccess access,
                             const GrFlushInfo& info,
                             const skgpu::MutableTextureState* newState) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    GrResourceProvider* resourceProvider = fContext->priv().resourceProvider();

    std::unique_ptr<std::unique_ptr<GrSemaphore>[]> semaphores(
            new std::unique_ptr<GrSemaphore>[info.fNumSemaphores]);
    if (this->caps()->semaphoreSupport() && info.fNumSemaphores) {
        for (size_t i = 0; i < info.fNumSemaphores; ++i) {
            if (info.fSignalSemaphores[i].isInitialized()) {
                semaphores[i] = resourceProvider->wrapBackendSemaphore(
                    info.fSignalSemaphores[i],
                    GrSemaphoreWrapType::kWillSignal,
                    kBorrow_GrWrapOwnership);
                // If we failed to wrap the semaphore it means the client didn't give us a valid
                // semaphore to begin with. Therefore, it is fine to not signal it.
                if (semaphores[i]) {
                    this->insertSemaphore(semaphores[i].get());
                }
            } else {
                semaphores[i] = resourceProvider->makeSemaphore(false);
                if (semaphores[i]) {
                    this->insertSemaphore(semaphores[i].get());
                    info.fSignalSemaphores[i] = semaphores[i]->backendSemaphore();
                }
            }
        }
    }

    if (info.fFinishedProc) {
        this->addFinishedProc(info.fFinishedProc, info.fFinishedContext);
    }

    if (info.fSubmittedProc) {
        fSubmittedProcs.emplace_back(info.fSubmittedProc, info.fSubmittedContext);
    }

    // We currently don't support passing in new surface state for multiple proxies here. The only
    // time we have multiple proxies is if we are flushing a yuv SkImage which won't have state
    // updates anyways.
    SkASSERT(!newState || proxies.size() == 1);
    SkASSERT(!newState || access == SkSurfaces::BackendSurfaceAccess::kNoAccess);
    this->prepareSurfacesForBackendAccessAndStateUpdates(proxies, access, newState);
}

GrOpsRenderPass* GrGpu::getOpsRenderPass(
        GrRenderTarget* renderTarget,
        bool useMSAASurface,
        GrAttachment* stencil,
        GrSurfaceOrigin origin,
        const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
        const TArray<GrSurfaceProxy*, true>& sampledProxies,
        GrXferBarrierFlags renderPassXferBarriers) {
#if SK_HISTOGRAMS_ENABLED
    fCurrentSubmitRenderPassCount++;
#endif
    fStats.incRenderPasses();
    return this->onGetOpsRenderPass(renderTarget, useMSAASurface, stencil, origin, bounds,
                                    colorInfo, stencilInfo, sampledProxies, renderPassXferBarriers);
}

bool GrGpu::submitToGpu(GrSyncCpu sync) {
    this->stats()->incNumSubmitToGpus();

    if (auto manager = this->stagingBufferManager()) {
        manager->detachBuffers();
    }

    if (auto uniformsBuffer = this->uniformsRingBuffer()) {
        uniformsBuffer->startSubmit(this);
    }

    bool submitted = this->onSubmitToGpu(sync);

    this->callSubmittedProcs(submitted);

    this->reportSubmitHistograms();

    return submitted;
}

void GrGpu::reportSubmitHistograms() {
#if SK_HISTOGRAMS_ENABLED
    // The max allowed value for SK_HISTOGRAM_EXACT_LINEAR is 100. If we want to support higher
    // values we can add SK_HISTOGRAM_CUSTOM_COUNTS but this has a number of buckets that is less
    // than the number of actual values
    static constexpr int kMaxRenderPassBucketValue = 100;
    SK_HISTOGRAM_EXACT_LINEAR("SubmitRenderPasses",
                              std::min(fCurrentSubmitRenderPassCount, kMaxRenderPassBucketValue),
                              kMaxRenderPassBucketValue);
    fCurrentSubmitRenderPassCount = 0;
#endif

    this->onReportSubmitHistograms();
}

bool GrGpu::checkAndResetOOMed() {
    if (fOOMed) {
        fOOMed = false;
        return true;
    }
    return false;
}

void GrGpu::callSubmittedProcs(bool success) {
    for (int i = 0; i < fSubmittedProcs.size(); ++i) {
        fSubmittedProcs[i].fProc(fSubmittedProcs[i].fContext, success);
    }
    fSubmittedProcs.clear();
}

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"

void GrGpu::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    // TODO: Is there anything useful in the base class to dump here?

    this->onDumpJSON(writer);

    writer->endObject();
}
#else
void GrGpu::dumpJSON(SkJSONWriter* writer) const { }
#endif

#if defined(GR_TEST_UTILS)

#if GR_GPU_STATS

void GrGpu::Stats::dump(SkString* out) {
    out->appendf("Textures Created: %d\n", fTextureCreates);
    out->appendf("Texture Uploads: %d\n", fTextureUploads);
    out->appendf("Transfers to Texture: %d\n", fTransfersToTexture);
    out->appendf("Transfers from Surface: %d\n", fTransfersFromSurface);
    out->appendf("Stencil Buffer Creates: %d\n", fStencilAttachmentCreates);
    out->appendf("MSAA Attachment Creates: %d\n", fMSAAAttachmentCreates);
    out->appendf("Number of draws: %d\n", fNumDraws);
    out->appendf("Number of Scratch Textures reused %d\n", fNumScratchTexturesReused);
    out->appendf("Number of Scratch MSAA Attachments reused %d\n",
                 fNumScratchMSAAAttachmentsReused);
    out->appendf("Number of Render Passes: %d\n", fRenderPasses);
    out->appendf("Reordered DAGs Over Budget: %d\n", fNumReorderedDAGsOverBudget);

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

void GrGpu::Stats::dumpKeyValuePairs(TArray<SkString>* keys, TArray<double>* values) {
    keys->push_back(SkString("render_passes"));
    values->push_back(fRenderPasses);
    keys->push_back(SkString("reordered_dags_over_budget"));
    values->push_back(fNumReorderedDAGsOverBudget);
}

#endif // GR_GPU_STATS
#endif // defined(GR_TEST_UTILS)

bool GrGpu::CompressedDataIsCorrect(SkISize dimensions,
                                    SkTextureCompressionType compressionType,
                                    skgpu::Mipmapped mipmapped,
                                    const void* data,
                                    size_t length) {
    size_t computedSize = SkCompressedDataSize(
            compressionType, dimensions, nullptr, mipmapped == skgpu::Mipmapped::kYes);
    return computedSize == length;
}

GrBackendTexture GrGpu::createBackendTexture(SkISize dimensions,
                                             const GrBackendFormat& format,
                                             GrRenderable renderable,
                                             skgpu::Mipmapped mipmapped,
                                             GrProtected isProtected,
                                             std::string_view label) {
    const GrCaps* caps = this->caps();

    if (!format.isValid()) {
        return {};
    }

    if (caps->isFormatCompressed(format)) {
        // Compressed formats must go through the createCompressedBackendTexture API
        return {};
    }

    if (dimensions.isEmpty() || dimensions.width()  > caps->maxTextureSize() ||
                                dimensions.height() > caps->maxTextureSize()) {
        return {};
    }

    if (mipmapped == skgpu::Mipmapped::kYes && !this->caps()->mipmapSupport()) {
        return {};
    }

    return this->onCreateBackendTexture(
            dimensions, format, renderable, mipmapped, isProtected, label);
}

bool GrGpu::clearBackendTexture(const GrBackendTexture& backendTexture,
                                sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                std::array<float, 4> color) {
    if (!backendTexture.isValid()) {
        return false;
    }

    if (backendTexture.hasMipmaps() && !this->caps()->mipmapSupport()) {
        return false;
    }

    return this->onClearBackendTexture(backendTexture, std::move(finishedCallback), color);
}

GrBackendTexture GrGpu::createCompressedBackendTexture(SkISize dimensions,
                                                       const GrBackendFormat& format,
                                                       skgpu::Mipmapped mipmapped,
                                                       GrProtected isProtected) {
    const GrCaps* caps = this->caps();

    if (!format.isValid()) {
        return {};
    }

    SkTextureCompressionType compressionType = GrBackendFormatToCompressionType(format);
    if (compressionType == SkTextureCompressionType::kNone) {
        // Uncompressed formats must go through the createBackendTexture API
        return {};
    }

    if (dimensions.isEmpty() ||
        dimensions.width()  > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        return {};
    }

    if (mipmapped == skgpu::Mipmapped::kYes && !this->caps()->mipmapSupport()) {
        return {};
    }

    return this->onCreateCompressedBackendTexture(dimensions, format, mipmapped, isProtected);
}

bool GrGpu::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                           sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                           const void* data,
                                           size_t length) {
    SkASSERT(data);

    if (!backendTexture.isValid()) {
        return false;
    }

    GrBackendFormat format = backendTexture.getBackendFormat();

    SkTextureCompressionType compressionType = GrBackendFormatToCompressionType(format);
    if (compressionType == SkTextureCompressionType::kNone) {
        // Uncompressed formats must go through the createBackendTexture API
        return false;
    }

    if (backendTexture.hasMipmaps() && !this->caps()->mipmapSupport()) {
        return false;
    }

    skgpu::Mipmapped mipmapped =
            backendTexture.hasMipmaps() ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    if (!CompressedDataIsCorrect(backendTexture.dimensions(),
                                 compressionType,
                                 mipmapped,
                                 data,
                                 length)) {
        return false;
    }

    return this->onUpdateCompressedBackendTexture(backendTexture,
                                                  std::move(finishedCallback),
                                                  data,
                                                  length);
}
