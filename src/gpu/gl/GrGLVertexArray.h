/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexArray_DEFINED
#define GrGLVertexArray_DEFINED

#include "GrGpuResource.h"
#include "GrTypesPriv.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLTypes.h"
#include "SkTArray.h"

class GrBuffer;
class GrGLGpu;

/**
 * This sets and tracks the vertex attribute array state. It is used internally by GrGLVertexArray
 * (below) but is separate because it is also used to track the state of vertex array object 0.
 */
class GrGLAttribArrayState {
public:
    explicit GrGLAttribArrayState(int arrayCount = 0) {
        this->resize(arrayCount);
    }

    void resize(int newCount) {
        fAttribArrayStates.resize_back(newCount);
        this->invalidate();
    }

    /**
     * This function enables and sets vertex attrib state for the specified attrib index. It is
     * assumed that the GrGLAttribArrayState is tracking the state of the currently bound vertex
     * array object.
     */
    void set(GrGLGpu*,
             int attribIndex,
             const GrBuffer* vertexBuffer,
             GrVertexAttribType type,
             GrGLsizei stride,
             size_t offsetInBytes,
             int divisor = 0);

    enum class EnablePrimitiveRestart : bool {
        kYes = true,
        kNo = false
    };

    /**
     * This function enables the first 'enabledCount' vertex arrays and disables the rest.
     */
    void enableVertexArrays(const GrGLGpu*, int enabledCount,
                            EnablePrimitiveRestart = EnablePrimitiveRestart::kNo);

    void invalidate() {
        int count = fAttribArrayStates.count();
        for (int i = 0; i < count; ++i) {
            fAttribArrayStates[i].invalidate();
        }
        fEnableStateIsValid = false;
    }

    /**
     * The number of attrib arrays that this object is configured to track.
     */
    int count() const { return fAttribArrayStates.count(); }

private:
    static constexpr int kInvalidDivisor = -1;

    /**
     * Tracks the state of glVertexAttribArray for an attribute index.
     */
    struct AttribArrayState {
        void invalidate() {
            fVertexBufferUniqueID.makeInvalid();
            fDivisor = kInvalidDivisor;
        }

        GrGpuResource::UniqueID   fVertexBufferUniqueID;
        GrVertexAttribType        fType;
        GrGLsizei                 fStride;
        size_t                    fOffset;
        int                       fDivisor;
    };

    SkSTArray<16, AttribArrayState, true> fAttribArrayStates;
    int fNumEnabledArrays;
    EnablePrimitiveRestart fPrimitiveRestartEnabled;
    bool fEnableStateIsValid = false;
};

/**
 * This class represents an OpenGL vertex array object. It manages the lifetime of the vertex array
 * and is used to track the state of the vertex array to avoid redundant GL calls.
 */
class GrGLVertexArray {
public:
    GrGLVertexArray(GrGLint id, int attribCount);

    /**
     * Binds this vertex array. If the ID has been deleted or abandoned then nullptr is returned.
     * Otherwise, the GrGLAttribArrayState that is tracking this vertex array's attrib bindings is
     * returned.
     */
    GrGLAttribArrayState* bind(GrGLGpu*);

    /**
     * This is a version of the above function that also binds an index buffer to the vertex
     * array object.
     */
    GrGLAttribArrayState* bindWithIndexBuffer(GrGLGpu* gpu, const GrBuffer* indexBuffer);

    GrGLuint arrayID() const { return fID; }

    void invalidateCachedState();

private:
    GrGLuint                  fID;
    GrGLAttribArrayState      fAttribArrays;
    GrGpuResource::UniqueID   fIndexBufferUniqueID;
};

#endif
