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

// probably makes no sense for this to be less than a page
static size_t VERTEX_POOL_VB_SIZE = 1 << 12;

///////////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////

extern void gr_run_unittests();

GrGpu::GrGpu() : f8bitPaletteSupport(false),
                 fCurrPoolVertexBuffer(NULL),
                 fCurrPoolStartVertex(0),
                 fCurrPoolIndexBuffer(NULL),
                 fCurrPoolStartIndex(0),
                 fVertexPool(NULL),
                 fIndexPool(NULL),
                 fQuadIndexBuffer(NULL),
                 fUnitSquareVertexBuffer(NULL) {
#if GR_DEBUG
//    gr_run_unittests();
#endif
    resetStats();
}

GrGpu::~GrGpu() {
    GrSafeUnref(fQuadIndexBuffer);
    GrSafeUnref(fUnitSquareVertexBuffer);
    delete fVertexPool;
    delete fIndexPool;
}

void GrGpu::resetContext() {
}

void GrGpu::unimpl(const char msg[]) {
#if GR_DEBUG
    GrPrintf("--- GrGpu unimplemented(\"%s\")\n", msg);
#endif
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void GrGpu::clipWillBeSet(const GrClip& newClip) {
    if (newClip != fClip) {
        fClipState.fClipIsDirty = true;
    }
}

bool GrGpu::setupClipAndFlushState(PrimitiveType type) {
    const GrIRect* r = NULL;

    // we check this early because we need a valid
    // render target to setup stencil clipping
    // before even going into flushGraphicsState
    if (NULL == fCurrDrawState.fRenderTarget) {
        GrAssert(!"No render target bound.");
        return false;
    }

    if (fCurrDrawState.fFlagBits & kClip_StateBit) {
        fClipState.fClipInStencil = fClip.countRects() > 1;

        if (fClipState.fClipInStencil &&
            (fClipState.fClipIsDirty ||
             fClipState.fStencilClipTarget != fCurrDrawState.fRenderTarget)) {

            AutoStateRestore asr(this);
            AutoGeometrySrcRestore agsr(this);

            // We have to use setVertexSourceToBuffer (and index) in order
            // to ensure we correctly restore the client's geom sources.
            // We tack the clip verts onto the vertex pool but we don't
            // use the various helper functions because of their side effects.

            int rectTotal = fClip.countRects();
            if (NULL == fVertexPool) {
                fVertexPool = new GrVertexBufferAllocPool(this,
                                                          true,
                                                          VERTEX_POOL_VB_SIZE,
                                                          1);
            }
            const GrVertexBuffer* vertexBuffer;
            int vStart;
            GrPoint* rectVertices =
                reinterpret_cast<GrPoint*>(fVertexPool->makeSpace(0,
                                                                  rectTotal * 4,
                                                                  &vertexBuffer,
                                                                  &vStart));
            for (int r = 0; r < rectTotal; ++r) {
                const GrIRect& rect = fClip.getRects()[r];
                rectVertices[4 * r].setIRectFan(rect.fLeft, rect.fTop,
                                                rect.fRight, rect.fBottom);
            }
            fVertexPool->unlock();
            this->setVertexSourceToBuffer(0, vertexBuffer);
            this->setIndexSourceToBuffer(getQuadIndexBuffer());
            this->setViewMatrix(GrMatrix::I());
            // don't clip the clip or recurse!
            this->disableState(kClip_StateBit);
            this->eraseStencilClip();
            this->setStencilPass((GrDrawTarget::StencilPass)kSetClip_StencilPass);
            int currRect = 0;
            while (currRect < rectTotal) {
                int rectCount = GrMin(MAX_QUADS,
                                      rectTotal - currRect);
                this->drawIndexed(kTriangles_PrimitiveType,
                                  vStart + currRect * 4,
                                  0,
                                  rectCount*4,
                                  rectCount*6);
                currRect += rectCount;
            }
            fClipState.fStencilClipTarget = fCurrDrawState.fRenderTarget;
        }

        fClipState.fClipIsDirty = false;
        if (!fClipState.fClipInStencil) {
            r = &fClip.getBounds();
        }
    }
    // Must flush the scissor after graphics state
    if (!flushGraphicsState(type)) {
        return false;
    }
    flushScissor(r);
    return true;
}


///////////////////////////////////////////////////////////////////////////////

void GrGpu::drawIndexed(PrimitiveType type,
                        int startVertex,
                        int startIndex,
                        int vertexCount,
                        int indexCount) {
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fVertexSrc ||
             fReservedGeometry.fLocked);
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fIndexSrc ||
             fReservedGeometry.fLocked);

    if (!setupClipAndFlushState(type)) {
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

void GrGpu::drawNonIndexed(PrimitiveType type,
                           int startVertex,
                           int vertexCount) {
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fVertexSrc ||
             fReservedGeometry.fLocked);

    if (!setupClipAndFlushState(type)) {
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
        fVertexPool = new GrVertexBufferAllocPool(this, true, VERTEX_POOL_VB_SIZE, 1);
    } else {
        fVertexPool->reset();
    }
}

void GrGpu::prepareIndexPool() {
    if (NULL == fVertexPool) {
        fIndexPool = new GrIndexBufferAllocPool(this, true, 0, 1);
    } else {
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

        prepareVertexPool();

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

        prepareIndexPool();

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

///////////////////////////////////////////////////////////////////////////////

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




