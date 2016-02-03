/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLTransferBuffer_DEFINED
#define GrGLTransferBuffer_DEFINED

#include "GrTransferBuffer.h"
#include "GrGLBufferImpl.h"
#include "gl/GrGLInterface.h"

class GrGLGpu;

class GrGLTransferBuffer : public GrTransferBuffer {

public:
    typedef GrGLBufferImpl::Desc Desc;

    GrGLTransferBuffer(GrGLGpu* gpu, const Desc& desc, GrGLenum type);

    GrGLuint bufferID() const { return fImpl.bufferID(); }
    size_t baseOffset() const { return fImpl.baseOffset(); }
    GrGLenum bufferType() const { return fImpl.bufferType(); }

protected:
    void onAbandon() override;
    void onRelease() override;
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

private:
    void* onMap() override;
    void onUnmap() override;

    GrGLGpu* getGpuGL() const {
        SkASSERT(!this->wasDestroyed());
        return (GrGLGpu*)(this->getGpu());
    }

    GrGLBufferImpl fImpl;

    typedef GrTransferBuffer INHERITED;
};

#endif
