
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
#include "GrResourceCache.h"
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
    , fGpuTraceMarkerCount(0)
    , fContext(context) {
}

GrGpu::~GrGpu() {}

void GrGpu::contextAbandoned() {}

////////////////////////////////////////////////////////////////////////////////

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
        return NULL;
    }

    bool isRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (isRT && !this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return NULL;
    }

    // We currently not support multisampled textures
    if (!isRT && desc.fSampleCnt > 0) {
        return NULL;
    }

    GrTexture *tex = NULL;

    if (isRT) {
        int maxRTSize = this->caps()->maxRenderTargetSize();
        if (desc.fWidth > maxRTSize || desc.fHeight > maxRTSize) {
            return NULL;
        }
    } else {
        int maxSize = this->caps()->maxTextureSize();
        if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
            return NULL;
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
            return NULL;
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

bool GrGpu::attachStencilAttachmentToRenderTarget(GrRenderTarget* rt) {
    SkASSERT(NULL == rt->renderTargetPriv().getStencilAttachment());
    GrUniqueKey sbKey;

    int width = rt->width();
    int height = rt->height();
#if 0
    if (this->caps()->oversizedStencilSupport()) {
        width  = SkNextPow2(width);
        height = SkNextPow2(height);
    }
#endif

    GrStencilAttachment::ComputeSharedStencilAttachmentKey(width, height,
        rt->numStencilSamples(), &sbKey);
    SkAutoTUnref<GrStencilAttachment> sb(static_cast<GrStencilAttachment*>(
        this->getContext()->getResourceCache()->findAndRefUniqueResource(sbKey)));
    if (sb) {
        if (this->attachStencilAttachmentToRenderTarget(sb, rt)) {
            rt->renderTargetPriv().didAttachStencilAttachment(sb);
            return true;
        }
        return false;
    }
    if (this->createStencilAttachmentForRenderTarget(rt, width, height)) {
        // Right now we're clearing the stencil buffer here after it is
        // attached to an RT for the first time. When we start matching
        // stencil buffers with smaller color targets this will no longer
        // be correct because it won't be guaranteed to clear the entire
        // sb.
        // We used to clear down in the GL subclass using a special purpose
        // FBO. But iOS doesn't allow a stencil-only FBO. It reports unsupported
        // FBO status.
        this->clearStencil(rt);
        GrStencilAttachment* sb = rt->renderTargetPriv().getStencilAttachment();
        sb->resourcePriv().setUniqueKey(sbKey);
        return true;
    } else {
        return false;
    }
}

GrTexture* GrGpu::wrapBackendTexture(const GrBackendTextureDesc& desc, GrWrapOwnership ownership) {
    this->handleDirtyContext();
    GrTexture* tex = this->onWrapBackendTexture(desc, ownership);
    if (NULL == tex) {
        return NULL;
    }
    // TODO: defer this and attach dynamically
    GrRenderTarget* tgt = tex->asRenderTarget();
    if (tgt && !this->attachStencilAttachmentToRenderTarget(tgt)) {
        tex->unref();
        return NULL;
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

typedef GrTraceMarkerSet::Iter TMIter;
void GrGpu::saveActiveTraceMarkers() {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(0 == fStoredTraceMarkers.count());
        fStoredTraceMarkers.addSet(fActiveTraceMarkers);
        for (TMIter iter = fStoredTraceMarkers.begin(); iter != fStoredTraceMarkers.end(); ++iter) {
            this->removeGpuTraceMarker(&(*iter));
        }
    }
}

void GrGpu::restoreActiveTraceMarkers() {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(0 == fActiveTraceMarkers.count());
        for (TMIter iter = fStoredTraceMarkers.begin(); iter != fStoredTraceMarkers.end(); ++iter) {
            this->addGpuTraceMarker(&(*iter));
        }
        for (TMIter iter = fActiveTraceMarkers.begin(); iter != fActiveTraceMarkers.end(); ++iter) {
            this->fStoredTraceMarkers.remove(*iter);
        }
    }
}

void GrGpu::addGpuTraceMarker(const GrGpuTraceMarker* marker) {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(fGpuTraceMarkerCount >= 0);
        this->fActiveTraceMarkers.add(*marker);
        this->didAddGpuTraceMarker();
        ++fGpuTraceMarkerCount;
    }
}

void GrGpu::removeGpuTraceMarker(const GrGpuTraceMarker* marker) {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(fGpuTraceMarkerCount >= 1);
        this->fActiveTraceMarkers.remove(*marker);
        this->didRemoveGpuTraceMarker();
        --fGpuTraceMarkerCount;
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrGpu::draw(const DrawArgs& args, const GrVertices& vertices) {
    this->handleDirtyContext();
    GrVertices::Iterator iter;
    const GrNonInstancedVertices* verts = iter.init(vertices);
    do {
        this->onDraw(args, *verts);
    } while ((verts = iter.next()));
}
