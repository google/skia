
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpu.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrIndexBuffer.h"
#include "GrPathRendering.h"
#include "GrPipeline.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"
#include "GrSurfacePriv.h"
#include "GrVertexBuffer.h"
#include "GrVertices.h"

GrVertices& GrVertices::operator =(const GrVertices& di) {
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
}

GrGpu::~GrGpu() {}

void GrGpu::contextAbandoned() {}

////////////////////////////////////////////////////////////////////////////////

bool GrGpu::makeCopyForTextureParams(int width, int height, const GrTextureParams& textureParams,
                                     GrTextureProducer::CopyParams* copyParams) const {
    const GrCaps& caps = *this->caps();
    if (textureParams.isTiled() && !caps.npotTextureTileSupport() &&
        (!SkIsPow2(width) || !SkIsPow2(height))) {
        copyParams->fWidth = GrNextPow2(width);
        copyParams->fHeight = GrNextPow2(height);
        switch (textureParams.filterMode()) {
            case GrTextureParams::kNone_FilterMode:
                copyParams->fFilter = GrTextureParams::kNone_FilterMode;
                break;
            case GrTextureParams::kBilerp_FilterMode:
            case GrTextureParams::kMipMap_FilterMode:
                // We are only ever scaling up so no reason to ever indicate kMipMap.
                copyParams->fFilter = GrTextureParams::kBilerp_FilterMode;
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

GrTexture* GrGpu::createTexture(const GrSurfaceDesc& origDesc, bool budgeted,
                                const void* srcData, size_t rowBytes) {
    GrSurfaceDesc desc = origDesc;

    if (!this->caps()->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    bool isRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (isRT && !this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }

    // We currently do not support multisampled textures
    if (!isRT && desc.fSampleCnt > 0) {
        return nullptr;
    }

    GrTexture *tex = nullptr;

    if (isRT) {
        int maxRTSize = this->caps()->maxRenderTargetSize();
        if (desc.fWidth > maxRTSize || desc.fHeight > maxRTSize) {
            return nullptr;
        }
    } else {
        int maxSize = this->caps()->maxTextureSize();
        if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
            return nullptr;
        }
    }

    GrGpuResource::LifeCycle lifeCycle = budgeted ? GrGpuResource::kCached_LifeCycle :
                                                    GrGpuResource::kUncached_LifeCycle;

    desc.fSampleCnt = SkTMin(desc.fSampleCnt, this->caps()->maxSampleCount());
    // Attempt to catch un- or wrongly initialized sample counts;
    SkASSERT(desc.fSampleCnt >= 0 && desc.fSampleCnt <= 64);

    desc.fOrigin = resolve_origin(desc.fOrigin, isRT);

    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        // We shouldn't be rendering into this
        SkASSERT(!isRT);
        SkASSERT(0 == desc.fSampleCnt);

        if (!this->caps()->npotTextureTileSupport() &&
            (!SkIsPow2(desc.fWidth) || !SkIsPow2(desc.fHeight))) {
            return nullptr;
        }

        this->handleDirtyContext();
        tex = this->onCreateCompressedTexture(desc, lifeCycle, srcData);
    } else {
        this->handleDirtyContext();
        tex = this->onCreateTexture(desc, lifeCycle, srcData, rowBytes);
    }
    if (!this->caps()->reuseScratchTextures() && !isRT) {
        tex->resourcePriv().removeScratchKey();
    }
    if (tex) {
        fStats.incTextureCreates();
        if (srcData) {
            fStats.incTextureUploads();
        }
    }
    return tex;
}

GrTexture* GrGpu::wrapBackendTexture(const GrBackendTextureDesc& desc, GrWrapOwnership ownership) {
    this->handleDirtyContext();
    GrTexture* tex = this->onWrapBackendTexture(desc, ownership);
    if (nullptr == tex) {
        return nullptr;
    }
    // TODO: defer this and attach dynamically
    GrRenderTarget* tgt = tex->asRenderTarget();
    if (tgt && !fContext->resourceProvider()->attachStencilAttachment(tgt)) {
        tex->unref();
        return nullptr;
    } else {
        return tex;
    }
}

GrRenderTarget* GrGpu::wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc,
                                               GrWrapOwnership ownership) {
    this->handleDirtyContext();
    return this->onWrapBackendRenderTarget(desc, ownership);
}

GrVertexBuffer* GrGpu::createVertexBuffer(size_t size, bool dynamic) {
    this->handleDirtyContext();
    GrVertexBuffer* vb = this->onCreateVertexBuffer(size, dynamic);
    if (!this->caps()->reuseScratchBuffers()) {
        vb->resourcePriv().removeScratchKey();
    }
    return vb;
}

GrIndexBuffer* GrGpu::createIndexBuffer(size_t size, bool dynamic) {
    this->handleDirtyContext();
    GrIndexBuffer* ib = this->onCreateIndexBuffer(size, dynamic);
    if (!this->caps()->reuseScratchBuffers()) {
        ib->resourcePriv().removeScratchKey();
    }
    return ib;
}

void GrGpu::clear(const SkIRect& rect,
                  GrColor color,
                  GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    SkASSERT(SkIRect::MakeWH(renderTarget->width(), renderTarget->height()).contains(rect));
    this->handleDirtyContext();
    this->onClear(renderTarget, rect, color);
}

void GrGpu::clearStencilClip(const SkIRect& rect,
                             bool insideClip,
                             GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    this->handleDirtyContext();
    this->onClearStencilClip(renderTarget, rect, insideClip);
}

bool GrGpu::copySurface(GrSurface* dst,
                        GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint) {
    SkASSERT(dst && src);
    this->handleDirtyContext();
    return this->onCopySurface(dst, src, srcRect, dstPoint);
}

bool GrGpu::getReadPixelsInfo(GrSurface* srcSurface, int width, int height, size_t rowBytes,
                              GrPixelConfig readConfig, DrawPreference* drawPreference,
                              ReadPixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(drawPreference);
    SkASSERT(tempDrawInfo);
    SkASSERT(kGpuPrefersDraw_DrawPreference != *drawPreference);

    // We currently do not support reading into a compressed buffer
    if (GrPixelConfigIsCompressed(readConfig)) {
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
bool GrGpu::getWritePixelsInfo(GrSurface* dstSurface, int width, int height, size_t rowBytes,
                               GrPixelConfig srcConfig, DrawPreference* drawPreference,
                               WritePixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(drawPreference);
    SkASSERT(tempDrawInfo);
    SkASSERT(kGpuPrefersDraw_DrawPreference != *drawPreference);

    if (GrPixelConfigIsCompressed(dstSurface->desc().fConfig) &&
        dstSurface->desc().fConfig != srcConfig) {
        return false;
    }

    if (this->caps()->useDrawInsteadOfPartialRenderTargetWrite() &&
        SkToBool(dstSurface->asRenderTarget()) &&
        (width < dstSurface->width() || height < dstSurface->height())) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    if (!this->onGetWritePixelsInfo(dstSurface, width, height, rowBytes, srcConfig, drawPreference,
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
    this->handleDirtyContext();

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

    return this->onReadPixels(surface,
                              left, top, width, height,
                              config, buffer,
                              rowBytes);
}

bool GrGpu::writePixels(GrSurface* surface,
                        int left, int top, int width, int height,
                        GrPixelConfig config, const void* buffer,
                        size_t rowBytes) {
    if (!buffer) {
        return false;
    }

    this->handleDirtyContext();
    if (this->onWritePixels(surface, left, top, width, height, config, buffer, rowBytes)) {
        fStats.incTextureUploads();
        return true;
    }
    return false;
}

void GrGpu::resolveRenderTarget(GrRenderTarget* target) {
    SkASSERT(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target);
}

////////////////////////////////////////////////////////////////////////////////

void GrGpu::draw(const DrawArgs& args, const GrVertices& vertices) {
    this->handleDirtyContext();
    if (GrXferBarrierType barrierType = args.fPipeline->xferBarrierType(*this->caps())) {
        this->xferBarrier(args.fPipeline->getRenderTarget(), barrierType);
    }

    GrVertices::Iterator iter;
    const GrNonInstancedVertices* verts = iter.init(vertices);
    do {
        this->onDraw(args, *verts);
        fStats.incNumDraws();
    } while ((verts = iter.next()));
}
