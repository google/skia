/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBuffer_DEFINED
#define GrGLBuffer_DEFINED

#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/GrGpuBuffer.h"

class GrGLGpu;
class GrGLCaps;

class GrGLBuffer : public GrGpuBuffer {
public:
    static sk_sp<GrGLBuffer> Make(GrGLGpu*, size_t size, GrGpuBufferType intendedType,
                                  GrAccessPattern, const void* data = nullptr);

    ~GrGLBuffer() override {
        // either release or abandon should have been called by the owner of this object.
        SkASSERT(0 == fBufferID);
    }

    GrGLuint bufferID() const { return fBufferID; }

    /**
     * Returns the actual size of the underlying GL buffer object. In certain cases we may make this
     * smaller than the size reported by GrGpuBuffer.
     */
    size_t glSizeInBytes() const { return fGLSizeInBytes; }

    void setHasAttachedToTexture() { fHasAttachedToTexture = true; }
    bool hasAttachedToTexture() const { return fHasAttachedToTexture; }

protected:
    GrGLBuffer(GrGLGpu*, size_t size, GrGpuBufferType intendedType, GrAccessPattern,
               const void* data);

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

    GrGpuBufferType fIntendedType;
    GrGLuint        fBufferID;
    GrGLenum        fUsage;
    size_t          fGLSizeInBytes;
    bool            fHasAttachedToTexture;

    typedef GrGpuBuffer INHERITED;
};

#endif
