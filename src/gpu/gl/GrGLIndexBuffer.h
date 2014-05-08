/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLIndexBuffer_DEFINED
#define GrGLIndexBuffer_DEFINED

#include "GrIndexBuffer.h"
#include "GrGLBufferImpl.h"
#include "gl/GrGLInterface.h"

class GrGpuGL;

class GrGLIndexBuffer : public GrIndexBuffer {

public:
    typedef GrGLBufferImpl::Desc Desc;

    GrGLIndexBuffer(GrGpuGL* gpu, const Desc& desc);
    virtual ~GrGLIndexBuffer() { this->release(); }

    GrGLuint bufferID() const { return fImpl.bufferID(); }
    size_t baseOffset() const { return fImpl.baseOffset(); }

    void bind() const {
        if (!this->wasDestroyed()) {
            fImpl.bind(this->getGpuGL());
        }
    }

protected:
    virtual void onAbandon() SK_OVERRIDE;
    virtual void onRelease() SK_OVERRIDE;

private:
    virtual void* onMap() SK_OVERRIDE;
    virtual void onUnmap() SK_OVERRIDE;
    virtual bool onUpdateData(const void* src, size_t srcSizeInBytes) SK_OVERRIDE;

    GrGpuGL* getGpuGL() const {
        SkASSERT(!this->wasDestroyed());
        return (GrGpuGL*)(this->getGpu());
    }

    GrGLBufferImpl fImpl;

    typedef GrIndexBuffer INHERITED;
};

#endif
