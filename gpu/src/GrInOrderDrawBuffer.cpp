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


#include "GrInOrderDrawBuffer.h"
#include "GrTexture.h"
#include "GrBufferAllocPool.h"
#include "GrGpu.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool) :
        fDraws(DRAWS_BLOCK_SIZE, fDrawsStorage),
        fStates(STATES_BLOCK_SIZE, fStatesStorage),
        fClips(CLIPS_BLOCK_SIZE, fClipsStorage),
        fClipChanged(true),
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
    reset();
}

void GrInOrderDrawBuffer::initializeDrawStateAndClip(const GrDrawTarget& target) {
    this->copyDrawState(target);
    this->setClip(target.getClip());
}

void GrInOrderDrawBuffer::drawIndexed(PrimitiveType primitiveType,
                                      int startVertex,
                                      int startIndex,
                                      int vertexCount,
                                      int indexCount) {

    if (!vertexCount || !indexCount) {
        return;
    }

    Draw& draw = fDraws.push_back();
    draw.fPrimitiveType = primitiveType;
    draw.fStartVertex   = startVertex;
    draw.fStartIndex    = startIndex;
    draw.fVertexCount   = vertexCount;
    draw.fIndexCount    = indexCount;
    draw.fClipChanged   = grabClip();
    draw.fStateChanged  = grabState();

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
}

void GrInOrderDrawBuffer::drawNonIndexed(PrimitiveType primitiveType,
                                         int startVertex,
                                         int vertexCount) {
    if (!vertexCount) {
        return;
    }

    Draw& draw = fDraws.push_back();
    draw.fPrimitiveType = primitiveType;
    draw.fStartVertex   = startVertex;
    draw.fStartIndex    = 0;
    draw.fVertexCount   = vertexCount;
    draw.fIndexCount    = 0;

    draw.fClipChanged   = grabClip();
    draw.fStateChanged  = grabState();

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
}

void GrInOrderDrawBuffer::reset() {
    GrAssert(!fReservedGeometry.fLocked);
    uint32_t numStates = fStates.count();
    for (uint32_t i = 0; i < numStates; ++i) {
        for (int s = 0; s < kNumStages; ++s) {
            GrTexture* tex = accessSavedDrawState(fStates[i]).fTextures[s];
            if (NULL != tex) {
                tex->unref();
            }
        }
    }
    fDraws.reset();
    fStates.reset();

    fVertexPool.reset();
    fIndexPool.reset();

    fClips.reset();
}

void GrInOrderDrawBuffer::playback(GrDrawTarget* target) {
    GrAssert(NULL != target);
    GrAssert(target != this); // not considered and why?

    uint32_t numDraws = fDraws.count();
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

    uint32_t currState = ~0;
    uint32_t currClip  = ~0;

    for (uint32_t i = 0; i < numDraws; ++i) {
        const Draw& draw = fDraws[i];
        if (draw.fStateChanged) {
            ++currState;
            target->restoreDrawState(fStates[currState]);
        }
        if (draw.fClipChanged) {
            ++currClip;
            target->setClip(fClips[currClip]);
        }
        uint32_t vertexReserveCount = 0;
        uint32_t indexReserveCount = 0;

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
        if (vertexReserveCount || indexReserveCount) {
            target->releaseReservedGeometry();
        }
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

bool GrInOrderDrawBuffer::acquireGeometryHelper(GrVertexLayout vertexLayout,
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

void GrInOrderDrawBuffer::releaseGeometryHelper() {
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

void GrInOrderDrawBuffer::setVertexSourceToArrayHelper(const void* vertexArray,
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

void GrInOrderDrawBuffer::setIndexSourceToArrayHelper(const void* indexArray,
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

bool GrInOrderDrawBuffer::grabState() {
    bool newState;
    if (fStates.empty()) {
        newState = true;
    } else {
        const DrState& old = accessSavedDrawState(fStates.back());
        newState = old != fCurrDrawState;
    }
    if (newState) {
        for (int s = 0; s < kNumStages; ++s) {
            if (NULL != fCurrDrawState.fTextures[s]) {
                fCurrDrawState.fTextures[s]->ref();
            }
        }
        saveCurrentDrawState(&fStates.push_back());
    }
    return newState;
}

bool GrInOrderDrawBuffer::grabClip() {
    if ((fCurrDrawState.fFlagBits & kClip_StateBit) &&
        (fClipChanged || fClips.empty())) {

        fClips.push_back() = fClip;
        fClipChanged = false;
        return true;
    }
    return false;
}

void GrInOrderDrawBuffer::clipWillChange(const GrClip& clip)  {
    fClipChanged = true;
}

