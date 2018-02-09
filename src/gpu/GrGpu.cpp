/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpu.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrBuffer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrMesh.h"
#include "GrPathRendering.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrStencilAttachment.h"
#include "GrStencilSettings.h"
#include "GrSurfacePriv.h"
#include "GrTexturePriv.h"
#include "GrTracing.h"
#include "SkJSONWriter.h"
#include "SkMathPriv.h"

////////////////////////////////////////////////////////////////////////////////

GrGpu::GrGpu(GrContext* context)
    : fResetTimestamp(kExpiredTimestamp+1)
    , fResetBits(kAll_GrBackendState)
    , fContext(context) {
    fMultisampleSpecs.emplace_back(0, 0, nullptr); // Index 0 is an invalid unique id.
}

GrGpu::~GrGpu() {}

void GrGpu::disconnect(DisconnectType) {}

////////////////////////////////////////////////////////////////////////////////

bool GrGpu::isACopyNeededForTextureParams(int width, int height,
                                          const GrSamplerState& textureParams,
                                          GrTextureProducer::CopyParams* copyParams,
                                          SkScalar scaleAdjust[2]) const {
    const GrCaps& caps = *this->caps();
    if (textureParams.isRepeated() && !caps.npotTextureTileSupport() &&
        (!SkIsPow2(width) || !SkIsPow2(height))) {
        SkASSERT(scaleAdjust);
        copyParams->fWidth = GrNextPow2(width);
        copyParams->fHeight = GrNextPow2(height);
        scaleAdjust[0] = ((SkScalar) copyParams->fWidth) / width;
        scaleAdjust[1] = ((SkScalar) copyParams->fHeight) / height;
        switch (textureParams.filter()) {
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
                                           GrWrapOwnership ownership) {
    this->handleDirtyContext();
    if (!this->caps()->isConfigTexturable(backendTex.config())) {
        return nullptr;
    }
    if (backendTex.width() > this->caps()->maxTextureSize() ||
        backendTex.height() > this->caps()->maxTextureSize()) {
        return nullptr;
    }
    sk_sp<GrTexture> tex = this->onWrapBackendTexture(backendTex, ownership);
    if (!tex) {
        return nullptr;
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::wrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                     int sampleCnt, GrWrapOwnership ownership) {
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
    sk_sp<GrTexture> tex = this->onWrapRenderableBackendTexture(backendTex, sampleCnt, ownership);
    if (!tex) {
        return nullptr;
    }
    SkASSERT(tex->asRenderTarget());
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

GrBuffer* GrGpu::createBuffer(size_t size, GrBufferType intendedType,
                              GrAccessPattern accessPattern, const void* data) {
    this->handleDirtyContext();
    GrBuffer* buffer = this->onCreateBuffer(size, intendedType, accessPattern, data);
    if (!this->caps()->reuseScratchBuffers()) {
        buffer->resourcePriv().removeScratchKey();
    }
    return buffer;
}

bool GrGpu::copySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                        GrSurface* src, GrSurfaceOrigin srcOrigin,
                        const SkIRect& srcRect, const SkIPoint& dstPoint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrGpu", "copySurface", fContext);
    SkASSERT(dst && src);
    this->handleDirtyContext();
    return this->onCopySurface(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint);
}

bool GrGpu::getReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                              int width, int height, size_t rowBytes,
                              GrPixelConfig readConfig, DrawPreference* drawPreference,
                              ReadPixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(drawPreference);
    SkASSERT(tempDrawInfo);
    SkASSERT(srcSurface);
    SkASSERT(kGpuPrefersDraw_DrawPreference != *drawPreference);

    // Default values for intermediate draws. The intermediate texture config matches the src's
    // config, is approx sized to the read rect, no swizzling or spoofing of the dst config.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 1;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcSurface->config();
    tempDrawInfo->fTempSurfaceFit = SkBackingFit::kApprox;
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();
    tempDrawInfo->fReadConfig = readConfig;

    // We currently do not support reading into the packed formats 565 or 4444 as they are not
    // required to have read back support on all devices and backends.
    if (kRGB_565_GrPixelConfig == readConfig || kRGBA_4444_GrPixelConfig == readConfig) {
        return false;
    }

   if (!this->onGetReadPixelsInfo(srcSurface, srcOrigin, width, height, rowBytes, readConfig,
                                  drawPreference, tempDrawInfo)) {
        return false;
    }

    // Check to see if we're going to request that the caller draw when drawing is not possible.
    if (!srcSurface->asTexture() ||
        !this->caps()->isConfigRenderable(tempDrawInfo->fTempSurfaceDesc.fConfig)) {
        // If we don't have a fallback to a straight read then fail.
        if (kRequireDraw_DrawPreference == *drawPreference) {
            return false;
        }
        *drawPreference = kNoDraw_DrawPreference;
    }

    return true;
}
bool GrGpu::getWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                               int width, int height,
                               GrPixelConfig srcConfig, DrawPreference* drawPreference,
                               WritePixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(drawPreference);
    SkASSERT(tempDrawInfo);
    SkASSERT(dstSurface);
    SkASSERT(kGpuPrefersDraw_DrawPreference != *drawPreference);

    // Default values for intermediate draws. The intermediate texture config matches the dst's
    // config, is approx sized to the write rect, no swizzling or sppofing of the src config.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kNone_GrSurfaceFlags;
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcConfig;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 1;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();
    tempDrawInfo->fWriteConfig = srcConfig;

    if (!this->onGetWritePixelsInfo(dstSurface, dstOrigin, width, height, srcConfig, drawPreference,
                                    tempDrawInfo)) {
        return false;
    }

    // Check to see if we're going to request that the caller draw when drawing is not possible.
    if (!dstSurface->asRenderTarget() ||
        !this->caps()->isConfigTexturable(tempDrawInfo->fTempSurfaceDesc.fConfig)) {
        // If we don't have a fallback to a straight upload then fail.
        if (kRequireDraw_DrawPreference == *drawPreference ||
            !this->caps()->isConfigTexturable(srcConfig)) {
            return false;
        }
        *drawPreference = kNoDraw_DrawPreference;
    }
    return true;
}

bool GrGpu::readPixels(GrSurface* surface, GrSurfaceOrigin origin,
                       int left, int top, int width, int height,
                       GrPixelConfig config, void* buffer,
                       size_t rowBytes) {
    SkASSERT(surface);

    size_t bpp = GrBytesPerPixel(config);
    if (!GrSurfacePriv::AdjustReadPixelParams(surface->width(), surface->height(), bpp,
                                              &left, &top, &width, &height,
                                              &buffer,
                                              &rowBytes)) {
        return false;
    }

    this->handleDirtyContext();

    return this->onReadPixels(surface, origin,
                              left, top, width, height,
                              config, buffer,
                              rowBytes);
}

bool GrGpu::writePixels(GrSurface* surface, GrSurfaceOrigin origin,
                        int left, int top, int width, int height,
                        GrPixelConfig config, const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(surface);
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
    if (this->onWritePixels(surface, origin, left, top, width, height, config,
                            texels, mipLevelCount)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(surface, origin, &rect, mipLevelCount);
        fStats.incTextureUploads();
        return true;
    }
    return false;
}

bool GrGpu::writePixels(GrSurface* surface, GrSurfaceOrigin origin,
                        int left, int top, int width, int height,
                        GrPixelConfig config, const void* buffer,
                        size_t rowBytes) {
    GrMipLevel mipLevel = { buffer, rowBytes };

    return this->writePixels(surface, origin, left, top, width, height, config, &mipLevel, 1);
}

bool GrGpu::transferPixels(GrTexture* texture,
                           int left, int top, int width, int height,
                           GrPixelConfig config, GrBuffer* transferBuffer,
                           size_t offset, size_t rowBytes) {
    SkASSERT(transferBuffer);

    // We require that the write region is contained in the texture
    SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
    SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
    if (!bounds.contains(subRect)) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onTransferPixels(texture, left, top, width, height, config,
                               transferBuffer, offset, rowBytes)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(texture, kTopLeft_GrSurfaceOrigin, &rect);
        fStats.incTransfersToTexture();

        return true;
    }
    return false;
}

void GrGpu::resolveRenderTarget(GrRenderTarget* target) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target);
}

void GrGpu::didWriteToSurface(GrSurface* surface, GrSurfaceOrigin origin, const SkIRect* bounds,
                              uint32_t mipLevels) const {
    SkASSERT(surface);
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

const GrGpu::MultisampleSpecs& GrGpu::queryMultisampleSpecs(const GrPipeline& pipeline) {
    GrRenderTarget* rt = pipeline.renderTarget();
    SkASSERT(rt->numStencilSamples() > 1);

    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        // TODO: attach stencil and create settings during render target flush.
        SkASSERT(rt->renderTargetPriv().getStencilAttachment());
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                      rt->renderTargetPriv().numStencilBits());
    }

    int effectiveSampleCnt;
    SkSTArray<16, SkPoint, true> pattern;
    this->onQueryMultisampleSpecs(rt, pipeline.proxy()->origin(), stencil,
                                  &effectiveSampleCnt, &pattern);
    SkASSERT(effectiveSampleCnt >= rt->numStencilSamples());

    uint8_t id;
    if (this->caps()->sampleLocationsSupport()) {
        SkASSERT(pattern.count() == effectiveSampleCnt);
        const auto& insertResult = fMultisampleSpecsIdMap.insert(
            MultisampleSpecsIdMap::value_type(pattern, SkTMin(fMultisampleSpecs.count(), 255)));
        id = insertResult.first->second;
        if (insertResult.second) {
            // This means the insert did not find the pattern in the map already, and therefore an
            // actual insertion took place. (We don't expect to see many unique sample patterns.)
            const SkPoint* sampleLocations = insertResult.first->first.begin();
            SkASSERT(id == fMultisampleSpecs.count());
            fMultisampleSpecs.emplace_back(id, effectiveSampleCnt, sampleLocations);
        }
    } else {
        id = effectiveSampleCnt;
        for (int i = fMultisampleSpecs.count(); i <= id; ++i) {
            fMultisampleSpecs.emplace_back(i, i, nullptr);
        }
    }
    SkASSERT(id > 0);

    return fMultisampleSpecs[id];
}

bool GrGpu::SamplePatternComparator::operator()(const SamplePattern& a,
                                                const SamplePattern& b) const {
    if (a.count() != b.count()) {
        return a.count() < b.count();
    }
    for (int i = 0; i < a.count(); ++i) {
        // This doesn't have geometric meaning. We just need to define an ordering for std::map.
        if (a[i].x() != b[i].x()) {
            return a[i].x() < b[i].x();
        }
        if (a[i].y() != b[i].y()) {
            return a[i].y() < b[i].y();
        }
    }
    return false; // Equal.
}

GrSemaphoresSubmitted GrGpu::finishFlush(int numSemaphores,
                                         GrBackendSemaphore backendSemaphores[]) {
    GrResourceProvider* resourceProvider = fContext->contextPriv().resourceProvider();

    if (this->caps()->fenceSyncSupport()) {
        for (int i = 0; i < numSemaphores; ++i) {
            sk_sp<GrSemaphore> semaphore;
            if (backendSemaphores[i].isInitialized()) {
                semaphore = resourceProvider->wrapBackendSemaphore(
                        backendSemaphores[i], GrResourceProvider::SemaphoreWrapType::kWillSignal,
                        kBorrow_GrWrapOwnership);
            } else {
                semaphore = resourceProvider->makeSemaphore(false);
            }
            this->insertSemaphore(semaphore, false);

            if (!backendSemaphores[i].isInitialized()) {
                semaphore->setBackendSemaphore(&backendSemaphores[i]);
            }
        }
    }
    this->onFinishFlush((numSemaphores > 0 && this->caps()->fenceSyncSupport()));
    return this->caps()->fenceSyncSupport() ? GrSemaphoresSubmitted::kYes
                                            : GrSemaphoresSubmitted::kNo;
}

void GrGpu::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    // TODO: Is there anything useful in the base class to dump here?

    this->onDumpJSON(writer);

    writer->endObject();
}
