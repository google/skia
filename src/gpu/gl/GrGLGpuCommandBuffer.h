/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLGpuCommandBuffer_DEFINED
#define GrGLGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

class GrGLGpu;

class GrGLGpuCommandBuffer : public GrGpuCommandBuffer {
public:
    GrGLGpuCommandBuffer(GrGLGpu* gpu) /*: fGpu(gpu)*/ {}

    virtual ~GrGLGpuCommandBuffer() {}

    void end() override {}

    void submit() override {}

private:
    // commented out to appease clang compiler warning about unused private field
   // GrGLGpu*                    fGpu;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

