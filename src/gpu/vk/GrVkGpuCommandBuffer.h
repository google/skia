/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkGpuCommandBuffer_DEFINED
#define GrVkGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

#include "GrColor.h"

class GrVkGpu;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;

class GrVkGpuCommandBuffer : public GrGpuCommandBuffer {
public:
    GrVkGpuCommandBuffer(GrVkGpu* gpu,
                         const GrVkRenderTarget&,
                         LoadAndStoreOp colorOp, GrColor colorClear,
                         LoadAndStoreOp stencilOp, GrColor stencilClear);

    virtual ~GrVkGpuCommandBuffer();

    void end() override;

    void submit() override;

private:
    const GrVkRenderPass*       fRenderPass;
    GrVkSecondaryCommandBuffer* fCommandBuffer;
    GrVkGpu*                    fGpu;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif
