/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBufferImpl_DEFINED
#define GrGLBufferImpl_DEFINED

#include "GrNoncopyable.h"
#include "gl/GrGLFunctions.h"

class GrGpuGL;

/**
 * This class serves as the implementation of GrGL*Buffer classes. It was written to avoid code
 * duplication in those classes.
 */
class GrGLBufferImpl : public GrNoncopyable {
public:
    struct Desc {
        bool        fIsWrapped;
        GrGLuint    fID;
        size_t      fSizeInBytes;
        bool        fDynamic;
    };

    GrGLBufferImpl(GrGpuGL*, const Desc&, GrGLenum bufferType);
    ~GrGLBufferImpl() {
        // either release or abandon should have been called by the owner of this object.
        GrAssert(0 == fDesc.fID);
    }

    void abandon();
    void release(GrGpuGL* gpu);

    GrGLuint bufferID() const { return fDesc.fID; }
    size_t baseOffset() const { return 0; }

    void bind(GrGpuGL* gpu) const;

    void* lock(GrGpuGL* gpu);
    void* lockPtr() const { return fLockPtr; }
    void unlock(GrGpuGL* gpu);
    bool isLocked() const;
    bool updateData(GrGpuGL* gpu, const void* src, size_t srcSizeInBytes);

private:
    Desc         fDesc;
    GrGLenum     fBufferType; // GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
    void*        fLockPtr;

    typedef GrNoncopyable INHERITED;
};

#endif
