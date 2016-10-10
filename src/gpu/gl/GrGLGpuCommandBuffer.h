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

class GrGLRenderTarget;

class GrGLGpuCommandBuffer : public GrGpuCommandBuffer {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLGpuCommandBuffer(GrGLGpu* gpu, GrGLRenderTarget* rt) : fGpu(gpu), fRenderTarget(rt) {}

    virtual ~GrGLGpuCommandBuffer() {}

    void end() override {}

    void discard() override {}

private:
    GrGpu* gpu() override { return fGpu; }
    GrRenderTarget* renderTarget() override { return fRenderTarget; }

    void onSubmit() override {}

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh* mesh,
                int meshCount,
                const SkRect& bounds) override {
        fGpu->draw(pipeline, primProc, mesh, meshCount);
    }

    void onClear(const GrFixedClip& clip, GrColor color) override {
        fGpu->clear(clip, color, fRenderTarget);
    }

    void onClearStencilClip(const GrFixedClip& clip,
                            bool insideStencilMask) override {
        fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget);
    }

    GrGLGpu*                    fGpu;
    GrGLRenderTarget*           fRenderTarget;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

