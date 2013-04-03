
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
    , fVertexPool(NULL)
    , fIndexPool(NULL)
    , fVertexPoolUseCnt(0)
    , fIndexPoolUseCnt(0)
    , fUnitSquareVertexBuffer(NULL)
    , fQuadIndexBuffer(NULL)
    , fContextIsDirty(true) {

    fClipMaskManager.setGpu(this);

    fGeomPoolStateStack.push_back();
#if GR_DEBUG
    GeometryPoolState& poolState = fGeomPoolStateStack.back();
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
    poolState.fPoolStartVertex = DEBUG_INVAL_START_IDX;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
    poolState.fPoolStartIndex = DEBUG_INVAL_START_IDX;
#endif

    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        fConfigRenderSupport[i] = false;
    };
}

GrGpu::~GrGpu() {
    this->releaseResources();
}

void GrGpu::abandonResources() {

    fClipMaskManager.releaseResources();

    while (NULL != fResourceList.head()) {
        fResourceList.head()->abandon();
    }

    GrAssert(NULL == fQuadIndexBuffer || !fQuadIndexBuffer->isValid());
    GrAssert(NULL == fUnitSquareVertexBuffer ||
             !fUnitSquareVertexBuffer->isValid());
    GrSafeSetNull(fQuadIndexBuffer);
    GrSafeSetNull(fUnitSquareVertexBuffer);
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

    GrAssert(NULL == fQuadIndexBuffer || !fQuadIndexBuffer->isValid());
    GrAssert(NULL == fUnitSquareVertexBuffer ||
             !fUnitSquareVertexBuffer->isValid());
    GrSafeSetNull(fQuadIndexBuffer);
    GrSafeSetNull(fUnitSquareVertexBuffer);
    delete fVertexPool;
    fVertexPool = NULL;
    delete fIndexPool;
    fIndexPool = NULL;
}

void GrGpu::insertResource(GrResource* resource) {
    GrAssert(NULL != resource);
    GrAssert(this == resource->getGpu());

    fResourceList.addToHead(resource);
}

void GrGpu::removeResource(GrResource* resource) {
    GrAssert(NULL != resource);
    GrAssert(this == resource->getGpu());

    fResourceList.remove(resource);
}


void GrGpu::unimpl(const char msg[]) {
#if GR_DEBUG
    GrPrintf("--- GrGpu unimplemented(\"%s\")\n", msg);
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrGpu::createTexture(const GrTextureDesc& desc,
                                const void* srcData, size_t rowBytes) {
    if (kUnknown_GrPixelConfig == desc.fConfig) {
        return NULL;
    }

    this->handleDirtyContext();
    GrTexture* tex = this->onCreateTexture(desc, srcData, rowBytes);
    if (NULL != tex &&
        (kRenderTarget_GrTextureFlagBit & desc.fFlags) &&
        !(kNoStencil_GrTextureFlagBit & desc.fFlags)) {
        GrAssert(NULL != tex->asRenderTarget());
        // TODO: defer this and attach dynamically
        if (!this->attachStencilBufferToRenderTarget(tex->asRenderTarget())) {
            tex->unref();
            return NULL;
        }
    }
    return tex;
}

bool GrGpu::attachStencilBufferToRenderTarget(GrRenderTarget* rt) {
    GrAssert(NULL == rt->getStencilBuffer());
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

GrVertexBuffer* GrGpu::createVertexBuffer(uint32_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->onCreateVertexBuffer(size, dynamic);
}

GrIndexBuffer* GrGpu::createIndexBuffer(uint32_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->onCreateIndexBuffer(size, dynamic);
}

GrPath* GrGpu::createPath(const SkPath& path) {
    GrAssert(this->caps()->pathStencilingSupport());
    this->handleDirtyContext();
    return this->onCreatePath(path);
}

void GrGpu::clear(const GrIRect* rect,
                  GrColor color,
                  GrRenderTarget* renderTarget) {
    GrDrawState::AutoRenderTargetRestore art;
    if (NULL != renderTarget) {
        art.set(this->drawState(), renderTarget);
    }
    if (NULL == this->getDrawState().getRenderTarget()) {
        return;
    }
    this->handleDirtyContext();
    this->onClear(rect, color);
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
    GrAssert(target);
    this->handleDirtyContext();
    this->onResolveRenderTarget(target);
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
                indices = (uint16_t*)GrMalloc(SIZE);
                fill_indices(indices, MAX_QUADS);
                if (!fQuadIndexBuffer->updateData(indices, SIZE)) {
                    fQuadIndexBuffer->unref();
                    fQuadIndexBuffer = NULL;
                    GrCrash("Can't get indices into buffer!");
                }
                GrFree(indices);
            }
        }
    }

    return fQuadIndexBuffer;
}

const GrVertexBuffer* GrGpu::getUnitSquareVertexBuffer() const {
    if (NULL == fUnitSquareVertexBuffer) {

        static const GrPoint DATA[] = {
            { 0,            0 },
            { SK_Scalar1,   0 },
            { SK_Scalar1,   SK_Scalar1 },
            { 0,            SK_Scalar1 }
#if 0
            GrPoint(0,         0),
            GrPoint(SK_Scalar1,0),
            GrPoint(SK_Scalar1,SK_Scalar1),
            GrPoint(0,         SK_Scalar1)
#endif
        };
        static const size_t SIZE = sizeof(DATA);

        GrGpu* me = const_cast<GrGpu*>(this);
        fUnitSquareVertexBuffer = me->createVertexBuffer(SIZE, false);
        if (NULL != fUnitSquareVertexBuffer) {
            if (!fUnitSquareVertexBuffer->updateData(DATA, SIZE)) {
                fUnitSquareVertexBuffer->unref();
                fUnitSquareVertexBuffer = NULL;
                GrCrash("Can't get vertices into buffer!");
            }
        }
    }

    return fUnitSquareVertexBuffer;
}

////////////////////////////////////////////////////////////////////////////////

bool GrGpu::setupClipAndFlushState(DrawType type, const GrDeviceCoordTexture* dstCopy) {

    if (!fClipMaskManager.setupClipping(this->getClip())) {
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
#if GR_DEBUG
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
    GrAssert(fGeomPoolStateStack.count() > 1);
    fGeomPoolStateStack.pop_back();
}

void GrGpu::onDraw(const DrawInfo& info) {
    this->handleDirtyContext();
    if (!this->setupClipAndFlushState(PrimTypeToDrawType(info.primitiveType()),
                                      info.getDstCopy())) {
        return;
    }
    this->onGpuDraw(info);
}

void GrGpu::onStencilPath(const GrPath* path, const SkStrokeRec&, SkPath::FillType fill) {
    this->handleDirtyContext();

    // TODO: make this more efficient (don't copy and copy back)
    GrAutoTRestore<GrStencilSettings> asr(this->drawState()->stencil());

    this->setStencilPathSettings(*path, fill, this->drawState()->stencil());
    if (!this->setupClipAndFlushState(kStencilPath_DrawType, NULL)) {
        return;
    }

    this->onGpuStencilPath(path, fill);
}

void GrGpu::finalizeReservedVertices() {
    GrAssert(NULL != fVertexPool);
    fVertexPool->unlock();
}

void GrGpu::finalizeReservedIndices() {
    GrAssert(NULL != fIndexPool);
    fIndexPool->unlock();
}

void GrGpu::prepareVertexPool() {
    if (NULL == fVertexPool) {
        GrAssert(0 == fVertexPoolUseCnt);
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
        GrAssert(0 == fIndexPoolUseCnt);
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

    GrAssert(vertexCount > 0);
    GrAssert(NULL != vertices);

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

    GrAssert(indexCount > 0);
    GrAssert(NULL != indices);

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
    GrAssert(kReserved_GeometrySrcType == geoSrc.fVertexSrc);
    size_t bytes = geoSrc.fVertexCount * geoSrc.fVertexSize;
    fVertexPool->putBack(bytes);
    --fVertexPoolUseCnt;
}

void GrGpu::releaseReservedIndexSpace() {
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    GrAssert(kReserved_GeometrySrcType == geoSrc.fIndexSrc);
    size_t bytes = geoSrc.fIndexCount * sizeof(uint16_t);
    fIndexPool->putBack(bytes);
    --fIndexPoolUseCnt;
}

void GrGpu::onSetVertexSourceToArray(const void* vertexArray, int vertexCount) {
    this->prepareVertexPool();
    GeometryPoolState& geomPoolState = fGeomPoolStateStack.back();
#if GR_DEBUG
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
#if GR_DEBUG
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
    GrAssert(kArray_GeometrySrcType == geoSrc.fVertexSrc);
    size_t bytes = geoSrc.fVertexCount * geoSrc.fVertexSize;
    fVertexPool->putBack(bytes);
    --fVertexPoolUseCnt;
}

void GrGpu::releaseIndexArray() {
    // if index source was array, we stowed data in the pool
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    GrAssert(kArray_GeometrySrcType == geoSrc.fIndexSrc);
    size_t bytes = geoSrc.fIndexCount * sizeof(uint16_t);
    fIndexPool->putBack(bytes);
    --fIndexPoolUseCnt;
}
