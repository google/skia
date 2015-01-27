
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpu.h"

#include "GrBufferAllocPool.h"
#include "GrContext.h"
#include "GrDrawTargetCaps.h"
#include "GrIndexBuffer.h"
#include "GrResourceCache2.h"
#include "GrStencilBuffer.h"
#include "GrVertexBuffer.h"

////////////////////////////////////////////////////////////////////////////////

GrGpu::GrGpu(GrContext* context)
    : fResetTimestamp(kExpiredTimestamp+1)
    , fResetBits(kAll_GrBackendState)
    , fQuadIndexBuffer(NULL)
    , fContext(context) {
}

GrGpu::~GrGpu() {
    SkSafeSetNull(fQuadIndexBuffer);
}

void GrGpu::contextAbandoned() {}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrGpu::createTexture(const GrSurfaceDesc& desc, bool budgeted,
                                const void* srcData, size_t rowBytes) {
    if (!this->caps()->isConfigTexturable(desc.fConfig)) {
        return NULL;
    }

    bool isRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (isRT && !this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return NULL;
    }

    GrTexture *tex = NULL;
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        // We shouldn't be rendering into this
        SkASSERT((desc.fFlags & kRenderTarget_GrSurfaceFlag) == 0);

        if (!this->caps()->npotTextureTileSupport() &&
            (!SkIsPow2(desc.fWidth) || !SkIsPow2(desc.fHeight))) {
            return NULL;
        }

        this->handleDirtyContext();
        tex = this->onCreateCompressedTexture(desc, budgeted, srcData);
    } else {
        this->handleDirtyContext();
        tex = this->onCreateTexture(desc, budgeted, srcData, rowBytes);
        if (tex &&
            (kRenderTarget_GrSurfaceFlag & desc.fFlags) &&
            !(kNoStencil_GrSurfaceFlag & desc.fFlags)) {
            SkASSERT(tex->asRenderTarget());
            // TODO: defer this and attach dynamically
            if (!this->attachStencilBufferToRenderTarget(tex->asRenderTarget(), budgeted)) {
                tex->unref();
                return NULL;
            }
        }
    }
    if (!this->caps()->reuseScratchTextures() && !isRT) {
        tex->cacheAccess().removeScratchKey();
    }
    return tex;
}

bool GrGpu::attachStencilBufferToRenderTarget(GrRenderTarget* rt, bool budgeted) {
    SkASSERT(NULL == rt->getStencilBuffer());
    GrScratchKey sbKey;
    GrStencilBuffer::ComputeKey(rt->width(), rt->height(), rt->numSamples(), &sbKey);
    SkAutoTUnref<GrStencilBuffer> sb(static_cast<GrStencilBuffer*>(
        this->getContext()->getResourceCache2()->findAndRefScratchResource(sbKey)));
    if (sb) {
        rt->setStencilBuffer(sb);
        bool attached = this->attachStencilBufferToRenderTarget(sb, rt);
        if (!attached) {
            rt->setStencilBuffer(NULL);
        }
        return attached;
    }
    if (this->createStencilBufferForRenderTarget(rt, budgeted, rt->width(), rt->height())) {
        // Right now we're clearing the stencil buffer here after it is
        // attached to an RT for the first time. When we start matching
        // stencil buffers with smaller color targets this will no longer
        // be correct because it won't be guaranteed to clear the entire
        // sb.
        // We used to clear down in the GL subclass using a special purpose
        // FBO. But iOS doesn't allow a stencil-only FBO. It reports unsupported
        // FBO status.
        this->clearStencil(rt);
        return true;
    } else {
        return false;
    }
}

GrTexture* GrGpu::wrapBackendTexture(const GrBackendTextureDesc& desc) {
    this->handleDirtyContext();
    GrTexture* tex = this->onWrapBackendTexture(desc);
    if (NULL == tex) {
        return NULL;
    }
    // TODO: defer this and attach dynamically
    GrRenderTarget* tgt = tex->asRenderTarget();
    if (tgt &&
        !this->attachStencilBufferToRenderTarget(tgt, true /*budgeted*/)) {
        tex->unref();
        return NULL;
    } else {
        return tex;
    }
}

GrRenderTarget* GrGpu::wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc) {
    this->handleDirtyContext();
    return this->onWrapBackendRenderTarget(desc);
}

GrVertexBuffer* GrGpu::createVertexBuffer(size_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->onCreateVertexBuffer(size, dynamic);
}

GrIndexBuffer* GrGpu::createIndexBuffer(size_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->onCreateIndexBuffer(size, dynamic);
}

GrIndexBuffer* GrGpu::createInstancedIndexBuffer(const uint16_t* pattern,
                                                 int patternSize,
                                                 int reps,
                                                 int vertCount,
                                                 bool isDynamic) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);
    GrGpu* me = const_cast<GrGpu*>(this);
    GrIndexBuffer* buffer = me->createIndexBuffer(bufferSize, isDynamic);
    if (buffer) {
        uint16_t* data = (uint16_t*) buffer->map();
        bool useTempData = (NULL == data);
        if (useTempData) {
            data = SkNEW_ARRAY(uint16_t, reps * patternSize);
        }
        for (int i = 0; i < reps; ++i) {
            int baseIdx = i * patternSize;
            uint16_t baseVert = (uint16_t)(i * vertCount);
            for (int j = 0; j < patternSize; ++j) {
                data[baseIdx+j] = baseVert + pattern[j];
            }
        }
        if (useTempData) {
            if (!buffer->updateData(data, bufferSize)) {
                SkFAIL("Can't get indices into buffer!");
            }
            SkDELETE_ARRAY(data);
        } else {
            buffer->unmap();
        }
    }
    return buffer;
}

void GrGpu::clear(const SkIRect* rect,
                  GrColor color,
                  bool canIgnoreRect,
                  GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    this->handleDirtyContext();
    this->onClear(renderTarget, rect, color, canIgnoreRect);
}

void GrGpu::clearStencilClip(const SkIRect& rect,
                             bool insideClip,
                             GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    this->handleDirtyContext();
    this->onClearStencilClip(renderTarget, rect, insideClip);
}

bool GrGpu::readPixels(GrRenderTarget* target,
                       int left, int top, int width, int height,
                       GrPixelConfig config, void* buffer,
                       size_t rowBytes) {
    this->handleDirtyContext();
    return this->onReadPixels(target, left, top, width, height,
                              config, buffer, rowBytes);
}

bool GrGpu::writeTexturePixels(GrTexture* texture,
                               int left, int top, int width, int height,
                               GrPixelConfig config, const void* buffer,
                               size_t rowBytes) {
    this->handleDirtyContext();
    return this->onWriteTexturePixels(texture, left, top, width, height,
                                      config, buffer, rowBytes);
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

static const int MAX_QUADS = 1 << 12; // max possible: (1 << 14) - 1;

GR_STATIC_ASSERT(4 * MAX_QUADS <= 65535);

static const uint16_t gQuadIndexPattern[] = {
  0, 1, 2, 0, 2, 3
};

const GrIndexBuffer* GrGpu::getQuadIndexBuffer() const {
    if (NULL == fQuadIndexBuffer || fQuadIndexBuffer->wasDestroyed()) {
        SkSafeUnref(fQuadIndexBuffer);
        GrGpu* me = const_cast<GrGpu*>(this);
        fQuadIndexBuffer = me->createInstancedIndexBuffer(gQuadIndexPattern,
                                                          6,
                                                          MAX_QUADS,
                                                          4);
    }

    return fQuadIndexBuffer;
}

////////////////////////////////////////////////////////////////////////////////

void GrGpu::draw(const DrawArgs& args, const GrDrawTarget::DrawInfo& info) {
    this->handleDirtyContext();
    this->onDraw(args, info);
}

void GrGpu::stencilPath(const GrPath* path, const StencilPathState& state) {
    this->handleDirtyContext();
    this->onStencilPath(path, state);
}

void GrGpu::drawPath(const DrawArgs& args,
                     const GrPath* path,
                     const GrStencilSettings& stencilSettings) {
    this->handleDirtyContext();
    this->onDrawPath(args, path, stencilSettings);
}

void GrGpu::drawPaths(const DrawArgs& args,
                      const GrPathRange* pathRange,
                      const void* indices,
                      GrDrawTarget::PathIndexType indexType,
                      const float transformValues[],
                      GrDrawTarget::PathTransformType transformType,
                      int count,
                      const GrStencilSettings& stencilSettings) {
    this->handleDirtyContext();
    pathRange->willDrawPaths(indices, indexType, count);
    this->onDrawPaths(args, pathRange, indices, indexType, transformValues,
                      transformType, count, stencilSettings);
}
