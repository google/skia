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
#include "GrVertexBufferAllocPool.h"
#include "GrGpu.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrVertexBufferAllocPool* pool) :
        fDraws(DRAWS_BLOCK_SIZE, fDrawsStorage),
        fStates(STATES_BLOCK_SIZE, fStatesStorage),
        fClips(CLIPS_BLOCK_SIZE, fClipsStorage),
        fClipChanged(true),
        fCPUVertices((NULL == pool) ? 0 : VERTEX_BLOCK_SIZE),
        fBufferVertices(pool),
        fIndices(INDEX_BLOCK_SIZE),
        fCurrReservedVertices(NULL),
        fCurrReservedIndices(NULL),
        fCurrVertexBuffer(NULL),
        fReservedVertexBytes(0),
        fReservedIndexBytes(0),
        fUsedReservedVertexBytes(0),
        fUsedReservedIndexBytes(0) {
    GrAssert(NULL == pool || pool->getGpu()->supportsBufferLocking());
}

GrInOrderDrawBuffer::~GrInOrderDrawBuffer() {
    reset();
}

void GrInOrderDrawBuffer::initializeDrawStateAndClip(const GrDrawTarget& target) {
    this->copyDrawState(target);
    this->setClip(target.getClip());
}

void GrInOrderDrawBuffer::drawIndexed(PrimitiveType type,
                                      uint32_t startVertex,
                                      uint32_t startIndex,
                                      uint32_t vertexCount,
                                      uint32_t indexCount) {

    if (!vertexCount || !indexCount) {
        return;
    }

    Draw& draw = fDraws.push_back();
    draw.fType          = type;
    draw.fStartVertex   = startVertex;
    draw.fStartIndex    = startIndex;
    draw.fVertexCount   = vertexCount;
    draw.fIndexCount    = indexCount;
    draw.fClipChanged   = grabClip();
    draw.fStateChange   = grabState();

    draw.fVertexLayout = fGeometrySrc.fVertexLayout;
    switch (fGeometrySrc.fVertexSrc) {
    case kArray_GeometrySrcType:
        draw.fUseVertexBuffer = false;
        draw.fVertexArray = fGeometrySrc.fVertexArray;
        break;
    case kReserved_GeometrySrcType: {
        draw.fUseVertexBuffer = NULL != fBufferVertices;
        if (draw.fUseVertexBuffer) {
            draw.fVertexBuffer = fCurrVertexBuffer;
            draw.fStartVertex += fCurrStartVertex;
        } else {
            draw.fVertexArray = fCurrReservedVertices;
        }
        size_t vertexBytes = (vertexCount + startVertex) *
                             VertexSize(fGeometrySrc.fVertexLayout);
        fUsedReservedVertexBytes = GrMax(fUsedReservedVertexBytes,
                                         vertexBytes);
        } break;
    case kBuffer_GeometrySrcType:
        draw.fUseVertexBuffer = true;
        draw.fVertexBuffer = fGeometrySrc.fVertexBuffer;
        break;
    }

    switch (fGeometrySrc.fIndexSrc) {
    case kArray_GeometrySrcType:
        draw.fUseIndexBuffer = false;
        draw.fIndexArray = fGeometrySrc.fIndexArray;
        break;
    case kReserved_GeometrySrcType: {
        draw.fUseIndexBuffer = false;
        draw.fIndexArray = fCurrReservedIndices;
        size_t indexBytes = (indexCount + startIndex) * sizeof(uint16_t);
        fUsedReservedIndexBytes = GrMax(fUsedReservedIndexBytes, indexBytes);
        } break;
    case kBuffer_GeometrySrcType:
        draw.fUseIndexBuffer = true;
        draw.fIndexBuffer = fGeometrySrc.fIndexBuffer;
        break;
    }
}

void GrInOrderDrawBuffer::drawNonIndexed(PrimitiveType type,
                                         uint32_t startVertex,
                                         uint32_t vertexCount) {
    if (!vertexCount) {
        return;
    }

    Draw& draw = fDraws.push_back();
    draw.fType          = type;
    draw.fStartVertex   = startVertex;
    draw.fStartIndex    = 0;
    draw.fVertexCount   = vertexCount;
    draw.fIndexCount    = 0;

    draw.fClipChanged   = grabClip();
    draw.fStateChange   = grabState();

    draw.fVertexLayout = fGeometrySrc.fVertexLayout;
    switch (fGeometrySrc.fVertexSrc) {
    case kArray_GeometrySrcType:
        draw.fUseVertexBuffer = false;
        draw.fVertexArray = fGeometrySrc.fVertexArray;
        break;
    case kReserved_GeometrySrcType: {
        draw.fUseVertexBuffer = NULL != fBufferVertices;
        if (draw.fUseVertexBuffer) {
            draw.fVertexBuffer = fCurrVertexBuffer;
            draw.fStartVertex += fCurrStartVertex;
        } else {
            draw.fVertexArray = fCurrReservedVertices;
        }
        size_t vertexBytes = (vertexCount + startVertex) *
                             VertexSize(fGeometrySrc.fVertexLayout);
        fUsedReservedVertexBytes = GrMax(fUsedReservedVertexBytes,
                                         vertexBytes);
        } break;
    case kBuffer_GeometrySrcType:
        draw.fUseVertexBuffer = true;
        draw.fVertexBuffer = fGeometrySrc.fVertexBuffer;
        break;
    }
}

void GrInOrderDrawBuffer::reset() {
    GrAssert(!fReservedGeometry.fLocked);
    uint32_t numStates = fStates.count();
    for (uint32_t i = 0; i < numStates; ++i) {
        GrTexture* tex = accessSavedDrawState(fStates[i]).fTexture;
        if (NULL != tex) {
            tex->unref();
        }
    }
    fDraws.reset();
    fStates.reset();
    if (NULL == fBufferVertices) {
        fCPUVertices.reset();
    } else {
        fBufferVertices->reset();
    }
    fIndices.reset();
    fClips.reset();
}

void GrInOrderDrawBuffer::playback(GrDrawTarget* target) {
    GrAssert(NULL != target);
    GrAssert(target != this); // not considered and why?

    uint32_t numDraws = fDraws.count();
    if (!numDraws) {
        return;
    }

    if (NULL != fBufferVertices) {
        fBufferVertices->unlock();
    }

    GrDrawTarget::AutoStateRestore asr(target);
    GrDrawTarget::AutoClipRestore acr(target);
    // important to not mess with reserve/lock geometry in the target with this
    // on the stack.
    GrDrawTarget::AutoGeometrySrcRestore agsr(target);

    uint32_t currState = ~0;
    uint32_t currClip  = ~0;

    for (uint32_t i = 0; i < numDraws; ++i) {
        const Draw& draw = fDraws[i];
        if (draw.fStateChange) {
            ++currState;
            target->restoreDrawState(fStates[currState]);
        }
        if (draw.fClipChanged) {
            ++currClip;
            target->setClip(fClips[currClip]);
        }
        if (draw.fUseVertexBuffer) {
            target->setVertexSourceToBuffer(draw.fVertexBuffer, draw.fVertexLayout);
        } else {
            target->setVertexSourceToArray(draw.fVertexArray, draw.fVertexLayout);
        }
        if (draw.fIndexCount) {
            if (draw.fUseIndexBuffer) {
                target->setIndexSourceToBuffer(draw.fIndexBuffer);
            } else {
                target->setIndexSourceToArray(draw.fIndexArray);
            }
            target->drawIndexed(draw.fType,
                                draw.fStartVertex,
                                draw.fStartIndex,
                                draw.fVertexCount,
                                draw.fIndexCount);
        } else {
            target->drawNonIndexed(draw.fType,
                                   draw.fStartVertex,
                                   draw.fVertexCount);
        }
    }
}

bool GrInOrderDrawBuffer::geometryHints(GrVertexLayout vertexLayout,
                                        int32_t* vertexCount,
                                        int32_t* indexCount) const {
    bool flush = false;
    if (NULL != indexCount) {
        *indexCount  = -1;
    }
    if (NULL != vertexCount) {
        if (NULL != fBufferVertices) {
            // we will recommend a flush if the verts could fit in a single
            // preallocated vertex buffer but none are left and it can't fit
            // in the current VB (which may not be prealloced).
            if (*vertexCount > fBufferVertices->currentBufferVertices(vertexLayout) &&
                (!fBufferVertices->preallocatedBuffersRemaining() &&
                 *vertexCount <= fBufferVertices->preallocatedBufferVertices(vertexLayout))) {

                flush = true;
            }
            *vertexCount = fBufferVertices->currentBufferVertices(vertexLayout);
        } else {
            *vertexCount = -1;
        }
    }
    return flush;
}

bool GrInOrderDrawBuffer::acquireGeometryHelper(GrVertexLayout vertexLayout,
                                                void**         vertices,
                                                void**         indices) {
    if (fReservedGeometry.fVertexCount) {
        fReservedVertexBytes = VertexSize(vertexLayout) *
                               fReservedGeometry.fVertexCount;
        if (NULL == fBufferVertices) {
            fCurrReservedVertices = fCPUVertices.alloc(fReservedVertexBytes);
        } else {
            fCurrReservedVertices = fBufferVertices->alloc(vertexLayout,
                                                           fReservedGeometry.fVertexCount,
                                                           &fCurrVertexBuffer,
                                                           &fCurrStartVertex);
        }
        if (NULL != vertices) {
            *vertices = fCurrReservedVertices;
        }
        if (NULL == fCurrReservedVertices) {
            return false;
        }
    }
    if (fReservedGeometry.fIndexCount) {
        fReservedIndexBytes = sizeof(uint16_t) * fReservedGeometry.fIndexCount;
        fCurrReservedIndices = fIndices.alloc(fReservedIndexBytes);
        if (NULL != indices) {
            *indices = fCurrReservedIndices;
        }
        if (NULL == fCurrReservedIndices) {
            return false;
        }
    }
    return true;
}

void GrInOrderDrawBuffer::releaseGeometryHelper() {
    GrAssert(fUsedReservedVertexBytes <= fReservedVertexBytes);
    GrAssert(fUsedReservedIndexBytes <= fReservedIndexBytes);

    size_t vertexSlack = fReservedVertexBytes - fUsedReservedVertexBytes;
    if (NULL == fBufferVertices) {
        fCPUVertices.release(vertexSlack);
    } else {
        fBufferVertices->release(vertexSlack);
        GR_DEBUGCODE(fCurrVertexBuffer = NULL);
        GR_DEBUGCODE(fCurrStartVertex  = 0);
    }

    fIndices.release(fReservedIndexBytes - fUsedReservedIndexBytes);

    fCurrReservedVertices = NULL;
    fCurrReservedIndices  = NULL;
    fReservedVertexBytes = 0;
    fReservedIndexBytes  = 0;
    fUsedReservedVertexBytes = 0;
    fUsedReservedIndexBytes  = 0;
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
        if (NULL != fCurrDrawState.fTexture) {
            fCurrDrawState.fTexture->ref();
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

