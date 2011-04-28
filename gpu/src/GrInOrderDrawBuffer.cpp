/*
    Copyright 2011 Google Inc.

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


#include "GrInOrderDrawBuffer.h"
#include "GrTexture.h"
#include "GrBufferAllocPool.h"
#include "GrIndexBuffer.h"
#include "GrVertexBuffer.h"
#include "GrGpu.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool) :
        fDraws(&fDrawStorage),
        fStates(&fStateStorage),
        fClears(&fClearStorage),
        fClips(&fClipStorage),
        fClipSet(true),

        fLastRectVertexLayout(0),
        fQuadIndexBuffer(NULL),
        fMaxQuads(0),
        fCurrQuad(0),

        fVertexPool(*vertexPool),
        fCurrPoolVertexBuffer(NULL),
        fCurrPoolStartVertex(0),
        fIndexPool(*indexPool),
        fCurrPoolIndexBuffer(NULL),
        fCurrPoolStartIndex(0),
        fReservedVertexBytes(0),
        fReservedIndexBytes(0),
        fUsedReservedVertexBytes(0),
        fUsedReservedIndexBytes(0) {
    GrAssert(NULL != vertexPool);
    GrAssert(NULL != indexPool);
}

GrInOrderDrawBuffer::~GrInOrderDrawBuffer() {
    this->reset();
    GrSafeUnref(fQuadIndexBuffer);
}

void GrInOrderDrawBuffer::initializeDrawStateAndClip(const GrDrawTarget& target) {
    this->copyDrawState(target);
    this->setClip(target.getClip());
}

void GrInOrderDrawBuffer::setQuadIndexBuffer(const GrIndexBuffer* indexBuffer) {
    bool newIdxBuffer = fQuadIndexBuffer != indexBuffer;
    if (newIdxBuffer) {
        GrSafeUnref(fQuadIndexBuffer);
        fQuadIndexBuffer = indexBuffer;
        GrSafeRef(fQuadIndexBuffer);
        fCurrQuad = 0;
        fMaxQuads = (NULL == indexBuffer) ? 0 : indexBuffer->maxQuads();
    } else {
        GrAssert((NULL == indexBuffer && 0 == fMaxQuads) ||
                 (indexBuffer->maxQuads() == fMaxQuads));
    }
}

void GrInOrderDrawBuffer::drawRect(const GrRect& rect,
                                   const GrMatrix* matrix,
                                   StageBitfield stageEnableBitfield,
                                   const GrRect* srcRects[],
                                   const GrMatrix* srcMatrices[]) {

    GrAssert(!(NULL == fQuadIndexBuffer && fCurrQuad));
    GrAssert(!(fDraws.empty() && fCurrQuad));
    GrAssert(!(0 != fMaxQuads && NULL == fQuadIndexBuffer));

    // if we have a quad IB then either append to the previous run of
    // rects or start a new run
    if (fMaxQuads) {

        bool appendToPreviousDraw = false;
        GrVertexLayout layout = GetRectVertexLayout(stageEnableBitfield, srcRects);
        AutoReleaseGeometry geo(this, layout, 4, 0);
        AutoViewMatrixRestore avmr(this);
        GrMatrix combinedMatrix = this->getViewMatrix();
        this->setViewMatrix(GrMatrix::I());
        if (NULL != matrix) {
            combinedMatrix.preConcat(*matrix);
        }

        SetRectVertices(rect, &combinedMatrix, srcRects, srcMatrices, layout, geo.vertices());

        // we don't want to miss an opportunity to batch rects together
        // simply because the clip has changed if the clip doesn't affect
        // the rect.
        bool disabledClip = false;
        if (this->isClipState() && fClip.isRect()) {

            GrRect clipRect = fClip.getRect(0);
            // If the clip rect touches the edge of the viewport, extended it
            // out (close) to infinity to avoid bogus intersections.
            // We might consider a more exact clip to viewport if this
            // conservative test fails.
            const GrRenderTarget* target = this->getRenderTarget();
            if (0 >= clipRect.fLeft) {
                clipRect.fLeft = GR_ScalarMin;
            }
            if (target->width() <= clipRect.fRight) {
                clipRect.fRight = GR_ScalarMax;
            }
            if (0 >= clipRect.top()) {
                clipRect.fTop = GR_ScalarMin;
            }
            if (target->height() <= clipRect.fBottom) {
                clipRect.fBottom = GR_ScalarMax;
            }
            int stride = VertexSize(layout);
            bool insideClip = true;
            for (int v = 0; v < 4; ++v) {
                const GrPoint& p = *GetVertexPoint(geo.vertices(), v, stride);
                if (!clipRect.contains(p)) {
                    insideClip = false;
                    break;
                }
            }
            if (insideClip) {
                this->disableState(kClip_StateBit);
                disabledClip = true;
            }
        }
        if (!needsNewClip() && !needsNewState() && fCurrQuad > 0 &&
            fCurrQuad < fMaxQuads && layout == fLastRectVertexLayout) {

            int vsize = VertexSize(layout);

            Draw& lastDraw = fDraws.back();

            GrAssert(lastDraw.fIndexBuffer == fQuadIndexBuffer);
            GrAssert(kTriangles_PrimitiveType == lastDraw.fPrimitiveType);
            GrAssert(0 == lastDraw.fVertexCount % 4);
            GrAssert(0 == lastDraw.fIndexCount % 6);
            GrAssert(0 == lastDraw.fStartIndex);

            bool clearSinceLastDraw =
                            fClears.count() && 
                            fClears.back().fBeforeDrawIdx == fDraws.count();

            appendToPreviousDraw =  !clearSinceLastDraw &&
                                    lastDraw.fVertexBuffer == fCurrPoolVertexBuffer &&
                                   (fCurrQuad * 4 + lastDraw.fStartVertex) == fCurrPoolStartVertex;
            if (appendToPreviousDraw) {
                lastDraw.fVertexCount += 4;
                lastDraw.fIndexCount += 6;
                fCurrQuad += 1;
                GrAssert(0 == fUsedReservedVertexBytes);
                fUsedReservedVertexBytes = 4 * vsize;
            }
        }
        if (!appendToPreviousDraw) {
            this->setIndexSourceToBuffer(fQuadIndexBuffer);
            drawIndexed(kTriangles_PrimitiveType, 0, 0, 4, 6);
            fCurrQuad = 1;
            fLastRectVertexLayout = layout;
        }
        if (disabledClip) {
            this->enableState(kClip_StateBit);
        }
    } else {
        INHERITED::drawRect(rect, matrix, stageEnableBitfield, srcRects, srcMatrices);
    }
}

void GrInOrderDrawBuffer::drawIndexed(GrPrimitiveType primitiveType,
                                      int startVertex,
                                      int startIndex,
                                      int vertexCount,
                                      int indexCount) {

    if (!vertexCount || !indexCount) {
        return;
    }

    fCurrQuad = 0;

    Draw& draw = fDraws.push_back();
    draw.fPrimitiveType = primitiveType;
    draw.fStartVertex   = startVertex;
    draw.fStartIndex    = startIndex;
    draw.fVertexCount   = vertexCount;
    draw.fIndexCount    = indexCount;

    draw.fClipChanged = this->needsNewClip();
    if (draw.fClipChanged) {
       this->pushClip();
    }

    draw.fStateChanged = this->needsNewState();
    if (draw.fStateChanged) {
        this->pushState();
    }

    draw.fVertexLayout = fGeometrySrc.fVertexLayout;
    switch (fGeometrySrc.fVertexSrc) {
    case kBuffer_GeometrySrcType:
        draw.fVertexBuffer = fGeometrySrc.fVertexBuffer;
        break;
    case kReserved_GeometrySrcType: {
        size_t vertexBytes = (vertexCount + startVertex) *
        VertexSize(fGeometrySrc.fVertexLayout);
        fUsedReservedVertexBytes = GrMax(fUsedReservedVertexBytes, vertexBytes);
    } // fallthrough
    case kArray_GeometrySrcType:
        draw.fVertexBuffer = fCurrPoolVertexBuffer;
        draw.fStartVertex += fCurrPoolStartVertex;
        break;
    default:
        GrCrash("unknown geom src type");
    }
    draw.fVertexBuffer->ref();

    switch (fGeometrySrc.fIndexSrc) {
    case kBuffer_GeometrySrcType:
        draw.fIndexBuffer = fGeometrySrc.fIndexBuffer;
        break;
    case kReserved_GeometrySrcType: {
        size_t indexBytes = (indexCount + startIndex) * sizeof(uint16_t);
        fUsedReservedIndexBytes = GrMax(fUsedReservedIndexBytes, indexBytes);
    } // fallthrough
    case kArray_GeometrySrcType:
        draw.fIndexBuffer = fCurrPoolIndexBuffer;
        draw.fStartIndex += fCurrPoolStartVertex;
        break;
    default:
        GrCrash("unknown geom src type");
    }
    draw.fIndexBuffer->ref();
}

void GrInOrderDrawBuffer::drawNonIndexed(GrPrimitiveType primitiveType,
                                         int startVertex,
                                         int vertexCount) {
    if (!vertexCount) {
        return;
    }

    fCurrQuad = 0;

    Draw& draw = fDraws.push_back();
    draw.fPrimitiveType = primitiveType;
    draw.fStartVertex   = startVertex;
    draw.fStartIndex    = 0;
    draw.fVertexCount   = vertexCount;
    draw.fIndexCount    = 0;

    draw.fClipChanged = this->needsNewClip();
    if (draw.fClipChanged) {
        this->pushClip();
    }

    draw.fStateChanged = this->needsNewState();
    if (draw.fStateChanged) {
        this->pushState();
    }

    draw.fVertexLayout = fGeometrySrc.fVertexLayout;
    switch (fGeometrySrc.fVertexSrc) {
    case kBuffer_GeometrySrcType:
        draw.fVertexBuffer = fGeometrySrc.fVertexBuffer;
        break;
    case kReserved_GeometrySrcType: {
        size_t vertexBytes = (vertexCount + startVertex) *
        VertexSize(fGeometrySrc.fVertexLayout);
        fUsedReservedVertexBytes = GrMax(fUsedReservedVertexBytes,
                                         vertexBytes);
    } // fallthrough
    case kArray_GeometrySrcType:
        draw.fVertexBuffer = fCurrPoolVertexBuffer;
        draw.fStartVertex += fCurrPoolStartVertex;
        break;
    default:
        GrCrash("unknown geom src type");
    }
    draw.fVertexBuffer->ref();
    draw.fIndexBuffer = NULL;
}

void GrInOrderDrawBuffer::clear(const GrIRect* rect, GrColor color) {
    GrIRect r;
    if (NULL == rect) {
        // We could do something smart and remove previous draws and clears to
        // the current render target. If we get that smart we have to make sure
        // those draws aren't read before this clear (render-to-texture).
        r.setLTRB(0, 0, 
                  this->getRenderTarget()->width(), 
                  this->getRenderTarget()->height());
        rect = &r;
    }
    Clear& clr = fClears.push_back();
    clr.fColor = color;
    clr.fBeforeDrawIdx = fDraws.count();
    clr.fRect = *rect;
}

void GrInOrderDrawBuffer::reset() {
    GrAssert(!fReservedGeometry.fLocked);
    uint32_t numStates = fStates.count();
    for (uint32_t i = 0; i < numStates; ++i) {
        const DrState& dstate = this->accessSavedDrawState(fStates[i]);
        for (int s = 0; s < kNumStages; ++s) {
            GrSafeUnref(dstate.fTextures[s]);
        }
        GrSafeUnref(dstate.fRenderTarget);
    }
    int numDraws = fDraws.count();
    for (int d = 0; d < numDraws; ++d) {
        // we always have a VB, but not always an IB
        GrAssert(NULL != fDraws[d].fVertexBuffer);
        fDraws[d].fVertexBuffer->unref();
        GrSafeUnref(fDraws[d].fIndexBuffer);
    }
    fDraws.reset();
    fStates.reset();

    fClears.reset();

    fVertexPool.reset();
    fIndexPool.reset();

    fClips.reset();

    fCurrQuad = 0;
}

void GrInOrderDrawBuffer::playback(GrDrawTarget* target) {
    GrAssert(!fReservedGeometry.fLocked);
    GrAssert(NULL != target);
    GrAssert(target != this); // not considered and why?

    int numDraws = fDraws.count();
    if (!numDraws) {
        return;
    }

    fVertexPool.unlock();
    fIndexPool.unlock();

    GrDrawTarget::AutoStateRestore asr(target);
    GrDrawTarget::AutoClipRestore acr(target);
    // important to not mess with reserve/lock geometry in the target with this
    // on the stack.
    GrDrawTarget::AutoGeometrySrcRestore agsr(target);

    int currState = ~0;
    int currClip  = ~0;
    int currClear = 0;

    for (int i = 0; i < numDraws; ++i) {
        while (currClear < fClears.count() && 
               i == fClears[currClear].fBeforeDrawIdx) {
            target->clear(&fClears[currClear].fRect, fClears[currClear].fColor);
            ++currClear;
        }

        const Draw& draw = fDraws[i];
        if (draw.fStateChanged) {
            ++currState;
            target->restoreDrawState(fStates[currState]);
        }
        if (draw.fClipChanged) {
            ++currClip;
            target->setClip(fClips[currClip]);
        }

        target->setVertexSourceToBuffer(draw.fVertexLayout, draw.fVertexBuffer);

        if (draw.fIndexCount) {
            target->setIndexSourceToBuffer(draw.fIndexBuffer);
        }

        if (draw.fIndexCount) {
            target->drawIndexed(draw.fPrimitiveType,
                                draw.fStartVertex,
                                draw.fStartIndex,
                                draw.fVertexCount,
                                draw.fIndexCount);
        } else {
            target->drawNonIndexed(draw.fPrimitiveType,
                                   draw.fStartVertex,
                                   draw.fVertexCount);
        }
    }
    while (currClear < fClears.count()) {
        GrAssert(fDraws.count() == fClears[currClear].fBeforeDrawIdx);
        target->clear(&fClears[currClear].fRect, fClears[currClear].fColor);
        ++currClear;
    }
}

bool GrInOrderDrawBuffer::geometryHints(GrVertexLayout vertexLayout,
                                        int* vertexCount,
                                        int* indexCount) const {
    // we will recommend a flush if the data could fit in a single
    // preallocated buffer but none are left and it can't fit
    // in the current buffer (which may not be prealloced).
    bool flush = false;
    if (NULL != indexCount) {
        int32_t currIndices = fIndexPool.currentBufferIndices();
        if (*indexCount > currIndices &&
            (!fIndexPool.preallocatedBuffersRemaining() &&
             *indexCount <= fIndexPool.preallocatedBufferIndices())) {

            flush = true;
        }
        *indexCount = currIndices;
    }
    if (NULL != vertexCount) {
        int32_t currVertices = fVertexPool.currentBufferVertices(vertexLayout);
        if (*vertexCount > currVertices &&
            (!fVertexPool.preallocatedBuffersRemaining() &&
             *vertexCount <= fVertexPool.preallocatedBufferVertices(vertexLayout))) {

            flush = true;
        }
        *vertexCount = currVertices;
    }
    return flush;
}

bool GrInOrderDrawBuffer::onAcquireGeometry(GrVertexLayout vertexLayout,
                                            void**         vertices,
                                            void**         indices) {
    GrAssert(!fReservedGeometry.fLocked);
    if (fReservedGeometry.fVertexCount) {
        GrAssert(NULL != vertices);
        GrAssert(0 == fReservedVertexBytes);
        GrAssert(0 == fUsedReservedVertexBytes);

        fReservedVertexBytes = VertexSize(vertexLayout) *
                               fReservedGeometry.fVertexCount;
        *vertices = fVertexPool.makeSpace(vertexLayout,
                                          fReservedGeometry.fVertexCount,
                                          &fCurrPoolVertexBuffer,
                                          &fCurrPoolStartVertex);
        if (NULL == *vertices) {
            return false;
        }
    }
    if (fReservedGeometry.fIndexCount) {
        GrAssert(NULL != indices);
        GrAssert(0 == fReservedIndexBytes);
        GrAssert(0 == fUsedReservedIndexBytes);

        *indices = fIndexPool.makeSpace(fReservedGeometry.fIndexCount,
                                        &fCurrPoolIndexBuffer,
                                        &fCurrPoolStartIndex);
        if (NULL == *indices) {
            fVertexPool.putBack(fReservedVertexBytes);
            fReservedVertexBytes = 0;
            fCurrPoolVertexBuffer = NULL;
            return false;
        }
    }
    return true;
}

void GrInOrderDrawBuffer::onReleaseGeometry() {
    GrAssert(fUsedReservedVertexBytes <= fReservedVertexBytes);
    GrAssert(fUsedReservedIndexBytes <= fReservedIndexBytes);

    size_t vertexSlack = fReservedVertexBytes - fUsedReservedVertexBytes;
    fVertexPool.putBack(vertexSlack);

    size_t indexSlack = fReservedIndexBytes - fUsedReservedIndexBytes;
    fIndexPool.putBack(indexSlack);

    fReservedVertexBytes = 0;
    fReservedIndexBytes  = 0;
    fUsedReservedVertexBytes = 0;
    fUsedReservedIndexBytes  = 0;
    fCurrPoolVertexBuffer = 0;
    fCurrPoolStartVertex = 0;

}

void GrInOrderDrawBuffer::onSetVertexSourceToArray(const void* vertexArray,
                                                   int vertexCount) {
    GrAssert(!fReservedGeometry.fLocked || !fReservedGeometry.fVertexCount);
#if GR_DEBUG
    bool success =
#endif
    fVertexPool.appendVertices(fGeometrySrc.fVertexLayout,
                               vertexCount,
                               vertexArray,
                               &fCurrPoolVertexBuffer,
                               &fCurrPoolStartVertex);
    GR_DEBUGASSERT(success);
}

void GrInOrderDrawBuffer::onSetIndexSourceToArray(const void* indexArray,
                                                  int indexCount) {
    GrAssert(!fReservedGeometry.fLocked || !fReservedGeometry.fIndexCount);
#if GR_DEBUG
    bool success =
#endif
    fIndexPool.appendIndices(indexCount,
                             indexArray,
                             &fCurrPoolIndexBuffer,
                             &fCurrPoolStartIndex);
    GR_DEBUGASSERT(success);
}

bool GrInOrderDrawBuffer::needsNewState() const {
     if (fStates.empty()) {
        return true;
     } else {
         const DrState& old = this->accessSavedDrawState(fStates.back());
        return old != fCurrDrawState;
     }
}

void GrInOrderDrawBuffer::pushState() {
    for (int s = 0; s < kNumStages; ++s) {
        GrSafeRef(fCurrDrawState.fTextures[s]);
    }
    GrSafeRef(fCurrDrawState.fRenderTarget);
    this->saveCurrentDrawState(&fStates.push_back());
 }

bool GrInOrderDrawBuffer::needsNewClip() const {
   if (fCurrDrawState.fFlagBits & kClip_StateBit) {
       if (fClips.empty() || (fClipSet && fClips.back() != fClip)) {
           return true;
       }
    }
    return false;
}

void GrInOrderDrawBuffer::pushClip() {
    fClips.push_back() = fClip;
    fClipSet = false;
}

void GrInOrderDrawBuffer::clipWillBeSet(const GrClip& newClip)  {
    fClipSet = true;
}
