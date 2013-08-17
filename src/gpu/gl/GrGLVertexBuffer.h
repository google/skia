/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexBuffer_DEFINED
#define GrGLVertexBuffer_DEFINED

#include "GrVertexBuffer.h"
#include "GrGLBufferImpl.h"
#include "gl/GrGLInterface.h"

class GrGpuGL;

class GrGLVertexBuffer : public GrVertexBuffer {

public:
    typedef GrGLBufferImpl::Desc Desc;

    GrGLVertexBuffer(GrGpuGL* gpu, const Desc& desc);
    virtual ~GrGLVertexBuffer() { this->release(); }

    GrGLuint bufferID() const { return fImpl.bufferID(); }
    size_t baseOffset() const { return fImpl.baseOffset(); }

    void bind() const {
        if (this->isValid()) {
            fImpl.bind(this->getGpuGL());
        }
    }

    // overrides of GrVertexBuffer
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
    GrGpuGL* getGpuGL() const {
        SkASSERT(this->isValid());
        return (GrGpuGL*)(this->getGpu());
    }

    GrGLBufferImpl fImpl;

    typedef GrVertexBuffer INHERITED;
};

#endif
