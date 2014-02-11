
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
    : GrDrawTarget(context)
    , fResetTimestamp(kExpiredTimestamp+1)
    , fResetBits(kAll_GrBackendState)
    , fVertexPool(NULL)
    , fIndexPool(NULL)
    , fVertexPoolUseCnt(0)
    , fIndexPoolUseCnt(0)
    , fQuadIndexBuffer(NULL) {

    fClipMaskManager.setGpu(this);

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
    this->releaseResources();
}

void GrGpu::abandonResources() {

    fClipMaskManager.releaseResources();

    while (NULL != fResourceList.head()) {
        fResourceList.head()->abandon();
    }

    SkASSERT(NULL == fQuadIndexBuffer || !fQuadIndexBuffer->isValid());
    SkSafeSetNull(fQuadIndexBuffer);
    delete fVertexPool;
    fVertexPool = NULL;
    delete fIndexPool;
    fIndexPool = NULL;
}

void GrGpu::releaseResources() {

    fClipMaskManager.releaseResources();

    while (NULL != fResourceList.head()) {
        fResourceList.head()->release();
    }

    SkASSERT(NULL == fQuadIndexBuffer || !fQuadIndexBuffer->isValid());
    SkSafeSetNull(fQuadIndexBuffer);
    delete fVertexPool;
    fVertexPool = NULL;
    delete fIndexPool;
    fIndexPool = NULL;
}

void GrGpu::insertResource(GrResource* resource) {
    SkASSERT(NULL != resource);
    SkASSERT(this == resource->getGpu());

    fResourceList.addToHead(resource);
}

void GrGpu::removeResource(GrResource* resource) {
    SkASSERT(NULL != resource);
    SkASSERT(this == resource->getGpu());

    fResourceList.remove(resource);
}


void GrGpu::unimpl(const char msg[]) {
#ifdef SK_DEBUG
    GrPrintf("--- GrGpu unimplemented(\"%s\")\n", msg);
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrGpu::createTexture(const GrTextureDesc& desc,
                                const void* srcData, size_t rowBytes) {
    if (kUnknown_GrPixelConfig == desc.fConfig) {
        return NULL;
    }
    if ((desc.fFlags & kRenderTarget_GrTextureFlagBit) &&
        !this->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return NULL;
    }

    this->handleDirtyContext();
    GrTexture* tex = this->onCreateTexture(desc, srcData, rowBytes);
    if (NULL != tex &&
        (kRenderTarget_GrTextureFlagBit & desc.fFlags) &&
        !(kNoStencil_GrTextureFlagBit & desc.fFlags)) {
        SkASSERT(NULL != tex->asRenderTarget());
        // TODO: defer this and attach dynamically
        if (!this->attachStencilBufferToRenderTarget(tex->asRenderTarget())) {
            tex->unref();
            return NULL;
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
    if (NULL != sb) {
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
        GrDrawState::AutoRenderTargetRestore artr(this->drawState(), rt);
        this->clearStencil();
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
    if (NULL != tgt &&
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

GrPath* GrGpu::createPath(const SkPath& path, const SkStrokeRec& stroke) {
    SkASSERT(this->caps()->pathRenderingSupport());
    this->handleDirtyContext();
    return this->onCreatePath(path, stroke);
}

void GrGpu::clear(const SkIRect* rect,
                  GrColor color,
                  bool canIgnoreRect,
                  GrRenderTarget* renderTarget) {
    GrDrawState::AutoRenderTargetRestore art;
    if (NULL != renderTarget) {
        art.set(this->drawState(), renderTarget);
    }
    if (NULL == this->getDrawState().getRenderTarget()) {
        SkASSERT(0);
        return;
    }
    this->handleDirtyContext();
    this->onClear(rect, color, canIgnoreRect);
}

void GrGpu::forceRenderTargetFlush() {
    this->handleDirtyContext();
    this->onForceRenderTargetFlush();
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

void GrGpu::getPathStencilSettingsForFillType(SkPath::FillType fill, GrStencilSettings* outStencilSettings) {

    switch (fill) {
        default:
            GrCrash("Unexpected path fill.");
            /* fallthrough */;
        case SkPath::kWinding_FillType:
        case SkPath::kInverseWinding_FillType:
            *outStencilSettings = winding_path_stencil_settings();
            break;
        case SkPath::kEvenOdd_FillType:
        case SkPath::kInverseEvenOdd_FillType:
            *outStencilSettings = even_odd_path_stencil_settings();
            break;
    }
    fClipMaskManager.adjustPathStencilParams(outStencilSettings);
}


////////////////////////////////////////////////////////////////////////////////

static const int MAX_QUADS = 1 << 12; // max possible: (1 << 14) - 1;

GR_STATIC_ASSERT(4 * MAX_QUADS <= 65535);

static inline void fill_indices(uint16_t* indices, int quadCount) {
    for (int i = 0; i < quadCount; ++i) {
        indices[6 * i + 0] = 4 * i + 0;
        indices[6 * i + 1] = 4 * i + 1;
        indices[6 * i + 2] = 4 * i + 2;
        indices[6 * i + 3] = 4 * i + 0;
        indices[6 * i + 4] = 4 * i + 2;
        indices[6 * i + 5] = 4 * i + 3;
    }
}

const GrIndexBuffer* GrGpu::getQuadIndexBuffer() const {
    if (NULL == fQuadIndexBuffer) {
        static const int SIZE = sizeof(uint16_t) * 6 * MAX_QUADS;
        GrGpu* me = const_cast<GrGpu*>(this);
        fQuadIndexBuffer = me->createIndexBuffer(SIZE, false);
        if (NULL != fQuadIndexBuffer) {
            uint16_t* indices = (uint16_t*)fQuadIndexBuffer->lock();
            if (NULL != indices) {
                fill_indices(indices, MAX_QUADS);
                fQuadIndexBuffer->unlock();
            } else {
                indices = (uint16_t*)sk_malloc_throw(SIZE);
                fill_indices(indices, MAX_QUADS);
                if (!fQuadIndexBuffer->updateData(indices, SIZE)) {
                    fQuadIndexBuffer->unref();
                    fQuadIndexBuffer = NULL;
                    GrCrash("Can't get indices into buffer!");
                }
                sk_free(indices);
            }
        }
    }

    return fQuadIndexBuffer;
}

////////////////////////////////////////////////////////////////////////////////

bool GrGpu::setupClipAndFlushState(DrawType type, const GrDeviceCoordTexture* dstCopy,
                                   GrDrawState::AutoRestoreEffects* are,
                                   const SkRect* devBounds) {
    if (!fClipMaskManager.setupClipping(this->getClip(), are, devBounds)) {
        return false;
    }

    if (!this->flushGraphicsState(type, dstCopy)) {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

void GrGpu::geometrySourceWillPush() {
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    if (kArray_GeometrySrcType == geoSrc.fVertexSrc ||
        kReserved_GeometrySrcType == geoSrc.fVertexSrc) {
        this->finalizeReservedVertices();
    }
    if (kArray_GeometrySrcType == geoSrc.fIndexSrc ||
        kReserved_GeometrySrcType == geoSrc.fIndexSrc) {
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
    if (!this->setupClipAndFlushState(PrimTypeToDrawType(info.primitiveType()),
                                      info.getDstCopy(), &are, info.getDevBounds())) {
        return;
    }
    this->onGpuDraw(info);
}

void GrGpu::onStencilPath(const GrPath* path, SkPath::FillType fill) {
    this->handleDirtyContext();

    GrDrawState::AutoRestoreEffects are;
    if (!this->setupClipAndFlushState(kStencilPath_DrawType, NULL, &are, NULL)) {
        return;
    }

    this->onGpuStencilPath(path, fill);
}


void GrGpu::onDrawPath(const GrPath* path, SkPath::FillType fill,
                       const GrDeviceCoordTexture* dstCopy) {
    this->handleDirtyContext();

    drawState()->setDefaultVertexAttribs();

    GrDrawState::AutoRestoreEffects are;
    if (!this->setupClipAndFlushState(kDrawPath_DrawType, dstCopy, &are, NULL)) {
        return;
    }

    this->onGpuDrawPath(path, fill);
}

void GrGpu::finalizeReservedVertices() {
    SkASSERT(NULL != fVertexPool);
    fVertexPool->unlock();
}

void GrGpu::finalizeReservedIndices() {
    SkASSERT(NULL != fIndexPool);
    fIndexPool->unlock();
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
    SkASSERT(NULL != vertices);

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
    SkASSERT(NULL != indices);

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

void GrGpu::onSetVertexSourceToArray(const void* vertexArray, int vertexCount) {
    this->prepareVertexPool();
    GeometryPoolState& geomPoolState = fGeomPoolStateStack.back();
#ifdef SK_DEBUG
    bool success =
#endif
    fVertexPool->appendVertices(this->getVertexSize(),
                                vertexCount,
                                vertexArray,
                                &geomPoolState.fPoolVertexBuffer,
                                &geomPoolState.fPoolStartVertex);
    ++fVertexPoolUseCnt;
    GR_DEBUGASSERT(success);
}

void GrGpu::onSetIndexSourceToArray(const void* indexArray, int indexCount) {
    this->prepareIndexPool();
    GeometryPoolState& geomPoolState = fGeomPoolStateStack.back();
#ifdef SK_DEBUG
    bool success =
#endif
    fIndexPool->appendIndices(indexCount,
                              indexArray,
                              &geomPoolState.fPoolIndexBuffer,
                              &geomPoolState.fPoolStartIndex);
    ++fIndexPoolUseCnt;
    GR_DEBUGASSERT(success);
}

void GrGpu::releaseVertexArray() {
    // if vertex source was array, we stowed data in the pool
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    SkASSERT(kArray_GeometrySrcType == geoSrc.fVertexSrc);
    size_t bytes = geoSrc.fVertexCount * geoSrc.fVertexSize;
    fVertexPool->putBack(bytes);
    --fVertexPoolUseCnt;
}

void GrGpu::releaseIndexArray() {
    // if index source was array, we stowed data in the pool
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    SkASSERT(kArray_GeometrySrcType == geoSrc.fIndexSrc);
    size_t bytes = geoSrc.fIndexCount * sizeof(uint16_t);
    fIndexPool->putBack(bytes);
    --fIndexPoolUseCnt;
}
