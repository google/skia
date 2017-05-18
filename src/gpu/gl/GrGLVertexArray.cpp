/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexArray.h"
#include "GrGLBuffer.h"
#include "GrGLGpu.h"

struct AttribLayout {
    bool        fNormalized;  // Only used by floating point types.
    uint8_t     fCount;
    uint16_t    fType;
};

GR_STATIC_ASSERT(4 == sizeof(AttribLayout));

static AttribLayout attrib_layout(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return {false, 1, GR_GL_FLOAT};
        case kVec2f_GrVertexAttribType:
            return {false, 2, GR_GL_FLOAT};
        case kVec3f_GrVertexAttribType:
            return {false, 3, GR_GL_FLOAT};
        case kVec4f_GrVertexAttribType:
            return {false, 4, GR_GL_FLOAT};
        case kVec2i_GrVertexAttribType:
            return {false, 2, GR_GL_INT};
        case kVec3i_GrVertexAttribType:
            return {false, 3, GR_GL_INT};
        case kVec4i_GrVertexAttribType:
            return {false, 4, GR_GL_INT};
        case kUByte_GrVertexAttribType:
            return {true, 1, GR_GL_UNSIGNED_BYTE};
        case kVec4ub_GrVertexAttribType:
            return {true, 4, GR_GL_UNSIGNED_BYTE};
        case kVec2us_GrVertexAttribType:
            return {true, 2, GR_GL_UNSIGNED_SHORT};
        case kInt_GrVertexAttribType:
            return {false, 1, GR_GL_INT};
        case kUint_GrVertexAttribType:
            return {false, 1, GR_GL_UNSIGNED_INT};
    }
    SkFAIL("Unknown vertex attrib type");
    return {false, 0, 0};
};

void GrGLAttribArrayState::set(GrGLGpu* gpu,
                               int index,
                               const GrBuffer* vertexBuffer,
                               GrVertexAttribType type,
                               GrGLsizei stride,
                               size_t offsetInBytes) {
    SkASSERT(index >= 0 && index < fAttribArrayStates.count());
    AttribArrayState* array = &fAttribArrayStates[index];
    if (array->fVertexBufferUniqueID != vertexBuffer->uniqueID() ||
        array->fType != type ||
        array->fStride != stride ||
        array->fOffset != offsetInBytes) {
        gpu->bindBuffer(kVertex_GrBufferType, vertexBuffer);
        const AttribLayout& layout = attrib_layout(type);
        const GrGLvoid* offsetAsPtr = reinterpret_cast<const GrGLvoid*>(offsetInBytes);
        if (!GrVertexAttribTypeIsIntType(type)) {
            GR_GL_CALL(gpu->glInterface(), VertexAttribPointer(index,
                                                               layout.fCount,
                                                               layout.fType,
                                                               layout.fNormalized,
                                                               stride,
                                                               offsetAsPtr));
        } else {
            SkASSERT(gpu->caps()->shaderCaps()->integerSupport());
            SkASSERT(!layout.fNormalized);
            GR_GL_CALL(gpu->glInterface(), VertexAttribIPointer(index,
                                                                layout.fCount,
                                                                layout.fType,
                                                                stride,
                                                                offsetAsPtr));
        }
        array->fVertexBufferUniqueID = vertexBuffer->uniqueID();
        array->fType = type;
        array->fStride = stride;
        array->fOffset = offsetInBytes;
    }
}

void GrGLAttribArrayState::enableVertexArrays(const GrGLGpu* gpu, int enabledCount) {
    SkASSERT(enabledCount <= fAttribArrayStates.count());

    int firstIdxToEnable = fEnabledCountIsValid ? fNumEnabledArrays : 0;
    for (int i = firstIdxToEnable; i < enabledCount; ++i) {
        GR_GL_CALL(gpu->glInterface(), EnableVertexAttribArray(i));
    }

    int endIdxToDisable = fEnabledCountIsValid ? fNumEnabledArrays : fAttribArrayStates.count();
    for (int i = enabledCount; i < endIdxToDisable; ++i) {
        GR_GL_CALL(gpu->glInterface(), DisableVertexAttribArray(i));
    }

    fNumEnabledArrays = enabledCount;
    fEnabledCountIsValid = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLVertexArray::GrGLVertexArray(GrGLint id, int attribCount)
    : fID(id)
    , fAttribArrays(attribCount)
    , fIndexBufferUniqueID(SK_InvalidUniqueID) {
}

GrGLAttribArrayState* GrGLVertexArray::bind(GrGLGpu* gpu) {
    if (0 == fID) {
        return nullptr;
    }
    gpu->bindVertexArray(fID);
    return &fAttribArrays;
}

GrGLAttribArrayState* GrGLVertexArray::bindWithIndexBuffer(GrGLGpu* gpu, const GrBuffer* ibuff) {
    GrGLAttribArrayState* state = this->bind(gpu);
    if (state && fIndexBufferUniqueID != ibuff->uniqueID()) {
        if (ibuff->isCPUBacked()) {
            GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, 0));
        } else {
            const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(ibuff);
            GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER,
                                                      glBuffer->bufferID()));
        }
        fIndexBufferUniqueID = ibuff->uniqueID();
    }
    return state;
}

void GrGLVertexArray::invalidateCachedState() {
    fAttribArrays.invalidate();
    fIndexBufferUniqueID.makeInvalid();
}
