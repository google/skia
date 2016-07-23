/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBufferAccess_DEFINED
#define GrBufferAccess_DEFINED

#include "GrBuffer.h"
#include "GrGpuResourceRef.h"

/**
 * Used to represent a texel buffer that will be read in a GrProcessor. It holds a GrBuffer along
 * with an associated offset and texel config.
 */
class GrBufferAccess : public SkNoncopyable {
public:
    /**
     * Must be initialized before adding to a GrProcessor's buffer access list.
     */
    void reset(GrPixelConfig texelConfig, GrBuffer* buffer,
               GrShaderFlags visibility = kFragment_GrShaderFlag) {
        fTexelConfig = texelConfig;
        fBuffer.set(SkRef(buffer), kRead_GrIOType);
        fVisibility = visibility;
    }

    bool operator==(const GrBufferAccess& that) const {
        return fTexelConfig == that.fTexelConfig &&
               this->buffer() == that.buffer() &&
               fVisibility == that.fVisibility;
    }

    bool operator!=(const GrBufferAccess& that) const { return !(*this == that); }

    GrPixelConfig texelConfig() const { return fTexelConfig; }
    GrBuffer* buffer() const { return fBuffer.get(); }
    GrShaderFlags visibility() const { return fVisibility; }

    /**
     * For internal use by GrProcessor.
     */
    const GrGpuResourceRef* getProgramBuffer() const { return &fBuffer;}

private:
    GrPixelConfig                 fTexelConfig;
    GrTGpuResourceRef<GrBuffer>   fBuffer;
    GrShaderFlags                 fVisibility;

    typedef SkNoncopyable INHERITED;
};

#endif
