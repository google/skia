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


#include "GrDrawTarget.h"
#include "GrGpuVertex.h"

#define VERTEX_LAYOUT_ASSERTS \
    GrAssert(!(vertexLayout & kTextFormat_VertexLayoutBit) ||           \
             vertexLayout == kTextFormat_VertexLayoutBit);              \
    GrAssert(!(vertexLayout & kSeparateTexCoord_VertexLayoutBit) ||     \
             !(vertexLayout & kPositionAsTexCoord_VertexLayoutBit));

size_t GrDrawTarget::VertexSize(GrVertexLayout vertexLayout) {
    VERTEX_LAYOUT_ASSERTS
    if ((vertexLayout & kTextFormat_VertexLayoutBit)) {
        return 2 * sizeof(GrGpuTextVertex);
    } else {
        size_t size = sizeof(GrPoint);
        if (vertexLayout & kSeparateTexCoord_VertexLayoutBit) {
            size += sizeof(GrPoint);
        }
        if (vertexLayout & kColor_VertexLayoutBit) {
            size += sizeof(GrColor);
        }
        return size;
    }
}

int GrDrawTarget::VertexTexCoordOffset(GrVertexLayout vertexLayout) {
    VERTEX_LAYOUT_ASSERTS
    if ((vertexLayout & kTextFormat_VertexLayoutBit)) {
        return sizeof(GrGpuTextVertex);
    } else if (vertexLayout & kSeparateTexCoord_VertexLayoutBit) {
        return sizeof(GrPoint);
    } else if (vertexLayout & kPositionAsTexCoord_VertexLayoutBit) {
        return 0;
    }
    return -1;
}

int  GrDrawTarget::VertexColorOffset(GrVertexLayout vertexLayout) {
    VERTEX_LAYOUT_ASSERTS
    if (vertexLayout & kColor_VertexLayoutBit) {
        if (vertexLayout & kSeparateTexCoord_VertexLayoutBit) {
            return 2 * sizeof(GrPoint);
        } else {
            return sizeof(GrPoint);
        }
    }
    return -1;
}

int GrDrawTarget::VertexSizeAndOffsets(GrVertexLayout vertexLayout,
                                       int* texCoordOffset,
                                       int* colorOffset) {
    VERTEX_LAYOUT_ASSERTS

    GrAssert(NULL != texCoordOffset);
    GrAssert(NULL != colorOffset);

    if ((vertexLayout & kTextFormat_VertexLayoutBit)) {
        *texCoordOffset = sizeof(GrGpuTextVertex);
        *colorOffset = 0;
        return 2 * sizeof(GrGpuTextVertex);
    } else {
        size_t size = sizeof(GrPoint);
        if (vertexLayout & kSeparateTexCoord_VertexLayoutBit) {
            *texCoordOffset = sizeof(GrPoint);
            size += sizeof(GrPoint);
        } else if (vertexLayout & kPositionAsTexCoord_VertexLayoutBit) {
            *texCoordOffset = 0;
        } else {
            *texCoordOffset = -1;
        }
        if (vertexLayout & kColor_VertexLayoutBit) {
            *colorOffset = size;
            size += sizeof(GrColor);
        } else {
            *colorOffset = -1;
        }
        return size;
    }
}

bool GrDrawTarget::VertexHasTexCoords(GrVertexLayout vertexLayout) {
    return !!(vertexLayout & (kSeparateTexCoord_VertexLayoutBit   |
                              kPositionAsTexCoord_VertexLayoutBit |
                              kTextFormat_VertexLayoutBit));
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::GrDrawTarget() {
    fReservedGeometry.fLocked = false;
#if GR_DEBUG
    fReservedGeometry.fVertexCount  = ~0;
    fReservedGeometry.fIndexCount   = ~0;
#endif
    fGeometrySrc.fVertexSrc = kReserved_GeometrySrcType;
    fGeometrySrc.fIndexSrc  = kReserved_GeometrySrcType;
}

void GrDrawTarget::setClip(const GrClip& clip) {
    clipWillChange(clip);
    fClip = clip;
}

const GrClip& GrDrawTarget::getClip() const {
    return fClip;
}

void GrDrawTarget::setTexture(GrTexture* tex) {
    fCurrDrawState.fTexture = tex;
}

GrTexture* GrDrawTarget::currentTexture() const {
    return fCurrDrawState.fTexture;
}

void GrDrawTarget::setRenderTarget(GrRenderTarget* target) {
    fCurrDrawState.fRenderTarget = target;
}

GrRenderTarget* GrDrawTarget::currentRenderTarget() const {
    return fCurrDrawState.fRenderTarget;
}

void GrDrawTarget::concatViewMatrix(const GrMatrix& matrix) {
    GrMatrix mv;
    mv.setConcat(fCurrDrawState.fMatrixModeCache[kModelView_MatrixMode], matrix);
    this->loadMatrix(mv, kModelView_MatrixMode);
}

void GrDrawTarget::getViewMatrix(GrMatrix* matrix) const {
    *matrix = fCurrDrawState.fMatrixModeCache[kModelView_MatrixMode];
}

bool GrDrawTarget::getViewInverse(GrMatrix* matrix) const {
    // Can we cache this somewhere?

    GrMatrix inverse;
    if (fCurrDrawState.fMatrixModeCache[kModelView_MatrixMode].invert(&inverse)) {
        if (matrix) {
            *matrix = inverse;
        }
        return true;
    }
    return false;
}

void GrDrawTarget::setSamplerState(const GrSamplerState& state) {
    fCurrDrawState.fSamplerState = state;
}

void GrDrawTarget::setStencilPass(StencilPass pass) {
    fCurrDrawState.fStencilPass = pass;
}

void GrDrawTarget::setReverseFill(bool reverse) {
    fCurrDrawState.fReverseFill = reverse;
}

void GrDrawTarget::enableState(uint32_t bits) {
    fCurrDrawState.fFlagBits |= bits;
}

void GrDrawTarget::disableState(uint32_t bits) {
    fCurrDrawState.fFlagBits &= ~(bits);
}

void GrDrawTarget::loadMatrix(const GrMatrix& matrix, MatrixMode m) {
    fCurrDrawState.fMatrixModeCache[m] = matrix;
}

void GrDrawTarget::setPointSize(float size) {
    fCurrDrawState.fPointSize = size;
}

void GrDrawTarget::setBlendFunc(BlendCoeff srcCoef,
                                BlendCoeff dstCoef) {
    fCurrDrawState.fSrcBlend = srcCoef;
    fCurrDrawState.fDstBlend = dstCoef;
}

void GrDrawTarget::setColor(GrColor c) {
    fCurrDrawState.fColor = c;
}

void GrDrawTarget::setAlpha(uint8_t a) {
    this->setColor((a << 24) | (a << 16) | (a << 8) | a);
}

void GrDrawTarget::saveCurrentDrawState(SavedDrawState* state) const {
    state->fState = fCurrDrawState;
}

void GrDrawTarget::restoreDrawState(const SavedDrawState& state) {
    fCurrDrawState = state.fState;
}

void GrDrawTarget::copyDrawState(const GrDrawTarget& srcTarget) {
    fCurrDrawState = srcTarget.fCurrDrawState;
}


bool GrDrawTarget::reserveAndLockGeometry(GrVertexLayout    vertexLayout,
                                          uint32_t          vertexCount,
                                          uint32_t          indexCount,
                                          void**            vertices,
                                          void**            indices) {
    GrAssert(!fReservedGeometry.fLocked);
    fReservedGeometry.fVertexCount  = vertexCount;
    fReservedGeometry.fIndexCount   = indexCount;

    fReservedGeometry.fLocked = acquireGeometryHelper(vertexLayout,
                                                      vertices,
                                                      indices);
    if (fReservedGeometry.fLocked) {
        if (vertexCount) {
            fGeometrySrc.fVertexSrc = kReserved_GeometrySrcType;
            fGeometrySrc.fVertexLayout = vertexLayout;
        }
        if (indexCount) {
            fGeometrySrc.fIndexSrc = kReserved_GeometrySrcType;
        }
    }
    return fReservedGeometry.fLocked;
}

bool GrDrawTarget::geometryHints(GrVertexLayout vertexLayout,
                                 int32_t* vertexCount,
                                 int32_t* indexCount) const {
    GrAssert(!fReservedGeometry.fLocked);
    if (NULL != vertexCount) {
        *vertexCount = -1;
    }
    if (NULL != indexCount) {
        *indexCount = -1;
    }
    return false;
}

void GrDrawTarget::releaseReservedGeometry() {
    GrAssert(fReservedGeometry.fLocked);
    releaseGeometryHelper();
    fReservedGeometry.fLocked = false;
}

void GrDrawTarget::setVertexSourceToArray(const void* array,
                                          GrVertexLayout vertexLayout) {
    fGeometrySrc.fVertexSrc    = kArray_GeometrySrcType;
    fGeometrySrc.fVertexArray  = array;
    fGeometrySrc.fVertexLayout = vertexLayout;
}

void GrDrawTarget::setIndexSourceToArray(const void* array) {
    fGeometrySrc.fIndexSrc   = kArray_GeometrySrcType;
    fGeometrySrc.fIndexArray = array;
}

void GrDrawTarget::setVertexSourceToBuffer(const GrVertexBuffer* buffer,
                                           GrVertexLayout vertexLayout) {
    fGeometrySrc.fVertexSrc    = kBuffer_GeometrySrcType;
    fGeometrySrc.fVertexBuffer = buffer;
    fGeometrySrc.fVertexLayout = vertexLayout;
}

void GrDrawTarget::setIndexSourceToBuffer(const GrIndexBuffer* buffer) {
    fGeometrySrc.fIndexSrc     = kBuffer_GeometrySrcType;
    fGeometrySrc.fIndexBuffer  = buffer;
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoStateRestore::AutoStateRestore(GrDrawTarget* target) {
    fDrawTarget = target;
    fDrawTarget->saveCurrentDrawState(&fDrawState);
}

GrDrawTarget::AutoStateRestore::~AutoStateRestore() {
    fDrawTarget->restoreDrawState(fDrawState);
}
