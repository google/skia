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
                 fNPOTTextureSupport(kNone_NPOTTextureType),
                 fQuadIndexBuffer(NULL) {
#if GR_DEBUG
//    gr_run_unittests();
#endif
    resetStats();
}

GrGpu::~GrGpu() {
    if (NULL != fQuadIndexBuffer) {
        fQuadIndexBuffer->unref();
    }
}

void GrGpu::resetContext() {
}

void GrGpu::unimpl(const char msg[]) {
//    GrPrintf("--- GrGpu unimplemented(\"%s\")\n", msg);
}

///////////////////////////////////////////////////////////////////////////////

bool GrGpu::canDisableBlend() const {
    if ((kOne_BlendCoeff == fCurrDrawState.fSrcBlend) &&
        (kZero_BlendCoeff == fCurrDrawState.fDstBlend)) {
            return true;
    }

    // If we have vertex color without alpha then we can't force blend off
    if ((fGeometrySrc.fVertexLayout & kColor_VertexLayoutBit) ||
         0xff != GrColorUnpackA(fCurrDrawState.fColor)) {
        return false;
    }

    // If the src coef will always be 1...
    if (kSA_BlendCoeff != fCurrDrawState.fSrcBlend &&
        kOne_BlendCoeff != fCurrDrawState.fSrcBlend) {
        return false;
    }

    // ...and the dst coef is always 0...
    if (kISA_BlendCoeff != fCurrDrawState.fDstBlend &&
        kZero_BlendCoeff != fCurrDrawState.fDstBlend) {
        return false;
    }

    // ...and there isn't a texture with an alpha channel...
    for (int s = 0; s < kNumStages; ++s) {
        if (VertexUsesStage(s, fGeometrySrc.fVertexLayout)) {
            GrAssert(NULL != fCurrDrawState.fTextures[s]);
            GrTexture::PixelConfig config = fCurrDrawState.fTextures[s]->config();
            
            if (GrTexture::kRGB_565_PixelConfig != config &&
                GrTexture::kRGBX_8888_PixelConfig != config) {
                return false;
            }
        }
    }

    // ...then we disable blend.
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static const int MAX_QUADS = 512; // max possible: (1 << 14) - 1;

GR_STATIC_ASSERT(4 * MAX_QUADS <= 65535);

static inline void fillIndices(uint16_t* indices, int quadCount) {
    for (int i = 0; i < quadCount; ++i) {
        indices[6 * i + 0] = 4 * i + 0;
        indices[6 * i + 1] = 4 * i + 1;
        indices[6 * i + 2] = 4 * i + 2;
        indices[6 * i + 3] = 4 * i + 0;
        indices[6 * i + 4] = 4 * i + 2;
        indices[6 * i + 5] = 4 * i + 3;
    }
}

const GrIndexBuffer* GrGpu::quadIndexBuffer() const {
    if (NULL == fQuadIndexBuffer) {
        static const int SIZE = sizeof(uint16_t) * 6 * MAX_QUADS;
        GrGpu* me = const_cast<GrGpu*>(this);
        fQuadIndexBuffer = me->createIndexBuffer(SIZE, false);
        if (NULL != fQuadIndexBuffer) {
            uint16_t* indices = (uint16_t*)fQuadIndexBuffer->lock();
            if (NULL != indices) {
                fillIndices(indices, MAX_QUADS);
                fQuadIndexBuffer->unlock();
            } else {
                indices = (uint16_t*)GrMalloc(SIZE);
                fillIndices(indices, MAX_QUADS);
                if (!fQuadIndexBuffer->updateData(indices, SIZE)) {
                    fQuadIndexBuffer->unref();
                    fQuadIndexBuffer = NULL;
                    GrAssert(!"Can't get indices into buffer!");
                }
                GrFree(indices);
            }
        }
    }

    return fQuadIndexBuffer;
}

int GrGpu::maxQuadsInIndexBuffer() const {
    return (NULL == this->quadIndexBuffer()) ? 0 : MAX_QUADS;
}

///////////////////////////////////////////////////////////////////////////////

void GrGpu::clipWillChange(const GrClip& clip) {
    if (clip != fClip) {
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
            this->disableState(kClip_StateBit);
            eraseStencilClip();

            int rectTotal = fClip.countRects();
            static const int PtsPerRect = 4;
            // this may be called while geometry is already reserved by the
            // client. So we use our own vertex array where we avoid malloc
            // if we have 4 or fewer rects.
            GrAutoSTMalloc<PtsPerRect * 4, GrPoint> vertices(PtsPerRect *
                                                             rectTotal);
            this->setVertexSourceToArray(vertices.get(), 0);
            int currRect = 0;
            while (currRect < rectTotal) {
                int rectCount = GrMin(this->maxQuadsInIndexBuffer(),
                                      rectTotal - currRect);

                GrPoint* verts = (GrPoint*)vertices +
                                 (currRect * PtsPerRect);

                for (int i = 0; i < rectCount; i++) {
                    GrRect r(fClip.getRects()[i + currRect]);
                    verts = r.setRectFan(verts);
                }
                this->setIndexSourceToBuffer(quadIndexBuffer());

                this->setViewMatrix(GrMatrix::I());
                this->setStencilPass((GrDrawTarget::StencilPass)kSetClip_StencilPass);
                this->drawIndexed(GrGpu::kTriangles_PrimitiveType,
                                  currRect * PtsPerRect, 0,
                                  rectCount * PtsPerRect, rectCount * 6);

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
                        uint32_t startVertex,
                        uint32_t startIndex,
                        uint32_t vertexCount,
                        uint32_t indexCount) {
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

    setupGeometry(startVertex, startIndex, vertexCount, indexCount);

    drawIndexedHelper(type, startVertex, startIndex,
                      vertexCount, indexCount);
}

void GrGpu::drawNonIndexed(PrimitiveType type,
                           uint32_t startVertex,
                           uint32_t vertexCount) {
    GrAssert(kReserved_GeometrySrcType != fGeometrySrc.fVertexSrc ||
             fReservedGeometry.fLocked);

    if (!setupClipAndFlushState(type)) {
        return;
    }
#if GR_COLLECT_STATS
    fStats.fVertexCnt += vertexCount;
    fStats.fDrawCnt   += 1;
#endif

    setupGeometry(startVertex, 0, vertexCount, 0);

    drawNonIndexedHelper(type, startVertex, vertexCount);
}

bool GrGpu::acquireGeometryHelper(GrVertexLayout vertexLayout,
                                  void**         vertices,
                                  void**         indices) {
    GrAssert((fReservedGeometry.fVertexCount == 0) ||
             (NULL != vertices));
    if (NULL != vertices) {
        *vertices = fVertices.realloc(VertexSize(vertexLayout) *
                                      fReservedGeometry.fVertexCount);
        if (!*vertices && fReservedGeometry.fVertexCount) {
            return false;
        }
    }
    GrAssert((fReservedGeometry.fIndexCount == 0) ||
             (NULL != indices));
    if (NULL != indices) {
        *indices =  fIndices.realloc(sizeof(uint16_t) *
                                     fReservedGeometry.fIndexCount);
        if (!*indices && fReservedGeometry.fIndexCount) {
            return false;
        }
    }
    return true;
}

void GrGpu::releaseGeometryHelper() {
    return;
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
    false);




