/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLIndexBuffer_DEFINED
#define GrGLIndexBuffer_DEFINED

#include "GrIndexBuffer.h"
#include "gl/GrGLInterface.h"

class GrGpuGL;

class GrGLIndexBuffer : public GrIndexBuffer {

public:

    GrGLIndexBuffer(GrGpuGL* gpu,
                    bool isWrapped,
                    GrGLuint id,
                    size_t sizeInBytes,
                    bool dynamic);

    virtual ~GrGLIndexBuffer() { this->release(); }

    GrGLuint bufferID() const { return fBufferID; }
    size_t baseOffset() const { return 0; }

    void bind() const;

    // overrides of GrIndexBuffer
    virtual void* lock();
    virtual void* lockPtr() const;
    virtual void unlock();
    virtual bool isLocked() const;
    virtual bool updateData(const void* src, size_t srcSizeInBytes);

protected:

    // overrides of GrResource
    virtual void onAbandon() SK_OVERRIDE;
    virtual void onRelease() SK_OVERRIDE;

private:

    GrGLuint     fBufferID;
    void*        fLockPtr;

    typedef GrIndexBuffer INHERITED;
};

#endif
