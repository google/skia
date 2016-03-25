/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBuffer_DEFINED
#define GrGLBuffer_DEFINED

#include "GrBuffer.h"
#include "gl/GrGLTypes.h"

class GrGLGpu;
class GrGLCaps;

class GrGLBuffer : public GrBuffer {
public:
    static GrGLBuffer* Create(GrGLGpu*, GrBufferType, size_t size, GrAccessPattern);

    ~GrGLBuffer() {
        // either release or abandon should have been called by the owner of this object.
        SkASSERT(0 == fBufferID);
    }

    GrGLenum target() const { return fTarget; }
    GrGLuint bufferID() const { return fBufferID; }
    size_t baseOffset() const { return reinterpret_cast<size_t>(fCPUData); }

protected:
    GrGLBuffer(GrGLGpu*, GrBufferType, size_t size, GrAccessPattern, bool cpuBacked);

    void onAbandon() override;
    void onRelease() override;
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

private:
    GrGLGpu* glGpu() const;
    const GrGLCaps& glCaps() const;

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

#ifdef SK_DEBUG
    void validate() const;
#endif

    void*        fCPUData;
    GrGLenum     fTarget; // GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER, e.g.
    GrGLuint     fBufferID;
    size_t       fSizeInBytes;
    GrGLenum     fUsage;
    size_t       fGLSizeInBytes;     // In certain cases we make the size of the GL buffer object
                                     // smaller or larger than the size in fDesc.

    typedef GrBuffer INHERITED;
};

#endif
