/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLGpuCommandBuffer_DEFINED
#define GrGLGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

#include "GrGLGpu.h"

class GrGLGpuCommandBuffer : public GrGpuCommandBuffer {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLGpuCommandBuffer(GrGLGpu* gpu) : fGpu(gpu) {}

    virtual ~GrGLGpuCommandBuffer() {}

    void end() override {}

    void discard(GrRenderTarget* rt) override { fGpu->discard(rt); }

private:
    GrGpu* gpu() override { return fGpu; }

    void onSubmit(const SkIRect& bounds) override {}

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh* mesh,
                int meshCount) override {
        fGpu->draw(pipeline, primProc, mesh, meshCount);
    }

    void onClear(GrRenderTarget* rt, const SkIRect& rect, GrColor color) override {
        fGpu->clear(rect, color, rt);
    }

    void onClearStencilClip(GrRenderTarget* rt, const SkIRect& rect, bool insideClip) override {
        fGpu->clearStencilClip(rect, insideClip, rt);
    }

    GrGLGpu*                    fGpu;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

