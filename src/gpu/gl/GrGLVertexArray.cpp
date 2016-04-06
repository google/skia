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
    GrGLint     fCount;
    GrGLenum    fType;
    GrGLboolean fNormalized;  // Only used by floating point types.
};

static const AttribLayout gLayouts[kGrVertexAttribTypeCount] = {
    {1, GR_GL_FLOAT, false},         // kFloat_GrVertexAttribType
    {2, GR_GL_FLOAT, false},         // kVec2f_GrVertexAttribType
    {3, GR_GL_FLOAT, false},         // kVec3f_GrVertexAttribType
    {4, GR_GL_FLOAT, false},         // kVec4f_GrVertexAttribType
    {1, GR_GL_UNSIGNED_BYTE, true},  // kUByte_GrVertexAttribType
    {4, GR_GL_UNSIGNED_BYTE, true},  // kVec4ub_GrVertexAttribType
    {2, GR_GL_UNSIGNED_SHORT, true}, // kVec2s_GrVertexAttribType
    {1, GR_GL_INT, false},           // kInt_GrVertexAttribType
    {1, GR_GL_UNSIGNED_INT, false},  // kUint_GrVertexAttribType
};

GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
GR_STATIC_ASSERT(4 == kUByte_GrVertexAttribType);
GR_STATIC_ASSERT(5 == kVec4ub_GrVertexAttribType);
GR_STATIC_ASSERT(6 == kVec2us_GrVertexAttribType);
GR_STATIC_ASSERT(7 == kInt_GrVertexAttribType);
GR_STATIC_ASSERT(8 == kUint_GrVertexAttribType);

void GrGLAttribArrayState::set(GrGLGpu* gpu,
                               int index,
                               const GrGLBuffer* vertexBuffer,
                               GrVertexAttribType type,
                               GrGLsizei stride,
                               GrGLvoid* offset) {
    SkASSERT(index >= 0 && index < fAttribArrayStates.count());
    AttribArrayState* array = &fAttribArrayStates[index];
    if (!array->fEnableIsValid || !array->fEnabled) {
        GR_GL_CALL(gpu->glInterface(), EnableVertexAttribArray(index));
        array->fEnableIsValid = true;
        array->fEnabled = true;
    }
    if (array->fVertexBufferUniqueID != vertexBuffer->getUniqueID() ||
        array->fType != type ||
        array->fStride != stride ||
        array->fOffset != offset) {
        gpu->bindBuffer(kVertex_GrBufferType, vertexBuffer);
        const AttribLayout& layout = gLayouts[type];
        if (!GrVertexAttribTypeIsIntType(type)) {
            GR_GL_CALL(gpu->glInterface(), VertexAttribPointer(index,
                                                               layout.fCount,
                                                               layout.fType,
                                                               layout.fNormalized,
                                                               stride,
                                                               offset));
        } else {
            SkASSERT(gpu->caps()->shaderCaps()->integerSupport());
            SkASSERT(!layout.fNormalized);
            GR_GL_CALL(gpu->glInterface(), VertexAttribIPointer(index,
                                                                layout.fCount,
                                                                layout.fType,
                                                                stride,
                                                                offset));
        }
        array->fVertexBufferUniqueID = vertexBuffer->getUniqueID();
        array->fType = type;
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
    , fIndexBufferUniqueID(SK_InvalidUniqueID) {
}

GrGLAttribArrayState* GrGLVertexArray::bind(GrGLGpu* gpu) {
    if (0 == fID) {
        return nullptr;
    }
    gpu->bindVertexArray(fID);
    return &fAttribArrays;
}

GrGLAttribArrayState* GrGLVertexArray::bindWithIndexBuffer(GrGLGpu* gpu, const GrGLBuffer* ibuff) {
    GrGLAttribArrayState* state = this->bind(gpu);
    if (state && fIndexBufferUniqueID != ibuff->getUniqueID()) {
        GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, ibuff->bufferID()));
        fIndexBufferUniqueID = ibuff->getUniqueID();
    }
    return state;
}

void GrGLVertexArray::invalidateCachedState() {
    fAttribArrays.invalidate();
    fIndexBufferUniqueID = SK_InvalidUniqueID;
}
