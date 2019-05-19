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
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
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

GrGpu::GrGpu(GrContext* context)
    : fResetTimestamp(kExpiredTimestamp+1)
    , fResetBits(kAll_GrBackendState)
    , fContext(context) {
}

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

sk_sp<GrTexture> GrGpu::createTexture(const GrSurfaceDesc& origDesc, SkBudgeted budgeted,
                                      const GrMipLevel texels[], int mipLevelCount) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrGpu", "createTexture", fContext);
    GrSurfaceDesc desc = origDesc;

    GrMipMapped mipMapped = mipLevelCount > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
    if (!this->caps()->validateSurfaceDesc(desc, mipMapped)) {
        return nullptr;
    }

    bool isRT = desc.fFlags & kRenderTarget_GrSurfaceFlag;
    if (isRT) {
        desc.fSampleCnt = this->caps()->getRenderTargetSampleCount(desc.fSampleCnt, desc.fConfig);
    }
    // Attempt to catch un- or wrongly initialized sample counts.
    SkASSERT(desc.fSampleCnt > 0 && desc.fSampleCnt <= 64);

    if (mipLevelCount && (desc.fFlags & kPerformInitialClear_GrSurfaceFlag)) {
        return nullptr;
    }

    // We shouldn't be rendering into compressed textures
    SkASSERT(!GrPixelConfigIsCompressed(desc.fConfig) || !isRT);
    SkASSERT(!GrPixelConfigIsCompressed(desc.fConfig) || 1 == desc.fSampleCnt);

    this->handleDirtyContext();
    sk_sp<GrTexture> tex = this->onCreateTexture(desc, budgeted, texels, mipLevelCount);
    if (tex) {
        if (!this->caps()->reuseScratchTextures() && !isRT) {
            tex->resourcePriv().removeScratchKey();
        }
        fStats.incTextureCreates();
        if (mipLevelCount) {
            if (texels[0].fPixels) {
                fStats.incTextureUploads();
            }
        }
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted) {
    return this->createTexture(desc, budgeted, nullptr, 0);
}

sk_sp<GrTexture> GrGpu::wrapBackendTexture(const GrBackendTexture& backendTex,
                                           GrWrapOwnership ownership, GrWrapCacheable cacheable,
                                           GrIOType ioType) {
    SkASSERT(ioType != kWrite_GrIOType);
    this->handleDirtyContext();
    SkASSERT(this->caps());
    if (!this->caps()->isConfigTexturable(backendTex.config())) {
        return nullptr;
    }
    if (backendTex.width() > this->caps()->maxTextureSize() ||
        backendTex.height() > this->caps()->maxTextureSize()) {
        return nullptr;
    }
    return this->onWrapBackendTexture(backendTex, ownership, cacheable, ioType);
}

sk_sp<GrTexture> GrGpu::wrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                     int sampleCnt, GrWrapOwnership ownership,
                                                     GrWrapCacheable cacheable) {
    this->handleDirtyContext();
    if (sampleCnt < 1) {
        return nullptr;
    }
    if (!this->caps()->isConfigTexturable(backendTex.config()) ||
        !this->caps()->getRenderTargetSampleCount(sampleCnt, backendTex.config())) {
        return nullptr;
    }

    if (backendTex.width() > this->caps()->maxRenderTargetSize() ||
        backendTex.height() > this->caps()->maxRenderTargetSize()) {
        return nullptr;
    }
    sk_sp<GrTexture> tex =
            this->onWrapRenderableBackendTexture(backendTex, sampleCnt, ownership, cacheable);
    SkASSERT(!tex || tex->asRenderTarget());
    return tex;
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    if (0 == this->caps()->getRenderTargetSampleCount(backendRT.sampleCnt(), backendRT.config())) {
        return nullptr;
    }
    this->handleDirtyContext();
    return this->onWrapBackendRenderTarget(backendRT);
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                              int sampleCnt) {
    if (0 == this->caps()->getRenderTargetSampleCount(sampleCnt, tex.config())) {
        return nullptr;
    }
    int maxSize = this->caps()->maxTextureSize();
    if (tex.width() > maxSize || tex.height() > maxSize) {
        return nullptr;
    }
    this->handleDirtyContext();
    return this->onWrapBackendTextureAsRenderTarget(tex, sampleCnt);
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
    this->handleDirtyContext();
    sk_sp<GrGpuBuffer> buffer = this->onCreateBuffer(size, intendedType, accessPattern, data);
    if (!this->caps()->reuseScratchBuffers()) {
        buffer->resourcePriv().removeScratchKey();
    }
    return buffer;
}

bool GrGpu::copySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                        GrSurface* src, GrSurfaceOrigin srcOrigin,
                        const SkIRect& srcRect, const SkIPoint& dstPoint,
                        bool canDiscardOutsideDstRect) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrGpu", "copySurface", fContext);
    SkASSERT(dst && src);

    if (dst->readOnly()) {
        return false;
    }

    this->handleDirtyContext();

    return this->onCopySurface(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint,
                               canDiscardOutsideDstRect);
}

bool GrGpu::readPixels(GrSurface* surface, int left, int top, int width, int height,
                       GrColorType dstColorType, void* buffer, size_t rowBytes) {
    SkASSERT(surface);

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    if (!GrSurfacePriv::AdjustReadPixelParams(surface->width(), surface->height(), bpp,
                                              &left, &top, &width, &height,
                                              &buffer,
                                              &rowBytes)) {
        return false;
    }

    if (GrPixelConfigIsCompressed(surface->config())) {
        return false;
    }

    this->handleDirtyContext();

    return this->onReadPixels(surface, left, top, width, height, dstColorType, buffer, rowBytes);
}

bool GrGpu::writePixels(GrSurface* surface, int left, int top, int width, int height,
                        GrColorType srcColorType, const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(surface);

    if (surface->readOnly()) {
        return false;
    }

    if (1 == mipLevelCount) {
        // We require that if we are not mipped, then the write region is contained in the surface
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(surface->width(), surface->height());
        if (!bounds.contains(subRect)) {
            return false;
        }
    } else if (0 != left || 0 != top || width != surface->width() || height != surface->height()) {
        // We require that if the texels are mipped, than the write region is the entire surface
        return false;
    }

    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (!texels[currentMipLevel].fPixels ) {
            return false;
        }
    }

    this->handleDirtyContext();
    if (this->onWritePixels(surface, left, top, width, height, srcColorType, texels,
                            mipLevelCount)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(surface, kTopLeft_GrSurfaceOrigin, &rect, mipLevelCount);
        fStats.incTextureUploads();
        return true;
    }
    return false;
}

bool GrGpu::transferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                             GrColorType bufferColorType, GrGpuBuffer* transferBuffer,
                             size_t offset, size_t rowBytes) {
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

    this->handleDirtyContext();
    if (this->onTransferPixelsTo(texture, left, top, width, height, bufferColorType, transferBuffer,
                                 offset, rowBytes)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(texture, kTopLeft_GrSurfaceOrigin, &rect);
        fStats.incTransfersToTexture();

        return true;
    }
    return false;
}

bool GrGpu::transferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                               GrColorType bufferColorType, GrGpuBuffer* transferBuffer,
                               size_t offset) {
    SkASSERT(surface);
    SkASSERT(transferBuffer);
    SkASSERT(this->caps()->transferFromOffsetAlignment(bufferColorType));
    SkASSERT(offset % this->caps()->transferFromOffsetAlignment(bufferColorType) == 0);

    // We require that the write region is contained in the texture
    SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
    SkIRect bounds = SkIRect::MakeWH(surface->width(), surface->height());
    if (!bounds.contains(subRect)) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onTransferPixelsFrom(surface, left, top, width, height, bufferColorType,
                                   transferBuffer, offset)) {
        fStats.incTransfersFromSurface();
        return true;
    }
    return false;
}

bool GrGpu::regenerateMipMapLevels(GrTexture* texture) {
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

void GrGpu::resolveRenderTarget(GrRenderTarget* target) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target);
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
    SkASSERT(renderTarget->numStencilSamples() > 1);

    SkSTArray<16, SkPoint> sampleLocations;
    this->querySampleLocations(renderTarget, &sampleLocations);
    return fSamplePatternDictionary.findOrAssignSamplePatternKey(sampleLocations);
}

GrSemaphoresSubmitted GrGpu::finishFlush(GrSurfaceProxy* proxies[],
                                         int n,
                                         SkSurface::BackendSurfaceAccess access,
                                         const GrFlushInfo& info,
                                         const GrPrepareForExternalIORequests& externalRequests) {
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

GrBackendTexture GrGpu::createTestingOnlyBackendTexture(int w, int h, SkColorType colorType,
                                                        GrMipMapped mipMapped,
                                                        GrRenderable renderable,
                                                        const void* pixels, size_t rowBytes) {
    GrBackendFormat format = this->caps()->getBackendFormatFromColorType(colorType);
    if (!format.isValid()) {
        return GrBackendTexture();
    }

    return this->createTestingOnlyBackendTexture(w, h, format, mipMapped, renderable,
                                                 pixels, rowBytes);
}

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
}

void GrGpu::Stats::dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) {
    keys->push_back(SkString("render_target_binds")); values->push_back(fRenderTargetBinds);
    keys->push_back(SkString("shader_compilations")); values->push_back(fShaderCompilations);
}

#endif

#endif
