/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "GrGpu.h"
#include "GrMemory.h"
#include "GrTextStrike.h"
#include "GrTextureCache.h"
#include "GrClipIterator.h"
#include "GrIndexBuffer.h"
#include "GrVertexBuffer.h"
#include "GrBufferAllocPool.h"
#include "GrPathRenderer.h"

// probably makes no sense for this to be less than a page
static const size_t VERTEX_POOL_VB_SIZE = 1 << 12;
static const int VERTEX_POOL_VB_COUNT = 1;


////////////////////////////////////////////////////////////////////////////////

extern void gr_run_unittests();

#define DEBUG_INVAL_BUFFER    0xdeadcafe
#define DEBUG_INVAL_START_IDX -1

GrGpu::GrGpu()
    : f8bitPaletteSupport(false)
    , fContext(NULL)
    , fVertexPool(NULL)
    , fIndexPool(NULL)
    , fVertexPoolUseCnt(0)
    , fIndexPoolUseCnt(0)
    , fGeomPoolStateStack(&fGeoSrcStateStackStorage)
    , fQuadIndexBuffer(NULL)
    , fUnitSquareVertexBuffer(NULL)
    , fDefaultPathRenderer(NULL)
    , fClientPathRenderer(NULL)
    , fContextIsDirty(true)
    , fResourceHead(NULL) {

#if GR_DEBUG
    //gr_run_unittests();
#endif
        
    fGeomPoolStateStack.push_back();
#if GR_DEBUG
    GeometryPoolState& poolState = fGeomPoolStateStack.back();
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
    poolState.fPoolStartVertex = DEBUG_INVAL_START_IDX;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
    poolState.fPoolStartIndex = DEBUG_INVAL_START_IDX;
#endif
    resetStats();
}

GrGpu::~GrGpu() {
    releaseResources();
}

void GrGpu::abandonResources() {

    while (NULL != fResourceHead) {
        fResourceHead->abandon();
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

    while (NULL != fResourceHead) {
        fResourceHead->release();
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
    GrAssert(NULL == resource->fNext);
    GrAssert(NULL == resource->fPrevious);

    resource->fNext = fResourceHead;
    if (NULL != fResourceHead) {
        GrAssert(NULL == fResourceHead->fPrevious);
        fResourceHead->fPrevious = resource;
    }
    fResourceHead = resource;
}

void GrGpu::removeResource(GrResource* resource) {
    GrAssert(NULL != resource);
    GrAssert(NULL != fResourceHead);

    if (fResourceHead == resource) {
        GrAssert(NULL == resource->fPrevious);
        fResourceHead = resource->fNext;
    } else {
        GrAssert(NULL != fResourceHead);
        resource->fPrevious->fNext = resource->fNext;
    }
    if (NULL != resource->fNext) {
        resource->fNext->fPrevious = resource->fPrevious;
    }
    resource->fNext = NULL;
    resource->fPrevious = NULL;
}


void GrGpu::unimpl(const char msg[]) {
#if GR_DEBUG
    GrPrintf("--- GrGpu unimplemented(\"%s\")\n", msg);
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrGpu::createTexture(const GrTextureDesc& desc,
                                const void* srcData, size_t rowBytes) {
    this->handleDirtyContext();
    return this->onCreateTexture(desc, srcData, rowBytes);
}

GrRenderTarget* GrGpu::createRenderTargetFrom3DApiState() {
    this->handleDirtyContext();
    return this->onCreateRenderTargetFrom3DApiState();
}

GrResource* GrGpu::createPlatformSurface(const GrPlatformSurfaceDesc& desc) {
    this->handleDirtyContext();
    return this->onCreatePlatformSurface(desc);
}

GrVertexBuffer* GrGpu::createVertexBuffer(uint32_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->onCreateVertexBuffer(size, dynamic);
}

GrIndexBuffer* GrGpu::createIndexBuffer(uint32_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->onCreateIndexBuffer(size, dynamic);
}

void GrGpu::clear(const GrIRect* rect, GrColor color) {
    this->handleDirtyContext();
    this->onClear(rect, color);
}

void GrGpu::forceRenderTargetFlush() {
    this->handleDirtyContext();
    this->onForceRenderTargetFlush();
}

bool GrGpu::readPixels(GrRenderTarget* target,
                       int left, int top, int width, int height,
                       GrPixelConfig config, void* buffer) {

    this->handleDirtyContext();
    return this->onReadPixels(target, left, top, width, height, config, buffer);
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
            { GR_Scalar1,   0 },
            { GR_Scalar1,   GR_Scalar1 },
            { 0,            GR_Scalar1 }
#if 0
            GrPoint(0,         0),
            GrPoint(GR_Scalar1,0),
            GrPoint(GR_Scalar1,GR_Scalar1),
            GrPoint(0,         GR_Scalar1)
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

void GrGpu::clipWillBeSet(const GrClip& newClip) {
    if (newClip != fClip) {
        fClipState.fClipIsDirty = true;
    }
}

////////////////////////////////////////////////////////////////////////////////

// stencil settings to use when clip is in stencil
const GrStencilSettings GrGpu::gClipStencilSettings = {
    kKeep_StencilOp,             kKeep_StencilOp,
    kKeep_StencilOp,             kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc, kAlwaysIfInClip_StencilFunc,
    0,                           0,
    0,                           0,
    0,                           0
};

// mapping of clip-respecting stencil funcs to normal stencil funcs
// mapping depends on whether stencil-clipping is in effect.
static const GrStencilFunc gGrClipToNormalStencilFunc[2][kClipStencilFuncCount] = {
    {// Stencil-Clipping is DISABLED, effectively always inside the clip
        // In the Clip Funcs
        kAlways_StencilFunc,          // kAlwaysIfInClip_StencilFunc
        kEqual_StencilFunc,           // kEqualIfInClip_StencilFunc
        kLess_StencilFunc,            // kLessIfInClip_StencilFunc
        kLEqual_StencilFunc,          // kLEqualIfInClip_StencilFunc
        // Special in the clip func that forces user's ref to be 0.
        kNotEqual_StencilFunc,        // kNonZeroIfInClip_StencilFunc
                                      // make ref 0 and do normal nequal.
    },
    {// Stencil-Clipping is ENABLED
        // In the Clip Funcs
        kEqual_StencilFunc,           // kAlwaysIfInClip_StencilFunc
                                      // eq stencil clip bit, mask
                                      // out user bits.

        kEqual_StencilFunc,           // kEqualIfInClip_StencilFunc
                                      // add stencil bit to mask and ref

        kLess_StencilFunc,            // kLessIfInClip_StencilFunc
        kLEqual_StencilFunc,          // kLEqualIfInClip_StencilFunc
                                      // for both of these we can add
                                      // the clip bit to the mask and
                                      // ref and compare as normal
        // Special in the clip func that forces user's ref to be 0.
        kLess_StencilFunc,            // kNonZeroIfInClip_StencilFunc
                                      // make ref have only the clip bit set
                                      // and make comparison be less
                                      // 10..0 < 1..user_bits..
    }
};

GrStencilFunc GrGpu::ConvertStencilFunc(bool stencilInClip, GrStencilFunc func) {
    GrAssert(func >= 0);
    if (func >= kBasicStencilFuncCount) {
        GrAssert(func < kStencilFuncCount);
        func = gGrClipToNormalStencilFunc[stencilInClip ? 1 : 0][func - kBasicStencilFuncCount];
        GrAssert(func >= 0 && func < kBasicStencilFuncCount);
    }
    return func;
}

void GrGpu::ConvertStencilFuncAndMask(GrStencilFunc func,
                                      bool clipInStencil,
                                      unsigned int clipBit,
                                      unsigned int userBits,
                                      unsigned int* ref,
                                      unsigned int* mask) {
    if (func < kBasicStencilFuncCount) {
        *mask &= userBits;
        *ref &= userBits;
    } else {
        if (clipInStencil) {
            switch (func) {
                case kAlwaysIfInClip_StencilFunc:
                    *mask = clipBit;
                    *ref = clipBit;
                    break;
                case kEqualIfInClip_StencilFunc:
                case kLessIfInClip_StencilFunc:
                case kLEqualIfInClip_StencilFunc:
                    *mask = (*mask & userBits) | clipBit;
                    *ref = (*ref & userBits) | clipBit;
                    break;
                case kNonZeroIfInClip_StencilFunc:
                    *mask = (*mask & userBits) | clipBit;
                    *ref = clipBit;
                    break;
                default:
                    GrCrash("Unknown stencil func");
            }
        } else {
            *mask &= userBits;
            *ref &= userBits;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

#define VISUALIZE_COMPLEX_CLIP 0

#if VISUALIZE_COMPLEX_CLIP
    #include "GrRandom.h"
    GrRandom gRandom;
    #define SET_RANDOM_COLOR this->setColor(0xff000000 | gRandom.nextU());
#else
    #define SET_RANDOM_COLOR
#endif

bool GrGpu::setupClipAndFlushState(GrPrimitiveType type) {
    const GrIRect* r = NULL;
    GrIRect clipRect;

    // we check this early because we need a valid
    // render target to setup stencil clipping
    // before even going into flushGraphicsState
    if (NULL == fCurrDrawState.fRenderTarget) {
        GrAssert(!"No render target bound.");
        return false;
    }

    if (fCurrDrawState.fFlagBits & kClip_StateBit) {
        GrRenderTarget& rt = *fCurrDrawState.fRenderTarget;

        GrRect bounds;
        GrRect rtRect;
        rtRect.setLTRB(0, 0,
                       GrIntToScalar(rt.width()), GrIntToScalar(rt.height()));
        if (fClip.hasConservativeBounds()) {
            bounds = fClip.getConservativeBounds();
            if (!bounds.intersect(rtRect)) {
                bounds.setEmpty();
            }
        } else {
            bounds = rtRect;
        }

        bounds.roundOut(&clipRect);
        if  (clipRect.isEmpty()) {
            clipRect.setLTRB(0,0,0,0);
        }
        r = &clipRect;

        fClipState.fClipInStencil = !fClip.isRect() &&
                                    !fClip.isEmpty() &&
                                    !bounds.isEmpty();

        if (fClipState.fClipInStencil &&
            (fClipState.fClipIsDirty ||
             fClip != rt.fLastStencilClip)) {

            rt.fLastStencilClip = fClip;
            // we set the current clip to the bounds so that our recursive
            // draws are scissored to them. We use the copy of the complex clip
            // in the rt to render
            const GrClip& clip = rt.fLastStencilClip;
            fClip.setFromRect(bounds);

            AutoStateRestore asr(this);
            AutoGeometryPush agp(this);

            this->setViewMatrix(GrMatrix::I());
            this->clearStencilClip(clipRect);
            this->flushScissor(NULL);
#if !VISUALIZE_COMPLEX_CLIP
            this->enableState(kNoColorWrites_StateBit);
#else
            this->disableState(kNoColorWrites_StateBit);
#endif
            int count = clip.getElementCount();
            int clipBit = rt.stencilBits();
            clipBit = (1 << (clipBit-1));

            // often we'll see the first two elements of the clip are
            // the full rt size and another element intersected with it.
            // We can skip the first full-size rect and save a big rect draw.
            int firstElement = 0;
            if (clip.getElementCount() > 1 &&
                kRect_ClipType == clip.getElementType(0) &&
                kIntersect_SetOp == clip.getOp(1)&&
                clip.getRect(0).contains(bounds)) {
                firstElement = 1;
            }

            // walk through each clip element and perform its set op
            // with the existing clip.
            for (int c = firstElement; c < count; ++c) {
                GrPathFill fill;
                // enabled at bottom of loop
                this->disableState(kModifyStencilClip_StateBit);

                bool canRenderDirectToStencil; // can the clip element be drawn
                                               // directly to the stencil buffer
                                               // with a non-inverted fill rule
                                               // without extra passes to
                                               // resolve in/out status.

                GrPathRenderer* pr = NULL;
                const GrPath* clipPath = NULL;
                if (kRect_ClipType == clip.getElementType(c)) {
                    canRenderDirectToStencil = true;
                    fill = kEvenOdd_PathFill;
                } else {
                    fill = clip.getPathFill(c);
                    clipPath = &clip.getPath(c);
                    pr = this->getClipPathRenderer(*clipPath, NonInvertedFill(fill));
                    canRenderDirectToStencil =
                        !pr->requiresStencilPass(this, *clipPath,
                                                 NonInvertedFill(fill));
                }

                GrSetOp op = firstElement == c ? kReplace_SetOp : clip.getOp(c);
                int passes;
                GrStencilSettings stencilSettings[GrStencilSettings::kMaxStencilClipPasses];

                bool canDrawDirectToClip; // Given the renderer, the element,
                                          // fill rule, and set operation can
                                          // we render the element directly to
                                          // stencil bit used for clipping.
                canDrawDirectToClip =
                    GrStencilSettings::GetClipPasses(op,
                                                     canRenderDirectToStencil,
                                                     clipBit,
                                                     IsFillInverted(fill),
                                                     &passes, stencilSettings);

                // draw the element to the client stencil bits if necessary
                if (!canDrawDirectToClip) {
                    static const GrStencilSettings gDrawToStencil = {
                        kIncClamp_StencilOp, kIncClamp_StencilOp,
                        kIncClamp_StencilOp, kIncClamp_StencilOp,
                        kAlways_StencilFunc, kAlways_StencilFunc,
                        0xffffffff,          0xffffffff,
                        0x00000000,          0x00000000,
                        0xffffffff,          0xffffffff,
                    };
                    SET_RANDOM_COLOR
                    if (kRect_ClipType == clip.getElementType(c)) {
                        this->setStencil(gDrawToStencil);
                        this->drawSimpleRect(clip.getRect(c), NULL, 0);
                    } else {
                        if (canRenderDirectToStencil) {
                            this->setStencil(gDrawToStencil);
                            pr->drawPath(this, 0, *clipPath, NonInvertedFill(fill),
                                         NULL);
                        } else {
                            pr->drawPathToStencil(this, *clipPath,
                                                  NonInvertedFill(fill),
                                                  NULL);
                        }
                    }
                }

                // now we modify the clip bit by rendering either the clip
                // element directly or a bounding rect of the entire clip.
                this->enableState(kModifyStencilClip_StateBit);
                for (int p = 0; p < passes; ++p) {
                    this->setStencil(stencilSettings[p]);
                    if (canDrawDirectToClip) {
                        if (kRect_ClipType == clip.getElementType(c)) {
                            SET_RANDOM_COLOR
                            this->drawSimpleRect(clip.getRect(c), NULL, 0);
                        } else {
                            SET_RANDOM_COLOR
                            GrAssert(!IsFillInverted(fill));
                            pr->drawPath(this, 0, *clipPath, fill, NULL);
                        }
                    } else {
                        SET_RANDOM_COLOR
                        this->drawSimpleRect(bounds, NULL, 0);
                    }
                }
            }
            fClip = clip;
            // recusive draws would have disabled this.
            fClipState.fClipInStencil = true;
        }

        fClipState.fClipIsDirty = false;
    }

    // Must flush the scissor after graphics state
    if (!this->flushGraphicsState(type)) {
        return false;
    }
    this->flushScissor(r);
    return true;
}

GrPathRenderer* GrGpu::getClipPathRenderer(const GrPath& path,
                                           GrPathFill fill) {
    if (NULL != fClientPathRenderer &&
        fClientPathRenderer->canDrawPath(this, path, fill)) {
            return fClientPathRenderer;
    } else {
        if (NULL == fDefaultPathRenderer) {
            fDefaultPathRenderer =
                new GrDefaultPathRenderer(this->supportsTwoSidedStencil(),
                                          this->supportsStencilWrapOps());
        }
        GrAssert(fDefaultPathRenderer->canDrawPath(this, path, fill));
        return fDefaultPathRenderer;
    }
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
#endif
}

void GrGpu::geometrySourceWillPop(const GeometrySrcState& restoredState) {
    // if popping last entry then pops are unbalanced with pushes
    GrAssert(fGeomPoolStateStack.count() > 1);
    fGeomPoolStateStack.pop_back();
}

void GrGpu::onDrawIndexed(GrPrimitiveType type,
                          int startVertex,
                          int startIndex,
                          int vertexCount,
                          int indexCount) {

    this->handleDirtyContext();

    if (!this->setupClipAndFlushState(type)) {
        return;
    }

#if GR_COLLECT_STATS
    fStats.fVertexCnt += vertexCount;
    fStats.fIndexCnt  += indexCount;
    fStats.fDrawCnt   += 1;
#endif

    int sVertex = startVertex;
    int sIndex = startIndex;
    setupGeometry(&sVertex, &sIndex, vertexCount, indexCount);

    this->onGpuDrawIndexed(type, sVertex, sIndex,
                           vertexCount, indexCount);
}

void GrGpu::onDrawNonIndexed(GrPrimitiveType type,
                           int startVertex,
                           int vertexCount) {
    this->handleDirtyContext();

    if (!this->setupClipAndFlushState(type)) {
        return;
    }
#if GR_COLLECT_STATS
    fStats.fVertexCnt += vertexCount;
    fStats.fDrawCnt   += 1;
#endif

    int sVertex = startVertex;
    setupGeometry(&sVertex, NULL, vertexCount, 0);

    this->onGpuDrawNonIndexed(type, sVertex, vertexCount);
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
        fVertexPool = new GrVertexBufferAllocPool(this, true,
                                                  VERTEX_POOL_VB_SIZE,
                                                  VERTEX_POOL_VB_COUNT);
        fVertexPool->releaseGpuRef();
    } else if (!fVertexPoolUseCnt) {
        // the client doesn't have valid data in the pool
        fVertexPool->reset();
    }
}

void GrGpu::prepareIndexPool() {
    if (NULL == fIndexPool) {
        GrAssert(0 == fIndexPoolUseCnt);
        fIndexPool = new GrIndexBufferAllocPool(this, true, 0, 1);
        fIndexPool->releaseGpuRef();
    } else if (!fIndexPoolUseCnt) {
        // the client doesn't have valid data in the pool
        fIndexPool->reset();
    }
}

bool GrGpu::onReserveVertexSpace(GrVertexLayout vertexLayout,
                                 int vertexCount,
                                 void** vertices) {
    GeometryPoolState& geomPoolState = fGeomPoolStateStack.back();
    
    GrAssert(vertexCount > 0);
    GrAssert(NULL != vertices);
    
    this->prepareVertexPool();
    
    *vertices = fVertexPool->makeSpace(vertexLayout,
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
    size_t bytes = geoSrc.fVertexCount * VertexSize(geoSrc.fVertexLayout);
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
    fVertexPool->appendVertices(this->getGeomSrc().fVertexLayout,
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
    size_t bytes = geoSrc.fVertexCount * VertexSize(geoSrc.fVertexLayout);
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

////////////////////////////////////////////////////////////////////////////////

const GrGpuStats& GrGpu::getStats() const {
    return fStats;
}

void GrGpu::resetStats() {
    memset(&fStats, 0, sizeof(fStats));
}

void GrGpu::printStats() const {
    if (GR_COLLECT_STATS) {
     GrPrintf(
     "-v-------------------------GPU STATS----------------------------v-\n"
     "Stats collection is: %s\n"
     "Draws: %04d, Verts: %04d, Indices: %04d\n"
     "ProgChanges: %04d, TexChanges: %04d, RTChanges: %04d\n"
     "TexCreates: %04d, RTCreates:%04d\n"
     "-^--------------------------------------------------------------^-\n",
     (GR_COLLECT_STATS ? "ON" : "OFF"),
    fStats.fDrawCnt, fStats.fVertexCnt, fStats.fIndexCnt,
    fStats.fProgChngCnt, fStats.fTextureChngCnt, fStats.fRenderTargetChngCnt,
    fStats.fTextureCreateCnt, fStats.fRenderTargetCreateCnt);
    }
}

////////////////////////////////////////////////////////////////////////////////
const GrSamplerState GrSamplerState::gClampNoFilter(
    GrSamplerState::kClamp_WrapMode,
    GrSamplerState::kClamp_WrapMode,
    GrSamplerState::kNormal_SampleMode,
    GrMatrix::I(),
    GrSamplerState::kNearest_Filter);




