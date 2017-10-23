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
        case kFloat2_GrVertexAttribType:
            return {false, 2, GR_GL_FLOAT};
        case kFloat3_GrVertexAttribType:
            return {false, 3, GR_GL_FLOAT};
        case kFloat4_GrVertexAttribType:
            return {false, 4, GR_GL_FLOAT};
        case kHalf_GrVertexAttribType:
            return {false, 1, GR_GL_FLOAT};
        case kHalf2_GrVertexAttribType:
            return {false, 2, GR_GL_FLOAT};
        case kHalf3_GrVertexAttribType:
            return {false, 3, GR_GL_FLOAT};
        case kHalf4_GrVertexAttribType:
            return {false, 4, GR_GL_FLOAT};
        case kInt2_GrVertexAttribType:
            return {false, 2, GR_GL_INT};
        case kInt3_GrVertexAttribType:
            return {false, 3, GR_GL_INT};
        case kInt4_GrVertexAttribType:
            return {false, 4, GR_GL_INT};
        case kUByte_norm_GrVertexAttribType:
            return {true, 1, GR_GL_UNSIGNED_BYTE};
        case kUByte4_norm_GrVertexAttribType:
            return {true, 4, GR_GL_UNSIGNED_BYTE};
        case kShort2_GrVertexAttribType:
            return {false, 2, GR_GL_SHORT};
        case kUShort2_GrVertexAttribType:
            return {false, 2, GR_GL_UNSIGNED_SHORT};
        case kUShort2_norm_GrVertexAttribType:
            return {true, 2, GR_GL_UNSIGNED_SHORT};
        case kInt_GrVertexAttribType:
            return {false, 1, GR_GL_INT};
        case kUint_GrVertexAttribType:
            return {false, 1, GR_GL_UNSIGNED_INT};
    }
    SK_ABORT("Unknown vertex attrib type");
    return {false, 0, 0};
};

static bool GrVertexAttribTypeIsIntType(const GrShaderCaps* shaderCaps,
                                        GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return false;
        case kFloat2_GrVertexAttribType:
            return false;
        case kFloat3_GrVertexAttribType:
            return false;
        case kFloat4_GrVertexAttribType:
            return false;
        case kHalf_GrVertexAttribType:
            return false;
        case kHalf2_GrVertexAttribType:
            return false;
        case kHalf3_GrVertexAttribType:
            return false;
        case kHalf4_GrVertexAttribType:
            return false;
        case kInt2_GrVertexAttribType:
            return true;
        case kInt3_GrVertexAttribType:
            return true;
        case kInt4_GrVertexAttribType:
            return true;
        case kUByte_norm_GrVertexAttribType:
            return false;
        case kUByte4_norm_GrVertexAttribType:
            return false;
        case kShort2_GrVertexAttribType:
            return true;
        case kUShort2_GrVertexAttribType:
            return shaderCaps->integerSupport(); // FIXME: caller should handle this.
        case kUShort2_norm_GrVertexAttribType:
            return false;
        case kInt_GrVertexAttribType:
            return true;
        case kUint_GrVertexAttribType:
            return true;
    }
    SK_ABORT("Unexpected attribute type");
    return false;
}

void GrGLAttribArrayState::set(GrGLGpu* gpu,
                               int index,
                               const GrBuffer* vertexBuffer,
                               GrVertexAttribType type,
                               GrGLsizei stride,
                               size_t offsetInBytes,
                               int divisor) {
    SkASSERT(index >= 0 && index < fAttribArrayStates.count());
    SkASSERT(0 == divisor || gpu->caps()->instanceAttribSupport());
    AttribArrayState* array = &fAttribArrayStates[index];
    if (array->fVertexBufferUniqueID != vertexBuffer->uniqueID() ||
        array->fType != type ||
        array->fStride != stride ||
        array->fOffset != offsetInBytes) {
        gpu->bindBuffer(kVertex_GrBufferType, vertexBuffer);
        const AttribLayout& layout = attrib_layout(type);
        const GrGLvoid* offsetAsPtr = reinterpret_cast<const GrGLvoid*>(offsetInBytes);
        if (!GrVertexAttribTypeIsIntType(gpu->caps()->shaderCaps(), type)) {
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
    if (gpu->caps()->instanceAttribSupport() && array->fDivisor != divisor) {
        SkASSERT(0 == divisor || 1 == divisor); // not necessarily a requirement but what we expect.
        GR_GL_CALL(gpu->glInterface(), VertexAttribDivisor(index, divisor));
        array->fDivisor = divisor;
    }
}

void GrGLAttribArrayState::enableVertexArrays(const GrGLGpu* gpu, int enabledCount) {
    SkASSERT(enabledCount <= fAttribArrayStates.count());
    if (fEnabledCountIsValid && enabledCount == fNumEnabledArrays) {
        return;
    }

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
