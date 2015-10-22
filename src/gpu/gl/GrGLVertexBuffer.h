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

protected:
    void onAbandon() override;
    void onRelease() override;
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

private:
    void* onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrGLGpu* getGpuGL() const {
        SkASSERT(!this->wasDestroyed());
        return (GrGLGpu*)(this->getGpu());
    }

    GrGLBufferImpl fImpl;

    typedef GrVertexBuffer INHERITED;
};

#endif
