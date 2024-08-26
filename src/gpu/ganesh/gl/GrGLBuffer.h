/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBuffer_DEFINED
#define GrGLBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <cstddef>
#include <string_view>

class GrGLCaps;
class GrGLGpu;
class SkString;
class SkTraceMemoryDump;

class GrGLBuffer : public GrGpuBuffer {
public:
    static sk_sp<GrGLBuffer> Make(GrGLGpu*,
                                  size_t size,
                                  GrGpuBufferType intendedType,
                                  GrAccessPattern);

    ~GrGLBuffer() override {
        // either release or abandon should have been called by the owner of this object.
        SkASSERT(0 == fBufferID);
    }

    GrGLuint bufferID() const { return fBufferID; }

    void setHasAttachedToTexture() { fHasAttachedToTexture = true; }
    bool hasAttachedToTexture() const { return fHasAttachedToTexture; }

protected:
    GrGLBuffer(GrGLGpu*,
               size_t size,
               GrGpuBufferType intendedType,
               GrAccessPattern,
               std::string_view label);

    void onAbandon() override;
    void onRelease() override;
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

private:
    GrGLGpu* glGpu() const;
    const GrGLCaps& glCaps() const;

    void onMap(MapType) override;
    void onUnmap(MapType) override;
    bool onClearToZero() override;
    bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) override;

    void onSetLabel() override;

    GrGpuBufferType fIntendedType;
    GrGLuint        fBufferID;
    GrGLenum        fUsage;
    bool            fHasAttachedToTexture;

    using INHERITED = GrGpuBuffer;
};

#endif
