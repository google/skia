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
#include "GrGLRenderTarget.h"
#include "GrOpFlushState.h"

class GrGLGpu;
class GrGLRenderTarget;

class GrGLGpuCommandBuffer : public GrGpuCommandBuffer {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLGpuCommandBuffer(GrGLGpu* gpu) : fGpu(gpu), fRenderTarget(nullptr) {}

    ~GrGLGpuCommandBuffer() override {}

    void end() override {}

    void discard(GrRenderTarget* rt) override {
        GrGLRenderTarget* target = static_cast<GrGLRenderTarget*>(rt);
        if (!fRenderTarget) {
            fRenderTarget = target;
        }
        SkASSERT(target == fRenderTarget);
    }

    void inlineUpload(GrOpFlushState* state, GrDrawOp::DeferredUploadFn& upload,
                      GrRenderTarget*) override {
        state->doUpload(upload);
    }

private:
    GrGpu* gpu() override { return fGpu; }
    GrRenderTarget* renderTarget() override { return fRenderTarget; }

    void onSubmit() override {}

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh mesh[],
                const GrPipeline::DynamicState dynamicStates[],
                int meshCount,
                const SkRect& bounds) override {
        GrGLRenderTarget* target = static_cast<GrGLRenderTarget*>(pipeline.getRenderTarget());
        if (!fRenderTarget) {
            fRenderTarget = target;
        }
        SkASSERT(target == fRenderTarget);
        fGpu->draw(pipeline, primProc, mesh, dynamicStates, meshCount);
    }

    void onClear(GrRenderTarget* rt, const GrFixedClip& clip, GrColor color) override {
        GrGLRenderTarget* target = static_cast<GrGLRenderTarget*>(rt);
        if (!fRenderTarget) {
            fRenderTarget = target;
        }
        SkASSERT(target == fRenderTarget);
        fGpu->clear(clip, color, fRenderTarget);
    }

    void onClearStencilClip(GrRenderTarget* rt, const GrFixedClip& clip,
                            bool insideStencilMask) override {
        GrGLRenderTarget* target = static_cast<GrGLRenderTarget*>(rt);
        if (!fRenderTarget) {
            fRenderTarget = target;
        }
        SkASSERT(target == fRenderTarget);
        fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget);
    }

    GrGLGpu*                    fGpu;
    GrGLRenderTarget*           fRenderTarget;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

