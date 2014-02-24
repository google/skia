/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexArray.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X);

void GrGLAttribArrayState::set(const GrGpuGL* gpu,
                               int index,
                               GrGLVertexBuffer* buffer,
                               GrGLint size,
                               GrGLenum type,
                               GrGLboolean normalized,
                               GrGLsizei stride,
                               GrGLvoid* offset) {
    SkASSERT(index >= 0 && index < fAttribArrayStates.count());
    AttribArrayState* array = &fAttribArrayStates[index];
    if (!array->fEnableIsValid || !array->fEnabled) {
        GR_GL_CALL(gpu->glInterface(), EnableVertexAttribArray(index));
        array->fEnableIsValid = true;
        array->fEnabled = true;
    }
    if (!array->fAttribPointerIsValid ||
        array->fVertexBufferID != buffer->bufferID() ||
        array->fSize != size ||
        array->fNormalized != normalized ||
        array->fStride != stride ||
        array->fOffset != offset) {

        buffer->bind();
        GR_GL_CALL(gpu->glInterface(), VertexAttribPointer(index,
                                                           size,
                                                           type,
                                                           normalized,
                                                           stride,
                                                           offset));
        array->fAttribPointerIsValid = true;
        array->fVertexBufferID = buffer->bufferID();
        array->fSize = size;
        array->fNormalized = normalized;
        array->fStride = stride;
        array->fOffset = offset;
    }
}

void GrGLAttribArrayState::disableUnusedArrays(const GrGpuGL* gpu, uint64_t usedMask) {
    int count = fAttribArrayStates.count();
    for (int i = 0; i < count; ++i) {
        if (!(usedMask & 0x1)) {
            if (!fAttribArrayStates[i].fEnableIsValid || fAttribArrayStates[i].fEnabled) {
                GR_GL_CALL(gpu->glInterface(), DisableVertexAttribArray(i));
                fAttribArrayStates[i].fEnableIsValid = true;
                fAttribArrayStates[i].fEnabled = false;
            }
        } else {
            SkASSERT(fAttribArrayStates[i].fEnableIsValid && fAttribArrayStates[i].fEnabled);
        }
        // if the count is greater than 64 then this will become 0 and we will disable arrays 64+.
        usedMask >>= 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLVertexArray::GrGLVertexArray(GrGpuGL* gpu, GrGLint id, int attribCount)
    : GrResource(gpu, false)
    , fID(id)
    , fAttribArrays(attribCount)
    , fIndexBufferIDIsValid(false) {
}

void GrGLVertexArray::onAbandon() {
    fID = 0;
    INHERITED::onAbandon();
}

void GrGLVertexArray::onRelease() {
    if (0 != fID) {
        GL_CALL(DeleteVertexArrays(1, &fID));
        GPUGL->notifyVertexArrayDelete(fID);
        fID = 0;
    }
    INHERITED::onRelease();
}

GrGLAttribArrayState* GrGLVertexArray::bind() {
    if (0 == fID) {
        return NULL;
    }
    GPUGL->bindVertexArray(fID);
    return &fAttribArrays;
}

GrGLAttribArrayState* GrGLVertexArray::bindWithIndexBuffer(const GrGLIndexBuffer* buffer) {
    GrGLAttribArrayState* state = this->bind();
    if (NULL != state && NULL != buffer) {
        GrGLuint bufferID = buffer->bufferID();
        if (!fIndexBufferIDIsValid || bufferID != fIndexBufferID) {
            GL_CALL(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, bufferID));
            fIndexBufferIDIsValid = true;
            fIndexBufferID = bufferID;
        }
    }
    return state;
}

void GrGLVertexArray::notifyIndexBufferDelete(GrGLuint bufferID) {
    if (fIndexBufferIDIsValid && bufferID == fIndexBufferID) {
        fIndexBufferID = 0;
    }
 }

void GrGLVertexArray::invalidateCachedState() {
    fAttribArrays.invalidate();
    fIndexBufferIDIsValid = false;
}
