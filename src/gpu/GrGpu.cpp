
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
#include "GrStencilBuffer.h"
#include "GrVertexBuffer.h"

// probably makes no sense for this to be less than a page
static const size_t VERTEX_POOL_VB_SIZE = 1 << 18;
static const int VERTEX_POOL_VB_COUNT = 4;
static const size_t INDEX_POOL_IB_SIZE = 1 << 16;
static const int INDEX_POOL_IB_COUNT = 4;

////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INVAL_BUFFER    0xdeadcafe
#define DEBUG_INVAL_START_IDX -1

GrGpu::GrGpu(GrContext* context)
    : INHERITED(context)
    , fResetTimestamp(kExpiredTimestamp+1)
    , fResetBits(kAll_GrBackendState)
    , fVertexPool(NULL)
    , fIndexPool(NULL)
    , fVertexPoolUseCnt(0)
    , fIndexPoolUseCnt(0)
    , fQuadIndexBuffer(NULL) {
    fGeomPoolStateStack.push_back();
#ifdef SK_DEBUG
    GeometryPoolState& poolState = fGeomPoolStateStack.back();
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
    poolState.fPoolStartVertex = DEBUG_INVAL_START_IDX;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
    poolState.fPoolStartIndex = DEBUG_INVAL_START_IDX;
#endif
}

GrGpu::~GrGpu() {
    SkSafeSetNull(fQuadIndexBuffer);
    delete fVertexPool;
    fVertexPool = NULL;
    delete fIndexPool;
    fIndexPool = NULL;
}

void GrGpu::contextAbandoned() {}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrGpu::createTexture(const GrSurfaceDesc& desc,
                                const void* srcData, size_t rowBytes) {
    if (!this->caps()->isConfigTexturable(desc.fConfig)) {
        return NULL;
    }

    if ((desc.fFlags & kRenderTarget_GrSurfaceFlag) &&
        !this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
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
        tex = this->onCreateCompressedTexture(desc, srcData);
    } else {
        this->handleDirtyContext();
        tex = this->onCreateTexture(desc, srcData, rowBytes);
        if (tex &&
            (kRenderTarget_GrSurfaceFlag & desc.fFlags) &&
            !(kNoStencil_GrSurfaceFlag & desc.fFlags)) {
            SkASSERT(tex->asRenderTarget());
            // TODO: defer this and attach dynamically
            if (!this->attachStencilBufferToRenderTarget(tex->asRenderTarget())) {
                tex->unref();
                return NULL;
            }
        }
    }
    return tex;
}

bool GrGpu::attachStencilBufferToRenderTarget(GrRenderTarget* rt) {
    SkASSERT(NULL == rt->getStencilBuffer());
    GrStencilBuffer* sb =
        this->getContext()->findStencilBuffer(rt->width(),
                                              rt->height(),
                                              rt->numSamples());
    if (sb) {
        rt->setStencilBuffer(sb);
        bool attached = this->attachStencilBufferToRenderTarget(sb, rt);
        if (!attached) {
            rt->setStencilBuffer(NULL);
        }
        return attached;
    }
    if (this->createStencilBufferForRenderTarget(rt,
                                                 rt->width(), rt->height())) {
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
        !this->attachStencilBufferToRenderTarget(tgt)) {
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

void GrGpu::onClear(const SkIRect* rect,
                    GrColor color,
                    bool canIgnoreRect,
                    GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    this->handleDirtyContext();
    this->onGpuClear(renderTarget, rect, color, canIgnoreRect);
}

void GrGpu::clearStencilClip(const SkIRect& rect,
                             bool insideClip,
                             GrRenderTarget* renderTarget) {
    if (NULL == renderTarget) {
        renderTarget = this->getDrawState().getRenderTarget();
    }
    if (NULL == renderTarget) {
        SkASSERT(0);
        return;
    }
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

bool GrGpu::setupClipAndFlushState(DrawType type,
                                   const GrDeviceCoordTexture* dstCopy,
                                   const SkRect* devBounds,
                                   GrDrawState::AutoRestoreEffects* are,
                                   GrDrawState::AutoRestoreStencil* ars) {
    GrClipMaskManager::ScissorState scissorState;
    if (!fClipMaskManager.setupClipping(this->getClip(),
                                        devBounds,
                                        are,
                                        ars,
                                        &scissorState)) {
        return false;
    }

    if (!this->flushGraphicsState(type, scissorState, dstCopy)) {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

void GrGpu::geometrySourceWillPush() {
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    if (kReserved_GeometrySrcType == geoSrc.fVertexSrc) {
        this->finalizeReservedVertices();
    }
    if (kReserved_GeometrySrcType == geoSrc.fIndexSrc) {
        this->finalizeReservedIndices();
    }
    GeometryPoolState& newState = fGeomPoolStateStack.push_back();
#ifdef SK_DEBUG
    newState.fPoolVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
    newState.fPoolStartVertex = DEBUG_INVAL_START_IDX;
    newState.fPoolIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
    newState.fPoolStartIndex = DEBUG_INVAL_START_IDX;
#else
    (void) newState; // silence compiler warning
#endif
}

void GrGpu::geometrySourceWillPop(const GeometrySrcState& restoredState) {
    // if popping last entry then pops are unbalanced with pushes
    SkASSERT(fGeomPoolStateStack.count() > 1);
    fGeomPoolStateStack.pop_back();
}

void GrGpu::onDraw(const DrawInfo& info) {
    this->handleDirtyContext();
    GrDrawState::AutoRestoreEffects are;
    GrDrawState::AutoRestoreStencil ars;
    if (!this->setupClipAndFlushState(PrimTypeToDrawType(info.primitiveType()),
                                      info.getDstCopy(),
                                      info.getDevBounds(),
                                      &are,
                                      &ars)) {
        return;
    }
    this->onGpuDraw(info);
}


// TODO hack
static const GrStencilSettings& winding_path_stencil_settings() {
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kIncClamp_StencilOp,
        kIncClamp_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0xFFFF, 0xFFFF, 0xFFFF);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}

static const GrStencilSettings& even_odd_path_stencil_settings() {
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kInvert_StencilOp,
        kInvert_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0xFFFF, 0xFFFF, 0xFFFF);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}

static void get_path_stencil_settings_for_filltype(GrPathRendering::FillType fill,
                                                   GrStencilSettings* outStencilSettings) {

    switch (fill) {
        default:
            SkFAIL("Unexpected path fill.");
        case GrPathRendering::kWinding_FillType:
            *outStencilSettings = winding_path_stencil_settings();
            break;
        case GrPathRendering::kEvenOdd_FillType:
            *outStencilSettings = even_odd_path_stencil_settings();
            break;
    }
}

void GrGpu::onStencilPath(const GrPath* path, GrPathRendering::FillType fill) {
    this->handleDirtyContext();

    GrDrawState::AutoRestoreEffects are;
    GrDrawState::AutoRestoreStencil ars;
    if (!this->setupClipAndFlushState(kStencilPath_DrawType, NULL, NULL, &are, &ars)) {
        return;
    }

    GrStencilSettings stencilSettings;
    get_path_stencil_settings_for_filltype(fill, &stencilSettings);
    fClipMaskManager.adjustPathStencilParams(&stencilSettings);

    this->pathRendering()->stencilPath(path, stencilSettings);
}


void GrGpu::onDrawPath(const GrPath* path, GrPathRendering::FillType fill,
                       const GrDeviceCoordTexture* dstCopy) {
    this->handleDirtyContext();

    drawState()->setDefaultVertexAttribs();

    GrDrawState::AutoRestoreEffects are;
    GrDrawState::AutoRestoreStencil ars;
    if (!this->setupClipAndFlushState(kDrawPath_DrawType, dstCopy, NULL, &are, &ars)) {
        return;
    }

    GrStencilSettings stencilSettings;
    get_path_stencil_settings_for_filltype(fill, &stencilSettings);
    fClipMaskManager.adjustPathStencilParams(&stencilSettings);

    this->pathRendering()->drawPath(path, stencilSettings);
}

void GrGpu::onDrawPaths(const GrPathRange* pathRange,
                        const uint32_t indices[], int count,
                        const float transforms[], PathTransformType transformsType,
                        GrPathRendering::FillType fill, const GrDeviceCoordTexture* dstCopy) {
    this->handleDirtyContext();

    drawState()->setDefaultVertexAttribs();

    GrDrawState::AutoRestoreEffects are;
    GrDrawState::AutoRestoreStencil ars;
    if (!this->setupClipAndFlushState(kDrawPaths_DrawType, dstCopy, NULL, &are, &ars)) {
        return;
    }

    GrStencilSettings stencilSettings;
    get_path_stencil_settings_for_filltype(fill, &stencilSettings);
    fClipMaskManager.adjustPathStencilParams(&stencilSettings);

    pathRange->willDrawPaths(indices, count);
    this->pathRendering()->drawPaths(pathRange, indices, count, transforms, transformsType,
                                     stencilSettings);
}

void GrGpu::finalizeReservedVertices() {
    SkASSERT(fVertexPool);
    fVertexPool->unmap();
}

void GrGpu::finalizeReservedIndices() {
    SkASSERT(fIndexPool);
    fIndexPool->unmap();
}

void GrGpu::prepareVertexPool() {
    if (NULL == fVertexPool) {
        SkASSERT(0 == fVertexPoolUseCnt);
        fVertexPool = SkNEW_ARGS(GrVertexBufferAllocPool, (this, true,
                                                  VERTEX_POOL_VB_SIZE,
                                                  VERTEX_POOL_VB_COUNT));
        fVertexPool->releaseGpuRef();
    } else if (!fVertexPoolUseCnt) {
        // the client doesn't have valid data in the pool
        fVertexPool->reset();
    }
}

void GrGpu::prepareIndexPool() {
    if (NULL == fIndexPool) {
        SkASSERT(0 == fIndexPoolUseCnt);
        fIndexPool = SkNEW_ARGS(GrIndexBufferAllocPool, (this, true,
                                                INDEX_POOL_IB_SIZE,
                                                INDEX_POOL_IB_COUNT));
        fIndexPool->releaseGpuRef();
    } else if (!fIndexPoolUseCnt) {
        // the client doesn't have valid data in the pool
        fIndexPool->reset();
    }
}

bool GrGpu::onReserveVertexSpace(size_t vertexSize,
                                 int vertexCount,
                                 void** vertices) {
    GeometryPoolState& geomPoolState = fGeomPoolStateStack.back();

    SkASSERT(vertexCount > 0);
    SkASSERT(vertices);

    this->prepareVertexPool();

    *vertices = fVertexPool->makeSpace(vertexSize,
                                       vertexCount,
                                       &geomPoolState.fPoolVertexBuffer,
                                       &geomPoolState.fPoolStartVertex);
    if (NULL == *vertices) {
        return false;
    }
    ++fVertexPoolUseCnt;
    return true;
}

bool GrGpu::onReserveIndexSpace(int indexCount, void** indices) {
    GeometryPoolState& geomPoolState = fGeomPoolStateStack.back();

    SkASSERT(indexCount > 0);
    SkASSERT(indices);

    this->prepareIndexPool();

    *indices = fIndexPool->makeSpace(indexCount,
                                     &geomPoolState.fPoolIndexBuffer,
                                     &geomPoolState.fPoolStartIndex);
    if (NULL == *indices) {
        return false;
    }
    ++fIndexPoolUseCnt;
    return true;
}

void GrGpu::releaseReservedVertexSpace() {
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fVertexSrc);
    size_t bytes = geoSrc.fVertexCount * geoSrc.fVertexSize;
    fVertexPool->putBack(bytes);
    --fVertexPoolUseCnt;
}

void GrGpu::releaseReservedIndexSpace() {
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fIndexSrc);
    size_t bytes = geoSrc.fIndexCount * sizeof(uint16_t);
    fIndexPool->putBack(bytes);
    --fIndexPoolUseCnt;
}
