/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexArray.h"
#include "GrGLGpu.h"


void GrGLAttribArrayState::set(const GrGLGpu* gpu,
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

void GrGLAttribArrayState::disableUnusedArrays(const GrGLGpu* gpu, uint64_t usedMask) {
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

GrGLVertexArray::GrGLVertexArray(GrGLint id, int attribCount)
    : fID(id)
    , fAttribArrays(attribCount)
    , fIndexBufferIDIsValid(false) {
}

GrGLAttribArrayState* GrGLVertexArray::bind(GrGLGpu* gpu) {
    if (0 == fID) {
        return NULL;
    }
    gpu->bindVertexArray(fID);
    return &fAttribArrays;
}

GrGLAttribArrayState* GrGLVertexArray::bindWithIndexBuffer(GrGLGpu* gpu,
                                                           const GrGLIndexBuffer* buffer) {
    GrGLAttribArrayState* state = this->bind(gpu);
    if (state && buffer) {
        GrGLuint bufferID = buffer->bufferID();
        if (!fIndexBufferIDIsValid || bufferID != fIndexBufferID) {            
            GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, bufferID));
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
