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

class GrGLGpu;

class GrGLVertexBuffer : public GrVertexBuffer {

public:
    typedef GrGLBufferImpl::Desc Desc;

    GrGLVertexBuffer(GrGLGpu* gpu, const Desc& desc);

    GrGLuint bufferID() const { return fImpl.bufferID(); }
    size_t baseOffset() const { return fImpl.baseOffset(); }

    void bind() const {
        if (!this->wasDestroyed()) {
            fImpl.bind(this->getGpuGL());
        }
    }

protected:
    void onAbandon() SK_OVERRIDE;
    void onRelease() SK_OVERRIDE;

private:
    void* onMap() SK_OVERRIDE;
    void onUnmap() SK_OVERRIDE;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) SK_OVERRIDE;

    GrGLGpu* getGpuGL() const {
        SkASSERT(!this->wasDestroyed());
        return (GrGLGpu*)(this->getGpu());
    }

    GrGLBufferImpl fImpl;

    typedef GrVertexBuffer INHERITED;
};

#endif
