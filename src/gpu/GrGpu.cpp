/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpu.h"

#include "GrBuffer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrMesh.h"
#include "GrPathRendering.h"
#include "GrPipeline.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"
#include "GrStencilSettings.h"
#include "GrSurfacePriv.h"
#include "GrTexturePriv.h"
#include "SkMathPriv.h"

GrMesh& GrMesh::operator =(const GrMesh& di) {
    fPrimitiveType  = di.fPrimitiveType;
    fStartVertex    = di.fStartVertex;
    fStartIndex     = di.fStartIndex;
    fVertexCount    = di.fVertexCount;
    fIndexCount     = di.fIndexCount;

    fInstanceCount          = di.fInstanceCount;
    fVerticesPerInstance    = di.fVerticesPerInstance;
    fIndicesPerInstance     = di.fIndicesPerInstance;
    fMaxInstancesPerDraw    = di.fMaxInstancesPerDraw;

    fVertexBuffer.reset(di.vertexBuffer());
    fIndexBuffer.reset(di.indexBuffer());

    return *this;
}

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
                                          const GrSamplerParams& textureParams,
                                          GrTextureProducer::CopyParams* copyParams,
                                          SkScalar scaleAdjust[2]) const {
    const GrCaps& caps = *this->caps();
    if (textureParams.isTiled() && !caps.npotTextureTileSupport() &&
        (!SkIsPow2(width) || !SkIsPow2(height))) {
        SkASSERT(scaleAdjust);
        copyParams->fWidth = GrNextPow2(width);
        copyParams->fHeight = GrNextPow2(height);
        scaleAdjust[0] = ((SkScalar) copyParams->fWidth) / width;
        scaleAdjust[1] = ((SkScalar) copyParams->fHeight) / height;
        switch (textureParams.filterMode()) {
            case GrSamplerParams::kNone_FilterMode:
                copyParams->fFilter = GrSamplerParams::kNone_FilterMode;
                break;
            case GrSamplerParams::kBilerp_FilterMode:
            case GrSamplerParams::kMipMap_FilterMode:
                // We are only ever scaling up so no reason to ever indicate kMipMap.
                copyParams->fFilter = GrSamplerParams::kBilerp_FilterMode;
                break;
        }
        return true;
    }
    return false;
}

static GrSurfaceOrigin resolve_origin(GrSurfaceOrigin origin, bool renderTarget) {
    // By default, GrRenderTargets are GL's normal orientation so that they
    // can be drawn to by the outside world without the client having
    // to render upside down.
    if (kDefault_GrSurfaceOrigin == origin) {
        return renderTarget ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
    } else {
        return origin;
    }
}

/**
 * Prior to creating a texture, make sure the type of texture being created is
 * supported by calling check_texture_creation_params.
 *
 * @param caps The capabilities of the GL device.
 * @param desc The descriptor of the texture to create.
 * @param isRT Indicates if the texture can be a render target.
 */
static bool check_texture_creation_params(const GrCaps& caps, const GrSurfaceDesc& desc,
                                          bool* isRT, const SkTArray<GrMipLevel>& texels) {
    if (!caps.isConfigTexturable(desc.fConfig)) {
        return false;
    }

    if (GrPixelConfigIsSint(desc.fConfig) && texels.count() > 1) {
        return false;
    }

    *isRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (*isRT && !caps.isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return false;
    }

    // We currently do not support multisampled textures
    if (!*isRT && desc.fSampleCnt > 0) {
        return false;
    }

    if (*isRT) {
        int maxRTSize = caps.maxRenderTargetSize();
        if (desc.fWidth > maxRTSize || desc.fHeight > maxRTSize) {
            return false;
        }
    } else {
        int maxSize = caps.maxTextureSize();
        if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
            return false;
        }
    }

    for (int i = 0; i < texels.count(); ++i) {
        if (!texels[i].fPixels) {
            return false;
        }
    }
    return true;
}

GrTexture* GrGpu::createTexture(const GrSurfaceDesc& origDesc, SkBudgeted budgeted,
                                const SkTArray<GrMipLevel>& texels) {
    GrSurfaceDesc desc = origDesc;

    const GrCaps* caps = this->caps();
    bool isRT = false;
    bool textureCreationParamsValid = check_texture_creation_params(*caps, desc, &isRT, texels);
    if (!textureCreationParamsValid) {
        return nullptr;
    }

    desc.fSampleCnt = SkTMin(desc.fSampleCnt, caps->maxSampleCount());
    // Attempt to catch un- or wrongly intialized sample counts;
    SkASSERT(desc.fSampleCnt >= 0 && desc.fSampleCnt <= 64);

    desc.fOrigin = resolve_origin(desc.fOrigin, isRT);

    GrTexture* tex = nullptr;

    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        // We shouldn't be rendering into this
        SkASSERT(!isRT);
        SkASSERT(0 == desc.fSampleCnt);

        if (!caps->npotTextureTileSupport() &&
            (!SkIsPow2(desc.fWidth) || !SkIsPow2(desc.fHeight))) {
            return nullptr;
        }

        this->handleDirtyContext();
        tex = this->onCreateCompressedTexture(desc, budgeted, texels);
    } else {
        this->handleDirtyContext();
        tex = this->onCreateTexture(desc, budgeted, texels);
    }
    if (tex) {
        if (!caps->reuseScratchTextures() && !isRT) {
            tex->resourcePriv().removeScratchKey();
        }
        fStats.incTextureCreates();
        if (!texels.empty()) {
            if (texels[0].fPixels) {
                fStats.incTextureUploads();
            }
        }
    }
    return tex;
}

sk_sp<GrTexture> GrGpu::wrapBackendTexture(const GrBackendTextureDesc& desc,
                                           GrWrapOwnership ownership) {
    this->handleDirtyContext();
    if (!this->caps()->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }
    if ((desc.fFlags & kRenderTarget_GrBackendTextureFlag) &&
        !this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }
    int maxSize = this->caps()->maxTextureSize();
    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }
    sk_sp<GrTexture> tex = this->onWrapBackendTexture(desc, ownership);
    if (!tex) {
        return nullptr;
    }
    // TODO: defer this and attach dynamically
    GrRenderTarget* tgt = tex->asRenderTarget();
    if (tgt && !fContext->resourceProvider()->attachStencilAttachment(tgt)) {
        return nullptr;
    }
    return tex;
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc) {
    if (!this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }
    this->handleDirtyContext();
    return this->onWrapBackendRenderTarget(desc);
}

sk_sp<GrRenderTarget> GrGpu::wrapBackendTextureAsRenderTarget(const GrBackendTextureDesc& desc) {
    this->handleDirtyContext();
    if (!(desc.fFlags & kRenderTarget_GrBackendTextureFlag)) {
      return nullptr;
    }
    if (!this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }
    int maxSize = this->caps()->maxTextureSize();
    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }
    return this->onWrapBackendTextureAsRenderTarget(desc);
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

gr_instanced::InstancedRendering* GrGpu::createInstancedRendering() {
    SkASSERT(GrCaps::InstancedSupport::kNone != this->caps()->instancedSupport());
    return this->onCreateInstancedRendering();
}

bool GrGpu::copySurface(GrSurface* dst,
                        GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint) {
    SkASSERT(dst && src);
    this->handleDirtyContext();
    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(dst->config()) != GrPixelConfigIsSint(src->config())) {
        return false;
    }
    if (GrPixelConfigIsCompressed(dst->config())) {
        return false;
    }
    return this->onCopySurface(dst, src, srcRect, dstPoint);
}

bool GrGpu::getReadPixelsInfo(GrSurface* srcSurface, int width, int height, size_t rowBytes,
                              GrPixelConfig readConfig, DrawPreference* drawPreference,
                              ReadPixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(drawPreference);
    SkASSERT(tempDrawInfo);
    SkASSERT(srcSurface);
    SkASSERT(kGpuPrefersDraw_DrawPreference != *drawPreference);

    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(srcSurface->config()) != GrPixelConfigIsSint(readConfig)) {
        return false;
    }

    // We currently do not support reading into a compressed buffer
    if (GrPixelConfigIsCompressed(readConfig)) {
        return false;
    }

    // We currently do not support reading into the packed formats 565 or 4444 as they are not
    // required to have read back support on all devices and backends.
    if (kRGB_565_GrPixelConfig == readConfig || kRGBA_4444_GrPixelConfig == readConfig) {
        return false;
    }

    if (!this->onGetReadPixelsInfo(srcSurface, width, height, rowBytes, readConfig, drawPreference,
                                   tempDrawInfo)) {
        return false;
    }

    // Check to see if we're going to request that the caller draw when drawing is not possible.
    if (!srcSurface->asTexture() ||
        !this->caps()->isConfigRenderable(tempDrawInfo->fTempSurfaceDesc.fConfig, false)) {
        // If we don't have a fallback to a straight read then fail.
        if (kRequireDraw_DrawPreference == *drawPreference) {
            return false;
        }
        *drawPreference = kNoDraw_DrawPreference;
    }

    return true;
}
bool GrGpu::getWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                               GrPixelConfig srcConfig, DrawPreference* drawPreference,
                               WritePixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(drawPreference);
    SkASSERT(tempDrawInfo);
    SkASSERT(dstSurface);
    SkASSERT(kGpuPrefersDraw_DrawPreference != *drawPreference);

    if (GrPixelConfigIsCompressed(dstSurface->desc().fConfig) &&
        dstSurface->desc().fConfig != srcConfig) {
        return false;
    }

    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(dstSurface->config()) != GrPixelConfigIsSint(srcConfig)) {
        return false;
    }

    if (SkToBool(dstSurface->asRenderTarget())) {
        if (this->caps()->useDrawInsteadOfAllRenderTargetWrites()) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        } else if (this->caps()->useDrawInsteadOfPartialRenderTargetWrite() &&
                   (width < dstSurface->width() || height < dstSurface->height())) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
    }

    if (!this->onGetWritePixelsInfo(dstSurface, width, height, srcConfig, drawPreference,
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

bool GrGpu::readPixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrPixelConfig config, void* buffer,
                       size_t rowBytes) {
    SkASSERT(surface);

    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(surface->config()) != GrPixelConfigIsSint(config)) {
        return false;
    }

    // We cannot read pixels into a compressed buffer
    if (GrPixelConfigIsCompressed(config)) {
        return false;
    }

    size_t bpp = GrBytesPerPixel(config);
    if (!GrSurfacePriv::AdjustReadPixelParams(surface->width(), surface->height(), bpp,
                                              &left, &top, &width, &height,
                                              &buffer,
                                              &rowBytes)) {
        return false;
    }

    this->handleDirtyContext();

    return this->onReadPixels(surface,
                              left, top, width, height,
                              config, buffer,
                              rowBytes);
}

bool GrGpu::writePixels(GrSurface* surface,
                        int left, int top, int width, int height,
                        GrPixelConfig config, const SkTArray<GrMipLevel>& texels) {
    SkASSERT(surface);
    for (int currentMipLevel = 0; currentMipLevel < texels.count(); currentMipLevel++) {
        if (!texels[currentMipLevel].fPixels ) {
            return false;
        }
    }

    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(surface->config()) != GrPixelConfigIsSint(config)) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onWritePixels(surface, left, top, width, height, config, texels)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(surface, &rect, texels.count());
        fStats.incTextureUploads();
        return true;
    }
    return false;
}

bool GrGpu::writePixels(GrSurface* surface,
                        int left, int top, int width, int height,
                        GrPixelConfig config, const void* buffer,
                        size_t rowBytes) {
    GrMipLevel mipLevel;
    mipLevel.fPixels = buffer;
    mipLevel.fRowBytes = rowBytes;
    SkSTArray<1, GrMipLevel> texels;
    texels.push_back(mipLevel);

    return this->writePixels(surface, left, top, width, height, config, texels);
}

bool GrGpu::transferPixels(GrSurface* surface,
                           int left, int top, int width, int height,
                           GrPixelConfig config, GrBuffer* transferBuffer,
                           size_t offset, size_t rowBytes, GrFence* fence) {
    SkASSERT(transferBuffer);
    SkASSERT(fence);

    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(surface->config()) != GrPixelConfigIsSint(config)) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onTransferPixels(surface, left, top, width, height, config,
                               transferBuffer, offset, rowBytes)) {
        SkIRect rect = SkIRect::MakeXYWH(left, top, width, height);
        this->didWriteToSurface(surface, &rect);
        fStats.incTransfersToTexture();

        if (*fence) {
            this->deleteFence(*fence);
        }
        *fence = this->insertFence();

        return true;
    }
    return false;
}

void GrGpu::resolveRenderTarget(GrRenderTarget* target) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target);
}

void GrGpu::didWriteToSurface(GrSurface* surface, const SkIRect* bounds, uint32_t mipLevels) const {
    SkASSERT(surface);
    // Mark any MIP chain and resolve buffer as dirty if and only if there is a non-empty bounds.
    if (nullptr == bounds || !bounds->isEmpty()) {
        if (GrRenderTarget* target = surface->asRenderTarget()) {
            target->flagAsNeedingResolve(bounds);
        }
        GrTexture* texture = surface->asTexture();
        if (texture && 1 == mipLevels) {
            texture->texturePriv().dirtyMipMaps(true);
        }
    }
}

const GrGpu::MultisampleSpecs& GrGpu::queryMultisampleSpecs(const GrPipeline& pipeline) {
    GrRenderTarget* rt = pipeline.getRenderTarget();
    SkASSERT(rt->desc().fSampleCnt > 1);

    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        // TODO: attach stencil and create settings during render target flush.
        SkASSERT(rt->renderTargetPriv().getStencilAttachment());
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                      rt->renderTargetPriv().numStencilBits());
    }

    int effectiveSampleCnt;
    SkSTArray<16, SkPoint, true> pattern;
    this->onQueryMultisampleSpecs(rt, stencil, &effectiveSampleCnt, &pattern);
    SkASSERT(effectiveSampleCnt >= rt->desc().fSampleCnt);

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
