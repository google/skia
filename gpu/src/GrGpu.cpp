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

size_t GrTexture::BytesPerPixel(PixelConfig config) {
    switch (config) {
        case kAlpha_8_PixelConfig:
        case kIndex_8_PixelConfig:
            return 1;
        case kRGB_565_PixelConfig:
        case kRGBA_4444_PixelConfig:
            return 2;
        case kRGBA_8888_PixelConfig:
        case kRGBX_8888_PixelConfig:
            return 4;
        default:
            return 0;
    }
}

bool GrTexture::PixelConfigIsOpaque(PixelConfig config) {
    switch (config) {
        case GrTexture::kRGB_565_PixelConfig:
        case GrTexture::kRGBX_8888_PixelConfig:
            return true;
        default:
            return false;
    }
}


////////////////////////////////////////////////////////////////////////////////

extern void gr_run_unittests();

GrGpu::GrGpu() : f8bitPaletteSupport(false),
                 fCurrPoolVertexBuffer(NULL),
                 fCurrPoolStartVertex(0),
                 fCurrPoolIndexBuffer(NULL),
                 fCurrPoolStartIndex(0),
                 fVertexPool(NULL),
                 fIndexPool(NULL),
                 fQuadIndexBuffer(NULL),
                 fUnitSquareVertexBuffer(NULL),
                 fPathRenderer(NULL),
                 fContextIsDirty(true),
                 fVertexPoolInUse(false),
                 fIndexPoolInUse(false) {
#if GR_DEBUG
    //gr_run_unittests();
#endif
    resetStats();
}

GrGpu::~GrGpu() {
    GrSafeUnref(fQuadIndexBuffer);
    GrSafeUnref(fUnitSquareVertexBuffer);
    delete fVertexPool;
    delete fIndexPool;
    delete fPathRenderer;
}

void GrGpu::resetContext() {
}

void GrGpu::unimpl(const char msg[]) {
#if GR_DEBUG
    GrPrintf("--- GrGpu unimplemented(\"%s\")\n", msg);
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrGpu::createTexture(const TextureDesc& desc,
                                const void* srcData, size_t rowBytes) {
    this->handleDirtyContext();
    return this->createTextureHelper(desc, srcData, rowBytes);
}

GrRenderTarget* GrGpu::createPlatformRenderTarget(intptr_t platformRenderTarget,
                                                  int stencilBits,
                                                  int width, int height) {
    this->handleDirtyContext();
    return this->createPlatformRenderTargetHelper(platformRenderTarget,
                                                  stencilBits,
                                                  width, height);
}

GrRenderTarget* GrGpu::createRenderTargetFrom3DApiState() {
    this->handleDirtyContext();
    return this->createRenderTargetFrom3DApiStateHelper();
}

GrVertexBuffer* GrGpu::createVertexBuffer(uint32_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->createVertexBufferHelper(size, dynamic);
}

GrIndexBuffer* GrGpu::createIndexBuffer(uint32_t size, bool dynamic) {
    this->handleDirtyContext();
    return this->createIndexBufferHelper(size, dynamic);
}

void GrGpu::eraseColor(GrColor color) {
    this->handleDirtyContext();
    this->eraseColorHelper(color);
}

void GrGpu::forceRenderTargetFlush() {
    this->handleDirtyContext();
    this->forceRenderTargetFlushHelper();
}

bool GrGpu::readPixels(int left, int top, int width, int height,
                       GrTexture::PixelConfig config, void* buffer) {
    this->handleDirtyContext();
    return this->readPixelsHelper(left, top, width, height, config, buffer);
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
            GrPoint(0,         0),
            GrPoint(GR_Scalar1,0),
            GrPoint(GR_Scalar1,GR_Scalar1),
            GrPoint(0,         GR_Scalar1)
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
            bounds.intersectWith(rtRect);
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
            AutoInternalDrawGeomRestore aidgr(this);

            this->setViewMatrix(GrMatrix::I());
            this->eraseStencilClip(clipRect);
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

                bool canDrawDirectToClip;
                if (kRect_ClipType == clip.getElementType(c)) {
                    canDrawDirectToClip = true;
                    fill = kEvenOdd_PathFill;
                } else {
                    fill = clip.getPathFill(c);
                    GrPathRenderer* pr = this->getPathRenderer();
                    canDrawDirectToClip = pr->requiresStencilPass(this, clip.getPath(c), fill);
                }

                GrSetOp op = firstElement == c ? kReplace_SetOp : clip.getOp(c);
                int passes;
                GrStencilSettings stencilSettings[GrStencilSettings::kMaxStencilClipPasses];

                canDrawDirectToClip = GrStencilSettings::GetClipPasses(op, canDrawDirectToClip,
                                                                       clipBit, IsFillInverted(fill),
                                                                       &passes, stencilSettings);

                // draw the element to the client stencil bits if necessary
                if (!canDrawDirectToClip) {
                    if (kRect_ClipType == clip.getElementType(c)) {
                        static const GrStencilSettings gDrawToStencil = {
                            kIncClamp_StencilOp, kIncClamp_StencilOp,
                            kIncClamp_StencilOp, kIncClamp_StencilOp,
                            kAlways_StencilFunc, kAlways_StencilFunc,
                            0xffffffff,          0xffffffff,
                            0x00000000,          0x00000000,
                            0xffffffff,          0xffffffff,
                        };
                        this->setStencil(gDrawToStencil);
                        SET_RANDOM_COLOR
                        this->drawSimpleRect(clip.getRect(c), NULL, 0);
                    } else {
                        SET_RANDOM_COLOR
                        getPathRenderer()->drawPathToStencil(this, clip.getPath(c),
                                                             NonInvertedFill(fill),
                                                             NULL);
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
                            getPathRenderer()->drawPath(this, 0,
                                                        clip.getPath(c),
                                                        fill, NULL);
                        }
                    } else {
                        SET_RANDOM_COLOR
                        this->drawSimpleRect(bounds, 0, NULL);
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

////////////////////////////////////////////////////////////////////////////////

void GrGpu::drawIndexed(GrPrimitiveType type,
                        int startVertex,
                        int startIndex,
                        int vertexCount,
                        int indexCount) {
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fVertexSrc ||
             fReservedGeometry.fLocked);
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fIndexSrc ||
             fReservedGeometry.fLocked);

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

    drawIndexedHelper(type, sVertex, sIndex,
                      vertexCount, indexCount);
}

void GrGpu::drawNonIndexed(GrPrimitiveType type,
                           int startVertex,
                           int vertexCount) {
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fVertexSrc ||
             fReservedGeometry.fLocked);

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

    drawNonIndexedHelper(type, sVertex, vertexCount);
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
        fVertexPool = new GrVertexBufferAllocPool(this, true,
                                                  VERTEX_POOL_VB_SIZE,
                                                  VERTEX_POOL_VB_COUNT);
    } else if (!fVertexPoolInUse) {
        // the client doesn't have valid data in the pool
        fVertexPool->reset();
    }
}

void GrGpu::prepareIndexPool() {
    if (NULL == fVertexPool) {
        fIndexPool = new GrIndexBufferAllocPool(this, true, 0, 1);
    } else if (!fIndexPoolInUse) {
        // the client doesn't have valid data in the pool
        fIndexPool->reset();
    }
}

bool GrGpu::acquireGeometryHelper(GrVertexLayout vertexLayout,
                                  void**         vertices,
                                  void**         indices) {
    GrAssert(!fReservedGeometry.fLocked);
    size_t reservedVertexSpace = 0;

    if (fReservedGeometry.fVertexCount) {
        GrAssert(NULL != vertices);

        this->prepareVertexPool();

        *vertices = fVertexPool->makeSpace(vertexLayout,
                                           fReservedGeometry.fVertexCount,
                                           &fCurrPoolVertexBuffer,
                                           &fCurrPoolStartVertex);
        if (NULL == *vertices) {
            return false;
        }
        reservedVertexSpace = VertexSize(vertexLayout) *
                              fReservedGeometry.fVertexCount;
    }
    if (fReservedGeometry.fIndexCount) {
        GrAssert(NULL != indices);

        this->prepareIndexPool();

        *indices = fIndexPool->makeSpace(fReservedGeometry.fIndexCount,
                                         &fCurrPoolIndexBuffer,
                                         &fCurrPoolStartIndex);
        if (NULL == *indices) {
            fVertexPool->putBack(reservedVertexSpace);
            fCurrPoolVertexBuffer = NULL;
            return false;
        }
    }
    return true;
}

void GrGpu::releaseGeometryHelper() {}

void GrGpu::setVertexSourceToArrayHelper(const void* vertexArray, int vertexCount) {
    GrAssert(!fReservedGeometry.fLocked || !fReservedGeometry.fVertexCount);
    prepareVertexPool();
#if GR_DEBUG
    bool success =
#endif
    fVertexPool->appendVertices(fGeometrySrc.fVertexLayout,
                                vertexCount,
                                vertexArray,
                                &fCurrPoolVertexBuffer,
                                &fCurrPoolStartVertex);
    GR_DEBUGASSERT(success);
}

void GrGpu::setIndexSourceToArrayHelper(const void* indexArray, int indexCount) {
    GrAssert(!fReservedGeometry.fLocked || !fReservedGeometry.fIndexCount);
    prepareIndexPool();
#if GR_DEBUG
    bool success =
#endif
    fIndexPool->appendIndices(indexCount,
                              indexArray,
                              &fCurrPoolIndexBuffer,
                              &fCurrPoolStartIndex);
    GR_DEBUGASSERT(success);
}

////////////////////////////////////////////////////////////////////////////////

GrPathRenderer* GrGpu::getPathRenderer() {
    if (NULL == fPathRenderer) {
        fPathRenderer = new GrDefaultPathRenderer(this->supportsTwoSidedStencil(),
                                                  this->supportsStencilWrapOps());
    }
    return fPathRenderer;
}

////////////////////////////////////////////////////////////////////////////////

const GrGpu::Stats& GrGpu::getStats() const {
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

GrTexture::~GrTexture() {
    // use this to set a break-point if needed
//    Gr_clz(3);
}

const GrSamplerState GrSamplerState::gClampNoFilter(
    GrSamplerState::kClamp_WrapMode,
    GrSamplerState::kClamp_WrapMode,
    GrSamplerState::kNormal_SampleMode,
    GrMatrix::I(),
    false);




