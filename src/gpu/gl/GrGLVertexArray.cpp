/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCpuBuffer.h"
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLVertexArray.h"

struct AttribLayout {
    bool        fNormalized;  // Only used by floating point types.
    uint8_t     fCount;
    uint16_t    fType;
};

static_assert(4 == sizeof(AttribLayout));

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
            return {false, 1, GR_GL_HALF_FLOAT};
        case kHalf2_GrVertexAttribType:
            return {false, 2, GR_GL_HALF_FLOAT};
        case kHalf4_GrVertexAttribType:
            return {false, 4, GR_GL_HALF_FLOAT};
        case kInt2_GrVertexAttribType:
            return {false, 2, GR_GL_INT};
        case kInt3_GrVertexAttribType:
            return {false, 3, GR_GL_INT};
        case kInt4_GrVertexAttribType:
            return {false, 4, GR_GL_INT};
        case kByte_GrVertexAttribType:
            return {false, 1, GR_GL_BYTE};
        case kByte2_GrVertexAttribType:
            return {false, 2, GR_GL_BYTE};
        case kByte4_GrVertexAttribType:
            return {false, 4, GR_GL_BYTE};
        case kUByte_GrVertexAttribType:
            return {false, 1, GR_GL_UNSIGNED_BYTE};
        case kUByte2_GrVertexAttribType:
            return {false, 2, GR_GL_UNSIGNED_BYTE};
        case kUByte4_GrVertexAttribType:
            return {false, 4, GR_GL_UNSIGNED_BYTE};
        case kUByte_norm_GrVertexAttribType:
            return {true, 1, GR_GL_UNSIGNED_BYTE};
        case kUByte4_norm_GrVertexAttribType:
            return {true, 4, GR_GL_UNSIGNED_BYTE};
        case kShort2_GrVertexAttribType:
            return {false, 2, GR_GL_SHORT};
        case kShort4_GrVertexAttribType:
            return {false, 4, GR_GL_SHORT};
        case kUShort2_GrVertexAttribType:
            return {false, 2, GR_GL_UNSIGNED_SHORT};
        case kUShort2_norm_GrVertexAttribType:
            return {true, 2, GR_GL_UNSIGNED_SHORT};
        case kInt_GrVertexAttribType:
            return {false, 1, GR_GL_INT};
        case kUint_GrVertexAttribType:
            return {false, 1, GR_GL_UNSIGNED_INT};
        case kUShort_norm_GrVertexAttribType:
            return {true, 1, GR_GL_UNSIGNED_SHORT};
        case kUShort4_norm_GrVertexAttribType:
            return {true, 4, GR_GL_UNSIGNED_SHORT};
    }
    SK_ABORT("Unknown vertex attrib type");
};

void GrGLAttribArrayState::set(GrGLGpu* gpu,
                               int index,
                               const GrBuffer* vertexBuffer,
                               GrVertexAttribType cpuType,
                               GrSLType gpuType,
                               GrGLsizei stride,
                               size_t offsetInBytes,
                               int divisor) {
    SkASSERT(index >= 0 && index < fAttribArrayStates.count());
    SkASSERT(0 == divisor || gpu->caps()->drawInstancedSupport());
    AttribArrayState* array = &fAttribArrayStates[index];
    const char* offsetAsPtr;
    bool bufferChanged = false;
    if (vertexBuffer->isCpuBuffer()) {
        if (!array->fUsingCpuBuffer) {
            bufferChanged = true;
            array->fUsingCpuBuffer = true;
        }
        offsetAsPtr = static_cast<const GrCpuBuffer*>(vertexBuffer)->data() + offsetInBytes;
    } else {
        auto gpuBuffer = static_cast<const GrGpuBuffer*>(vertexBuffer);
        if (array->fUsingCpuBuffer || array->fVertexBufferUniqueID != gpuBuffer->uniqueID()) {
            bufferChanged = true;
            array->fVertexBufferUniqueID = gpuBuffer->uniqueID();
        }
        offsetAsPtr = reinterpret_cast<const char*>(offsetInBytes);
    }
    if (bufferChanged ||
        array->fCPUType != cpuType ||
        array->fGPUType != gpuType ||
        array->fStride != stride ||
        array->fOffset != offsetAsPtr) {
        // We always have to call this if we're going to change the array pointer. 'array' is
        // tracking the last buffer used to setup attrib pointers, not the last buffer bound.
        // GrGLGpu will avoid redundant binds.
        gpu->bindBuffer(GrGpuBufferType::kVertex, vertexBuffer);
        const AttribLayout& layout = attrib_layout(cpuType);
        if (GrSLTypeIsFloatType(gpuType)) {
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
        array->fCPUType = cpuType;
        array->fGPUType = gpuType;
        array->fStride = stride;
        array->fOffset = offsetAsPtr;
    }
    if (gpu->caps()->drawInstancedSupport() && array->fDivisor != divisor) {
        SkASSERT(0 == divisor || 1 == divisor); // not necessarily a requirement but what we expect.
        GR_GL_CALL(gpu->glInterface(), VertexAttribDivisor(index, divisor));
        array->fDivisor = divisor;
    }
}

void GrGLAttribArrayState::enableVertexArrays(const GrGLGpu* gpu, int enabledCount,
                                              GrPrimitiveRestart enablePrimitiveRestart) {
    SkASSERT(enabledCount <= fAttribArrayStates.count());

    if (!fEnableStateIsValid || enabledCount != fNumEnabledArrays) {
        int firstIdxToEnable = fEnableStateIsValid ? fNumEnabledArrays : 0;
        for (int i = firstIdxToEnable; i < enabledCount; ++i) {
            GR_GL_CALL(gpu->glInterface(), EnableVertexAttribArray(i));
        }

        int endIdxToDisable = fEnableStateIsValid ? fNumEnabledArrays : fAttribArrayStates.count();
        for (int i = enabledCount; i < endIdxToDisable; ++i) {
            GR_GL_CALL(gpu->glInterface(), DisableVertexAttribArray(i));
        }

        fNumEnabledArrays = enabledCount;
    }

    SkASSERT(GrPrimitiveRestart::kNo == enablePrimitiveRestart ||
             gpu->caps()->usePrimitiveRestart());

    if (gpu->caps()->usePrimitiveRestart() &&
        (!fEnableStateIsValid || enablePrimitiveRestart != fPrimitiveRestartEnabled)) {
        if (GrPrimitiveRestart::kYes == enablePrimitiveRestart) {
            GR_GL_CALL(gpu->glInterface(), Enable(GR_GL_PRIMITIVE_RESTART_FIXED_INDEX));
        } else {
            GR_GL_CALL(gpu->glInterface(), Disable(GR_GL_PRIMITIVE_RESTART_FIXED_INDEX));
        }

        fPrimitiveRestartEnabled = enablePrimitiveRestart;
    }

    fEnableStateIsValid = true;
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
    if (!state) {
        return nullptr;
    }
    if (ibuff->isCpuBuffer()) {
        GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, 0));
    } else {
        const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(ibuff);
        if (fIndexBufferUniqueID != glBuffer->uniqueID()) {
            GR_GL_CALL(gpu->glInterface(),
                       BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, glBuffer->bufferID()));
            fIndexBufferUniqueID = glBuffer->uniqueID();
        }
    }
    return state;
}

void GrGLVertexArray::invalidateCachedState() {
    fAttribArrays.invalidate();
    fIndexBufferUniqueID.makeInvalid();
}
