
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLIndexBuffer_DEFINED
#define GrGLIndexBuffer_DEFINED

#include "GrIndexBuffer.h"
#include "GrGLInterface.h"

class GrGpuGL;

class GrGLIndexBuffer : public GrIndexBuffer {

public:

    virtual ~GrGLIndexBuffer() { this->release(); }

    GrGLuint bufferID() const;

    // overrides of GrIndexBuffer
    virtual void* lock();
    virtual void* lockPtr() const;
    virtual void unlock();
    virtual bool isLocked() const;
    virtual bool updateData(const void* src, size_t srcSizeInBytes);

protected:
    GrGLIndexBuffer(GrGpuGL* gpu,
                    GrGLuint id,
                    size_t sizeInBytes,
                    bool dynamic);

    // overrides of GrResource
    virtual void onAbandon();
    virtual void onRelease();

private:
    void bind() const;

    GrGLuint     fBufferID;
    void*        fLockPtr;

    friend class GrGpuGL;

    typedef GrIndexBuffer INHERITED;
};

#endif
